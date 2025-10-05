#pragma once
#include <iostream>
#include <vector>
#include <memory>
#include <functional>
#include <string>
#include "MenuItem.h"

namespace chat
{
	class ConsoleMenu
	{
	private:
		std::vector<std::unique_ptr<MenuItem>> m_Items;
	public:
		ConsoleMenu();
		~ConsoleMenu();
		ConsoleMenu* addItem(const std::string& text, std::function<void()> action);
		// Запрет копирования
		ConsoleMenu(const ConsoleMenu&) = delete;
		ConsoleMenu& operator=(const ConsoleMenu&) = delete;

		// Разрешить перемещение (опционально)
		ConsoleMenu(ConsoleMenu&&) = default;
		ConsoleMenu& operator=(ConsoleMenu&&) = default;

		void run();
		friend std::ostream& operator<<(std::ostream& os, ConsoleMenu& menu);
		const std::vector<std::unique_ptr<MenuItem>>& GetItems() const;
	};
}
