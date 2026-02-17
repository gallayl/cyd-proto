#pragma once

#include <functional>
#include <string>
#include <vector>
#include "container.h"
#include "../Renderer.h"
#include "../Theme.h"

namespace UI
{

struct FileItem
{
    std::string name;
    uint32_t size{0};
    bool isDir{false};
    time_t lastWrite{0};
};

enum class FileListViewMode
{
    Icons,
    List,
    Details
};

class FileListView : public Element
{
public:
    using ItemSelectedCb = std::function<void(int, const FileItem &)>;
    using ItemActivatedCb = std::function<void(int, const FileItem &)>;

    FileListView(int ix = 0, int iy = 0, int iw = 0, int ih = 0)
    {
        setBounds(ix, iy, iw, ih);
    }

    void setItems(const std::vector<FileItem> &items)
    {
        _items = items;
        scrollOffset = 0;
        selectedIndex = -1;
    }
    const std::vector<FileItem> &getItems() const
    {
        return _items;
    }

    void setViewMode(FileListViewMode mode)
    {
        viewMode = mode;
        scrollOffset = 0;
    }
    FileListViewMode getViewMode() const
    {
        return viewMode;
    }

    void setOnItemSelected(ItemSelectedCb cb)
    {
        onItemSelected = std::move(cb);
    }
    void setOnItemActivated(ItemActivatedCb cb)
    {
        onItemActivated = std::move(cb);
    }

    int getSelectedIndex() const
    {
        return selectedIndex;
    }

    void draw() override
    {
        if (!mounted)
            return;

        auto &c = canvas();

        // sunken border
        c.fillRect(drawX(), drawY(), width, height, TFT_WHITE);
        c.drawFastHLine(drawX(), drawY(), width, Theme::ButtonShadow);
        c.drawFastVLine(drawX(), drawY(), height, Theme::ButtonShadow);
        c.drawFastHLine(drawX(), drawY() + height - 1, width, Theme::ButtonHighlight);
        c.drawFastVLine(drawX() + width - 1, drawY(), height, Theme::ButtonHighlight);

        c.setClipRect(drawX() + 1, drawY() + 1, width - 2, height - 2);

        switch (viewMode)
        {
        case FileListViewMode::Icons:
            drawIconsView(c);
            break;
        case FileListViewMode::List:
            drawListView(c);
            break;
        case FileListViewMode::Details:
            drawDetailsView(c);
            break;
        }

        c.clearClipRect();
    }

    void onTouch(int px, int py) override
    {
        if (!mounted || !contains(px, py))
            return;

        if (dragging)
        {
            int delta = lastTouchY - py;
            scrollOffset += delta;
            clampScroll();
            lastTouchY = py;
            return;
        }

        touchStartY = py;
        lastTouchY = py;
        dragging = true;
    }

    void onTouchEnd(int px, int py) override
    {
        if (!mounted)
            return;

        bool wasDragging = dragging;
        dragging = false;

        // if barely moved, treat as tap (select item)
        if (wasDragging && abs(py - touchStartY) < 5)
        {
            int idx = itemIndexAt(px, py);
            if (idx >= 0 && idx < (int)_items.size())
            {
                if (idx == selectedIndex)
                {
                    if (onItemActivated)
                        onItemActivated(idx, _items[idx]);
                }
                else
                {
                    selectedIndex = idx;
                    if (onItemSelected)
                        onItemSelected(idx, _items[idx]);
                }
            }
        }
    }

private:
    std::vector<FileItem> _items;
    FileListViewMode viewMode{FileListViewMode::List};
    int selectedIndex{-1};
    int scrollOffset{0};
    bool dragging{false};
    int touchStartY{0};
    int lastTouchY{0};
    ItemSelectedCb onItemSelected;
    ItemActivatedCb onItemActivated;

    int totalContentHeight() const
    {
        switch (viewMode)
        {
        case FileListViewMode::Icons:
        {
            int cols = (width - 4) / Theme::FileListIconGridW;
            if (cols < 1)
                cols = 1;
            int rows = ((int)_items.size() + cols - 1) / cols;
            return rows * Theme::FileListIconGridH;
        }
        case FileListViewMode::List:
            return (int)_items.size() * Theme::FileListRowHeight;
        case FileListViewMode::Details:
            return Theme::FileListDetailRowHeight + (int)_items.size() * Theme::FileListDetailRowHeight;
        }
        return 0;
    }

