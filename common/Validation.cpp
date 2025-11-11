#include "Validation.h"
#include <cctype>

namespace chat 
{

    ValidationResult validateLogin(const std::string& login) 
    {
        if (login.empty()) 
        {
            return { false, "Логин не может быть пустым." };
        }
        if (login.size() > MAX_LOGIN_LENGTH) 
        {
            return { false, "Логин слишком длинный (макс. " + std::to_string(MAX_LOGIN_LENGTH) + " символов)." };
        }
        for (char c : login) 
        {
            if (std::isspace(static_cast<unsigned char>(c))) 
            {
                return { false, "Логин не может содержать пробелы." };
            }
        }
        return { true, "" };
    }

    ValidationResult validatePassword(const std::string& password) 
    {
        if (password.empty()) 
        {
            return { false, "Пароль не может быть пустым." };
        }
        if (password.size() < MIN_PASSWORD_LENGTH) 
        {
            return { false, "Пароль должен содержать минимум " + std::to_string(MIN_PASSWORD_LENGTH) + " символа." };
        }
        // Проверка на наличие хотя бы одного спецсимвола
        bool hasSpecial = false;
        for (char c : password) 
        {
            if (c == '$' || c == '&' || c == '*' || c == '!') {
                hasSpecial = true;
                break;
            }
        }
        if (!hasSpecial) 
        {
            return { false, "Пароль должен содержать хотя бы один из спецсимволов: $, &, *, !" };
        }
        return { true, "" };
    }

}