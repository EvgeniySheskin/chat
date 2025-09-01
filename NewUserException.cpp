#include "NewUserException.h"

NewUserException::NewUserException(string login)
{
	m_msg = "User " + login + " already exists!\n\n";
}
