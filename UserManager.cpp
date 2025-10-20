#include "UserManager.h"
#include <iostream>
#include <regex>
#include <algorithm>
#include <vector>

namespace chat 
{
constexpr auto MAX_PASS_LENGTH = 6;

    void UserManager::ValidatePassword(const std::string& password) const 
    {
        if (password.empty()) 
        {
            throw std::invalid_argument("Пустой пароль задавать нельзя!");
        }
        if (password.size() < MAX_PASS_LENGTH)
        {
            std::string msg = "Пароль должен содержать минимум " + to_string(MAX_PASS_LENGTH) + " символов";
            throw std::invalid_argument(msg);
        }
        bool hasDigit = std::regex_search(password, std::regex(R"(\d)"));
        bool hasSpecial = std::regex_search(password, std::regex(R"([\$&*!])"));
        if (!hasDigit || !hasSpecial) 
        {
            throw std::invalid_argument("Пароль должен содержать как минимум одну цифру и один спецсимвол: $, &, *, !");
        }
    }

    bool UserManager::IsLoginAvailable(const std::string& login) const noexcept 
    {
        return m_users.find(login) == m_users.end();
    }

    void UserManager::AddNewUser(std::string login, std::string password, std::string nickname) 
    {
        const std::string loginForOutput = login;

        /*if (!IsLoginAvailable(loginForOutput)) {
            throw NewUserException("Пользователь с таким логином уже зарегистрирован");
        }
        ValidatePassword(password);*/

        m_users.emplace(
            login, 
            User{ loginForOutput, std::move(password), std::move(nickname) }
        );
        std::cout << "Доброе пожаловать в чат, " << m_users.at(login).GetNickname() << "!\n\n";
    }

    void UserManager::DeleteUser(const std::string& login) 
    {
        auto it = m_users.find(login);
        if (it == m_users.end()) 
        {
            throw NoSuchUserException(login);
        }
        // Если удаляем активного — сбросить
        if (m_activeUser == &it->second) 
        {
            m_activeUser = nullptr;
        }
        m_users.erase(it);
    }

    User* UserManager::FindUserByLogin(const std::string& login) noexcept 
    {
        auto it = m_users.find(login);
        return (it != m_users.end()) ? &it->second : nullptr;
    }

    const User* UserManager::FindUserByLogin(const std::string& login) const noexcept 
    {
        auto it = m_users.find(login);
        return (it != m_users.end()) ? &it->second : nullptr;
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

    std::vector<const User*> UserManager::GetRegisteredUsers() const 
    {
        std::vector<const User*> result;
        result.reserve(m_users.size());
        for (const auto& pair : m_users) 
        {
            result.push_back(&pair.second);
        }
        return result;
    }
}