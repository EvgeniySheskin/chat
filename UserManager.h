#pragma once
#include "User.h"
#include <vector>
using namespace std;
// ���� �����:
// 1. ������ ���� ������ 
// 2. ������� �����
// 3. ��������� ������ �� ������������

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