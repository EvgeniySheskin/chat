#pragma once
#include "UserManager.h"
#include "Message.h"
#include "ConsoleMenu.h"
#include <vector>
#include <string>

namespace chat {

    class ChatApp {
    public:
        void Run();

    private:
        UserManager m_userManager;
        std::vector<Message> m_messages;

        // Меню
        void ShowMainMenu();
        void ShowChatMenu();

        // Обработчики
        void HandleRegister();
        void HandleLogin();
        void HandleLogout();
        void HandleSendMessage();
        void HandleViewMessages();
        void HandleExit();

        // Вспомогательные методы
        bool IsUserLoggedIn() const;
        std::string GetActiveLogin() const;
    };

}