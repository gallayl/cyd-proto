#pragma once

#include <functional>
#include <cstring>
#include "../Renderer.h"
#include "../Theme.h"

namespace UI
{

    class Keyboard
    {
    public:
        using KeyPressCb = std::function<void(char)>;

        void setOnKeyPress(KeyPressCb cb) { onKeyPress = std::move(cb); }

        bool isVisible() const { return visible; }

        void toggle()
        {
            if (visible)
                hide();
            else
                show();
        }

        void show()
        {
            visible = true;
            markDirty();
        }

        void hide()
        {
            visible = false;
            shifted = false;
            symbolMode = false;
            pressedIndex = -1;
            markDirty();
        }

        void draw()
        {
            if (!visible)
                return;
            auto &c = canvas();

            int oY = UI::stripOffsetY();
            int kbY = Theme::TaskbarY() - Theme::KeyboardHeight() - oY;

            // keyboard background
            c.fillRect(0, kbY, Theme::ScreenWidth(), Theme::KeyboardHeight(), Theme::ButtonFace);
            // top border
            c.drawFastHLine(0, kbY, Theme::ScreenWidth(), Theme::ButtonHighlight);

            if (symbolMode)
                drawSymbolLayout(c, kbY);
            else
                drawLetterLayout(c, kbY);
        }

        bool handleTouch(int px, int py)
        {
            if (!visible)
                return false;
            int kbY = Theme::TaskbarY() - Theme::KeyboardHeight();
            if (py < kbY || py >= Theme::TaskbarY())
                return false;

            pressedIndex = hitTest(px, py, kbY);
            markDirty();
            return true;
        }

        bool handleTouchEnd(int px, int py)
        {
            if (!visible)
                return false;
            int kbY = Theme::TaskbarY() - Theme::KeyboardHeight();
            if (pressedIndex < 0)
                return py >= kbY && py < Theme::TaskbarY();

            int released = hitTest(px, py, kbY);
            if (released == pressedIndex)
            {
                activateKey(pressedIndex);
            }
            pressedIndex = -1;
            markDirty();
            return true;
        }

    private:
        bool visible{false};
        bool shifted{false};
        bool symbolMode{false};
        int pressedIndex{-1};
        KeyPressCb onKeyPress;

        // key geometry
        static constexpr int kPad = 2;
        static constexpr int kGap = 2;
        static constexpr int kRows = 4;
        static int kRowHeight()
        {
            return (Theme::KeyboardHeight() - kPad * 2 - (kRows - 1) * kGap) / kRows;
        }

        // each key has a flat index across all rows for hit-testing
        // row 0: 10 keys (index 0-9)
        // row 1: 9 or 10 keys (index 10-18 or 10-19)
        // row 2: variable (shift/bksp row)
        // row 3: 3 keys (mode toggle, space, enter)

        struct KeyDef
        {
            const char *label;
            char code; // character to emit; 0 = special
            int widthUnits; // relative width (1 = standard key)
        };

        // --- letter mode layout ---

        static constexpr int kLetterRow0Count = 10;
        static constexpr KeyDef letterRow0[10] = {
            {"Q", 'q', 1}, {"W", 'w', 1}, {"E", 'e', 1}, {"R", 'r', 1}, {"T", 't', 1},
            {"Y", 'y', 1}, {"U", 'u', 1}, {"I", 'i', 1}, {"O", 'o', 1}, {"P", 'p', 1}};

        static constexpr int kLetterRow1Count = 9;
        static constexpr KeyDef letterRow1[9] = {
            {"A", 'a', 1}, {"S", 's', 1}, {"D", 'd', 1}, {"F", 'f', 1}, {"G", 'g', 1},
            {"H", 'h', 1}, {"J", 'j', 1}, {"K", 'k', 1}, {"L", 'l', 1}};

        static constexpr int kLetterRow2Count = 9;
        static constexpr KeyDef letterRow2[9] = {
            {"Sh", 0, 2}, // shift (special)
            {"Z", 'z', 1}, {"X", 'x', 1}, {"C", 'c', 1}, {"V", 'v', 1},
            {"B", 'b', 1}, {"N", 'n', 1}, {"M", 'm', 1},
            {"<-", '\b', 2}}; // backspace

        static constexpr int kLetterRow3Count = 3;
        static constexpr KeyDef letterRow3[3] = {
            {"123", 0, 3},       // mode toggle (special)
            {"Space", ' ', 10},  // space
            {"Ent", '\n', 3}     // enter
        };

        // --- symbol mode layout ---

