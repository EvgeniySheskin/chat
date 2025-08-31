#pragma once

template <typename T>
class IMessage
{
public:
	virtual T& Get() = 0;
};