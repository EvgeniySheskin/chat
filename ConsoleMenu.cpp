#include "ConsoleMenu.h"
#include "MenuItem.h"
#include <iostream>
#include <limits>
#include <algorithm>
#include <cctype>

namespace chat 
{

    ConsoleMenu& ConsoleMenu::addItem(std::string text, std::function<void()> action) 
    {
        m_items.push_back(std::make_unique<MenuItem>(std::move(text), std::move(action)));
        return *this;
    }

    void ConsoleMenu::run() 
    {
        if (m_items.empty()) 
        {
            std::cout << "Меню пустое.\n";
            return;
        }

        int choice;
        do {
            // Вывод меню с использованием алгоритма
            for (size_t i = 0; i < m_items.size(); ++i) 
            {
                std::cout << (i + 1) << ". " << m_items[i]->getText() << '\n';
            }
            //std::cout << "0. Выход\n";

            std::cout << "Выберите пункт: ";
            if (!(std::cin >> choice)) 
            {
                std::cin.clear();
                std::cin.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');
                std::cout << "Ошибка: введите целое число.\n\n";
                continue;
            }

            if (choice == 0) 
            {
                break;
            }
            else if (choice > 0 && choice <= static_cast<int>(m_items.size())) 
            {
                m_items[choice - 1]->execute();
                std::cout << '\n';
            }
            else 
            {
                std::cout << "Неверный выбор. Попробуйте снова.\n\n";
            }
        } while (true);
    }

}