    void clampScroll()
    {
        int max = totalContentHeight() - height + 4;
        if (max < 0)
            max = 0;
        if (scrollOffset < 0)
            scrollOffset = 0;
        if (scrollOffset > max)
            scrollOffset = max;
    }

    int itemIndexAt(int px, int py) const
    {
        int lx = px - (x + 2);
        int ly = py - (y + 2) + scrollOffset;

        switch (viewMode)
        {
        case FileListViewMode::Icons:
        {
            int cols = (width - 4) / Theme::FileListIconGridW;
            if (cols < 1)
                cols = 1;
            int col = lx / Theme::FileListIconGridW;
            int row = ly / Theme::FileListIconGridH;
            if (col < 0 || col >= cols)
                return -1;
            return row * cols + col;
        }
        case FileListViewMode::List:
            return ly / Theme::FileListRowHeight;
        case FileListViewMode::Details:
        {
            int headerH = Theme::FileListDetailRowHeight;
            if (ly < headerH)
                return -1;
            return (ly - headerH) / Theme::FileListDetailRowHeight;
        }
        }
        return -1;
    }

    void drawIconsView(LGFX_Sprite &c)
    {
        int cols = (width - 4) / Theme::FileListIconGridW;
        if (cols < 1)
            cols = 1;
        int startX = drawX() + 2;
        int startY = drawY() + 2 - scrollOffset;

        c.setTextSize(1);
        for (int i = 0; i < (int)_items.size(); i++)
        {
            int col = i % cols;
            int row = i / cols;
            int ix = startX + col * Theme::FileListIconGridW;
            int iy = startY + row * Theme::FileListIconGridH;

            if (iy + Theme::FileListIconGridH < drawY() || iy > drawY() + height)
                continue;

            bool sel = (i == selectedIndex);

            // default icon
            int iconX = ix + (Theme::FileListIconGridW - Theme::FileListIconSize) / 2;
            int iconY = iy + 2;
            drawDefaultIcon(c, iconX, iconY, _items[i].isDir);

            // name
            std::string displayName = _items[i].name;
            if (displayName.length() > 7)
                displayName = displayName.substr(0, 6) + "~";

            int16_t tw = c.textWidth(displayName.c_str());
            int tx = ix + (Theme::FileListIconGridW - tw) / 2;
            int ty = iconY + Theme::FileListIconSize + 2;

            if (sel)
            {
                c.fillRect(tx - 1, ty - 1, tw + 2, c.fontHeight() + 2, Theme::MenuHighlight);
                c.setTextColor(Theme::MenuHighlightText, Theme::MenuHighlight);
            }
            else
            {
                c.setTextColor(Theme::TextColor, TFT_WHITE);
            }
            c.setCursor(tx, ty);
            c.print(displayName.c_str());
        }
    }

    void drawListView(LGFX_Sprite &c)
    {
        int startY = drawY() + 2 - scrollOffset;
        int rowH = Theme::FileListRowHeight;

        c.setTextSize(1);
        for (int i = 0; i < (int)_items.size(); i++)
        {
            int ry = startY + i * rowH;
            if (ry + rowH < drawY() || ry > drawY() + height)
                continue;

            bool sel = (i == selectedIndex);

            if (sel)
            {
                c.fillRect(drawX() + 1, ry, width - 2, rowH, Theme::MenuHighlight);
                c.setTextColor(Theme::MenuHighlightText, Theme::MenuHighlight);
            }
            else
            {
                c.setTextColor(Theme::TextColor, TFT_WHITE);
            }

            // small icon
            int iconSz = rowH - 2;
            drawDefaultIcon(c, drawX() + 3, ry + 1, _items[i].isDir, iconSz);

            c.setCursor(drawX() + 3 + iconSz + 4, ry + (rowH - c.fontHeight()) / 2);
            c.print(_items[i].name.c_str());
        }
    }

