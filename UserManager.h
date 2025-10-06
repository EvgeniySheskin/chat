#pragma once

#include "User.h"
#include "NewUserException.h"
#include "NoSuchUserException.h"
#include <vector>
#include <string>
#include <stdexcept> // для стандартных исключений

namespace chat
{
    class UserManager {
    public:
        UserManager();
        ~UserManager() = default; 

        // Запреnт копирования 
        UserManager(const UserManager&) = delete;
        UserManager& operator=(const UserManager&) = delete;

        // Проверка корректности пароля (бросает исключение при ошибке)
        void ValidatePassword(const std::string& password) const;
        
        // Проверка, свободен ли никнейм
        bool IsNicknameAvailable(const std::string& nick) const;

        // Проверка, свободен ли логин
        bool IsLoginAvailable(const std::string& login) const;

        // Добавление нового пользователя
        void AddNewUser(const std::string& login, const std::string& password, const std::string& nickname);

        // Удаление пользователя по логину
        void DeleteUser(const std::string& login);

        // Поиск пользователя по логину (возвращает nullptr, если не найден)
        User* FindUserByLogin(const std::string& login) noexcept;

        // Установка активного пользователя
        void SetActiveUser(const std::string& login);
        User* GetActiveUser() const noexcept { return m_ActiveUser; }

        // Доступ к пользователю по индексу (с проверкой границ)
        User& operator[](size_t index);
        const User& operator[](size_t index) const;

        // Получение количества зарегистрированных пользователей
        size_t GetUserCount() const noexcept { return m_RegisteredUsers.size(); }

        // Получение списка зарегистрированных пользователей
        const std::vector<User>& GetRegisteredUsers() const noexcept { return m_RegisteredUsers; }

    private:
        std::vector<User> m_RegisteredUsers;
        User* m_ActiveUser = nullptr; // указывает на элемент вектора
    };
}

