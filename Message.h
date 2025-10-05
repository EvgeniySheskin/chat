#pragma once
#include <string>
#include <ctime>

namespace chat
{
    class Message
    {
    public:
        Message(const std::string& from, const std::string& to, const std::string& text);

        const std::string& GetFrom() const { return m_from; }
        const std::string& GetTo() const { return m_to; }
        const std::string& GetText() const { return m_text; }
        std::time_t GetTimestamp() const { return m_timestamp; }

        bool IsPrivate() const { return !m_to.empty(); }
        bool IsForUser(const std::string& user) const;

    private:
        std::string m_from;
        std::string m_to;       // пусто = общее сообщение
        std::string m_text;
        std::time_t m_timestamp;
    };
}