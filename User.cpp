/*Класс пользователя
Хранит данные, необходимые для идентификации и атуентификации пользователя*/
#include "User.h"
#include <iostream>

namespace chat
{
    User::User(const std::string& login, const std::string& password, const std::string& nickname)
        : m_Login(login), m_Password(password), m_Nickname(nickname) {
    }

    std::ostream& operator<<(std::ostream& os, const User& user)
    {
        os << "Login: " << user.m_Login
            << "\nNickname: " << user.m_Nickname << std::endl;
        return os;
    }
}