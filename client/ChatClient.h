#pragma once
#include "../common/Message.h"
#include "../common/Protocol.h"
#include "../common/ConsoleMenu.h"
#include "../common/Validation.h"
#include <string>
#include <thread>
#include <vector>
#include <atomic>
#include <memory>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
using SOCKET_TYPE = SOCKET;
using ssize_t = int;
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
using SOCKET_TYPE = int;
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket close
#endif

#define BUFFER_SIZE_CLIENT 4096

// Подключаем общие классы
#include "../common/Logger.h"
#include "../common/sha1.h"

namespace chat
{
    class ChatClient
    {
    public:
        ChatClient(const std::string& serverIp, int port);
        ~ChatClient();
        void run();

    private:
        void showAuthMenu();
        void showChatMenu();
        void showUserListMenu();
        void handleRegister();
        void handleLogin();
        bool connectToServer();
        void receiveMessages();
        void waitForStatusResponse();
        void safeSend(const std::string& data);
        void requestUserList();
        void requestMessageHistory();
        void handleUserList(const std::string& userListData);
        void handleMessageHistory(const std::string& history);
        void handleStatusResponse(const std::string& response);
        void sendToAll();
        void sendPrivate();
        void displayMessageHistory();
        void clearScreen();
        //void waitForEnter();

        std::string m_serverIp;
        int m_port;
        SOCKET_TYPE m_socket;
        std::string m_currentLogin;
        std::atomic<bool> m_running{ true };
        std::atomic<bool> m_authenticated{ false };
        std::atomic<bool> m_waitingForResponse{ false };
        std::thread m_receiverThread;
        std::vector<std::string> m_userList;
        std::string m_lastStatusResponse;
        std::string m_messageHistory;
        bool m_hasNewMessages = false;

        std::unique_ptr<Logger> m_logger; 

        static ChatClient* m_instance;
        static void signalHandler(int signal);
    };
}