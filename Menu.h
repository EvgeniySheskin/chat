#pragma once
#include "MenuItem.h"
#include <vector>

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
	void Execute(T code)
	{
		for (int i = 0, j = m_Items.size() - 1; i < j; ++i, --j)
		{
			if (m_Items[i].GetCode() == code)
			{
				m_Items[i].Execute();
				return;
			}
			if (m_Items[j].GetCode() == code)
			{
				m_Items[j].Execute();
				return;
			}
		}
	}
	friend ostream& operator<<(ostream& os, Menu<int>& menu)
	{
		auto items = menu.GetItems();
		for (int i = 0; i < items.size(); ++i)
			os << items[i] << endl;
		return os;
	}
};