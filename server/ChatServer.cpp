#include "ChatServer.h"
#include "../common/Protocol.h"
#include "../common/Validation.h"
#include "DatabaseManager.h"
#include <iostream>
#include <thread>
#include <sstream>
#include <regex>
#include <csignal>
#include <cerrno>
#include <cstring>
#include <iomanip>

#ifdef _WIN32
#pragma comment(lib, "ws2_32.lib")
#endif

namespace chat
{
    ChatServer* ChatServer::instance = nullptr;

    ChatServer::ChatServer(int port) : m_port(port), m_serverSocket(INVALID_SOCKET), m_db()
    {
#ifdef _WIN32
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        {
            throw std::runtime_error("WSAStartup failed");
        }
#endif
        instance = this;
        std::signal(SIGINT, signalHandler);

        // Подключение к БД и инициализация
        if (!m_db.connect("DRIVER={MySQL ODBC 9.5 ANSI Driver};SERVER=localhost;PORT=3306;UID=root;PWD=8lindGuardianMySQL;")) 
        {
            throw std::runtime_error("Database connection failed");
        }
        m_db.initDatabase();
    }

    ChatServer::~ChatServer()
    {
        m_running = false;
        if (m_serverSocket != INVALID_SOCKET)
        {
            closesocket(m_serverSocket);
            m_serverSocket = INVALID_SOCKET;
        }
#ifdef _WIN32
        WSACleanup();
#endif
    }

