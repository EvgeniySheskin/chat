#include "NoSuchUserException.h"

NoSuchUserException::NoSuchUserException(string login)
{
	m_msg = "Пользователя с таким логином не существует: " + login + "\n\n";
}


