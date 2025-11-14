#include "Message.h"
#include <ctime>

namespace chat
{
    Message::Message(std::string from, std::string to, std::string text)
        : m_from(std::move(from)), m_to(std::move(to)), m_text(std::move(text)), m_timestamp(std::time(nullptr)) {}

    Message::Message(std::string from, std::string to, std::string text, std::time_t timestamp)
        : m_from(std::move(from)), m_to(std::move(to)), m_text(std::move(text)), m_timestamp(timestamp) {}

    bool Message::IsForUser(const std::string& user) const 
    {
        return m_to.empty() || m_to == user || m_from == user;
    }

    std::string Message::serialize() const 
    {
        return m_from + "|" + m_to + "|" + m_text + "|" + std::to_string(m_timestamp);
    }
}