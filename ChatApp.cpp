#include "ChatApp.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <ctime>
#include "NewUserException.h"
#include "NoSuchUserException.h"


namespace chat
{
    void ChatApp::Run() 
    {
        ShowMainMenu();
    }

    void ChatApp::ShowMainMenu() 
    {
        ConsoleMenu menu;
        menu.addItem("Войти", [this]() { HandleLogin(); });
        menu.addItem("Зарегистрироваться", [this]() { HandleRegister(); });
        menu.addItem("Выход", [this]() { HandleExit(); });
        menu.run();
    }

    void ChatApp::ShowChatMenu() 
    {
        ConsoleMenu menu;
        menu.addItem("Отправить сообщение", [this]() { HandleSendMessage(); });
        menu.addItem("Просмотреть сообщения", [this]() { HandleViewMessages(); });
        menu.addItem("Выйти из аккаунта", [this]() { HandleLogout(); });
        menu.run();
    }

    void ChatApp::HandleRegister() 
    {
        try 
        {
            std::string login, password, nickname;
            std::cout << "Логин: ";
            std::cin >> login;
            std::cout << "Пароль (минимум 6 символов, цифра и спецсимвол $&*!): ";
            std::cin >> password;
            std::cout << "Никнейм: ";
            std::cin >> nickname;

            m_UserManager.AddNewUser(login, password, nickname);
            std::cout << "Регистрация успешна!\n\n";
        }
        catch (const NewUserException& e) {
            std::cout << "Ошибка: " << e.what() << "\n\n";
        }
    }

    void ChatApp::HandleLogin() {
        try {
            std::string login, password;
            std::cout << "Логин: ";
            std::cin >> login;
            std::cout << "Пароль: ";
            std::cin >> password;

            // Простая аутентификация: ищем пользователя и сравниваем пароль
            User* user = m_UserManager.FindUserByLogin(login);
            if (!user || user->GetPassword() != password) {
                std::cout << "Неверный логин или пароль.\n\n";
                return;
            }

            m_UserManager.SetActiveUser(login);
            std::cout << "Добро пожаловать, " << user->GetNickname() << "!\n\n";
            ShowChatMenu();
        }
        catch (const NoSuchUserException& e) {
            std::cout << "Ошибка: " << e.what() << "\n\n";
        }
    }

    void ChatApp::HandleLogout() 
    {
        m_UserManager.SetActiveUser(""); // сброс 
        std::cout << "Вы вышли из аккаунта.\n\n";
        return; 
    }

    void ChatApp::HandleSendMessage() 
    {
        if (!IsUserLoggedIn()) return;

        std::string choice;
        std::cout << "Отправить (1) личное сообщение или (2) в общий чат? ";
        std::cin >> choice;

        std::string recipient, text;
        if (choice == "1") 
        {
            std::cout << "Кому (логин): ";
            std::cin >> recipient;

            // Проверим, существует ли получатель
            if (!m_UserManager.FindUserByLogin(recipient)) 
            {
                std::cout << "Пользователь не найден.\n\n";
                return;
            }
            std::cin.ignore();
            std::cout << "Сообщение: ";
            std::getline(std::cin, text);
            m_Messages.emplace_back(GetActiveLogin(), recipient, text);
        }
        else if (choice == "2") 
        {
            std::cin.ignore();
            std::cout << "Сообщение в общий чат: ";
            std::getline(std::cin, text);
            m_Messages.emplace_back(GetActiveLogin(), "", text);
        }
        else 
        {
            std::cout << "Неверный выбор.\n\n";
            return;
        }
        std::cout << "Сообщение отправлено!\n\n";
    }

    void ChatApp::HandleViewMessages() 
    {
        if (!IsUserLoggedIn()) return;

        std::string activeLogin = GetActiveLogin();
        bool hasMessages = false;

        std::cout << "\n=== Ваши сообщения ===\n";
        for (const auto& msg : m_Messages) 
        {
            if (msg.IsForUser(activeLogin)) 
            {
                hasMessages = true;
                std::time_t time = msg.GetTimestamp();
                std::tm tmPtr;
                char buffer[20];
                if (localtime_s(&tmPtr, &time) != 0) 
                {
                    strcpy_s(buffer, "??");
                }
                else
                {
                    std::strftime(buffer, sizeof(buffer), "%H:%M", &tmPtr);
                }

                std::cout << "[" << buffer << "] ";
                if (msg.IsPrivate()) 
                {
                    std::cout << "Личное от " << msg.GetFrom() << ": ";
                }
                else 
                {
                    std::cout << msg.GetFrom() << " (все): ";
                }
                std::cout << msg.GetText() << "\n";
            }
        }

        if (!hasMessages) 
        {
            std::cout << "Нет сообщений.\n";
        }
        std::cout << "\n";
    }

    void ChatApp::HandleExit() 
    {
        std::cout << "До свидания!\n";
        exit(0);
    }

    bool ChatApp::IsUserLoggedIn() const 
    {
        return m_UserManager.GetActiveUser() != nullptr;
    }

    std::string ChatApp::GetActiveLogin() const 
    {
        auto* user = m_UserManager.GetActiveUser();
        return user ? user->GetLogin() : "";
    }
}