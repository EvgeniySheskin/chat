#include "UserManager.h"
#include "Validation.h"
#include <stdexcept>
#include <iostream>

namespace chat
{
    void UserManager::ValidatePassword(const std::string& password) const
    {
        ValidationResult result = chat::validatePassword(password);
        if (!result.success)
        {
            throw std::invalid_argument(result.errorMessage);
        }
    }

    void UserManager::ValidateLogin(const std::string& login) const
    {
        ValidationResult result = chat::validateLogin(login);
        if (!result.success)
        {
            throw std::invalid_argument(result.errorMessage);
        }
    }

    bool UserManager::IsLoginAvailable(const std::string& login) const noexcept {
        return m_users.find(login) == m_users.end();
    }

    void UserManager::AddNewUser(std::string login, std::string password, std::string nickname)
    {
        ValidateLogin(login);
        if (!IsLoginAvailable(login)) throw NewUserException{};
        ValidatePassword(password);
        std::string key = login;
        m_users.emplace(std::move(key), User{ std::move(login), std::move(password), std::move(nickname) });
        std::cout << "[UserManager] New user registered: " << key << std::endl;
    }

    void UserManager::DeleteUser(const std::string& login)
    {
        auto it = m_users.find(login);
        if (it == m_users.end()) throw NoSuchUserException{};
        m_users.erase(it);
    }

    const User* UserManager::FindUserByLogin(const std::string& login) const noexcept
    {
        auto it = m_users.find(login);
        if (it != m_users.end()) {
            std::cout << "[UserManager] Найден пользователь: " << login << std::endl;
            return &it->second;
        }
        else {
            std::cout << "[UserManager] Пользователь не найден: " << login << std::endl;
            return nullptr;
        }
    }

    /*std::vector<const User*> UserManager::GetRegisteredUsers() const
    {
        std::vector<const User*> result;
        result.reserve(m_users.size());
        for (const auto& pair : m_users)
        {
            result.push_back(&pair.second);
        }
        return result;
    }*/
}