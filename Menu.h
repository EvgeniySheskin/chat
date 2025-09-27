#pragma once
#include "MenuItem.h"
#include <vector>

namespace chat
{
	template <typename T = int>
	class Menu
	{
	private:
		vector<MenuItem<T>> m_Items;
	public:
		Menu() = default;
		Menu* AddItem(MenuItem<T>* newItem, int pos)
		{
			m_Items.insert(m_Items.begin() + pos, *newItem);
			return this;
		}
		Menu* DeleteItem(int pos)
		{
			m_Items.erase(m_Items.begin() + pos);
			return this;
		}
		vector<MenuItem<T>> GetItems()
		{
			return m_Items;
		}
		bool Execute(T code)
		{
			for (int i = 0, j = m_Items.size() - 1; i < j; ++i, --j)
			{
				if (m_Items[i].GetCode() == code)
				{
					m_Items[i].Execute();
					return true;
				}
				if (m_Items[j].GetCode() == code)
				{
					m_Items[j].Execute();
					return true;
				}
			}
			return false;
		}
		friend ostream& operator<<(ostream& os, Menu<T>& menu)
		{
			auto items = menu.GetItems();
			for (int i = 0; i < items.size(); ++i)
				os << items[i] << endl;
			return os;
		}
	};
}
