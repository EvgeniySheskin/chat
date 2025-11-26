// DatabaseManager.h
#pragma once

#include "../common/User.h"
#include "../common/Message.h"
#include "../common/sha1.h"
#include "../common/Logger.h"     

#ifdef _WIN32
#include <windows.h>
#endif

#include <sql.h>
#include <sqlext.h>
#include <sqltypes.h>

#include <string>
#include <vector>
#include <ctime>
#include <map>
#include <optional>
#include <mutex> // C++17: mutex для потокобезопасности
#include <memory> // C++17 для unique_ptr

namespace chat
{

    class DatabaseManager
    {
    public:
        DatabaseManager();
        ~DatabaseManager();

        bool connect(const std::string& connStr);
        void initDatabase();

        // Пароль принимается как хеш
        bool addUser(const std::string& login, const std::string& password_hash, const std::string& nickname);
        // Результат поиска пользователя для проверки пароля
        std::optional<User> findUserByLogin(const std::string& login);
        std::vector<User> getAllUsers();
        int getUserIDByLogin(const std::string& login);

        bool addMessage(const Message& msg);
        std::vector<Message> getMessagesForUser(const std::string& username);

    private:
        SQLHANDLE m_env{ nullptr };
        SQLHANDLE m_conn{ nullptr };
        SQLHANDLE m_stmt{ nullptr };

        // --- Используем рекурсивный мьютекс ---
        std::recursive_mutex m_db_mutex; // Мьютекс для синхронизации доступа к ODBC-хендлам
        // --- Конец изменения ---

        // --- Добавлено поле для логгера ---
        std::unique_ptr<Logger> m_logger; // C++17: unique_ptr для логгера
        // --- Конец добавленного поля ---

        // executeQueryDirect использует SQLExecDirectA
        bool executeQueryDirect(const std::string& query);
        std::vector<std::map<std::string, std::string>> fetchResults();

        // --- Вспомогательные методы для безопасности ---
        std::string hashPassword(const std::string& password); // Метод для хеширования
        // --- Конец вспомогательных методов ---
    };

}