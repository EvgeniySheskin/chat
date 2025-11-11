#include "Message.h"
#include <sstream>

namespace chat
{
    Message::Message(std::string from, std::string to, std::string text)
        : m_from(std::move(from)), m_to(std::move(to)), m_text(std::move(text)), m_timestamp(std::time(nullptr)) {}

    bool Message::IsForUser(const std::string& user) const
    {
        return !IsPrivate() || m_from == user || m_to == user;
    }

    std::string Message::serialize() const
    {
        std::ostringstream oss;
        oss << m_from << "|" << m_to << "|" << m_timestamp << "|" << m_text;
        return oss.str();
    }
}