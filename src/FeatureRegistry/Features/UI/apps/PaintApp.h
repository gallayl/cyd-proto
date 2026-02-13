#pragma once

#include "../App.h"
#include "../elements/container.h"
#include "../elements/button.h"
#include "../Renderer.h"
#include "../Theme.h"
#include <memory>
#include <vector>

namespace UI
{

    class PaintApp : public App
    {
    public:
        const char *name() const override { return "Paint"; }

        void setup(Container &cont, int w, int h) override
        {
            int cx, cy, cw, ch;
            cont.getBounds(cx, cy, cw, ch);

            const int toolbarH = 20;
            const int paletteH = 20;
            cvX = cx;
            cvY = cy + toolbarH;
            cvW = cw;
            cvH = ch - toolbarH - paletteH;
            palX = cx;
            palY = cvY + cvH;
            palW = cw;
            palH = paletteH;

            // paint sprite (8-bit color, same as screen canvas)
            sprite.setColorDepth(8);
            if (!sprite.createSprite(cvW, cvH))
                return;
            sprite.fillSprite(TFT_WHITE);

            // toolbar buttons
            int btnW = cw / 5;
            struct ToolDef
            {
                const char *label;
                Tool tool;
            };
            ToolDef defs[] = {
                {"Draw", Tool::DRAW},
                {"Erase", Tool::ERASE},
                {"Rect", Tool::RECT},
                {"Circ", Tool::CIRCLE},
                {"Fill", Tool::FILL},
            };
            for (int i = 0; i < 5; i++)
            {
                int bx = cx + i * btnW;
                int bw = (i < 4) ? btnW : (cw - i * btnW);
                auto btn = std::make_unique<Button>(defs[i].label, bx, cy, bw, toolbarH);
                btn->setBackgroundColor(Theme::ButtonFace);
                btn->setTextColor(Theme::TextColor, Theme::ButtonFace);
                btn->setTextSize(1);
                tbtn[i] = btn.get();
                Tool t = defs[i].tool;
                btn->setCallback([this, t]()
                                 {
                    tool = t;
                    updateToolVisual(); });
                cont.addChild(std::move(btn));
            }
            updateToolVisual();

            // canvas element (draws the sprite + shape preview)
            auto cvEl = std::make_unique<CanvasElement>(this);
            cvEl->setBounds(cvX, cvY, cvW, cvH);
            cont.addChild(std::move(cvEl));

            // palette element
            auto palEl = std::make_unique<PaletteElement>(this);
            palEl->setBounds(palX, palY, palW, palH);
            cont.addChild(std::move(palEl));

            // container-level touch handlers (for canvas drawing + palette)
            cont.setTouchHandler([this](int px, int py)
                                 { onContentTouch(px, py); });
            cont.setTouchEndHandler([this](int px, int py)
                                    { onContentTouchEnd(px, py); });
        }

        void teardown() override
        {
            sprite.deleteSprite();
        }

    private:
        // ---- types ----
        enum class Tool
        {
            DRAW,
            ERASE,
            RECT,
            CIRCLE,
            FILL
        };

        // ---- layout ----
        int cvX{0}, cvY{0}, cvW{0}, cvH{0};
        int palX{0}, palY{0}, palW{0}, palH{0};

        // ---- state ----
        Tool tool{Tool::DRAW};
        uint16_t color{TFT_BLACK};
        Button *tbtn[5]{};
        LGFX_Sprite sprite;

        // touch tracking (all in local canvas coords)
        bool touching{false};
        int tStartX{0}, tStartY{0};
        int tLastX{0}, tLastY{0};

        // ---- palette ----
        static constexpr int NCOLORS = 12;

        static const uint16_t *paletteColors()
        {
            static const uint16_t c[NCOLORS] = {
                (uint16_t)TFT_BLACK,
                (uint16_t)TFT_WHITE,
                0x8410, // gray
                0xF800, // red
                0xFD20, // orange
                0xFFE0, // yellow
                0x07E0, // green
                0x07FF, // cyan
                0x001F, // blue
                0x8010, // purple
                0xF81F, // magenta
                0x8200, // brown
            };
            return c;
        }

        // ---- elements ----

