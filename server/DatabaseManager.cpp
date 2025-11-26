#include "DatabaseManager.h"
#include <iostream>
#include <cstring>
#include <sstream>
#include <iomanip>
#include <algorithm> 
#include <cctype>    

namespace chat
{

    DatabaseManager::DatabaseManager() = default;

    DatabaseManager::~DatabaseManager() 
    {
        std::lock_guard<std::mutex> lock(m_db_mutex);
        if (m_stmt) SQLFreeHandle(SQL_HANDLE_STMT, m_stmt);
        if (m_conn) 
        {
            SQLDisconnect(m_conn);
            SQLFreeHandle(SQL_HANDLE_DBC, m_conn);
        }
        if (m_env) SQLFreeHandle(SQL_HANDLE_ENV, m_env);
    }

    bool DatabaseManager::connect(const std::string& connStr) 
    {
        std::lock_guard<std::mutex> lock(m_db_mutex);
        if (SQL_SUCCESS != SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &m_env))
            return false;

        if (SQL_SUCCESS != SQLSetEnvAttr(m_env, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0))
            return false;

        if (SQL_SUCCESS != SQLAllocHandle(SQL_HANDLE_DBC, m_env, &m_conn))
            return false;

        if (SQL_SUCCESS != SQLDriverConnectA(m_conn, NULL, (SQLCHAR*)connStr.c_str(), SQL_NTS, NULL, 0, NULL, SQL_DRIVER_COMPLETE))
            return false;

        if (SQL_SUCCESS != SQLAllocHandle(SQL_HANDLE_STMT, m_conn, &m_stmt))
            return false;

