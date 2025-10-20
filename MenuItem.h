#pragma once
#include <string>
#include <functional>

namespace chat {

    class MenuItem {
    public:
        MenuItem(std::string text, std::function<void()> action);

        // Запрет копирования
        MenuItem(const MenuItem&) = delete;
        MenuItem& operator=(const MenuItem&) = delete;

        // Разрешение перемещения (автоматически корректно)
        MenuItem(MenuItem&&) noexcept = default;
        MenuItem& operator=(MenuItem&&) noexcept = default;

        const std::string& getText() const noexcept;
        void execute() const;

    private:
        std::string m_text;
        std::function<void()> m_action;
    };
}