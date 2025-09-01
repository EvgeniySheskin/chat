#pragma once
#include "UserException.h"
using namespace std;

class NewUserException : public UserException
{
public:
	NewUserException(string login);
};