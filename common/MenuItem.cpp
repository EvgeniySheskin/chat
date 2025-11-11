#include "MenuItem.h"
#include <stdexcept>

namespace chat 
{
    MenuItem::MenuItem(std::string text, std::function<void()> action)
        : m_text(std::move(text))
        , m_action(std::move(action))
    {
        if (!m_action) { throw std::invalid_argument("MenuItem action must not be null"); }
    }

    const std::string& MenuItem::getText() const noexcept { return m_text; }

    void MenuItem::execute() const { m_action(); }
}