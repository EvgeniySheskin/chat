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

	friend ostream& operator<<(ostream& os, Menu<int>& menu)
	{
		auto items = menu.GetItems();
		for (int i = 0; i < items.size(); ++i)
			os << items[i] << endl;
		return os;
	}
};