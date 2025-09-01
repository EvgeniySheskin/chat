#pragma once
#include <exception>
#include <string>
using namespace std;

class UserException : public exception
{
protected:
	string m_msg;
public:
	const char* what() const noexcept override;
};