#include "UserManager.h"
#include <iostream>
#include "NewUserException.h"
#include "NoSuchUserException.h"
#include <regex>

bool UserManager::CheckPassword(string pass) throw()
{
	if (pass.empty())
	{
		throw "Empty password is not allowed\n\n";
	}
	if (pass.size() < 6)
	{
		throw "Password cannot be less than 6 symbols long\n\n";
	}
	if (!regex_search(pass, regex("([$&*!]|[\d])+")))
	{
		throw "Password must contain special symbols: $, &, *, !\n\n";
	}
	return true;
}

bool UserManager::CheckLogin(string login)
{
	for (int i = 0; i < m_registeredUsers.size(); i++)
		m_registeredUsers[i].CheckUser(login);
	return true;
}


bool UserManager::AddNewUser(string login, string password, string nickname) throw()
{
	User* newUser = new User(login, password, nickname);
	m_registeredUsers.push_back(*newUser);
	m_activeUser = move(newUser);
	cout << "Welcome to chat, " << newUser->GetNickname() << "!\n\n";
}

void UserManager::DeleteUser(User* user)
{
	m_registeredUsers.erase(find(m_registeredUsers.begin(), m_registeredUsers.end(), *user));
}

User* UserManager::FindUserByLogin(string login) throw(NoSuchUserException)
{
	for (int i = 0; i < m_registeredUsers.size(); i++)
	{
		if (m_registeredUsers[i].CompareLogin(login))
		{
			return &m_registeredUsers[i];
		}
	}
	throw NoSuchUserException(login);
}

void UserManager::Initialize()
{
	m_activeUser = nullptr;
}

User* UserManager::operator[](const int index)
{
	return &m_registeredUsers[index];
}
