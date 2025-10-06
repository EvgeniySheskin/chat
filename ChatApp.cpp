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
        menu.AddItem("Войти", [this]() { HandleLogin(); });
        menu.AddItem("Зарегистрироваться", [this]() { HandleRegister(); });
        menu.AddItem("Выход", [this]() { HandleExit(); });
        menu.Run();
    }

    void ChatApp::ShowChatMenu() 
    {
        ConsoleMenu menu;
        menu.AddItem("Отправить сообщение", [this]() { HandleSendMessage(); });
        menu.AddItem("Просмотреть сообщения", [this]() { HandleViewMessages(); });
        menu.AddItem("Выйти из аккаунта", [this]() { HandleLogout(); });
        menu.Run();
    }

    void ChatApp::HandleRegister() 
    {
        try 
        {
            std::string login, password, nickname;
            std::cout << "Логин: ";
            std::cin >> login;
            if (!m_UserManager.IsLoginAvailable(login))
                throw NewUserException("Юзер с таким логином уже есть");
            std::cout << "Пароль (минимум 6 символов, цифра и спецсимвол $&*!): ";
            std::cin >> password;
            m_UserManager.ValidatePassword(password);
            std::cout << "Никнейм: ";
            std::cin >> nickname;
            if (!m_UserManager.IsNicknameAvailable(login))
                throw NewUserException("Юзер с таким ником уже есть в чате");
            m_UserManager.AddNewUser(login, password, nickname);
            std::cout << "Регистрация успешна!\n\n";
        }
        catch (const NewUserException& e) 
        {
            std::cout << "Ошибка: " << e.what() << "\n\n";
        }
        catch (const std::invalid_argument& e)
        {
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
            std::string activeLogin = GetActiveLogin();
            const auto& users = m_UserManager.GetRegisteredUsers();

            // Фильтруем: все пользователи, кроме текущего
            std::vector<std::string> availableRecipients;
            std::cout << "\n=== Выберите получателя ===\n";
            int index = 1;
            for (const auto& user : users) {
                if (user.GetLogin() != activeLogin) {
                    std::cout << index << ". " << user.GetNickname() << "\n";
                    availableRecipients.push_back(user.GetNickname());
                    ++index;
                }
            }

            if (availableRecipients.empty()) {
                std::cout << "Нет других пользователей для отправки сообщения.\n\n";
                return;
            }

            std::cout << "\nВведите номер получателя: ";
            int recipientIndex;
            if (!(std::cin >> recipientIndex) 
                             || recipientIndex < 1 
                                 || recipientIndex > static_cast<int>(availableRecipients.size())) 
            {
                std::cout << "Неверный номер.\n\n";
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                return;
            }
            std::string recipientNickname = availableRecipients[recipientIndex - 1];
            std::cin.ignore();
            std::cout << "Сообщение пользователю " << recipientNickname << ": ";
            std::getline(std::cin, text);
            m_Messages.emplace_back(m_UserManager.GetActiveUser()->GetNickname(), recipientNickname, text);
            std::cout << "Личное сообщение отправлено пользователю " << recipientNickname << "!\n\n";
        }
        else if (choice == "2") 
        {
            std::cin.ignore();
            std::cout << "Сообщение в общий чат: ";
            std::getline(std::cin, text);
            m_Messages.emplace_back(GetActiveNickname(), "", text);
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
    std::string ChatApp::GetActiveNickname() const
    {
        auto* user = m_UserManager.GetActiveUser();
        return user ? user->GetNickname() : "";
    }
}