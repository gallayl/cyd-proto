#pragma once

#include <string>
#include "container.h"
#include "../Renderer.h"
#include "../BuiltinIcons.h"

namespace UI
{

class IconElement : public Element
{
public:
    IconElement(const char *iconName, int ix, int iy, int isize) : _iconName(iconName), _size(isize)
    {
        setBounds(ix, iy, isize, isize);
    }

    void setIconName(const char *name)
    {
        _iconName = name;
    }
    const std::string &getIconName() const
    {
        return _iconName;
    }

    void draw() override
    {
        if (!mounted)
            return;
        auto &c = canvas();
        drawBuiltinIcon(c, _iconName.c_str(), drawX(), drawY(), _size);
    }

private:
    std::string _iconName;
    int _size;
};

} // namespace UI
