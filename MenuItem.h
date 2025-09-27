#pragma once
#include <string>
#include <functional>
#include "Menu.h";
using namespace std;

namespace chat
{
	template <typename T = int>
	class MenuItem
	{
	private:
		string m_ItemText;
		T m_Code;
		function<void()> m_Exec;
		Menu<T>* m_NextStepMenu;
	public:
		MenuItem(string text, T code, function<void()> exec, Menu<T>* nextMenu) 
								: m_ItemText(text), m_Code(code), m_Exec(exec), m_NextStepMenu(nextMenu) {}
		T GetCode() const
		{
			return m_Code;
		}
		string GetText() const
		{
			return m_ItemText;
		}
		void ExecuteAndPassControlTo(Menu<T>* destMenu)
		{
			m_Exec();
			destMenu = m_NextStepMenu;
		}
		friend ostream& operator<<(ostream& os, MenuItem<T>& item)
		{
			string out = to_string(item.GetCode()) + ". " + item.GetText() + "\n";
			os << out;
			return os;
		}
	};
}
