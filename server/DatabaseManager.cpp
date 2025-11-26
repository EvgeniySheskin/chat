#include "DatabaseManager.h"
#include <iostream>
#include <cstring>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cctype>

namespace chat
{

    DatabaseManager::DatabaseManager() : m_logger(std::make_unique<Logger>("db.log"))
    {
        m_logger->start();
        m_logger->info("DatabaseManager starting...");
    }

    DatabaseManager::~DatabaseManager()
    {
        m_logger->info("DatabaseManager shutting down...");
        m_logger->stop();
        std::lock_guard<std::recursive_mutex> lock(m_db_mutex);
        if (m_stmt) SQLFreeHandle(SQL_HANDLE_STMT, m_stmt);
        if (m_conn) {
            SQLDisconnect(m_conn);
            SQLFreeHandle(SQL_HANDLE_DBC, m_conn);
        }
        if (m_env) SQLFreeHandle(SQL_HANDLE_ENV, m_env);
    }

    bool DatabaseManager::connect(const std::string& connStr)
    {
        std::lock_guard<std::recursive_mutex> lock(m_db_mutex);
        if (SQL_SUCCESS != SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &m_env))
            return false;

        if (SQL_SUCCESS != SQLSetEnvAttr(m_env, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0))
            return false;

        if (SQL_SUCCESS != SQLAllocHandle(SQL_HANDLE_DBC, m_env, &m_conn))
            return false;

        // ANSI- SQLDriverConnectA
        if (SQL_SUCCESS != SQLDriverConnectA(m_conn, NULL, (SQLCHAR*)connStr.c_str(), SQL_NTS, NULL, 0, NULL, SQL_DRIVER_COMPLETE))
            return false;

        if (SQL_SUCCESS != SQLAllocHandle(SQL_HANDLE_STMT, m_conn, &m_stmt))
            return false;