        class CanvasElement : public Element
        {
        public:
            explicit CanvasElement(PaintApp *a) : app(a) {}

            void draw() override
            {
                if (!mounted || !app)
                    return;
                // blit paint sprite onto the screen canvas
                app->sprite.pushSprite(&canvas(), x, y);

                // shape preview while dragging
                if (app->touching)
                {
                    auto &c = canvas();
                    int sx = app->tStartX + x;
                    int sy = app->tStartY + y;
                    int ex = app->tLastX + x;
                    int ey = app->tLastY + y;

                    if (app->tool == Tool::RECT)
                    {
                        int rx = (sx < ex) ? sx : ex;
                        int ry = (sy < ey) ? sy : ey;
                        int rw = abs(ex - sx) + 1;
                        int rh = abs(ey - sy) + 1;
                        c.drawRect(rx, ry, rw, rh, app->color);
                    }
                    else if (app->tool == Tool::CIRCLE)
                    {
                        int ecx = (sx + ex) / 2;
                        int ecy = (sy + ey) / 2;
                        int erx = abs(ex - sx) / 2;
                        int ery = abs(ey - sy) / 2;
                        if (erx > 0 && ery > 0)
                            c.drawEllipse(ecx, ecy, erx, ery, app->color);
                    }
                }
            }

        private:
            PaintApp *app;
        };

        class PaletteElement : public Element
        {
        public:
            explicit PaletteElement(PaintApp *a) : app(a) {}

            void draw() override
            {
                if (!mounted || !app)
                    return;
                auto &c = canvas();
                const uint16_t *cols = paletteColors();
                int sw = width / NCOLORS;

                for (int i = 0; i < NCOLORS; i++)
                {
                    int sx = x + i * sw;
                    int w = (i < NCOLORS - 1) ? sw : (width - i * sw);

                    c.fillRect(sx + 1, y + 1, w - 2, height - 2, cols[i]);

                    if (cols[i] == app->color)
                    {
                        // selected: double border (white outer, black inner)
                        c.drawRect(sx, y, w, height, TFT_WHITE);
                        c.drawRect(sx + 1, y + 1, w - 2, height - 2, TFT_BLACK);
                    }
                    else
                    {
                        c.drawRect(sx, y, w, height, Theme::ButtonShadow);
                    }
                }
            }

        private:
            PaintApp *app;
        };

        // ---- helpers ----

        void updateToolVisual()
        {
            for (int i = 0; i < 5; i++)
            {
                if (!tbtn[i])
                    continue;
                if (i == static_cast<int>(tool))
                    tbtn[i]->setBorderColors(Theme::ButtonShadow, Theme::ButtonHighlight);
                else
                    tbtn[i]->setBorderColors(Theme::ButtonHighlight, Theme::ButtonShadow);
            }
        }

        void drawThickLine(int x0, int y0, int x1, int y1, uint16_t col, int r)
        {
            int dx = abs(x1 - x0);
            int dy = abs(y1 - y0);
            int steps = (dx > dy) ? dx : dy;
            if (steps == 0)
            {
                sprite.fillCircle(x0, y0, r, col);
                return;
            }
            for (int i = 0; i <= steps; i++)
            {
                int px = x0 + (x1 - x0) * i / steps;
                int py = y0 + (y1 - y0) * i / steps;
                sprite.fillCircle(px, py, r, col);
            }
        }

        // ---- touch handling ----

