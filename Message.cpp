#include "Message.h"
#include <ctime>

namespace chat
{
    Message::Message(const std::string& from, const std::string& to, const std::string& text)
        : m_from(from), m_to(to), m_text(text), m_timestamp(std::time(nullptr)) {}

    bool Message::IsForUser(const std::string& user) const 
    {
        return (!IsPrivate()) || (m_to == user);
    }
}