        m_logger->info("Database connected successfully.");
        return true;
    }

    void DatabaseManager::initDatabase()
    {
        std::lock_guard<std::recursive_mutex> lock(m_db_mutex);
        m_logger->info("Initializing database...");
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
        m_logger->info("Database initialized.");
    }

    std::string DatabaseManager::hashPassword(const std::string& password)
    {
        return sha1::hash(password);
    }

    bool DatabaseManager::addUser(const std::string& login, const std::string& password_hash, const std::string& nickname)
    {
        std::lock_guard<std::recursive_mutex> lock(m_db_mutex);

        std::string query = "INSERT INTO users (login, password_hash, nickname) VALUES ('" + login + "','" + password_hash + "','" + nickname + "');";
        bool result = executeQueryDirect(query);
        if (result) 
        {
            m_logger->info("Added user: " + login);
        }
        else 
        {
            m_logger->error("Failed to add user: " + login);
        }
        return result;
    }

    std::optional<User> DatabaseManager::findUserByLogin(const std::string& login)
    {
        std::lock_guard<std::recursive_mutex> lock(m_db_mutex);

        std::string query = "SELECT login, password_hash, nickname FROM users WHERE login = '" + login + "';";
        if (!executeQueryDirect(query)) 
        {
            m_logger->error("Failed to execute query for findUserByLogin: " + login);
            return std::nullopt;
        }
        auto results = fetchResults();
        if (!results.empty()) 
        {
            auto& row = results[0];
            m_logger->debug("Found user: " + login);
            return User(row["login"], row["password_hash"], row["nickname"]);
        }
        m_logger->warning("User not found: " + login);
        return std::nullopt;
    }

    std::vector<User> DatabaseManager::getAllUsers()
    {
        std::lock_guard<std::recursive_mutex> lock(m_db_mutex);
        std::vector<User> users;

        std::string query = "SELECT login, password_hash, nickname FROM users;";
        if (!executeQueryDirect(query)) 
        {
            m_logger->error("Failed to execute query for getAllUsers.");
            return users; 
        }
        auto results = fetchResults();
        for (auto& row : results) 
        {
            users.emplace_back(row["login"], row["password_hash"], row["nickname"]);
        }
        m_logger->debug("Retrieved " + std::to_string(users.size()) + " users.");
        return users;
    }

    int DatabaseManager::getUserIDByLogin(const std::string& login)
    {
        std::lock_guard<std::recursive_mutex> lock(m_db_mutex);

        std::string query = "SELECT id FROM users WHERE login = '" + login + "';";
        if (!executeQueryDirect(query)) 
        {
            m_logger->error("Failed to execute query for getUserIDByLogin: " + login);
            return -1;
        }
        auto results = fetchResults();
        if (!results.empty()) 
        {
            int id = std::stoi(results[0]["id"]);
            m_logger->debug("Found user ID for login " + login + ": " + std::to_string(id));
            return id;
        }
        m_logger->warning("User ID not found for login: " + login);
        return -1;
    }

    bool DatabaseManager::addMessage(const Message& msg)
    {
        std::lock_guard<std::recursive_mutex> lock(m_db_mutex);

        int fromId = getUserIDByLogin(msg.GetFrom());
        if (fromId == -1)
        {
            m_logger->error("Cannot add message: sender user not found - " + msg.GetFrom());
            return false;
        }

        int toId = -1;
        if (msg.IsPrivate())
        {
            toId = getUserIDByLogin(msg.GetTo());
            if (toId == -1)
            {
                m_logger->error("Cannot add message: recipient user not found - " + msg.GetTo());
                return false;
            }
        }

        std::string toStr = (toId == -1) ? "NULL" : std::to_string(toId);
        std::string text = msg.GetText(); 
        std::string query = "INSERT INTO messages (from_user_id, to_user_id, message_text) VALUES (" + std::to_string(fromId) + ", " + toStr + ", '" + text + "');";
        bool result = executeQueryDirect(query);
        if (result)
        {
            m_logger->debug("Added message from " + msg.GetFrom() + " to " + (msg.IsPrivate() ? msg.GetTo() : "broadcast"));
        }
        else
        {
            m_logger->error("Failed to add message from " + msg.GetFrom());
        }
        return result;
    }

    std::vector<Message> DatabaseManager::getMessagesForUser(const std::string& username)
    {
        std::lock_guard<std::recursive_mutex> lock(m_db_mutex);
        int userId = getUserIDByLogin(username);
        if (userId == -1)
        {
            m_logger->error("Cannot get messages: user not found - " + username);
            return {};
        }

        std::string query = R"(
        SELECT u_from.login as from_login, u_to.login as to_login, m.message_text, m.timestamp
        FROM messages m
        JOIN users u_from ON m.from_user_id = u_from.id
        LEFT JOIN users u_to ON m.to_user_id = u_to.id
        WHERE m.to_user_id IS NULL OR m.to_user_id = )" + std::to_string(userId) + R"( OR m.from_user_id = )" + std::to_string(userId) + R"(;
    )";

        if (!executeQueryDirect(query)) {
            m_logger->error("Failed to execute query for getMessagesForUser: " + username);
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
        m_logger->debug("Retrieved " + std::to_string(messages.size()) + " messages for user: " + username);
        return messages;
    }

    bool DatabaseManager::executeQueryDirect(const std::string& query)
    {
        SQLRETURN close_ret = SQLCloseCursor(m_stmt);
        //if (close_ret != SQL_SUCCESS && close_ret != SQL_SUCCESS_WITH_INFO) {
            // m_logger->debug("SQLCloseCursor before executeQueryDirect failed (may be OK).");
        //}

        SQLRETURN ret = SQLExecDirectA(m_stmt, (SQLCHAR*)query.c_str(), SQL_NTS);
        if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
        {
            SQLCHAR sqlState[6];
            SQLINTEGER nativeError;
            SQLCHAR messageText[256];
            SQLSMALLINT textLength;
            SQLGetDiagRecA(SQL_HANDLE_STMT, m_stmt, 1, sqlState, &nativeError, messageText, sizeof(messageText), &textLength);
            m_logger->error("ODBC Error (ExecDirect): " + std::string((char*)messageText) + " (SQLState: " + std::string((char*)sqlState) + ", Native: " + std::to_string(nativeError) + ")");
            m_logger->error("Query was: " + query);
            return false;
        }
        m_logger->debug("SQLExecDirectA succeeded for query.");
        return true;
    }

    std::vector<std::map<std::string, std::string>> DatabaseManager::fetchResults()
    {
        std::vector<std::map<std::string, std::string>> results;
        SQLSMALLINT colCount = 0;
        SQLRETURN ret = SQLNumResultCols(m_stmt, &colCount);
        if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
        {
            m_logger->error("SQLNumResultCols failed in fetchResults.");
            // SQLCloseCursor(m_stmt); // Закрываем курсор в случае ошибки
            return results; // Возвращаем пустой результат
        }

        std::vector<std::string> colNames(colCount);
        for (int i = 0; i < colCount; ++i)
        {
            SQLCHAR name[256];
            SQLSMALLINT nameLength = 0;
            SQLRETURN descRet = SQLDescribeColA(m_stmt, i + 1, name, sizeof(name), &nameLength, nullptr, nullptr, nullptr, nullptr); // ANSI
            if (descRet == SQL_SUCCESS || descRet == SQL_SUCCESS_WITH_INFO)
            {
                colNames[i] = std::string((char*)name, nameLength); 
            }
            else
            {
                colNames[i] = "unknown_col_" + std::to_string(i);
                m_logger->warning("SQLDescribeColA failed for column " + std::to_string(i + 1) + ".");
            }
        }

        std::vector<SQLCHAR> buffer(1024);
        std::vector<SQLLEN> indicators(colCount);

        int row_num = 0;
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
                    m_logger->error("SQLGetData failed for column " + colNames[i] + " (index " + std::to_string(i + 1) + ").");
                }
            }
            results.push_back(row);
            row_num++;
        }
        // m_logger->debug("fetchResults: Fetched " + std::to_string(row_num) + " rows.");

        SQLRETURN closeRet = SQLCloseCursor(m_stmt);
        if (closeRet != SQL_SUCCESS && closeRet != SQL_SUCCESS_WITH_INFO) {
            m_logger->warning("SQLCloseCursor after fetchResults failed (may be OK).");
        }
        return results;
    }

}