        static constexpr int kSymRow0Count = 10;
        static constexpr KeyDef symRow0[10] = {
            {"1", '1', 1}, {"2", '2', 1}, {"3", '3', 1}, {"4", '4', 1}, {"5", '5', 1},
            {"6", '6', 1}, {"7", '7', 1}, {"8", '8', 1}, {"9", '9', 1}, {"0", '0', 1}};

        static constexpr int kSymRow1Count = 10;
        static constexpr KeyDef symRow1[10] = {
            {"@", '@', 1}, {"#", '#', 1}, {"$", '$', 1}, {"%", '%', 1}, {"&", '&', 1},
            {"-", '-', 1}, {"_", '_', 1}, {"+", '+', 1}, {"(", '(', 1}, {")", ')', 1}};

        static constexpr int kSymRow2Count = 9;
        static constexpr KeyDef symRow2[9] = {
            {"=", '=', 1}, {"*", '*', 1}, {"\"", '"', 1}, {"'", '\'', 1},
            {":", ':', 1}, {";", ';', 1}, {"!", '!', 1}, {"?", '?', 1},
            {"<-", '\b', 2}}; // backspace

        static constexpr int kSymRow3Count = 3;
        static constexpr KeyDef symRow3[3] = {
            {"ABC", 0, 3},       // mode toggle (special)
            {"Space", ' ', 10},  // space
            {"Ent", '\n', 3}     // enter
        };

        static int totalUnits(const KeyDef *keys, int count)
        {
            int sum = 0;
            for (int i = 0; i < count; i++)
                sum += keys[i].widthUnits;
            return sum;
        }

        // draw a single key with Win95 3D look
        void drawKey(LGFX_Sprite &c, int kx, int ky, int kw, int kh,
                     const char *label, bool pressed) const
        {
            uint16_t light = pressed ? Theme::ButtonShadow : Theme::ButtonHighlight;
            uint16_t dark = pressed ? Theme::ButtonHighlight : Theme::ButtonShadow;

            c.fillRect(kx, ky, kw, kh, Theme::ButtonFace);
            c.drawFastHLine(kx, ky, kw, light);
            c.drawFastVLine(kx, ky, kh, light);
            c.drawFastHLine(kx, ky + kh - 1, kw, dark);
            c.drawFastVLine(kx + kw - 1, ky, kh, dark);

            c.setTextColor(Theme::TextColor, Theme::ButtonFace);
            c.setTextSize(1);
            int16_t tw = c.textWidth(label);
            int16_t th = c.fontHeight();
            int tx = kx + (kw - tw) / 2 + (pressed ? 1 : 0);
            int ty = ky + (kh - th) / 2 + (pressed ? 1 : 0);
            c.setCursor(tx, ty);
            c.print(label);
        }

        void drawRow(LGFX_Sprite &c, int rowY, const KeyDef *keys, int count,
                     int baseIndex) const
        {
            int units = totalUnits(keys, count);
            int usable = Theme::ScreenWidth() - kPad * 2 - (count - 1) * kGap;
            int xOff = kPad;

            for (int i = 0; i < count; i++)
            {
                int kw = (keys[i].widthUnits * usable) / units;
                // adjust rounding on last key
                if (i == count - 1)
                    kw = Theme::ScreenWidth() - kPad - xOff;

                const char *displayLabel = keys[i].label;
                // for letter mode, show uppercase label when shifted
                char shiftedBuf[4] = {};
                if (!symbolMode && shifted && keys[i].code >= 'a' && keys[i].code <= 'z')
                {
                    shiftedBuf[0] = keys[i].code - 32;
                    shiftedBuf[1] = '\0';
                    displayLabel = shiftedBuf;
                }

                bool isPressed = (baseIndex + i == pressedIndex);
                drawKey(c, xOff, rowY, kw, kRowHeight(), displayLabel, isPressed);
                xOff += kw + kGap;
            }
        }

        void drawLetterLayout(LGFX_Sprite &c, int kbY)
        {
            int y0 = kbY + kPad;
            int rh = kRowHeight();
            int idx = 0;
            drawRow(c, y0, letterRow0, kLetterRow0Count, idx);
            idx += kLetterRow0Count;
            drawRow(c, y0 + (rh + kGap), letterRow1, kLetterRow1Count, idx);
            idx += kLetterRow1Count;
            drawRow(c, y0 + 2 * (rh + kGap), letterRow2, kLetterRow2Count, idx);
            idx += kLetterRow2Count;
            drawRow(c, y0 + 3 * (rh + kGap), letterRow3, kLetterRow3Count, idx);
        }

