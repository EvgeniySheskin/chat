#include "User.h"
#include "NewUserException.h"
#include <iostream>

User::User(string login, string password, string nickname)
	: m_login(login), m_password(password), m_nickname(nickname) {}

User::User(User&& other) noexcept
{
	m_login = other.m_login;
	m_password = other.m_password;
	m_nickname = other.m_nickname;
	other.m_login = nullptr;
	other.m_nickname = nullptr;
	other.m_password = nullptr;
}

User& User::operator=(User&& other) noexcept
{
	m_login = other.m_login;
	m_password = other.m_password;
	m_nickname = other.m_nickname;
	other.m_login = nullptr;
	other.m_nickname = nullptr;
	other.m_password = nullptr;
	return *this;
}

string User::GetNickname()
{
	return m_nickname;
}

void User::CheckUser(string login) throw(NewUserException)
{
	if (!CompareLogin(m_login))
	{
		throw NewUserException(m_login);
	}
}

bool User::CompareLogin(string login)
{
	return !((bool)login.compare(m_login));
}

bool User::operator==(User& other)
{
	return other.CompareLogin(m_login);
}

string User::GetLogin()
{
	return m_login;
}

ostream& operator<<(ostream& os, User& user)
{
	os << user.GetLogin() << endl << endl;
	return os;
}
