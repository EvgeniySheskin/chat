#include "ChatApp.h"
#include "LogoutException.h"
#include <iostream>
#include <vector>
#include <string>
#include <ctime>
#include <cstring>
#include <iomanip>
#include <algorithm>
#include <limits>
//#include <windows.h>

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
        try 
        {
            ConsoleMenu menu;
            menu.addItem("Отправить сообщение", [this]() { HandleSendMessage(); });
            menu.addItem("Просмотреть сообщения", [this]() { HandleViewMessages(); });
            menu.addItem("Выйти из аккаунта", [this]() { HandleLogout(); });
            menu.run();
        }
        catch (const LogoutException&) 
        {
            return;
        }
    }

    void ChatApp::HandleRegister() 
    {
        try 
        {
            std::string login, password, nickname;
            std::cout << "Логин: ";
            std::cin >> login;
            if (!m_userManager.IsLoginAvailable(login))
                throw NewUserException("Пользователь с таким логином уже зарегистрирован");
            std::cout << "Пароль (минимум 6 символов, цифра и спецсимвол $&*!): ";
            std::cin >> password;
            m_userManager.ValidatePassword(password);
            std::cout << "Никнейм: ";
            std::cin >> nickname;

            m_userManager.AddNewUser(std::move(login), std::move(password), std::move(nickname));
        }
        catch (const std::exception& e) 
        {
            std::cout << "Ошибка: " << e.what() << "\n\n";
        }
    }

    void ChatApp::HandleLogin() 
    {
        try 
        {
            std::string login, password;
            std::cout << "Логин: ";
            std::cin >> login;
            std::cout << "Пароль: ";
            std::cin >> password;

            const User* user = m_userManager.FindUserByLogin(login);
            if (!user || user->GetPassword() != password) 
            {
                std::cout << "Неверный логин или пароль.\n\n";
                return;
            }

            m_userManager.SetActiveUser(login);
            std::cout << "Добро пожаловать, " << user->GetNickname() << "!\n\n";
            ShowChatMenu();
        }
        catch (const std::exception& e) 
        {
            std::cout << "Ошибка: " << e.what() << "\n\n";
        }
    }

    void ChatApp::HandleLogout()
    {
        //std::cout << "Вы вышли из аккаунта.\n\n";
        // Активный пользователь сбрасывается внутри UserManager при следующем входе
        throw LogoutException{};
    }

    void ChatApp::HandleSendMessage() 
    {
        if (!IsUserLoggedIn()) return;

        std::string choice;
        std::cout << "\nОтправить (1) личное сообщение или (2) в общий чат? ";
        std::cin >> choice;

        if (choice == "2") 
        {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Сообщение в общий чат: ";
            std::string text;
            std::getline(std::cin, text);
            m_messages.emplace_back(GetActiveLogin(), "", std::move(text));
            std::cout << "Сообщение отправлено!\n\n";
            return;
        }

        if (choice != "1") 
        {
            std::cout << "Неверный выбор.\n\n";
            return;
        }

        const std::string activeLogin = GetActiveLogin();
        const auto users = m_userManager.GetRegisteredUsers();

        // Фильтруем: все, кроме текущего
        std::vector<const User*> recipients;
        std::cout << "\n=== Выберите получателя ===\n";
        for (const User* user : users) 
        {
            if (user->GetLogin() != activeLogin) 
            {
                recipients.push_back(user);
            }
        }

        if (recipients.empty()) 
        {
            std::cout << "Нет других пользователей для отправки сообщения.\n\n";
            return;
        }

        // Выводим нумерованный список
        for (size_t i = 0; i < recipients.size(); ++i) 
        {
            std::cout << (i + 1) << ". " << recipients[i]->GetNickname()
                << " (" << recipients[i]->GetLogin() << ")\n";
        }

        std::cout << "\nВведите номер получателя: ";
        size_t index;
        if (!(std::cin >> index) || index == 0 || index > recipients.size()) 
        {
            std::cout << "Неверный номер.\n\n";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return;
        }

        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Сообщение: ";
        std::string text;
        std::getline(std::cin, text);

        const std::string& recipientLogin = recipients[index - 1]->GetLogin();
        m_messages.emplace_back(activeLogin, recipientLogin, std::move(text));
        std::cout << "Личное сообщение отправлено пользователю " << recipientLogin << "!\n\n";
    }

    void ChatApp::HandleViewMessages() 
    {
        if (!IsUserLoggedIn()) return;

        const std::string activeLogin = GetActiveLogin();
        bool hasMessages = false;

        std::cout << "\n=== Ваши сообщения ===\n";
        for (const auto& msg : m_messages) 
        {
            if (msg.IsForUser(activeLogin)) 
            {
                hasMessages = true;

                // Безопасное форматирование времени (Windows-совместимое)
                std::time_t time = msg.GetTimestamp();
                std::tm tmBuffer{};
                if (localtime_s(&tmBuffer, &time) != 0) 
                {
                    continue; // пропустить, если ошибка
                }
                char buffer[20];
                std::strftime(buffer, sizeof(buffer), "%H:%M", &tmBuffer);

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
        std::exit(0);
    }

    bool ChatApp::IsUserLoggedIn() const 
    {
        return m_userManager.GetActiveUser() != nullptr;
    }

    std::string ChatApp::GetActiveLogin() const 
    {
        const User* user = m_userManager.GetActiveUser();
        return user ? user->GetLogin() : "";
    }

}