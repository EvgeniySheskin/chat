#pragma once
#include <string>
#include <iosfwd>

namespace chat 
{

    class User 
    {
    public:
        User() = default;
        User(std::string login, std::string password, std::string nickname);

        // Геттеры (все константные)
        const std::string& GetLogin() const noexcept { return m_login; }
        const std::string& GetPassword() const noexcept { return m_password; }
        const std::string& GetNickname() const noexcept { return m_nickname; }

        // Оператор вывода
        friend std::ostream& operator<<(std::ostream& os, const User& user);

    private:
        std::string m_login;
        std::string m_password;
        std::string m_nickname;
    };

}