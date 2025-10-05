#pragma once

#include "User.h"
#include "NewUserException.h"
#include "NoSuchUserException.h"
#include <vector>
#include <string>
#include <stdexcept> // ��� ����������� ����������

namespace chat
{
    class UserManager {
    public:
        UserManager();
        ~UserManager() = default; 

        // �����n� ����������� 
        UserManager(const UserManager&) = delete;
        UserManager& operator=(const UserManager&) = delete;

        // �������� ������������ ������ (������� ���������� ��� ������)
        void ValidatePassword(const std::string& password) const;

        // ��������, �������� �� �����
        bool IsLoginAvailable(const std::string& login) const;

        // ���������� ������ ������������
        void AddNewUser(const std::string& login, const std::string& password, const std::string& nickname);

        // �������� ������������ �� ������
        void DeleteUser(const std::string& login);

        // ����� ������������ �� ������ (���������� nullptr, ���� �� ������)
        User* FindUserByLogin(const std::string& login) noexcept;

        // ��������� ��������� ������������
        void SetActiveUser(const std::string& login);
        User* GetActiveUser() const noexcept { return m_activeUser; }

        // ������ � ������������ �� ������� (� ��������� ������)
        User& operator[](size_t index);
        const User& operator[](size_t index) const;

        // ��������� ���������� ������������������ �������������
        size_t GetUserCount() const noexcept { return m_registeredUsers.size(); }

    private:
        std::vector<User> m_registeredUsers;
        User* m_activeUser = nullptr; // ��������� �� ������� �������
    };
}

