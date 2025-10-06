#include "NewUserException.h"

NewUserException::NewUserException(string login)
{
	m_msg = "Пользователь " + login + " уже существует!\n\n";
}
