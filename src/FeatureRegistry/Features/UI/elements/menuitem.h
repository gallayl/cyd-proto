#pragma once

#include <functional>
#include <vector>

namespace UI
{

struct MenuItem
{
    const char *label;
    std::vector<MenuItem> children;
    std::function<void()> action;
    bool separator{false};

    bool hasSubmenu() const
    {
        return !children.empty();
    }
    bool isLeaf() const
    {
        return !hasSubmenu() && action != nullptr;
    }
    bool isSeparator() const
    {
        return separator;
    }

    static MenuItem Leaf(const char *label, std::function<void()> action)
    {
        return MenuItem{label, {}, std::move(action), false};
    }

    static MenuItem Submenu(const char *label, std::vector<MenuItem> children)
    {
        return MenuItem{label, std::move(children), nullptr, false};
    }

    static MenuItem Separator()
    {
        return MenuItem{"", {}, nullptr, true};
    }
};

} // namespace UI
