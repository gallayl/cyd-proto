#pragma once

#include "container.h"
#include "../Renderer.h"
#include "../Theme.h"

namespace UI
{

class PopupContainer : public Container
{
public:
    PopupContainer(int ix, int iy, int iw, int ih)
    {
        setBounds(ix, iy, iw, ih);
    }

    bool isVisible() const { return _visible; }

    void show()
    {
        _visible = true;
        mount();
    }

    void hide()
    {
        _visible = false;
        unmount();
    }

    void draw() override
    {
        if (!_visible)
            return;

        auto &c = canvas();

        // Win95 raised 3D menu panel
        c.fillRect(drawX(), drawY(), width, height, Theme::MenuBg);
        c.drawRect(drawX(), drawY(), width, height, Theme::WindowBorder);
        c.drawFastHLine(drawX() + 1, drawY() + 1, width - 2, Theme::ButtonHighlight);
        c.drawFastVLine(drawX() + 1, drawY() + 1, height - 2, Theme::ButtonHighlight);
        c.drawFastHLine(drawX() + 1, drawY() + height - 2, width - 2, Theme::ButtonShadow);
        c.drawFastVLine(drawX() + width - 2, drawY() + 1, height - 2, Theme::ButtonShadow);

        Container::draw();
    }

private:
    bool _visible{false};
};

} // namespace UI
