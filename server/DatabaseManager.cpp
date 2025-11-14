#include "DatabaseManager.h"
#include <iostream>
#include <cstring>
#include <sstream>
#include <iomanip>

namespace chat
{

    DatabaseManager::DatabaseManager() = default;

    DatabaseManager::~DatabaseManager() {
        if (m_stmt) SQLFreeHandle(SQL_HANDLE_STMT, m_stmt);
        if (m_conn) {
            SQLDisconnect(m_conn);
            SQLFreeHandle(SQL_HANDLE_DBC, m_conn);
        }
        if (m_env) SQLFreeHandle(SQL_HANDLE_ENV, m_env);
    }

    bool DatabaseManager::connect(const std::string& connStr) {
        if (SQL_SUCCESS != SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &m_env))
            return false;

        if (SQL_SUCCESS != SQLSetEnvAttr(m_env, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0))
            return false;

        if (SQL_SUCCESS != SQLAllocHandle(SQL_HANDLE_DBC, m_env, &m_conn))
            return false;

        // Используем ANSI-версию SQLDriverConnectA
        if (SQL_SUCCESS != SQLDriverConnectA(m_conn, NULL, (SQLCHAR*)connStr.c_str(), SQL_NTS, NULL, 0, NULL, SQL_DRIVER_COMPLETE))
            return false;

        if (SQL_SUCCESS != SQLAllocHandle(SQL_HANDLE_STMT, m_conn, &m_stmt))
            return false;

        return true;
    }

    void DatabaseManager::initDatabase() {
        executeQuery("CREATE DATABASE IF NOT EXISTS chatdb;");
        executeQuery("USE chatdb;");
        executeQuery(R"(
        CREATE TABLE IF NOT EXISTS users (
            id INT AUTO_INCREMENT PRIMARY KEY,
            login VARCHAR(50) UNIQUE NOT NULL,
            password_hash VARCHAR(255) NOT NULL,
            nickname VARCHAR(50) NOT NULL
        );
    )");
        executeQuery(R"(
        CREATE TABLE IF NOT EXISTS messages (
            id INT AUTO_INCREMENT PRIMARY KEY,
            from_user_id INT NOT NULL,
            to_user_id INT DEFAULT NULL,
            timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
            message_text TEXT NOT NULL,
            FOREIGN KEY (from_user_id) REFERENCES users(id),
            FOREIGN KEY (to_user_id) REFERENCES users(id)
        );
    )");
    }

    bool DatabaseManager::addUser(const std::string& login, const std::string& password, const std::string& nickname) {
        std::string query = "INSERT INTO users (login, password_hash, nickname) VALUES ('" + login + "','" + password + "','" + nickname + "');";
        return executeQuery(query);
    }

    std::optional<User> DatabaseManager::findUserByLogin(const std::string& login) {
        std::string query = "SELECT login, password_hash, nickname FROM users WHERE login = '" + login + "';";
        executeQuery(query);
        auto results = fetchResults();
        if (!results.empty()) {
            auto& row = results[0];
            return User(row["login"], row["password_hash"], row["nickname"]);
        }
        return std::nullopt;
    }

    std::vector<User> DatabaseManager::getAllUsers() {
        std::vector<User> users;
        executeQuery("SELECT login, password_hash, nickname FROM users;");
        auto results = fetchResults();
        for (auto& row : results) {
            users.emplace_back(row["login"], row["password_hash"], row["nickname"]);
        }
        return users;
    }

    int DatabaseManager::getUserIDByLogin(const std::string& login) {
        std::string query = "SELECT id FROM users WHERE login = '" + login + "';";
        executeQuery(query);
        auto results = fetchResults();
        if (!results.empty()) {
            return std::stoi(results[0]["id"]);
        }
        return -1;
    }

    bool DatabaseManager::addMessage(const Message& msg) {
        int fromId = getUserIDByLogin(msg.GetFrom());
        if (fromId == -1) return false; // отправитель не найден

        int toId = -1;
        if (msg.IsPrivate()) {
            toId = getUserIDByLogin(msg.GetTo());
            if (toId == -1) return false; // получатель не найден
        }

        std::string toStr = (toId == -1) ? "NULL" : std::to_string(toId);
        std::string text = msg.GetText();
        std::string query = "INSERT INTO messages (from_user_id, to_user_id, message_text) VALUES (" + std::to_string(fromId) + ", " + toStr + ", '" + text + "');";
        return executeQuery(query);
    }

    std::vector<Message> DatabaseManager::getMessagesForUser(const std::string& username) {
        int userId = getUserIDByLogin(username);
        if (userId == -1) return {};

        std::string query = R"(
        SELECT u_from.login as from_login, u_to.login as to_login, m.message_text, m.timestamp
        FROM messages m
        JOIN users u_from ON m.from_user_id = u_from.id
        LEFT JOIN users u_to ON m.to_user_id = u_to.id
        WHERE m.to_user_id IS NULL OR m.to_user_id = )" + std::to_string(userId) + R"( OR m.from_user_id = )" + std::to_string(userId) + R"(;
    )";

        executeQuery(query);
        auto results = fetchResults();
        std::vector<Message> messages;
        for (auto& row : results) {
            std::string from = row["from_login"];
            std::string to = (row["to_login"] == "NULL") ? "" : row["to_login"];
            std::string text = row["message_text"];

            std::tm tm = {};
            std::istringstream ss(row["timestamp"]);
            ss.imbue(std::locale::classic());
            ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
            std::time_t timestamp = (ss.fail()) ? std::time(nullptr) : std::mktime(&tm);

            messages.emplace_back(from, to, text, timestamp);
        }
        return messages;
    }

    bool DatabaseManager::executeQuery(const std::string& query) {
        // Используем ANSI-версию
        if (SQLExecDirectA(m_stmt, (SQLCHAR*)query.c_str(), SQL_NTS) == SQL_SUCCESS) {
            return true;
        }
        return false;
    }

    std::vector<std::map<std::string, std::string>> DatabaseManager::fetchResults() {
        std::vector<std::map<std::string, std::string>> results;
        SQLSMALLINT colCount;
        SQLNumResultCols(m_stmt, &colCount);

        std::vector<std::string> colNames(colCount);
        for (int i = 0; i < colCount; ++i) {
            SQLCHAR name[256];
            SQLDescribeColA(m_stmt, i + 1, name, sizeof(name), nullptr, nullptr, nullptr, nullptr, nullptr); // ANSI
            colNames[i] = std::string((char*)name);
        }

        while (SQLFetch(m_stmt) != SQL_NO_DATA) {
            std::map<std::string, std::string> row;
            for (int i = 1; i <= colCount; ++i) {
                SQLCHAR buffer[1024];
                SQLLEN indicator;
                SQLGetData(m_stmt, i, SQL_CHAR, buffer, sizeof(buffer), &indicator);
                if (indicator != SQL_NULL_DATA) {
                    row[colNames[i - 1]] = std::string((char*)buffer);
                }
                else {
                    row[colNames[i - 1]] = "NULL";
                }
            }
            results.push_back(row);
        }
        return results;
    }

}