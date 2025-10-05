#pragma once
#include <iostream>
#include <string>
#include <functional>

namespace chat
{
    class MenuItem
    {
    private:
        std::string m_Text;
        std::function<void()> m_Action;

    public:
        MenuItem(const std::string& text, std::function<void()> action);
        
        // Запрет копирования
        MenuItem(const MenuItem&) = delete;
        MenuItem& operator=(const MenuItem&) = delete;

        // Разрешение перемещения
        MenuItem(MenuItem&&) noexcept = default;
        MenuItem& operator=(MenuItem&&) noexcept = default;

        const std::string& getText() const;
        void execute() const;
        ~MenuItem() = default;
    };
}
