#pragma once
#include "MenuItem.h"
#include <vector>
#include <memory>
#include <functional>
#include <string>

namespace chat 
{

    class ConsoleMenu 
    {
    public:
        ConsoleMenu() = default;
        ~ConsoleMenu() = default;

        // Запрет копирования
        ConsoleMenu(const ConsoleMenu&) = delete;
        ConsoleMenu& operator=(const ConsoleMenu&) = delete;

        // Разрешение перемещения
        ConsoleMenu(ConsoleMenu&&) noexcept = default;
        ConsoleMenu& operator=(ConsoleMenu&&) noexcept = default;

        // Добавление пункта (цепочка для fluent interface)
        ConsoleMenu& addItem(std::string text, std::function<void()> action);

        // Запуск меню
        void run();

    private:
        std::vector<std::unique_ptr<MenuItem>> m_items;
    };

}