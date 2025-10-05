#include "UserManager.h"
#include <iostream>
#include <regex>

namespace chat
{
    UserManager::UserManager() 
    {
        m_activeUser = nullptr;
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

    bool UserManager::IsLoginAvailable(const std::string& login) const 
    {
        for (const auto& user : m_registeredUsers) 
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
        if (!IsLoginAvailable(login)) 
        {
            throw NewUserException("Login already exists.");
        }

        ValidatePassword(password);

        // Создаём и добавляем пользователя
        m_registeredUsers.emplace_back(login, password, nickname);
        std::cout << "Добро пожаловать в чат, " << m_registeredUsers.back().GetNickname() << "!\n\n";
    }

    void UserManager::DeleteUser(const std::string& login) 
    {
        for (auto it = m_registeredUsers.begin(); it != m_registeredUsers.end(); ++it) 
        {
            if (it->GetLogin() == login) {
                // Если удаляем активного пользователя — сбросить указатель
                if (m_activeUser == &(*it)) 
                {
                    m_activeUser = nullptr;
                }
                m_registeredUsers.erase(it);
                return;
            }
        }
        throw NoSuchUserException(login);
    }

    User* UserManager::FindUserByLogin(const std::string& login) noexcept 
    {
        for (auto& user : m_registeredUsers) 
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
        m_activeUser = user;
    }

    User& UserManager::operator[](size_t index) 
    {
        if (index >= m_registeredUsers.size()) 
        {
            throw std::out_of_range("UserManager::operator[]: index out of range");
        }
        return m_registeredUsers[index];
    }

    const User& UserManager::operator[](size_t index) const 
    {
        if (index >= m_registeredUsers.size()) 
        {
            throw std::out_of_range("UserManager::operator[]: index out of range");
        }
        return m_registeredUsers[index];
    }
}