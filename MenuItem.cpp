#include "MenuItem.h"

namespace chat
{
	MenuItem::MenuItem(const std::string& text, std::function<void()> action)
		: m_Text(text), m_Action(std::move(action)) {
	}

	const std::string& MenuItem::getText() const {
		return m_Text;
	}

	void MenuItem::execute() const {
		if (m_Action) {
			m_Action();
		}
	}
}