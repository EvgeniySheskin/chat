#include "ConsoleMenu.h"

namespace chat
{
	ConsoleMenu::ConsoleMenu() = default;
	ConsoleMenu::~ConsoleMenu() = default;

	ConsoleMenu* ConsoleMenu::AddItem(const std::string& text, std::function<void()> action) 
	{
		m_Items.push_back(std::make_unique<MenuItem>(text, std::move(action)));
		return this;
	}
	const std::vector<std::unique_ptr<MenuItem>>& ConsoleMenu::GetItems() const
	{
		return m_Items;
	}

	std::ostream& operator<<(std::ostream& os, ConsoleMenu& menu)
	{
		const auto& items = menu.GetItems();
		for (size_t i = 0; i < items.size(); ++i) {
			std::cout << (i + 1) << ". " << items[i]->getText() << '\n';
		}
		/*if (!items.empty()) {
			std::cout << "0. Выход\n";
		}*/
		return os;
	}
	void ConsoleMenu::Run() {
		if (m_Items.empty()) {
			std::cout << "Меню пусто.\n";
			return;
		}
		int choice;
		do {
			std::cout << *this;
			std::cout << "Выберите пункт: ";

			// Проверка корректности ввода
			if (!(std::cin >> choice)) {
				std::cin.clear();
				std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
				std::cout << "Ошибка: введите целое число.\n\n";
				continue;
			}

			/*if (choice == 0) {
				break; // Выход
			}*/
			else if (choice >= 1 && choice <= static_cast<int>(m_Items.size())) {
				m_Items[choice - 1]->execute();
				std::cout << '\n';
			}
			else {
				std::cout << "Неверный выбор. Попробуйте снова.\n\n";
			}
		} while (true);
	}
}