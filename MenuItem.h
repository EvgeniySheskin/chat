#pragma once
#include <string>
using namespace std;

template <typename T = int>
class MenuItem
{
private:
	string m_ItemText;
	T m_Code;
	void (*m_Exec)(string login, string ...);
public:
	MenuItem(string text, T code, void (*exec)(string login, string ...)) : m_ItemText(text), m_Code(code), m_Exec(*exec) {}
	T GetCode() const
	{
		return m_Code;
	}
	string GetText() const
	{
		return m_ItemText;
	}
	void Execute()
	{
		(*m_Exec)();
	}
	friend ostream& operator<<(ostream& os, MenuItem<int>& item)
	{
		string out = to_string(item.GetCode()) + ". " + item.GetText() + "\n";
		os << out;
		return os;
	}
};