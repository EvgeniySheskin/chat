#define NOMINMAX
#include "ChatClient.h"
#include <iostream>
#include <chrono>
#include <limits>
#include <csignal>
#include <algorithm>
#include <sstream>

#ifdef _WIN32
#pragma comment(lib, "ws2_32.lib")
#endif

namespace chat
{
    ChatClient* ChatClient::instance = nullptr;

    ChatClient::ChatClient(const std::string& serverIp, int port)
        : m_serverIp(serverIp), m_port(port), m_socket(INVALID_SOCKET)
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
    }

    ChatClient::~ChatClient() {
        m_running = false;
        if (m_receiverThread.joinable()) {
            m_receiverThread.join();
        }
        if (m_socket != INVALID_SOCKET) {
            closesocket(m_socket);
        }
#ifdef _WIN32
        WSACleanup();
#endif
    }

    void ChatClient::run()
    {
        if (!connectToServer())
        {
            std::cout << "Error: Unable to connect to server.\n";
            return;
        }

        m_receiverThread = std::thread(&ChatClient::receiveMessages, this);

        while (m_running)
        {
            if (!m_authenticated)
            {
                clearScreen();
                showAuthMenu();
            }
            else
            {
                clearScreen();
                showChatMenu();
            }
        }

        if (m_receiverThread.joinable())
        {
            m_receiverThread.join();
        }
    }

    bool ChatClient::connectToServer()
    {
        m_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (m_socket == INVALID_SOCKET)
        {
            std::cerr << "Socket creation failed\n";
            return false;
        }

        sockaddr_in serverAddr{};
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(m_port);

        if (inet_pton(AF_INET, m_serverIp.c_str(), &serverAddr.sin_addr) <= 0)
        {
            std::cerr << "Invalid address\n";
            closesocket(m_socket);
            return false;
        }

        if (connect(m_socket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
        {
            std::cerr << "Connection failed\n";
            closesocket(m_socket);
            m_socket = INVALID_SOCKET;
            return false;
        }

#ifdef _WIN32
        u_long mode = 1;
        ioctlsocket(m_socket, FIONBIO, &mode);
#else
        int flags = fcntl(m_socket, F_GETFL, 0);
        fcntl(m_socket, F_SETFL, flags | O_NONBLOCK);
#endif

        return true;
    }

    void ChatClient::receiveMessages()
    {
        char buffer[BUFFER_SIZE_CLIENT];

        while (m_running)
        {
            ssize_t bytes = recv(m_socket, buffer, sizeof(buffer) - 1, 0);
            if (bytes > 0)
            {
                buffer[bytes] = '\0';
                try
                {
                    NetworkMessage msg = NetworkMessage::deserialize(buffer);

                    if (msg.type == MessageType::USER_LIST)
                    {
                        handleUserList(msg.payload);
                    }
                    else if (msg.type == MessageType::MESSAGE_HISTORY)
                    {
                        handleMessageHistory(msg.payload);
                    }
                    else if (msg.type == MessageType::STATUS_RESPONSE)
                    {
                        handleStatusResponse(msg.payload);
                    }
                    else if (msg.type == MessageType::SEND_MESSAGE)
                    {
                        std::string displayMsg;
                        if (msg.recipient.empty())
                        {
                            displayMsg = "[" + msg.sender + "]: " + msg.payload + "\n";
                        }
                        else
                        {
                            displayMsg = "[PM from " + msg.sender + "]: " + msg.payload + "\n";
                        }

                        // Добавляем в историю
                        m_messageHistory += displayMsg;
                        m_hasNewMessages = true;
                        std::cout << "\n" << displayMsg << std::flush;
                    }
                }
                catch (const std::exception& e)
                {
                    std::cout << "\n[Deserialization error] " << e.what() << std::endl;
                    m_waitingForResponse = false;
                }
            }
            else if (bytes == 0)
            {
                std::cout << "\nServer suspended connection\n";
                m_running = false;
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
                    std::cout << "\nData accept failed\n";
                    m_running = false;
                }
                }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }

    void ChatClient::waitForStatusResponse()
    {
        m_waitingForResponse = true;
        int waitCount = 0;
        while (m_waitingForResponse && m_running && waitCount < 40)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(250));
            waitCount++;
        }

        if (m_waitingForResponse)
        {
            std::cout << "Server response timeout\n";
            m_waitingForResponse = false;
        }
    }

    /*void ChatClient::waitForEnter()
    {
        std::cout << "Нажмите Enter для продолжения...";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cin.get();
    }*/

    void ChatClient::showAuthMenu()
    {
        while (m_running && !m_authenticated)
        {
            std::cout << "\n=== Authentication ===\n\n";
            std::cout << "1. Register\n2. Login\n3. Exit\nYour choice: ";

            int choice = -1;
            if (!(std::cin >> choice))
            {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::cout << "Invalid input. Try again.\n";
                continue;
            }

            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

            if (choice == 1)
            {
                handleRegister();
                waitForStatusResponse();
            }
            else if (choice == 2)
            {
                handleLogin();
                waitForStatusResponse();
            }
            else if (choice == 3)
            {
                m_running = false;
            }
            else
            {
                std::cout << "Invalid input. Try again.\n";
            }
        }
    }

    void ChatClient::showChatMenu()
    {
        if (m_messageHistory.empty())
        {
            requestMessageHistory();
            waitForStatusResponse();
        }
        if (m_userList.empty()) {
            requestUserList();
            waitForStatusResponse();
        }

        while (m_running && m_authenticated)
        {
            clearScreen();
            displayMessageHistory();
            std::cout << "=== Chat ( " << m_currentLogin << " ) ===\n\n";

            if (m_hasNewMessages) 
            {
                std::cout << "You have new messages!\n";
                m_hasNewMessages = false;
            }

            std::cout << "\n1. Send broadcast messages\n";
            std::cout << "2. Send personal message\n";
            std::cout << "3. Refresh user list\n";
            std::cout << "4. Logout\n";
            std::cout << "Your choice: ";

            int choice = -1;
            if (!(std::cin >> choice))
            {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::cout << "Invalid input. Try again.\n";
                continue;
            }
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

            switch (choice)
            {
            case 1:
                sendToAll();
                break;
            case 2:
                sendPrivate();
                break;
            case 3:
                requestUserList();
                waitForStatusResponse();
                break;
            case 4:
            {
                NetworkMessage logoutMsg{ MessageType::LOGOUT, m_currentLogin, "", "" };
                safeSend(logoutMsg.serialize());
                std::cout << "Logging out...\n";
                waitForStatusResponse();
                break;
            }
            default:
                std::cout << "Invalid input. Try again.\n";
                break;
            }
        }
    }

    void ChatClient::clearScreen()
    {
#ifdef _WIN32
        system("cls");
#else
        system("clear");
#endif
    }

    void ChatClient::displayMessageHistory()
    {
        std::cout << "\n=== Message history ===\n\n";
        if (m_messageHistory.empty())
        {
            std::cout << "No messages yet\n";
        }
        else
        {
            std::cout << m_messageHistory << std::endl;
        }
        std::cout << "\n=== End ===\n\n";
    }

    void ChatClient::sendToAll()
    {
        
        std::string message;
        while (true)
        {
            clearScreen();
            displayMessageHistory();
            std::cout << "Enter broadcast message (type \"/exit\" to leave):\n";
            std::getline(std::cin, message);
            if (!message.empty() && message != "/exit")
            {
                NetworkMessage msg{ MessageType::SEND_MESSAGE, m_currentLogin, "", message };
                safeSend(msg.serialize());
                std::cout << "Sending message...\n";
                waitForStatusResponse();

                // Обновляем историю после отправки
                requestMessageHistory();
                waitForStatusResponse();
                //clearScreen();
                //displayMessageHistory();
            }
            else
            {
                break;
            }
        }
        
    }

    void ChatClient::sendPrivate()
    {
        if (m_userList.empty())
        {
            std::cout << "Empty user list. Refresh first.\n";
            return;
        }

        showUserListMenu();

        std::cout << "Choose user (type number): ";
        int userChoice = -1;
        if (!(std::cin >> userChoice))
        {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Invalid input.\n";
            return;
        }
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        if (userChoice < 1 || userChoice > static_cast<int>(m_userList.size()))
        {
            std::cout << "Invalid user number.\n";
            return;
        }

        std::string recipient = m_userList[userChoice - 1];
        if (recipient == m_currentLogin)
        {
            std::cout << "It is impossible to send message to yourself\n";
            return;
        }

        std::cout << "Type message for " << recipient << ": ";
        std::string message;
        std::getline(std::cin, message);

        if (!message.empty())
        {
            NetworkMessage msg{ MessageType::SEND_MESSAGE, m_currentLogin, recipient, message };
            safeSend(msg.serialize());
            std::cout << "Sending personal message...\n";
            waitForStatusResponse();

            requestMessageHistory();
            waitForStatusResponse();
            clearScreen();
        }
    }

    void ChatClient::showUserListMenu()
    {
        std::cout << "\n=== Registered User List ===\n\n";
        for (size_t i = 0; i < m_userList.size(); ++i)
        {
            std::cout << (i + 1) << ". " << m_userList[i] << "\n";
        }
        std::cout << "\n=======================\n\n";
    }

    void ChatClient::requestUserList()
    {
        NetworkMessage msg{ MessageType::GET_USERS, m_currentLogin, "", "" };
        safeSend(msg.serialize());
    }

    void ChatClient::requestMessageHistory()
    {
        NetworkMessage msg{ MessageType::GET_HISTORY, m_currentLogin, "", "" };
        safeSend(msg.serialize());
    }

    void ChatClient::handleUserList(const std::string & userListData)
    {
        m_userList.clear();
        std::istringstream iss(userListData);
        std::string user;

        while (std::getline(iss, user, '|'))
        {
            if (!user.empty() && user != m_currentLogin)
            {
                m_userList.push_back(user);
            }
        }

        std::cout << "User list is refreshed. " << m_userList.size() << " user(s) available.\n";
        m_waitingForResponse = false;
    }

    void ChatClient::handleMessageHistory(const std::string & history)
    {
        m_messageHistory = history;
        m_waitingForResponse = false;
    }

    void ChatClient::handleStatusResponse(const std::string & response)
    {
        if (response.find("SUCCESS:") == 0)
        {
            std::string message = response.substr(8);
            std::cout << message << std::endl;

            if (message.find("Successful login") != std::string::npos)
            {
                m_authenticated = true;
            }
                else if (message.find("Successful logout") != std::string::npos)
            {
                m_authenticated = false;
                m_currentLogin.clear();
                m_userList.clear();
                m_messageHistory.clear();
            }
        }
        else if (response.find("ERROR:") == 0)
        {
            std::string message = response.substr(6);
            std::cout << message << std::endl;
        }
        else if (response.find("NOTIFICATION:") == 0)
        {
            std::string message = response.substr(13);
            std::cout << message << std::endl;
            m_hasNewMessages = true;
        }

        m_waitingForResponse = false;
    }

    void ChatClient::handleRegister()
    {
        std::string login, password, nickname;
        std::cout << "\n=== Registration ===\n";
        std::cout << "Login: ";
        std::cin >> login;
        std::cout << "Password: ";
        std::cin >> password;
        std::cout << "Nickname: ";
        std::cin >> nickname;

        auto loginResult = validateLogin(login);
        if (!loginResult.success)
        {
            std::cout << "\nError: " << loginResult.errorMessage << "\n";
            return;
        }
        auto passResult = validatePassword(password);
        if (!passResult.success)
        {
            std::cout << "\nError: " << passResult.errorMessage << "\n";
            return;
        }

        m_currentLogin = login;

        NetworkMessage reg{ MessageType::REGISTER, login, nickname, password };
        safeSend(reg.serialize());
        std::cout << "Registration...\n";
    }

    void ChatClient::handleLogin()
    {
        std::string login, password;
        std::cout << "\n=== Login ===\n";
        std::cout << "Login: ";
        std::cin >> login;
        std::cout << "Password: ";
        std::cin >> password;

        if (login.empty() || password.empty())
        {
            std::cout << "\nLogin or password cannot be blank.\n";
            return;
        }

        m_currentLogin = login;

        NetworkMessage auth{ MessageType::LOGIN, login, "", password };
        safeSend(auth.serialize());
        std::cout << "Authorization...\n";
    }

    void ChatClient::safeSend(const std::string & data)
    {
        ssize_t totalSent = 0;
        while (totalSent < data.size())
        {
            ssize_t sent = send(m_socket, data.c_str() + totalSent, data.size() - totalSent, 0);
            if (sent <= 0)
            {
                std::cout << "Data send error\n";
                return;
            }
            totalSent += sent;
        }
    }

    void ChatClient::signalHandler(int signal)
    {
        if (instance)
        {
            instance->m_running = false;
        }
    }
}