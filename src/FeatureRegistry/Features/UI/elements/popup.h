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
        int bx, by, bw, bh;
        getBounds(bx, by, bw, bh);

        // Win95 raised 3D menu panel
        c.fillRect(bx, by, bw, bh, Theme::MenuBg);
        c.drawRect(bx, by, bw, bh, Theme::WindowBorder);
        c.drawFastHLine(bx + 1, by + 1, bw - 2, Theme::ButtonHighlight);
        c.drawFastVLine(bx + 1, by + 1, bh - 2, Theme::ButtonHighlight);
        c.drawFastHLine(bx + 1, by + bh - 2, bw - 2, Theme::ButtonShadow);
        c.drawFastVLine(bx + bw - 2, by + 1, bh - 2, Theme::ButtonShadow);

        Container::draw();
    }

private:
    bool _visible{false};
};

} // namespace UI
