/*Класс пользователя
Хранит данные, необходимые для идентификации и атуентификации пользователя*/
#include "User.h"
#include <iostream>

namespace chat 
{

    User::User(std::string login, std::string password, std::string nickname)
        : m_login(std::move(login))
        , m_password(std::move(password))
        , m_nickname(std::move(nickname))
    {}

    std::ostream& operator<<(std::ostream& os, const User& user) 
    {
        os << "Логин: " << user.m_login
            << " | Никнейм: " << user.m_nickname;
        return os;
    }

}