    void ChatServer::run()
    {
        m_serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (m_serverSocket == INVALID_SOCKET)
        {
            std::cerr << "Socket creation failed\n";
            return;
        }

        int opt = 1;
#ifdef _WIN32
        setsockopt(m_serverSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));
#else
        setsockopt(m_serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif

        sockaddr_in serverAddr{};
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = INADDR_ANY;
        serverAddr.sin_port = htons(m_port);

        if (bind(m_serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
        {
            std::cerr << "Bind failed\n";
            closesocket(m_serverSocket);
            return;
        }

        if (listen(m_serverSocket, 5) == SOCKET_ERROR)
        {
            std::cerr << "Listen failed\n";
            closesocket(m_serverSocket);
            return;
        }

        std::cout << "Server started on port " << m_port << "\n";

        while (m_running)
        {
            sockaddr_in clientAddr;
            socklen_t addrLen = sizeof(clientAddr);

            SOCKET_TYPE clientSocket = accept(m_serverSocket, (sockaddr*)&clientAddr, &addrLen);
            if (clientSocket == INVALID_SOCKET)
            {
                if (!m_running) break;
                std::cerr << "Accept failed\n";
                continue;
            }

            std::cout << "[Server]: New client connected\n";
            if (m_running)
            {
                std::thread(&ChatServer::handleClient, this, clientSocket).detach();
            }
            else
            {
                closesocket(clientSocket);
                break;
            }
        }

        if (m_serverSocket != INVALID_SOCKET)
        {
            closesocket(m_serverSocket);
            m_serverSocket = INVALID_SOCKET;
        }
    }

    void ChatServer::handleClient(SOCKET_TYPE clientSocket)
    {
        char buffer[BUFFER_SIZE_SRV];
        std::string currentLogin;

#ifdef _WIN32
        u_long mode = 1;
        ioctlsocket(clientSocket, FIONBIO, &mode);
#else
        int flags = fcntl(clientSocket, F_GETFL, 0);
        fcntl(clientSocket, F_SETFL, flags | O_NONBLOCK);
#endif

        while (m_running)
        {
            ssize_t bytes = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
            if (bytes > 0)
            {
                buffer[bytes] = '\0';
                try
                {
                    NetworkMessage msg = NetworkMessage::deserialize(buffer);

                    std::cout << "[Server] Deserialized: type=" << static_cast<int>(msg.type)
                        << ", sender=" << msg.sender << std::endl;

                    std::lock_guard<std::mutex> lock(m_mutex);

                    switch (msg.type)
                    {
                    case MessageType::REGISTER:
                    {
                        auto loginResult = chat::validateLogin(msg.sender);
                        if (!loginResult.success)
                        {
                            NetworkMessage resp{ MessageType::STATUS_RESPONSE, "", "", "ERROR:" + loginResult.errorMessage };
                            safeSend(clientSocket, resp.serialize());
                            break;
                        }

                        auto passResult = chat::validatePassword(msg.payload);
                        if (!passResult.success)
                        {
                            NetworkMessage resp{ MessageType::STATUS_RESPONSE, "", "", "ERROR:" + passResult.errorMessage };
                            safeSend(clientSocket, resp.serialize());
                            break;
                        }

                        // Проверка доступности логина через БД
                        auto existingUser = m_db.findUserByLogin(msg.sender);
                        if (existingUser.has_value())
                        {
                            NetworkMessage resp{ MessageType::STATUS_RESPONSE, "", "", "ERROR: There is a user with the same login" };
                            safeSend(clientSocket, resp.serialize());
                            break;
                        }

                        try
                        {
                            m_db.addUser(msg.sender, msg.payload, msg.recipient);
                            NetworkMessage resp{ MessageType::STATUS_RESPONSE, "", "", "SUCCESS: Registration successful! Now you need to authorize" };
                            safeSend(clientSocket, resp.serialize());
                        }
                        catch (...)
                        {
                            NetworkMessage resp{ MessageType::STATUS_RESPONSE, "", "", "ERROR: Server registration error" };
                            safeSend(clientSocket, resp.serialize());
                        }
                        break;
                    }

                    case MessageType::LOGIN:
                    {
                        if (msg.sender.empty())
                        {
                            NetworkMessage resp{ MessageType::STATUS_RESPONSE, "", "", "ERROR: Login cannot be blank." };
                            safeSend(clientSocket, resp.serialize());
                            break;
                        }
                        if (msg.payload.empty())
                        {
                            NetworkMessage resp{ MessageType::STATUS_RESPONSE, "", "", "ERROR: Password cannot be blank." };
                            safeSend(clientSocket, resp.serialize());
                            break;
                        }

                        auto user = m_db.findUserByLogin(msg.sender);
                        if (user && user->GetPassword() == msg.payload)
                        {
                            currentLogin = msg.sender;
                            m_sessions[clientSocket] = currentLogin;
                            NetworkMessage resp{ MessageType::STATUS_RESPONSE, "", "", "SUCCESS: Successful login!" };
                            safeSend(clientSocket, resp.serialize());
                        }
                        else
                        {
                            NetworkMessage resp{ MessageType::STATUS_RESPONSE, "", "", "ERROR: Incorrect login or password." };
                            safeSend(clientSocket, resp.serialize());
                        }
                        break;
                    }

                    case MessageType::SEND_MESSAGE:
                    {
                        if (currentLogin.empty())
                        {
                            NetworkMessage resp{ MessageType::STATUS_RESPONSE, "", "", "ERROR: Unauthorized access." };
                            safeSend(clientSocket, resp.serialize());
                            break;
                        }

                        Message message(currentLogin, msg.recipient, msg.payload);
                        if (m_db.addMessage(message))
                        {
                            NetworkMessage resp{ MessageType::STATUS_RESPONSE, "", "", "SUCCESS: Message sent" };
                            safeSend(clientSocket, resp.serialize());

                            // Рассылка всем онлайн-пользователям
                            NetworkMessage broadcastMsg{ MessageType::SEND_MESSAGE, currentLogin, msg.recipient, msg.payload };
                            std::string serialized = broadcastMsg.serialize();
                            for (const auto& session : m_sessions)
                            {
                                safeSend(session.first, serialized);
                            }
                        }
                        else
                        {
                            NetworkMessage resp{ MessageType::STATUS_RESPONSE, "", "", "ERROR: Failed to save message." };
                            safeSend(clientSocket, resp.serialize());
                        }
                        break;
                    }

                    case MessageType::LOGOUT:
                    {
                        m_sessions.erase(clientSocket);
                        currentLogin.clear();
                        NetworkMessage resp{ MessageType::STATUS_RESPONSE, "", "", "SUCCESS: Successful logout!" };
                        safeSend(clientSocket, resp.serialize());
                        break;
                    }

                    case MessageType::GET_USERS:
                    {
                        sendUserList(clientSocket);
                        break;
                    }

                    case MessageType::GET_HISTORY:
                    {
                        sendMessageHistory(clientSocket, currentLogin);
                        break;
                    }

                    default:
                        break;
                    }
                }
                catch (const std::exception& e)
                {
                    std::cout << "[Server] Deserialization error: " << e.what() << std::endl;
                    break;
                }
            }
            else if (bytes == 0)
            {
                break;
            }
            else
            {
#ifdef _WIN32
                if (WSAGetLastError() != WSAEWOULDBLOCK)
                {
#else
                if (errno != EAGAIN && errno != EWOULDBLOCK)
                {
#endif
                    break;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
            }

        std::cout << "[Server] Client disconnected: " << currentLogin << "\n";
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_sessions.erase(clientSocket);
        }
        closesocket(clientSocket);
        }

    void ChatServer::sendUserList(SOCKET_TYPE clientSocket)
    {
        std::string userList;
        auto users = m_db.getAllUsers();
        for (size_t i = 0; i < users.size(); ++i)
        {
            if (i > 0) userList += "|";
            userList += users[i].GetLogin();
        }
        NetworkMessage resp{ MessageType::USER_LIST, "", "", userList };
        safeSend(clientSocket, resp.serialize());
    }

    void ChatServer::sendMessageHistory(SOCKET_TYPE clientSocket, const std::string & username)
    {
        std::string history;
        int messageCount = 0;

        auto messages = m_db.getMessagesForUser(username);
        for (const auto& message : messages)
        {
            std::string prefix = message.GetTo().empty() ? "[Broadcast] " : "[Personal] ";
            std::string fromTo;
            if (message.GetTo().empty())
            {
                fromTo = message.GetFrom();
            }
            else
            {
                if (message.GetFrom() == username)
                    fromTo = "You -> " + message.GetTo();
                else
                    fromTo = message.GetFrom() + " -> To you";
            }

            std::time_t timestamp = message.GetTimestamp();
            std::tm timeTm = {};

#ifdef _WIN32
            localtime_s(&timeTm, &timestamp);
#else
            timeTm = *std::localtime(&timestamp);
#endif
            std::ostringstream timeStream;
            timeStream << std::put_time(&timeTm, "%H:%M:%S");
            std::string timeStr = timeStream.str();

            history += prefix
                + "["
                + timeStr
                + "] "
                + fromTo + ": " + message.GetText() + "\n";
            messageCount++;
        }

        if (history.empty())
        {
            history = "Message history is empty";
        }

        std::cout << "[Server] Sending history for " << username
            << ". Total messages: " << messages.size()
            << ", shown: " << messageCount << std::endl;

        // добавим явный разделитель для корректной работы getline
        history += (char)0;
        NetworkMessage resp{ MessageType::MESSAGE_HISTORY, "", "", history };
        safeSend(clientSocket, resp.serialize());
    }

    void ChatServer::safeSend(SOCKET_TYPE socket, const std::string & data)
    {
        ssize_t totalSent = 0;
        while (totalSent < data.size())
        {
            ssize_t sent = send(socket, data.c_str() + totalSent, data.size() - totalSent, 0);
            if (sent <= 0)
            {
                return;
            }
            totalSent += sent;
        }
    }

    void ChatServer::signalHandler(int signal)
    {
        if (instance)
        {
            instance->m_running = false;
        }
    }
}