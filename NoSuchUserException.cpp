#include "NoSuchUserException.h"

NoSuchUserException::NoSuchUserException(string login)
{
	m_msg = "There is no such registered user: " + login + "\n\n";
}


