#pragma once
#include "User.h"
#include <unordered_map>
#include <vector>
#include <string>

namespace chat
{
    struct NewUserException : public std::exception
    {
        const char* what() const noexcept override { return "User already exists"; }
    };
    struct NoSuchUserException : public std::exception
    {
        const char* what() const noexcept override { return "User not found"; }
    };

    class UserManager
    {
    public:
        UserManager() = default;
        void ValidatePassword(const std::string& password) const;
        bool IsLoginAvailable(const std::string& login) const noexcept;
        void AddNewUser(std::string login, std::string password, std::string nickname);
        void DeleteUser(const std::string& login);
        const User* FindUserByLogin(const std::string& login) const noexcept;
        //std::vector<const User*> GetRegisteredUsers() const;
        void ValidateLogin(const std::string& login) const;

        // Новый метод для получения всех пользователей
        const std::unordered_map<std::string, User>& GetAllUsers() const { return m_users; }

    private:
        std::unordered_map<std::string, User> m_users;
    };
}