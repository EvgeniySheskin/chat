#pragma once
#include <string>
#include <functional>
using namespace std;

template <typename T = int, typename V = UserManager>
class MenuItem
{
private:
	string m_ItemText;
	T m_Code;
	function<void()> m_Exec;
public:
	MenuItem(string text, T code, function<void()> exec) : m_ItemText(text), m_Code(code), m_Exec(exec) {}
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
		m_Exec();
	}
	friend ostream& operator<<(ostream& os, MenuItem<T, V>& item)
	{
		string out = to_string(item.GetCode()) + ". " + item.GetText() + "\n";
		os << out;
		return os;
	}
};