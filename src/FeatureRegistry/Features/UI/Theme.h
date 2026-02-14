#pragma once

#include <LovyanGFX.hpp>

namespace UI
{
    namespace Theme
    {
        // Win95 color palette (RGB565 values)
        constexpr uint16_t DesktopBg = 0x0410;        // teal  #008080
        constexpr uint16_t TitleBarActive = 0x0010;    // navy  #000080
        constexpr uint16_t TitleBarInactive = 0x8410;  // gray  #808080
        constexpr uint16_t TitleTextActive = TFT_WHITE;
        constexpr uint16_t TitleTextInactive = 0xC618; // light gray #C0C0C0
        constexpr uint16_t ButtonFace = 0xC618;        // #C0C0C0
        constexpr uint16_t ButtonHighlight = TFT_WHITE;
        constexpr uint16_t ButtonShadow = 0x8410;      // #808080
        constexpr uint16_t ButtonDarkShadow = TFT_BLACK;
        constexpr uint16_t WindowBg = 0xC618;          // #C0C0C0
        constexpr uint16_t WindowBorder = TFT_BLACK;
        constexpr uint16_t TextColor = TFT_BLACK;
        constexpr uint16_t TaskbarBg = 0xC618;         // #C0C0C0
        constexpr uint16_t MenuBg = 0xC618;
        constexpr uint16_t MenuHighlight = 0x0010;     // navy
        constexpr uint16_t MenuHighlightText = TFT_WHITE;
        constexpr uint16_t ScrollTrack = 0xC618;
        constexpr uint16_t ScrollThumb = 0xC618;
        constexpr uint16_t MenuSeparatorDark = 0x8410;  // #808080 (same as ButtonShadow)
        constexpr uint16_t MenuSeparatorLight = TFT_WHITE; // highlight line below dark

        // layout constants
        constexpr int ScreenWidth = 240;
        constexpr int ScreenHeight = 320;
        constexpr int TaskbarHeight = 26;
        constexpr int TitleBarHeight = 18;
        constexpr int DesktopY = 0;
        constexpr int DesktopHeight = ScreenHeight - TaskbarHeight;
        constexpr int TaskbarY = ScreenHeight - TaskbarHeight;
        constexpr int StartButtonWidth = 50;
        constexpr int CloseButtonSize = 14;
        constexpr int WindowBorderWidth = 2;
        constexpr int ScrollbarWidth = 14;
        constexpr int ThinScrollbarWidth = 5;
        constexpr int MenuItemHeight = 20;
        constexpr int MenuWidth = 120;
        constexpr int SubMenuWidth = 120;
        constexpr int MenuSeparatorHeight = 8;
        constexpr int KeyboardHeight = DesktopHeight / 2;
        constexpr int KeyboardToggleSize = 22;

    } // namespace Theme
} // namespace UI