    void drawDetailsView(LGFX_Sprite &c)
    {
        int headerH = Theme::FileListDetailRowHeight;
        int rowH = Theme::FileListDetailRowHeight;
        int startY = drawY() + 2 - scrollOffset;

        // column widths
        int nameW = width * 50 / 100;
        int sizeW = width * 25 / 100;
        // rest is modified

        c.setTextSize(1);

        // header
        {
            int hy = startY;
            c.fillRect(drawX() + 1, hy, width - 2, headerH, Theme::ButtonFace);
            c.drawFastHLine(drawX() + 1, hy, width - 2, Theme::ButtonHighlight);
            c.drawFastHLine(drawX() + 1, hy + headerH - 1, width - 2, Theme::ButtonShadow);

            c.setTextColor(Theme::TextColor, Theme::ButtonFace);
            int16_t th = c.fontHeight();
            int textY = hy + (headerH - th) / 2;

            c.setCursor(drawX() + 4, textY);
            c.print("Name");
            c.setCursor(drawX() + 4 + nameW, textY);
            c.print("Size");
            c.setCursor(drawX() + 4 + nameW + sizeW, textY);
            c.print("Modified");
        }

        // rows
        for (int i = 0; i < (int)_items.size(); i++)
        {
            int ry = startY + headerH + i * rowH;
            if (ry + rowH < drawY() || ry > drawY() + height)
                continue;

            bool sel = (i == selectedIndex);
            if (sel)
            {
                c.fillRect(drawX() + 1, ry, width - 2, rowH, Theme::MenuHighlight);
                c.setTextColor(Theme::MenuHighlightText, Theme::MenuHighlight);
            }
            else
            {
                c.setTextColor(Theme::TextColor, TFT_WHITE);
            }

            int16_t th = c.fontHeight();
            int textY = ry + (rowH - th) / 2;

            // name
            c.setCursor(drawX() + 4, textY);
            std::string dispName = _items[i].isDir ? (std::string("[") + _items[i].name + "]") : _items[i].name;
            c.print(dispName.c_str());

            // size
            c.setCursor(drawX() + 4 + nameW, textY);
            if (_items[i].isDir)
                c.print("<DIR>");
            else
                c.print(std::to_string(_items[i].size).c_str());

            // modified
            c.setCursor(drawX() + 4 + nameW + sizeW, textY);
            if (_items[i].lastWrite > 0)
            {
                struct tm *t = localtime(&_items[i].lastWrite);
                if (t)
                {
                    char buf[32];
                    snprintf(buf, sizeof(buf), "%02d/%02d/%02d", t->tm_mon + 1, t->tm_mday, t->tm_year % 100);
                    c.print(buf);
                }
            }
        }
    }

    static void drawDefaultIcon(LGFX_Sprite &c, int ix, int iy, bool isDir, int sz = 0)
    {
        if (sz == 0)
            sz = Theme::FileListIconSize;

        if (isDir)
        {
            // folder icon: yellow rectangle with tab
            uint16_t folderColor = 0xFE00; // yellow
            c.fillRect(ix, iy + sz / 4, sz, sz * 3 / 4, folderColor);
            c.fillRect(ix, iy + 2, sz / 2, sz / 4, folderColor);
            c.drawRect(ix, iy + sz / 4, sz, sz * 3 / 4, Theme::ButtonShadow);
            c.drawRect(ix, iy + 2, sz / 2, sz / 4, Theme::ButtonShadow);
        }
        else
        {
            // file icon: white rectangle with corner fold
            c.fillRect(ix, iy, sz, sz, TFT_WHITE);
            c.drawRect(ix, iy, sz, sz, Theme::ButtonShadow);
            // corner fold
            int foldSz = sz / 4;
            c.drawLine(ix + sz - foldSz, iy, ix + sz - 1, iy + foldSz, Theme::ButtonShadow);
            // text lines
            for (int l = 0; l < 3; l++)
            {
                int ly = iy + sz / 3 + l * 3;
                if (ly < iy + sz - 2)
                    c.drawFastHLine(ix + 2, ly, sz - 4, Theme::ButtonShadow);
            }
        }
    }
};

} // namespace UI
