#pragma once

#include <LovyanGFX.hpp>
#include "../../../hw/LovyanGFX_ILI9341_Settings.h"

extern LGFX tft;

namespace UI
{
namespace Theme
{
// Win95 color palette (RGB565 values)
constexpr uint16_t DesktopBg = 0x0410;        // teal  #008080
constexpr uint16_t TitleBarActive = 0x0010;   // navy  #000080
constexpr uint16_t TitleBarInactive = 0x8410; // gray  #808080
constexpr uint16_t TitleTextActive = TFT_WHITE;
constexpr uint16_t TitleTextInactive = 0xC618; // light gray #C0C0C0
constexpr uint16_t ButtonFace = 0xC618;        // #C0C0C0
constexpr uint16_t ButtonHighlight = TFT_WHITE;
constexpr uint16_t ButtonShadow = 0x8410; // #808080
constexpr uint16_t ButtonDarkShadow = TFT_BLACK;
constexpr uint16_t WindowBg = 0xC618; // #C0C0C0
constexpr uint16_t WindowBorder = TFT_BLACK;
constexpr uint16_t TextColor = TFT_BLACK;
constexpr uint16_t TaskbarBg = 0xC618; // #C0C0C0
constexpr uint16_t MenuBg = 0xC618;
constexpr uint16_t MenuHighlight = 0x0010; // navy
constexpr uint16_t MenuHighlightText = TFT_WHITE;
constexpr uint16_t ScrollTrack = 0xC618;
constexpr uint16_t ScrollThumb = 0xC618;
constexpr uint16_t MenuSeparatorDark = 0x8410;     // #808080 (same as ButtonShadow)
constexpr uint16_t MenuSeparatorLight = TFT_WHITE; // highlight line below dark

// layout constants (screen-dependent values are runtime functions)
inline int ScreenWidth()
{
    return tft.width();
}
inline int ScreenHeight()
{
    return tft.height();
}
constexpr int TaskbarHeight = 26;
constexpr int TitleBarHeight = 18;
constexpr int DesktopY = 0;
inline int DesktopHeight()
{
    return ScreenHeight() - TaskbarHeight;
}
inline int TaskbarY()
{
    return ScreenHeight() - TaskbarHeight;
}
constexpr int StartButtonWidth = 50;
constexpr int CloseButtonSize = 14;
constexpr int WindowBorderWidth = 2;
constexpr int ScrollbarWidth = 14;
constexpr int ThinScrollbarWidth = 5;
constexpr int MenuItemHeight = 20;
constexpr int MenuWidth = 120;
constexpr int SubMenuWidth = 120;
constexpr int MenuSeparatorHeight = 8;
inline int KeyboardHeight()
{
    return DesktopHeight() / 2;
}
constexpr int KeyboardToggleSize = 22;

// checkbox / radio
constexpr int CheckboxSize = 13;
constexpr int RadioSize = 12;
constexpr uint16_t CheckboxBg = TFT_WHITE;

// text field
constexpr uint16_t TextFieldBg = TFT_WHITE;
constexpr int TextFieldHeight = 18;

// tabs
constexpr int TabBarHeight = 20;

// menu bar
constexpr int MenuBarHeight = 18;

// group box
constexpr int GroupBoxPad = 6;
constexpr int GroupBoxTopPad = 12;

// file list
constexpr int FileListIconSize = 16;
constexpr int FileListRowHeight = 16;
constexpr int FileListDetailRowHeight = 14;
constexpr int FileListIconGridW = 48;
constexpr int FileListIconGridH = 40;

// window buttons
constexpr int WinBtnSize = 14;

} // namespace Theme
} // namespace UI
