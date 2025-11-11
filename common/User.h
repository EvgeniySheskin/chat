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
        const std::string& GetLogin() const { return m_login; }
        const std::string& GetPassword() const { return m_password; }
        const std::string& GetNickname() const { return m_nickname; }
        friend std::ostream& operator<<(std::ostream& os, const User& user);

    private:
        std::string m_login;
        std::string m_password;
        std::string m_nickname;
    };

}