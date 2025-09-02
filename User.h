#pragma once
#include <string>
using namespace std;
class User
{
public:
	User() = default;
	User(string login, string password, string nickname);
	User (User&& other) noexcept;
	User& operator=(User&& other) noexcept;

	string GetNickname();
	void CheckUser(string login);
	bool CompareLogin(string login);
	~User() = default;
	string GetLogin();
	friend ostream& operator<<(ostream& os, User& user);

private:
	string m_login;
	string m_password;
	string m_nickname;
};