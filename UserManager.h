#pragma once
#include "User.h"
#include <vector>
using namespace std;
// этот класс:
// 1. хранит всех юзеров 
// 2. создает новых
// 3. проверяет данные на корректность

class UserManager
{
private:
	vector<User> m_registeredUsers;
public:
	User* m_activeUser;

	bool CheckPassword(string pass);
	bool CheckLogin(string login);
	void AddNewUser(string login, string password, string nickname);
	void DeleteUser(string login);
	User* FindUserByLogin(string login);
	void Initialize();
	User* operator[](const int index);
};

