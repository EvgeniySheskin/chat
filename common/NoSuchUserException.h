#pragma once
#include "UserException.h"
using namespace std;

class NoSuchUserException : public UserException
{
public:
	NoSuchUserException(string login);
};