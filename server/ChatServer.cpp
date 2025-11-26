#include "ChatServer.h"
#include "../common/Protocol.h"
#include "../common/Validation.h"
#include "DatabaseManager.h"
#include "../common/Logger.h"
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
    ChatServer* ChatServer::m_instance = nullptr;

    ChatServer::ChatServer(int port) : m_port(port), m_serverSocket(INVALID_SOCKET), m_db(), m_logger(std::make_unique<Logger>("server.log"))
    {
#ifdef _WIN32
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        {
            throw std::runtime_error("WSAStartup failed");
        }
#endif
        m_instance = this;
        std::signal(SIGINT, signalHandler);

        m_logger->start();
        m_logger->info("Server starting...");

        if (!m_db.connect("DRIVER={MySQL ODBC 9.5 ANSI Driver};SERVER=localhost;PORT=3306;UID=root;PWD=8lindGuardianMySQL;"))
        {
            m_logger->error("Database connection failed");
            throw std::runtime_error("Database connection failed");
        }
        m_db.initDatabase();
        m_logger->info("Database connected and initialized.");
    }

    ChatServer::~ChatServer()
    {
        m_logger->info("Server shutting down...");
        m_running = false;
        if (m_serverSocket != INVALID_SOCKET)
        {
            closesocket(m_serverSocket);
            m_serverSocket = INVALID_SOCKET;
        }
        m_logger->stop();
#ifdef _WIN32
        WSACleanup();
#endif
    }

    void ChatServer::run()
    {
        m_serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (m_serverSocket == INVALID_SOCKET)
        {
            m_logger->error("Socket creation failed");
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
            m_logger->error("Bind failed");
            closesocket(m_serverSocket);
            return;
        }

        if (listen(m_serverSocket, 5) == SOCKET_ERROR)
        {
            m_logger->error("Listen failed");
            closesocket(m_serverSocket);
            return;
        }

        m_logger->info("Server started on port " + std::to_string(m_port));

        while (m_running)
        {
            sockaddr_in clientAddr;
            socklen_t addrLen = sizeof(clientAddr);

            SOCKET_TYPE clientSocket = accept(m_serverSocket, (sockaddr*)&clientAddr, &addrLen);
            if (clientSocket == INVALID_SOCKET)
            {
                if (!m_running) break;
                m_logger->error("Accept failed");
                continue;
            }

#ifdef _WIN32
            char clientIP[INET_ADDRSTRLEN];
            DWORD clientIPLen = INET_ADDRSTRLEN;
            if (InetNtopA(AF_INET, &clientAddr.sin_addr, clientIP, clientIPLen) != nullptr) {
                m_logger->info("New client connected from " + std::string(clientIP));
            }
            else {
                m_logger->info("New client connected from (could not resolve IP)");
            }
#else
            char clientIP[INET_ADDRSTRLEN];
            if (inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, INET_ADDRSTRLEN) != nullptr) {
                m_logger->info("New client connected from " + std::string(clientIP));
            }
            else {
                m_logger->info("New client connected from (could not resolve IP)");
            }
#endif
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
        std::string m_currentLogin;

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

                    m_logger->debug("Deserialized: type=" + std::to_string(static_cast<int>(msg.type)) + ", sender=" + msg.sender);

                    switch (msg.type)
                    {
                    case MessageType::REGISTER:
                    {
                        auto loginResult = chat::validateLogin(msg.sender);
                        if (!loginResult.success)
                        {
                            NetworkMessage resp{ MessageType::STATUS_RESPONSE, "", "", "L_ERROR:" + loginResult.errorMessage };
                            safeSend(clientSocket, resp.serialize());
                            m_logger->warning("Registration failed for login '" + msg.sender + "': " + loginResult.errorMessage);
                            break;
                        }

                        if (msg.payload.length() != 40) 
                        {
                            NetworkMessage resp{ MessageType::STATUS_RESPONSE, "", "", "L_ERROR: Invalid password hash format." };
                            safeSend(clientSocket, resp.serialize());
                            m_logger->warning("Registration failed for login '" + msg.sender + "': Invalid password hash length.");
                            break;
                        }
                        bool isHex = std::all_of(msg.payload.begin(), msg.payload.end(), 
                            [](char c) 
                            {
                                return std::isxdigit(static_cast<unsigned char>(c));
                            });
                        if (!isHex) 
                        {
                            NetworkMessage resp{ MessageType::STATUS_RESPONSE, "", "", "L_ERROR: Invalid password hash format." };
                            safeSend(clientSocket, resp.serialize());
                            m_logger->warning("Registration failed for login '" + msg.sender + "': Invalid password hash characters.");
                            break;
                        }

                        auto existingUser = m_db.findUserByLogin(msg.sender);
                        if (existingUser.has_value())
                        {
                            NetworkMessage resp{ MessageType::STATUS_RESPONSE, "", "", "L_ERROR: There is a user with the same login" };
                            safeSend(clientSocket, resp.serialize());
                            m_logger->warning("Registration failed for login '" + msg.sender + "': Login already exists");
                            break;
                        }

                        try
                        {
                            m_db.addUser(msg.sender, msg.payload, msg.recipient);
                            NetworkMessage resp{ MessageType::STATUS_RESPONSE, "", "", "SUCCESS: Registration successful! Now you need to authorize" };
                            safeSend(clientSocket, resp.serialize());
                            m_logger->info("New user registered: " + msg.sender);
                        }
                        catch (...)
                        {
                            NetworkMessage resp{ MessageType::STATUS_RESPONSE, "", "", "L_ERROR: Server registration error" };
                            safeSend(clientSocket, resp.serialize());
                            m_logger->error("Server registration error for login: " + msg.sender);
                        }
                        break;
                    }

                    case MessageType::LOGIN:
                    {
                        if (msg.sender.empty())
                        {
                            NetworkMessage resp{ MessageType::STATUS_RESPONSE, "", "", "L_ERROR: Login cannot be blank." };
                            safeSend(clientSocket, resp.serialize());
                            break;
                        }
                        if (msg.payload.length() != 40) 
                        {
                            NetworkMessage resp{ MessageType::STATUS_RESPONSE, "", "", "L_ERROR: Invalid password hash format." };
                            safeSend(clientSocket, resp.serialize());
                            m_logger->warning("Login failed for '" + msg.sender + "': Invalid password hash length.");
                            break;
                        }
                        bool isHex = std::all_of(msg.payload.begin(), msg.payload.end(), 
                            [](char c) 
                            {
                                return std::isxdigit(static_cast<unsigned char>(c));
                            });
                        if (!isHex) 
                        {
                            NetworkMessage resp{ MessageType::STATUS_RESPONSE, "", "", "L_ERROR: Invalid password hash format." };
                            safeSend(clientSocket, resp.serialize());
                            m_logger->warning("Login failed for '" + msg.sender + "': Invalid password hash characters.");
                            break;
                        }

                        auto user = m_db.findUserByLogin(msg.sender);
                        if (user) 
                        {
                            if (user->GetPassword() == msg.payload) 
                            {
                                m_currentLogin = msg.sender;
                                {
                                    std::lock_guard<std::shared_mutex> lock(m_sessions_mutex); 
                                    m_sessions[clientSocket] = m_currentLogin;
                                }
                                NetworkMessage resp{ MessageType::STATUS_RESPONSE, "", "", "SUCCESS: Successful login!" };
                                safeSend(clientSocket, resp.serialize());
                                m_logger->info("User logged in: " + m_currentLogin);
                            }
                            else 
                            {
                                NetworkMessage resp{ MessageType::STATUS_RESPONSE, "", "", "L_ERROR: Incorrect login or password." };
                                safeSend(clientSocket, resp.serialize());
                                m_logger->warning("Login failed for: " + msg.sender + " (password hash mismatch)");
                            }
                        }
                        else 
                        {
                            NetworkMessage resp{ MessageType::STATUS_RESPONSE, "", "", "L_ERROR: Incorrect login or password." };
                            safeSend(clientSocket, resp.serialize());
                            m_logger->warning("Login failed for: " + msg.sender + " (user not found)");
                        }
                        break;
                    }

                    case MessageType::SEND_MESSAGE:
                    {
                        if (m_currentLogin.empty())
                        {
                            NetworkMessage resp{ MessageType::STATUS_RESPONSE, "", "", "L_ERROR: Unauthorized access." };
                            safeSend(clientSocket, resp.serialize());
                            m_logger->warning("Unauthorized message attempt from socket.");
                            break;
                        }

                        Message message(m_currentLogin, msg.recipient, msg.payload);
                        if (m_db.addMessage(message))
                        {
                            NetworkMessage resp{ MessageType::STATUS_RESPONSE, "", "", "SUCCESS: Message sent" };
                            safeSend(clientSocket, resp.serialize());
                            m_logger->debug("Message saved to DB from " + m_currentLogin);

                            NetworkMessage broadcastMsg{ MessageType::SEND_MESSAGE, m_currentLogin, msg.recipient, msg.payload };
                            std::string serialized = broadcastMsg.serialize();

                            std::shared_lock<std::shared_mutex> lock(m_sessions_mutex);
                            for (const auto& session : m_sessions)
                            {
                                safeSend(session.first, serialized);
                            }
                            lock.unlock(); 

                            m_logger->info("Message broadcasted from " + m_currentLogin + " to " + std::to_string(m_sessions.size()) + " users.");
                        }
                        else
                        {
                            NetworkMessage resp{ MessageType::STATUS_RESPONSE, "", "", "L_ERROR: Failed to save message." };
                            safeSend(clientSocket, resp.serialize());
                            m_logger->error("Failed to save message from " + m_currentLogin);
                        }
                        break;
                    }

                    case MessageType::LOGOUT:
                    {
                        {
                            std::lock_guard<std::shared_mutex> lock(m_sessions_mutex); 
                            m_sessions.erase(clientSocket);
                        }
                        m_logger->info("User logged out: " + m_currentLogin);
                        m_currentLogin.clear();
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
                        sendMessageHistory(clientSocket, m_currentLogin);
                        break;
                    }

                    default:
                        break;
                    }
                }
                catch (const std::exception& e)
                {
                    m_logger->error("Deserialization error: " + std::string(e.what()));
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

        m_logger->info("Client disconnected: " + m_currentLogin);
        {
            std::lock_guard<std::shared_mutex> lock(m_sessions_mutex);
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
            std::string prefix = message.IsPrivate() ? "[Personal] " : "[Broadcast] ";
            std::string fromTo;
            if (!message.IsPrivate())
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

        m_logger->debug("Sending history for " + username + ". Total messages: " + std::to_string(messages.size()) + ", shown: " + std::to_string(messageCount));

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
        if (m_instance)
        {
            m_instance->m_running = false;
        }
    }
}