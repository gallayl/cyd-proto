#pragma once

#include <LovyanGFX.hpp>
#include "label.h"
#include "../ActionQueue.h" // for UI::queueAction

// display object from hardware layer
extern LGFX tft;

namespace UI
{

    // a basic rectangular button with a label. emits a callback when a
    // press is detected (touch down + up within bounds).
    class Button : public Element
    {
    public:
        using Callback = std::function<void()>;

        Button(const String &txt = String(), int ix = 0, int iy = 0, int iw = 0, int ih = 0)
            : label(txt, ix, iy, iw, ih)
        {
            // initial bounds must be propagated to label as well
            setBounds(ix, iy, iw, ih);
            // black text on our default background; store so we can keep
            // the two in sync if either one changes later
            labelFg = TFT_BLACK;
            label.setTextColor(labelFg, bgColor);
        }

        void setCallback(Callback cb) { onClick = std::move(cb); }
        void setLabel(const String &txt) { label.setText(txt); }

        void setBackgroundColor(uint16_t c)
        {
            bgColor = c;
            // update label background too so text doesn't vanish
            label.setTextColor(labelFg, bgColor);
        }
        // set label colour; background defaults to the button's fill
        void setTextColor(uint16_t fg)
        {
            labelFg = fg;
            label.setTextColor(fg, bgColor);
        }
        void setTextColor(uint16_t fg, uint16_t bg)
        {
            labelFg = fg;
            label.setTextColor(fg, bg);
        }
        void setTextSize(uint8_t size) { label.setTextSize(size); }

        void draw() override
        {
            if (!mounted)
                return;

            // colours for the 3‑D look; swap when pressed
            uint16_t light = pressed ? borderDarkColor : borderLightColor;
            uint16_t dark = pressed ? borderLightColor : borderDarkColor;

            // fill body
            tft.fillRect(x, y, width, height, bgColor);

            // top/left light edge
            tft.drawFastHLine(x, y, width, light);
            tft.drawFastVLine(x, y, height, light);
            // bottom/right dark edge
            tft.drawFastHLine(x, y + height - 1, width, dark);
            tft.drawFastVLine(x + width - 1, y, height, dark);

            // shift label if pressed for a simple animation
            if (pressed)
            {
                label.setBounds(x + 1, y + 1, width - 1, height - 1);
            }
            else
            {
                label.setBounds(x, y, width, height);
            }
            label.draw();
        }

        void onTouch(int px, int py) override
        {
            if (contains(px, py))
            {
                pressed = true;
                draw();
            }
        }

        void onTouchEnd(int px, int py) override
        {
            if (pressed && contains(px, py) && onClick)
            {
                // schedule the callback to run *after* the current
                // event dispatch has finished. executing a click
                // handler immediately is dangerous because the handler
                // may destroy the container that is currently being
                // iterated, leading to use‑after‑free (see crash
                // reported in issue).
                UI::queueAction(onClick);
            }
            pressed = false;
            draw();
        }

        void mount() override
        {
            Element::mount();
            // make sure the child label is marked as mounted too; otherwise
            // its draw() will early-return and nothing appears
            label.mount();
            draw();
        }

    private:
        // caption element, kept in sync with button bounds
        Label label;
        Callback onClick;
        bool pressed{false};
        uint16_t bgColor{TFT_LIGHTGREY};
        uint16_t labelFg{TFT_WHITE}; // track current label foreground colour

        // two colours used for the 3‑D effect; highlight is drawn on the
        // top/left edges and shadow on bottom/right. these may be swapped
        // when the button is pressed.
        uint16_t borderLightColor{TFT_WHITE};
        uint16_t borderDarkColor{TFT_BLACK};

    public:
        // change the two-tone border colours; useful if the default
        // black/white pair doesn't contrast well on a particular
        // background.
        void setBorderColors(uint16_t light, uint16_t dark)
        {
            borderLightColor = light;
            borderDarkColor = dark;
        }

        // override setBounds so that the contained label moves with the
        // button without requiring callers to set its bounds manually.
        void setBounds(int ix, int iy, int iw, int ih)
        {
            Element::setBounds(ix, iy, iw, ih);
            label.setBounds(ix, iy, iw, ih);
        }
    };

} // namespace UI
