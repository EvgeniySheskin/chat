#pragma once
#include "User.h"
#include "NewUserException.h"
#include "NoSuchUserException.h"
#include <unordered_map>
#include <vector>
#include <string>
#include <stdexcept>

namespace chat 
{
    class UserManager 
    {
    public:
        UserManager() = default;
        ~UserManager() = default;

        // Запрет копирования
        UserManager(const UserManager&) = delete;
        UserManager& operator=(const UserManager&) = delete;

        // Валидация пароля
        void ValidatePassword(const std::string& password) const;

        // Проверка доступности логина
        bool IsLoginAvailable(const std::string& login) const noexcept;

        // Регистрация
        void AddNewUser(std::string login, std::string password, std::string nickname);

        // Удаление
        void DeleteUser(const std::string& login);

        // Поиск
        User* FindUserByLogin(const std::string& login) noexcept;
        const User* FindUserByLogin(const std::string& login) const noexcept;

        // Активный пользователь
        void SetActiveUser(const std::string& login);
        User* GetActiveUser() const noexcept { return m_activeUser; }

        // Получить всех пользователей (для отображения списка)
        std::vector<const User*> GetRegisteredUsers() const;

    private:
        // Храним пользователей в unordered_map для O(1) поиска
        std::unordered_map<std::string, User> m_users;
        User* m_activeUser = nullptr;
    };
}