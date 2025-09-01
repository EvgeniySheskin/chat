#include "UserException.h"

const char* UserException::what() const noexcept
{
    return m_msg.c_str();
}