        void drawSymbolLayout(LGFX_Sprite &c, int kbY)
        {
            int y0 = kbY + kPad;
            int rh = kRowHeight();
            int idx = 0;
            drawRow(c, y0, symRow0, kSymRow0Count, idx);
            idx += kSymRow0Count;
            drawRow(c, y0 + (rh + kGap), symRow1, kSymRow1Count, idx);
            idx += kSymRow1Count;
            drawRow(c, y0 + 2 * (rh + kGap), symRow2, kSymRow2Count, idx);
            idx += kSymRow2Count;
            drawRow(c, y0 + 3 * (rh + kGap), symRow3, kSymRow3Count, idx);
        }

        int hitTestRow(int px, int py, int rowY, const KeyDef *keys, int count,
                       int baseIndex) const
        {
            if (py < rowY || py >= rowY + kRowHeight())
                return -1;

            int units = totalUnits(keys, count);
            int usable = Theme::ScreenWidth() - kPad * 2 - (count - 1) * kGap;
            int xOff = kPad;

            for (int i = 0; i < count; i++)
            {
                int kw = (keys[i].widthUnits * usable) / units;
                if (i == count - 1)
                    kw = Theme::ScreenWidth() - kPad - xOff;

                if (px >= xOff && px < xOff + kw)
                    return baseIndex + i;
                xOff += kw + kGap;
            }
            return -1;
        }

        int hitTest(int px, int py, int kbY) const
        {
            int y0 = kbY + kPad;
            int rh = kRowHeight();
            if (symbolMode)
            {
                int idx = 0;
                int r;
                r = hitTestRow(px, py, y0, symRow0, kSymRow0Count, idx);
                if (r >= 0) return r;
                idx += kSymRow0Count;
                r = hitTestRow(px, py, y0 + (rh + kGap), symRow1, kSymRow1Count, idx);
                if (r >= 0) return r;
                idx += kSymRow1Count;
                r = hitTestRow(px, py, y0 + 2 * (rh + kGap), symRow2, kSymRow2Count, idx);
                if (r >= 0) return r;
                idx += kSymRow2Count;
                r = hitTestRow(px, py, y0 + 3 * (rh + kGap), symRow3, kSymRow3Count, idx);
                return r;
            }
            else
            {
                int idx = 0;
                int r;
                r = hitTestRow(px, py, y0, letterRow0, kLetterRow0Count, idx);
                if (r >= 0) return r;
                idx += kLetterRow0Count;
                r = hitTestRow(px, py, y0 + (rh + kGap), letterRow1, kLetterRow1Count, idx);
                if (r >= 0) return r;
                idx += kLetterRow1Count;
                r = hitTestRow(px, py, y0 + 2 * (rh + kGap), letterRow2, kLetterRow2Count, idx);
                if (r >= 0) return r;
                idx += kLetterRow2Count;
                r = hitTestRow(px, py, y0 + 3 * (rh + kGap), letterRow3, kLetterRow3Count, idx);
                return r;
            }
        }

        const KeyDef &keyDefAt(int flatIndex) const
        {
            if (symbolMode)
            {
                if (flatIndex < kSymRow0Count) return symRow0[flatIndex];
                flatIndex -= kSymRow0Count;
                if (flatIndex < kSymRow1Count) return symRow1[flatIndex];
                flatIndex -= kSymRow1Count;
                if (flatIndex < kSymRow2Count) return symRow2[flatIndex];
                flatIndex -= kSymRow2Count;
                return symRow3[flatIndex];
            }
            else
            {
                if (flatIndex < kLetterRow0Count) return letterRow0[flatIndex];
                flatIndex -= kLetterRow0Count;
                if (flatIndex < kLetterRow1Count) return letterRow1[flatIndex];
                flatIndex -= kLetterRow1Count;
                if (flatIndex < kLetterRow2Count) return letterRow2[flatIndex];
                flatIndex -= kLetterRow2Count;
                return letterRow3[flatIndex];
            }
        }

        void activateKey(int flatIndex)
        {
            const KeyDef &key = keyDefAt(flatIndex);

            // special keys (code == 0)
            if (key.code == 0)
            {
                const char *lbl = key.label;
                if (strcmp(lbl, "Sh") == 0)
                {
                    shifted = !shifted;
                }
                else if (strcmp(lbl, "123") == 0 || strcmp(lbl, "ABC") == 0)
                {
                    symbolMode = !symbolMode;
                    shifted = false;
                }
                return;
            }

            char ch = key.code;
            if (!symbolMode && shifted && ch >= 'a' && ch <= 'z')
            {
                ch = ch - 32; // uppercase
                shifted = false;
            }

            if (onKeyPress)
                onKeyPress(ch);
        }
    };

} // namespace UI
