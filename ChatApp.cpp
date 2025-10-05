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
        menu.addItem("�����", [this]() { HandleLogin(); });
        menu.addItem("������������������", [this]() { HandleRegister(); });
        menu.addItem("�����", [this]() { HandleExit(); });
        menu.run();
    }

    void ChatApp::ShowChatMenu() 
    {
        ConsoleMenu menu;
        menu.addItem("��������� ���������", [this]() { HandleSendMessage(); });
        menu.addItem("����������� ���������", [this]() { HandleViewMessages(); });
        menu.addItem("����� �� ��������", [this]() { HandleLogout(); });
        menu.run();
    }

    void ChatApp::HandleRegister() 
    {
        try 
        {
            std::string login, password, nickname;
            std::cout << "�����: ";
            std::cin >> login;
            std::cout << "������ (������� 6 ��������, ����� � ���������� $&*!): ";
            std::cin >> password;
            std::cout << "�������: ";
            std::cin >> nickname;

            m_UserManager.AddNewUser(login, password, nickname);
            std::cout << "����������� �������!\n\n";
        }
        catch (const NewUserException& e) {
            std::cout << "������: " << e.what() << "\n\n";
        }
    }

    void ChatApp::HandleLogin() {
        try {
            std::string login, password;
            std::cout << "�����: ";
            std::cin >> login;
            std::cout << "������: ";
            std::cin >> password;

            // ������� ��������������: ���� ������������ � ���������� ������
            User* user = m_UserManager.FindUserByLogin(login);
            if (!user || user->GetPassword() != password) {
                std::cout << "�������� ����� ��� ������.\n\n";
                return;
            }

            m_UserManager.SetActiveUser(login);
            std::cout << "����� ����������, " << user->GetNickname() << "!\n\n";
            ShowChatMenu();
        }
        catch (const NoSuchUserException& e) {
            std::cout << "������: " << e.what() << "\n\n";
        }
    }

    void ChatApp::HandleLogout() 
    {
        m_UserManager.SetActiveUser(""); // ����� 
        std::cout << "�� ����� �� ��������.\n\n";
        return; 
    }

    void ChatApp::HandleSendMessage() 
    {
        if (!IsUserLoggedIn()) return;

        std::string choice;
        std::cout << "��������� (1) ������ ��������� ��� (2) � ����� ���? ";
        std::cin >> choice;

        std::string recipient, text;
        if (choice == "1") 
        {
            std::cout << "���� (�����): ";
            std::cin >> recipient;

            // ��������, ���������� �� ����������
            if (!m_UserManager.FindUserByLogin(recipient)) 
            {
                std::cout << "������������ �� ������.\n\n";
                return;
            }
            std::cin.ignore();
            std::cout << "���������: ";
            std::getline(std::cin, text);
            m_Messages.emplace_back(GetActiveLogin(), recipient, text);
        }
        else if (choice == "2") 
        {
            std::cin.ignore();
            std::cout << "��������� � ����� ���: ";
            std::getline(std::cin, text);
            m_Messages.emplace_back(GetActiveLogin(), "", text);
        }
        else 
        {
            std::cout << "�������� �����.\n\n";
            return;
        }
        std::cout << "��������� ����������!\n\n";
    }

    void ChatApp::HandleViewMessages() 
    {
        if (!IsUserLoggedIn()) return;

        std::string activeLogin = GetActiveLogin();
        bool hasMessages = false;

        std::cout << "\n=== ���� ��������� ===\n";
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
                    std::cout << "������ �� " << msg.GetFrom() << ": ";
                }
                else 
                {
                    std::cout << msg.GetFrom() << " (���): ";
                }
                std::cout << msg.GetText() << "\n";
            }
        }

        if (!hasMessages) 
        {
            std::cout << "��� ���������.\n";
        }
        std::cout << "\n";
    }

    void ChatApp::HandleExit() 
    {
        std::cout << "�� ��������!\n";
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