        void onContentTouch(int px, int py)
        {
            // canvas area
            if (px >= cvX && px < cvX + cvW &&
                py >= cvY && py < cvY + cvH)
            {
                int lx = px - cvX;
                int ly = py - cvY;

                if (!touching)
                {
                    touching = true;
                    tStartX = lx;
                    tStartY = ly;
                    tLastX = lx;
                    tLastY = ly;

                    if (tool == Tool::DRAW)
                        sprite.fillCircle(lx, ly, 1, color);
                    else if (tool == Tool::ERASE)
                        sprite.fillCircle(lx, ly, 3, TFT_WHITE);
                    else if (tool == Tool::FILL)
                        doFloodFill(lx, ly);
                }
                else
                {
                    if (tool == Tool::DRAW)
                        drawThickLine(tLastX, tLastY, lx, ly, color, 1);
                    else if (tool == Tool::ERASE)
                        drawThickLine(tLastX, tLastY, lx, ly, TFT_WHITE, 3);

                    tLastX = lx;
                    tLastY = ly;
                }
            }
            else if (touching)
            {
                // dragged outside canvas while drawing
                int lx = constrain(px - cvX, 0, cvW - 1);
                int ly = constrain(py - cvY, 0, cvH - 1);

                if (tool == Tool::DRAW)
                    drawThickLine(tLastX, tLastY, lx, ly, color, 1);
                else if (tool == Tool::ERASE)
                    drawThickLine(tLastX, tLastY, lx, ly, TFT_WHITE, 3);

                tLastX = lx;
                tLastY = ly;
            }

            // palette area (only if not drawing on canvas)
            if (!touching &&
                px >= palX && px < palX + palW &&
                py >= palY && py < palY + palH)
            {
                int sw = palW / NCOLORS;
                int idx = (px - palX) / sw;
                if (idx >= 0 && idx < NCOLORS)
                    color = paletteColors()[idx];
            }
        }

        void onContentTouchEnd(int px, int py)
        {
            if (!touching)
                return;

            int lx = constrain(px - cvX, 0, cvW - 1);
            int ly = constrain(py - cvY, 0, cvH - 1);

            if (tool == Tool::RECT)
            {
                int rx = (tStartX < lx) ? tStartX : lx;
                int ry = (tStartY < ly) ? tStartY : ly;
                int rw = abs(lx - tStartX) + 1;
                int rh = abs(ly - tStartY) + 1;
                sprite.drawRect(rx, ry, rw, rh, color);
            }
            else if (tool == Tool::CIRCLE)
            {
                int ecx = (tStartX + lx) / 2;
                int ecy = (tStartY + ly) / 2;
                int erx = abs(lx - tStartX) / 2;
                int ery = abs(ly - tStartY) / 2;
                if (erx > 0 && ery > 0)
                    sprite.drawEllipse(ecx, ecy, erx, ery, color);
            }

            touching = false;
        }

        // ---- flood fill (scanline, operates on raw 8-bit buffer) ----

        void doFloodFill(int fx, int fy)
        {
            if (fx < 0 || fx >= cvW || fy < 0 || fy >= cvH)
                return;

            uint8_t *buf = (uint8_t *)sprite.getBuffer();
            if (!buf)
                return;

            int stride = cvW;
            uint8_t target = buf[fy * stride + fx];

            // find out what the fill color looks like in 8-bit
            sprite.drawPixel(fx, fy, color);
            uint8_t fill8 = buf[fy * stride + fx];
            buf[fy * stride + fx] = target; // restore

            if (target == fill8)
                return;

            struct Seed
            {
                int16_t x, y;
            };
            std::vector<Seed> stack;
            stack.reserve(256);
            stack.push_back({(int16_t)fx, (int16_t)fy});

            while (!stack.empty())
            {
                Seed s = stack.back();
                stack.pop_back();

                if (s.x < 0 || s.x >= cvW || s.y < 0 || s.y >= cvH)
                    continue;
                if (buf[s.y * stride + s.x] != target)
                    continue;

                // scan left
                int left = s.x;
                while (left > 0 && buf[s.y * stride + left - 1] == target)
                    left--;

                int right = left;
                bool aboveQ = false, belowQ = false;

                while (right < cvW && buf[s.y * stride + right] == target)
                {
                    buf[s.y * stride + right] = fill8;

                    if (s.y > 0)
                    {
                        bool m = (buf[(s.y - 1) * stride + right] == target);
                        if (m && !aboveQ)
                        {
                            stack.push_back({(int16_t)right, (int16_t)(s.y - 1)});
                            aboveQ = true;
                        }
                        else if (!m)
                            aboveQ = false;
                    }

                    if (s.y < cvH - 1)
                    {
                        bool m = (buf[(s.y + 1) * stride + right] == target);
                        if (m && !belowQ)
                        {
                            stack.push_back({(int16_t)right, (int16_t)(s.y + 1)});
                            belowQ = true;
                        }
                        else if (!m)
                            belowQ = false;
                    }

                    right++;
                }

                if (stack.size() > 2000)
                    break;
            }
        }
    };

} // namespace UI
