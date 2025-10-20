#include "Message.h"
#include <ctime>

namespace chat 
{

    Message::Message(std::string from, std::string to, std::string text)
        : m_from(std::move(from))
        , m_to(std::move(to))
        , m_text(std::move(text))
        , m_timestamp(std::time(nullptr))
    {}

    bool Message::IsForUser(const std::string& user) const 
    {
        return !IsPrivate() || m_to == user;
    }

}