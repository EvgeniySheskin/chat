#pragma once
#include <string>
using namespace std;
class User
{
public:
	User() = default;
	User(string login, string password, string nickname);
	bool Authorize(string pass);
	~User() = default;
private:
	bool _ComparePassword(string pass);
	string m_login;
	string m_password;
	string m_nickname;
};