#include "Protocol.h"
#include <sstream>
#include <stdexcept>



namespace chat 
{
    const char DELIM = 0;
    std::string NetworkMessage::serialize() const 
    {
        std::ostringstream oss;
        oss << static_cast<int>(type) << "|"
            << sender << "|"
            << recipient << "|"
            << payload;
        return oss.str();
    }

    NetworkMessage NetworkMessage::deserialize(const std::string& data) 
    {
        NetworkMessage msg;
        std::istringstream iss(data);
        std::string typeStr;
        //iss.ignore(1000);
        if (std::getline(iss, typeStr, '|'))
        {
            msg.type = static_cast<MessageType>(std::stoi(typeStr));
        }
        if (std::getline(iss, msg.sender, '|')) {}
        if (std::getline(iss, msg.recipient, '|')) {}
        if (std::getline(iss, msg.payload, DELIM)) {}

        return msg;
    }

}