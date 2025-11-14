#pragma once

#include "../common/Message.h"
#include <unordered_map>
#include <mutex>

#define BUFFER_SIZE_SRV 4096
#define PORT_SRV 7777

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
using SOCKET_TYPE = SOCKET;
using ssize_t = int;
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#define SOCKET_TYPE int
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket close
#endif

#include "DatabaseManager.h"

namespace chat
{
    class ChatServer
    {
    public:
        ChatServer(int port = PORT_SRV);
        ~ChatServer();
        void run();

    private:
        void handleClient(SOCKET_TYPE clientSocket);
        void safeSend(SOCKET_TYPE socket, const std::string& data);
        void sendUserList(SOCKET_TYPE clientSocket);
        void sendMessageHistory(SOCKET_TYPE clientSocket, const std::string& username);
        void notifyNewMessage(const std::string& recipient, const std::string& from, const std::string& message, bool isPrivate);

        volatile bool m_running = true;
        static ChatServer* instance;
        static void signalHandler(int signal);
        int m_port;
        SOCKET_TYPE m_serverSocket;
        mutable std::mutex m_mutex;
        std::unordered_map<SOCKET_TYPE, std::string> m_sessions;
        DatabaseManager m_db;
    };
}