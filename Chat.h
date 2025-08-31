#pragma once
#include "IMessage.h"
#include <string>
#include "User.h"

using namespace std;

class Chat
{
private:
	IMessage<string>* m_messages;
	User* m_users;
};