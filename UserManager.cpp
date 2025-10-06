#include "UserManager.h"
#include <iostream>
#include <regex>

namespace chat
{
    UserManager::UserManager() 
    {
        m_ActiveUser = nullptr;
    }

    void UserManager::ValidatePassword(const std::string& password) const 
    {
        if (password.size() < 6) 
        {
            throw std::invalid_argument("Пароль должен содержать как минимум 6 символов.\n");
        }
        // Требует хотя бы одну цифру и хотя бы один спецсимвол
        bool hasDigit = std::regex_search(password, std::regex(R"(\d)"));
        bool hasSpecial = std::regex_search(password, std::regex(R"([\$&*!])"));

        if (!hasDigit || !hasSpecial) 
        {
            throw std::invalid_argument("Пароль должен содержать как минимум одну цифру и один спецсимвол: $, &, *, !");
        }
    }

    bool UserManager::IsNicknameAvailable(const std::string& nick) const
    {
        for (const auto& user : m_RegisteredUsers)
        {
            if (user.GetNickname() == nick)
            {
                return false;
            }
        }
        return true;
    }

    bool UserManager::IsLoginAvailable(const std::string& login) const 
    {
        for (const auto& user : m_RegisteredUsers) 
        {
            if (user.GetLogin() == login) 
            {
                return false;
            }
        }
        return true;
    }

    void UserManager::AddNewUser(const std::string& login, const std::string& password, const std::string& nickname) 
    {
        /*if (!IsLoginAvailable(login))
        {
            throw NewUserException("Юзер с таким логином уже есть");
        }

        ValidatePassword(password);*/

        // Создаём и добавляем пользователя
        m_RegisteredUsers.emplace_back(login, password, nickname);
        std::cout << "Добро пожаловать в чат, " << m_RegisteredUsers.back().GetNickname() << "!\n\n";
    }

    void UserManager::DeleteUser(const std::string& login) 
    {
        for (auto it = m_RegisteredUsers.begin(); it != m_RegisteredUsers.end(); ++it) 
        {
            if (it->GetLogin() == login) {
                // Если удаляем активного пользователя — сбросить указатель
                if (m_ActiveUser == &(*it)) 
                {
                    m_ActiveUser = nullptr;
                }
                m_RegisteredUsers.erase(it);
                return;
            }
        }
        throw NoSuchUserException(login);
    }

    User* UserManager::FindUserByLogin(const std::string& login) noexcept 
    {
        for (auto& user : m_RegisteredUsers) 
        {
            if (user.GetLogin() == login) 
            {
                return &user;
            }
        }
        return nullptr;
    }


    void UserManager::SetActiveUser(const std::string& login) 
    {
        User* user = FindUserByLogin(login);
        if (!user) 
        {
            throw NoSuchUserException(login);
        }
        m_ActiveUser = user;
    }

    User& UserManager::operator[](size_t index) 
    {
        if (index >= m_RegisteredUsers.size()) 
        {
            throw std::out_of_range("UserManager::operator[]: index out of range");
        }
        return m_RegisteredUsers[index];
    }

    const User& UserManager::operator[](size_t index) const 
    {
        if (index >= m_RegisteredUsers.size()) 
        {
            throw std::out_of_range("UserManager::operator[]: index out of range");
        }
        return m_RegisteredUsers[index];
    }
}