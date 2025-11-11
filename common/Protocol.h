#pragma once
#include <string>
#include <cstdint>
#include <sstream>

namespace chat
{
    
    enum class MessageType : uint8_t
    {
        REGISTER,
        LOGIN,
        LOGOUT,
        SEND_MESSAGE,
        STATUS_RESPONSE,
        GET_USERS,
        USER_LIST,
        GET_HISTORY,
        MESSAGE_HISTORY,
        INVALID
    };

    struct NetworkMessage
    {
        MessageType type;
        std::string sender;
        std::string recipient;
        std::string payload;

        std::string serialize() const;

        static NetworkMessage deserialize(const std::string& data);
    };
}