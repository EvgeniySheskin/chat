#pragma once
#include "User.h"
#include <vector>
using namespace std;
// этот класс:
// 1. хранит всех юзеров 
// 2. создает новых
// 3. проверяет данные на корректность

static class UserManager
{
private:
	static vector<User> m_registeredUsers;
public:
	static bool CheckPassword(string pass);
	static bool CheckLogin(string login);
	static User* m_activeUser;
	static bool AddNewUser(string login, string password, string nickname);
	static void DeleteUser(User* user);
	static User* FindUserByLogin(string login);

	static void Initialize();
	User* operator[](const int index);
};