        return true;
    }

    void DatabaseManager::initDatabase() 
    {
        std::lock_guard<std::mutex> lock(m_db_mutex);
        SQLExecDirectA(m_stmt, (SQLCHAR*)"CREATE DATABASE IF NOT EXISTS chatdb;", SQL_NTS);
        SQLExecDirectA(m_stmt, (SQLCHAR*)"USE chatdb;", SQL_NTS);
        SQLExecDirectA(m_stmt, (SQLCHAR*)R"(
        CREATE TABLE IF NOT EXISTS users (
            id INT AUTO_INCREMENT PRIMARY KEY,
            login VARCHAR(50) UNIQUE NOT NULL,
            password_hash VARCHAR(255) NOT NULL, -- Поле теперь для хеша
            nickname VARCHAR(50) NOT NULL
        );
    )", SQL_NTS);
        SQLExecDirectA(m_stmt, (SQLCHAR*)R"(
        CREATE TABLE IF NOT EXISTS messages (
            id INT AUTO_INCREMENT PRIMARY KEY,
            from_user_id INT NOT NULL,
            to_user_id INT DEFAULT NULL,
            timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
            message_text TEXT NOT NULL,
            FOREIGN KEY (from_user_id) REFERENCES users(id),
            FOREIGN KEY (to_user_id) REFERENCES users(id)
        );
    )", SQL_NTS);
    }

    bool DatabaseManager::addUser(const std::string& login, const std::string& password_hash, const std::string& nickname) 
    {
        std::lock_guard<std::mutex> lock(m_db_mutex);

        std::string query = "INSERT INTO users (login, password_hash, nickname) VALUES (?, ?, ?);";
        if (!prepareStatement(query)) 
        {
            return false;
        }

        SQLLEN login_len = login.length();
        SQLLEN pass_len = password_hash.length();
        SQLLEN nick_len = nickname.length();

        if (!bindParameter(1, login, &login_len) ||
            !bindParameter(2, password_hash, &pass_len) ||
            !bindParameter(3, nickname, &nick_len)) 
        {
            return false;
        }

        return executePreparedQuery();
    }

    std::optional<User> DatabaseManager::findUserByLogin(const std::string& login) 
    {
        std::lock_guard<std::mutex> lock(m_db_mutex);

        std::string query = "SELECT login, password_hash, nickname FROM users WHERE login = ?;";
        if (!prepareStatement(query)) 
        {
            return std::nullopt;
        }

        SQLLEN login_len = login.length();
        if (!bindParameter(1, login, &login_len)) 
        {
            return std::nullopt;
        }

        if (!executePreparedQuery()) 
        {
            return std::nullopt;
        }

        auto results = fetchResults();
        if (!results.empty()) 
        {
            auto& row = results[0];
            return User(row["login"], row["password_hash"], row["nickname"]);
        }
        return std::nullopt;
    }

    std::vector<User> DatabaseManager::getAllUsers() 
    {
        std::lock_guard<std::mutex> lock(m_db_mutex);
        std::vector<User> users;

        std::string query = "SELECT login, password_hash, nickname FROM users;";
        if (!prepareStatement(query)) 
        {
            return users; 
        }

        if (!executePreparedQuery()) 
        {
            return users; 
        }

        auto results = fetchResults();
        for (auto& row : results) 
        {
            users.emplace_back(row["login"], row["password_hash"], row["nickname"]);
        }
        return users;
    }

    int DatabaseManager::getUserIDByLogin(const std::string& login) 
    {
        std::lock_guard<std::mutex> lock(m_db_mutex);

        std::string query = "SELECT id FROM users WHERE login = ?;";
        if (!prepareStatement(query)) 
        {
            return -1;
        }

        SQLLEN login_len = login.length();
        if (!bindParameter(1, login, &login_len)) 
        {
            return -1;
        }

        if (!executePreparedQuery()) 
        {
            return -1;
        }

        auto results = fetchResults();
        if (!results.empty()) 
        {
            return std::stoi(results[0]["id"]);
        }
        return -1;
    }

    bool DatabaseManager::addMessage(const Message& msg) 
    {
        std::lock_guard<std::mutex> lock(m_db_mutex);

        int fromId = getUserIDByLogin(msg.GetFrom());
        if (fromId == -1) return false;

        int toId = -1;
        if (msg.IsPrivate()) 
        {
            toId = getUserIDByLogin(msg.GetTo());
            if (toId == -1) return false;
        }

        std::string query;
        if (msg.IsPrivate()) 
        {
            query = "INSERT INTO messages (from_user_id, to_user_id, message_text) VALUES (?, ?, ?);";
        }
        else 
        {
            query = "INSERT INTO messages (from_user_id, message_text) VALUES (?, ?);";
        }

        if (!prepareStatement(query)) 
        {
            return false;
        }

        SQLLEN fromId_len = SQL_NTS;
        SQLLEN toId_len = (toId == -1) ? SQL_NULL_DATA : SQL_NTS;
        SQLLEN text_len = msg.GetText().length();

        if (!bindParameter(1, fromId, &fromId_len) ||
            (!msg.IsPrivate() && !bindParameter(2, msg.GetText(), &text_len)) || 
            (msg.IsPrivate() && (!bindParameter(2, toId, &toId_len) || !bindParameter(3, msg.GetText(), &text_len)))) 
        {
            return false;
        }

        return executePreparedQuery();
    }

    std::vector<Message> DatabaseManager::getMessagesForUser(const std::string& username) 
    {
        std::lock_guard<std::mutex> lock(m_db_mutex);
        int userId = getUserIDByLogin(username);
        if (userId == -1) return {};

        std::string query = R"(
        SELECT u_from.login as from_login, u_to.login as to_login, m.message_text, m.timestamp
        FROM messages m
        JOIN users u_from ON m.from_user_id = u_from.id
        LEFT JOIN users u_to ON m.to_user_id = u_to.id
        WHERE m.to_user_id IS NULL OR m.to_user_id = ? OR m.from_user_id = ?;
    )";
        if (!prepareStatement(query)) 
        {
            return {};
        }

        SQLLEN userId_len = SQL_NTS;
        if (!bindParameter(1, userId, &userId_len) || !bindParameter(2, userId, &userId_len)) 
        {
            return {};
        }

        if (!executePreparedQuery()) 
        {
            return {}; 
        }

        auto results = fetchResults();
        std::vector<Message> messages;
        for (auto& row : results) 
        {
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

    bool DatabaseManager::prepareStatement(const std::string& query) 
    {
        SQLCloseCursor(m_stmt);

        SQLRETURN ret = SQLPrepareA(m_stmt, (SQLCHAR*)query.c_str(), SQL_NTS);
        if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) 
        {
            return false;
        }
        return true;
    }

    bool DatabaseManager::bindParameter(int param_index, const std::string& value, SQLLEN* indicator) 
    {
        SQLRETURN ret = SQLBindParameter(m_stmt, param_index, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, value.length(), 0, (SQLPOINTER)value.c_str(), value.length(), indicator);
        if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) 
        {
            return false;
        }
        return true;
    }

    bool DatabaseManager::bindParameter(int param_index, int value, SQLLEN* indicator) 
    {
        SQLRETURN ret = SQLBindParameter(m_stmt, param_index, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, (SQLPOINTER)(intptr_t)value, 0, indicator);
        if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) 
        {
            return false;
        }
        return true;
    }

    bool DatabaseManager::executePreparedQuery() 
    {
        SQLRETURN ret = SQLExecute(m_stmt);
        if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) 
        {
            SQLCHAR sqlState[6];
            SQLINTEGER nativeError;
            SQLCHAR messageText[256];
            SQLSMALLINT textLength;
            SQLGetDiagRecA(SQL_HANDLE_STMT, m_stmt, 1, sqlState, &nativeError, messageText, sizeof(messageText), &textLength);
            return false;
        }
        return true;
    }

    std::vector<std::map<std::string, std::string>> DatabaseManager::fetchResults() 
    {
        std::vector<std::map<std::string, std::string>> results;
        SQLSMALLINT colCount = 0;
        SQLRETURN ret = SQLNumResultCols(m_stmt, &colCount);
        if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) 
        {
            return results;
        }

        std::vector<std::string> colNames(colCount);
        for (int i = 0; i < colCount; ++i) 
        {
            SQLCHAR name[256];
            SQLSMALLINT nameLength = 0;
            SQLRETURN descRet = SQLDescribeColA(m_stmt, i + 1, name, sizeof(name), &nameLength, nullptr, nullptr, nullptr, nullptr); 
            if (descRet == SQL_SUCCESS || descRet == SQL_SUCCESS_WITH_INFO) 
            {
                colNames[i] = std::string((char*)name, nameLength); 
            }
            else 
            {
                colNames[i] = "unknown_col_" + std::to_string(i);
            }
        }

        std::vector<SQLCHAR> buffer(1024);
        std::vector<SQLLEN> indicators(colCount);

        while (SQLFetch(m_stmt) != SQL_NO_DATA) 
        {
            std::map<std::string, std::string> row;
            for (int i = 0; i < colCount; ++i) 
            {
                SQLRETURN getDataRet = SQLGetData(m_stmt, i + 1, SQL_CHAR, buffer.data(), buffer.size(), &indicators[i]);
                if (getDataRet == SQL_SUCCESS || getDataRet == SQL_SUCCESS_WITH_INFO) 
                {
                    if (indicators[i] != SQL_NULL_DATA) 
                    {
                        row[colNames[i]] = std::string((char*)buffer.data());
                    }
                    else 
                    {
                        row[colNames[i]] = "NULL";
                    }
                }
                else 
                {
                    row[colNames[i]] = "ERROR_FETCHING";
                }
            }
            results.push_back(row);
        }
        return results;
    }

}