#pragma once
#include <exception>

namespace chat {
    struct LogoutException : public std::exception 
    {
        const char* what() const noexcept override 
        {
            return "Вы вышли из аккаунта";
        }
    };
}