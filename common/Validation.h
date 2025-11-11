#pragma once
#include <string>

namespace chat 
{

    // Результат валидации
    struct ValidationResult 
    {
        bool success;
        std::string errorMessage;
    };

    // Общие правила
    constexpr size_t MIN_PASSWORD_LENGTH = 3;
    constexpr size_t MAX_LOGIN_LENGTH = 32;
    constexpr const char* SPECIAL_CHARS = "$&*!";

    // Функции валидации
    ValidationResult validateLogin(const std::string& login);
    ValidationResult validatePassword(const std::string& password);
}