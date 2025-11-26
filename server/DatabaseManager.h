#pragma once

#include "../common/User.h"
#include "../common/Message.h"
#include "../common/sha1.h"

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
#include <mutex>

namespace chat
{

    class DatabaseManager
    {
    public:
        DatabaseManager();
        ~DatabaseManager();

        bool connect(const std::string& connStr);
        void initDatabase();

        bool addUser(const std::string& login, const std::string& password_hash, const std::string& nickname);
        std::optional<User> findUserByLogin(const std::string& login);
        std::vector<User> getAllUsers();
        int getUserIDByLogin(const std::string& login);

        bool addMessage(const Message& msg);
        std::vector<Message> getMessagesForUser(const std::string& username);

    private:
        SQLHANDLE m_env{ nullptr };
        SQLHANDLE m_conn{ nullptr };
        SQLHANDLE m_stmt{ nullptr };

        std::mutex m_db_mutex; 

        bool executePreparedQuery();
        std::vector<std::map<std::string, std::string>> fetchResults();

        bool prepareStatement(const std::string& query);
        bool bindParameter(int param_index, const std::string& value, SQLLEN* indicator);
        bool bindParameter(int param_index, int value, SQLLEN* indicator);
    };

}