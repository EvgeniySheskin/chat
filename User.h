#pragma once

#include <string>
#include <iosfwd>

namespace chat
{
    class User
    {
    public:
        // Конструкторы
        User() = default;
        User(const std::string& login, const std::string& password, const std::string& nickname);

        // Правило пяти 
        User(const User&) = default;
        User(User&&) noexcept = default;
        User& operator=(const User&) = default;
        User& operator=(User&&) noexcept = default;
        ~User() = default;

        // Геттеры
        const std::string& GetLogin() const noexcept { return m_Login; }
        const std::string& GetPassword() const noexcept { return m_Password; }
        const std::string& GetNickname() const noexcept { return m_Nickname; }

        // Сравнение логина — эффективно и ясно
        bool HasLogin(const std::string& login) const noexcept { return m_Login == login; }

        // Оператор вывода
        friend std::ostream& operator<<(std::ostream& os, const User& user);

    private:
        std::string m_Login;
        std::string m_Password;
        std::string m_Nickname;
    };
}