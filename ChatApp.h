#pragma once
#include "UserManager.h"
#include "Message.h"
#include "ConsoleMenu.h"
#include <vector>
#include <memory>

namespace chat
{
    class ChatApp {
    public:
        void Run();

    private:
        UserManager m_UserManager;
        std::vector<Message> m_Messages;

        // ����
        void ShowMainMenu();
        void ShowChatMenu();
        //void ShowAuthMenu();

        // ��������
        void HandleRegister();
        void HandleLogin();
        void HandleLogout();
        void HandleSendMessage();
        void HandleViewMessages();
        void HandleExit();

        bool IsUserLoggedIn() const;
        std::string GetActiveLogin() const;
    };
}