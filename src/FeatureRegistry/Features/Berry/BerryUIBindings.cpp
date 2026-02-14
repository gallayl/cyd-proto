#include "BerryUIBindings.h"

#if ENABLE_BERRY

#include "BerryApp.h"
#include "../UI/elements/container.h"
#include "../UI/elements/label.h"
#include "../UI/elements/button.h"
#include "../UI/elements/scrollable.h"
#include "../UI/Theme.h"
#include "../UI/Renderer.h"
#include "../Logging.h"

#include <LovyanGFX.hpp>

// --- Current app context ---

static BerryApp *s_currentApp = nullptr;

BerryApp *berryCurrentApp() { return s_currentApp; }
void berrySetCurrentApp(BerryApp *app) { s_currentApp = app; }

// --- Canvas element (wraps LGFX_Sprite) ---

class BerryCanvasElement : public UI::Element
{
public:
    BerryCanvasElement(int w, int h, int depth = 16)
    {
        _depth = depth;
        sprite.setColorDepth(depth);
        _spriteOk = sprite.createSprite(w, h) != nullptr;
        if (_spriteOk)
            sprite.fillSprite(depth <= 8 ? 0 : TFT_WHITE);
    }

    ~BerryCanvasElement() override
    {
        if (_spriteOk)
            sprite.deleteSprite();
    }

    void draw() override
    {
        if (!mounted || !_spriteOk)
            return;
        auto &c = UI::canvas();
        sprite.pushSprite(&c, x, y);
    }

    void onTouch(int px, int py) override
    {
        if (!_touchApp || _touchCbId < 0)
            return;
        int lx = px - x;
        int ly = py - y;
        _touchApp->callBerryCallbackWithArgs(_touchCbId, [lx, ly](bvm *vm) -> int
                                             {
            be_pushint(vm, lx);
            be_pushint(vm, ly);
            return 2; });
    }

    void onTouchEnd(int px, int py) override
    {
        if (!_touchEndApp || _touchEndCbId < 0)
            return;
        int lx = px - x;
        int ly = py - y;
        _touchEndApp->callBerryCallbackWithArgs(_touchEndCbId, [lx, ly](bvm *vm) -> int
                                                {
            be_pushint(vm, lx);
            be_pushint(vm, ly);
            return 2; });
    }

    LGFX_Sprite sprite;
    bool _spriteOk{false};
    int _depth{16};

    BerryApp *_touchApp{nullptr};
    int _touchCbId{-1};
    BerryApp *_touchEndApp{nullptr};
    int _touchEndCbId{-1};
};

// --- Typed handle helpers ---

static UI::Container *asContainer(HandleEntry *h)
{
    if (!h || h->type != HandleType::CONTAINER)
        return nullptr;
    return static_cast<UI::Container *>(h->ptr);
}

static UI::Label *asLabel(HandleEntry *h)
{
    if (!h || h->type != HandleType::LABEL)
        return nullptr;
    return static_cast<UI::Label *>(h->ptr);
}

static UI::Button *asButton(HandleEntry *h)
{
    if (!h || h->type != HandleType::BUTTON)
        return nullptr;
    return static_cast<UI::Button *>(h->ptr);
}

static UI::ScrollableContainer *asScrollable(HandleEntry *h)
{
    if (!h || h->type != HandleType::SCROLLABLE)
        return nullptr;
    return static_cast<UI::ScrollableContainer *>(h->ptr);
}

static BerryCanvasElement *asCanvas(HandleEntry *h)
{
    if (!h || h->type != HandleType::CANVAS)
        return nullptr;
    return static_cast<BerryCanvasElement *>(h->ptr);
}

// helper to add child to either Container or ScrollableContainer
static bool addChildToParent(BerryApp *app, int parentHandle, std::unique_ptr<UI::Element> child)
{
    auto *h = app->getHandle(parentHandle);
    if (!h)
        return false;

    if (h->type == HandleType::SCROLLABLE)
    {
        static_cast<UI::ScrollableContainer *>(h->ptr)->addChild(std::move(child));
        return true;
    }
    if (h->type == HandleType::CONTAINER)
    {
        static_cast<UI::Container *>(h->ptr)->addChild(std::move(child));
        return true;
    }
    return false;
}

// helper to get parent bounds for relative->absolute translation
static void getParentBounds(BerryApp *app, int parentHandle, int &px, int &py, int &pw, int &ph)
{
    auto *h = app->getHandle(parentHandle);
    if (h && h->ptr)
        h->ptr->getBounds(px, py, pw, ph);
    else
        px = py = pw = ph = 0;
}

// =================================================================
// Native UI functions
// =================================================================

// ui.label(parent, text, x, y, w, h) -> handle
static int ui_label(bvm *vm)
{
    auto *app = berryCurrentApp();
    if (!app || be_top(vm) < 6)
        be_return_nil(vm);

    int parentH = be_toint(vm, 1);
    const char *text = be_tostring(vm, 2);
    int rx = be_toint(vm, 3);
    int ry = be_toint(vm, 4);
    int w = be_toint(vm, 5);
    int h = be_toint(vm, 6);

    int px, py, pw, ph;
    getParentBounds(app, parentH, px, py, pw, ph);

    auto lbl = std::make_unique<UI::Label>(text, px + rx, py + ry, w, h);
    lbl->setTextColor(UI::Theme::TextColor, UI::Theme::WindowBg);
    lbl->setTextSize(1);
    lbl->setAlign(UI::TextAlign::LEFT);

    UI::Label *ptr = lbl.get();
    if (!addChildToParent(app, parentH, std::move(lbl)))
        be_return_nil(vm);

    int handle = app->addHandle(ptr, HandleType::LABEL);
    be_pushint(vm, handle);
    be_return(vm);
}

// ui.button(parent, text, x, y, w, h) -> handle
static int ui_button(bvm *vm)
{
    auto *app = berryCurrentApp();
    if (!app || be_top(vm) < 6)
        be_return_nil(vm);

    int parentH = be_toint(vm, 1);
    const char *text = be_tostring(vm, 2);
    int rx = be_toint(vm, 3);
    int ry = be_toint(vm, 4);
    int w = be_toint(vm, 5);
    int h = be_toint(vm, 6);

    int px, py, pw, ph;
    getParentBounds(app, parentH, px, py, pw, ph);

    auto btn = std::make_unique<UI::Button>(text, px + rx, py + ry, w, h);
    btn->setBackgroundColor(UI::Theme::ButtonFace);
    btn->setTextColor(UI::Theme::TextColor, UI::Theme::ButtonFace);

    UI::Button *ptr = btn.get();
    if (!addChildToParent(app, parentH, std::move(btn)))
        be_return_nil(vm);

    int handle = app->addHandle(ptr, HandleType::BUTTON);
    be_pushint(vm, handle);
    be_return(vm);
}

// ui.scrollable(parent [, x, y, w, h]) -> handle
static int ui_scrollable(bvm *vm)
{
    auto *app = berryCurrentApp();
    if (!app || be_top(vm) < 1)
        be_return_nil(vm);

    int parentH = be_toint(vm, 1);

    int px, py, pw, ph;
    getParentBounds(app, parentH, px, py, pw, ph);

    auto sc = std::make_unique<UI::ScrollableContainer>();

    if (be_top(vm) >= 5)
    {
        int rx = be_toint(vm, 2);
        int ry = be_toint(vm, 3);
        int w = be_toint(vm, 4);
        int h = be_toint(vm, 5);
        sc->setBounds(px + rx, py + ry, w, h);
    }
    else
    {
        sc->setBounds(px, py, pw, ph);
    }

    UI::ScrollableContainer *ptr = sc.get();
    if (!addChildToParent(app, parentH, std::move(sc)))
        be_return_nil(vm);

    int handle = app->addHandle(ptr, HandleType::SCROLLABLE);
    be_pushint(vm, handle);
    be_return(vm);
}

// ui.canvas(parent, x, y, w, h [, depth]) -> handle
static int ui_canvas(bvm *vm)
{
    auto *app = berryCurrentApp();
    if (!app || be_top(vm) < 5)
        be_return_nil(vm);

    int parentH = be_toint(vm, 1);
    int rx = be_toint(vm, 2);
    int ry = be_toint(vm, 3);
    int w = be_toint(vm, 4);
    int h = be_toint(vm, 5);
    int depth = (be_top(vm) >= 6) ? be_toint(vm, 6) : 16;

    int px, py, pw, ph;
    getParentBounds(app, parentH, px, py, pw, ph);

    auto cv = std::make_unique<BerryCanvasElement>(w, h, depth);
    cv->setBounds(px + rx, py + ry, w, h);

    BerryCanvasElement *ptr = cv.get();
    if (!addChildToParent(app, parentH, std::move(cv)))
        be_return_nil(vm);

    int handle = app->addHandle(ptr, HandleType::CANVAS);
    be_pushint(vm, handle);
    be_return(vm);
}

// =================================================================
// Property setters
// =================================================================

// ui.set_text(handle, text)
static int ui_set_text(bvm *vm)
{
    auto *app = berryCurrentApp();
    if (!app || be_top(vm) < 2)
        be_return_nil(vm);

    auto *h = app->getHandle(be_toint(vm, 1));
    const char *text = be_tostring(vm, 2);

    auto *lbl = asLabel(h);
    if (lbl) { lbl->setText(text); be_return_nil(vm); }

    auto *btn = asButton(h);
    if (btn) { btn->setLabel(text); be_return_nil(vm); }

    be_return_nil(vm);
}

// ui.set_text_color(handle, fg [, bg])
static int ui_set_text_color(bvm *vm)
{
    auto *app = berryCurrentApp();
    if (!app || be_top(vm) < 2)
        be_return_nil(vm);

    auto *h = app->getHandle(be_toint(vm, 1));
    uint16_t fg = (uint16_t)be_toint(vm, 2);
    uint16_t bg = (be_top(vm) >= 3) ? (uint16_t)be_toint(vm, 3) : TFT_BLACK;

    auto *lbl = asLabel(h);
    if (lbl) { lbl->setTextColor(fg, bg); be_return_nil(vm); }

    auto *btn = asButton(h);
    if (btn) { btn->setTextColor(fg, bg); be_return_nil(vm); }

    be_return_nil(vm);
}

// ui.set_text_size(handle, size)
static int ui_set_text_size(bvm *vm)
{
    auto *app = berryCurrentApp();
    if (!app || be_top(vm) < 2)
        be_return_nil(vm);

    auto *h = app->getHandle(be_toint(vm, 1));
    uint8_t sz = (uint8_t)be_toint(vm, 2);

    auto *lbl = asLabel(h);
    if (lbl) { lbl->setTextSize(sz); be_return_nil(vm); }

    auto *btn = asButton(h);
    if (btn) { btn->setTextSize(sz); be_return_nil(vm); }

    be_return_nil(vm);
}

// ui.set_align(handle, align)  -- 0=CENTER, 1=LEFT, 2=RIGHT
static int ui_set_align(bvm *vm)
{
    auto *app = berryCurrentApp();
    if (!app || be_top(vm) < 2)
        be_return_nil(vm);

    auto *lbl = asLabel(app->getHandle(be_toint(vm, 1)));
    if (lbl)
        lbl->setAlign(static_cast<UI::TextAlign>(be_toint(vm, 2)));

    be_return_nil(vm);
}

// ui.set_bg_color(handle, color)
static int ui_set_bg_color(bvm *vm)
{
    auto *app = berryCurrentApp();
    if (!app || be_top(vm) < 2)
        be_return_nil(vm);

    auto *btn = asButton(app->getHandle(be_toint(vm, 1)));
    if (btn)
        btn->setBackgroundColor((uint16_t)be_toint(vm, 2));

    be_return_nil(vm);
}

// ui.set_border_colors(handle, light, dark)
static int ui_set_border_colors(bvm *vm)
{
    auto *app = berryCurrentApp();
    if (!app || be_top(vm) < 3)
        be_return_nil(vm);

    auto *btn = asButton(app->getHandle(be_toint(vm, 1)));
    if (btn)
        btn->setBorderColors((uint16_t)be_toint(vm, 2), (uint16_t)be_toint(vm, 3));

    be_return_nil(vm);
}

// ui.set_content_height(handle, h)
static int ui_set_content_height(bvm *vm)
{
    auto *app = berryCurrentApp();
    if (!app || be_top(vm) < 2)
        be_return_nil(vm);

    auto *sc = asScrollable(app->getHandle(be_toint(vm, 1)));
    if (sc)
        sc->setContentHeight(be_toint(vm, 2));

    be_return_nil(vm);
}

// ui.set_auto_content_height(handle, enabled)
static int ui_set_auto_content_height(bvm *vm)
{
    auto *app = berryCurrentApp();
    if (!app || be_top(vm) < 2)
        be_return_nil(vm);

    auto *sc = asScrollable(app->getHandle(be_toint(vm, 1)));
    if (sc)
        sc->setAutoContentHeight(be_tobool(vm, 2));

    be_return_nil(vm);
}

// ui.bounds(handle) -> [x, y, w, h]
static int ui_bounds(bvm *vm)
{
    auto *app = berryCurrentApp();
    if (!app || be_top(vm) < 1)
        be_return_nil(vm);

    auto *h = app->getHandle(be_toint(vm, 1));
    if (!h)
        be_return_nil(vm);

    int bx, by, bw, bh;
    h->ptr->getBounds(bx, by, bw, bh);

    be_newlist(vm);
    be_pushint(vm, bx);
    be_data_push(vm, -2);
    be_pop(vm, 1);
    be_pushint(vm, by);
    be_data_push(vm, -2);
    be_pop(vm, 1);
    be_pushint(vm, bw);
    be_data_push(vm, -2);
    be_pop(vm, 1);
    be_pushint(vm, bh);
    be_data_push(vm, -2);
    be_pop(vm, 1);

    be_return(vm);
}

// =================================================================
// Callbacks & timers
// =================================================================

// ui.on_click(handle, callback)
static int ui_on_click(bvm *vm)
{
    auto *app = berryCurrentApp();
    if (!app || be_top(vm) < 2)
        be_return_nil(vm);

    auto *btn = asButton(app->getHandle(be_toint(vm, 1)));
    if (!btn)
        be_return_nil(vm);

    int cbId = app->storeCallback(vm, 2);
    BerryApp *appPtr = app;
    btn->setCallback([appPtr, cbId]()
                     { appPtr->callBerryCallback(cbId); });

    be_return_nil(vm);
}

// ui.on_touch(handle, callback(x, y))
static int ui_on_touch(bvm *vm)
{
    auto *app = berryCurrentApp();
    if (!app || be_top(vm) < 2)
        be_return_nil(vm);

    auto *h = app->getHandle(be_toint(vm, 1));
    if (!h)
        be_return_nil(vm);

    int cbId = app->storeCallback(vm, 2);

    if (h->type == HandleType::CONTAINER)
    {
        BerryApp *appPtr = app;
        static_cast<UI::Container *>(h->ptr)->setTouchHandler(
            [appPtr, cbId](int px, int py)
            {
                appPtr->callBerryCallbackWithArgs(cbId, [px, py](bvm *v) -> int
                                                  {
                    be_pushint(v, px);
                    be_pushint(v, py);
                    return 2; });
            });
        be_return_nil(vm);
    }

    if (h->type == HandleType::CANVAS)
    {
        auto *cv = static_cast<BerryCanvasElement *>(h->ptr);
        cv->_touchApp = app;
        cv->_touchCbId = cbId;
        be_return_nil(vm);
    }

    be_return_nil(vm);
}

// ui.on_touch_end(handle, callback(x, y))
static int ui_on_touch_end(bvm *vm)
{
    auto *app = berryCurrentApp();
    if (!app || be_top(vm) < 2)
        be_return_nil(vm);

    auto *h = app->getHandle(be_toint(vm, 1));
    if (!h)
        be_return_nil(vm);

    int cbId = app->storeCallback(vm, 2);

    if (h->type == HandleType::CONTAINER)
    {
        BerryApp *appPtr = app;
        static_cast<UI::Container *>(h->ptr)->setTouchEndHandler(
            [appPtr, cbId](int px, int py)
            {
                appPtr->callBerryCallbackWithArgs(cbId, [px, py](bvm *v) -> int
                                                  {
                    be_pushint(v, px);
                    be_pushint(v, py);
                    return 2; });
            });
        be_return_nil(vm);
    }

    if (h->type == HandleType::CANVAS)
    {
        auto *cv = static_cast<BerryCanvasElement *>(h->ptr);
        cv->_touchEndApp = app;
        cv->_touchEndCbId = cbId;
        be_return_nil(vm);
    }

    be_return_nil(vm);
}

// ui.timer(ms, callback)
static int ui_timer(bvm *vm)
{
    auto *app = berryCurrentApp();
    if (!app || be_top(vm) < 2)
        be_return_nil(vm);

    uint32_t ms = (uint32_t)be_toint(vm, 1);
    int cbId = app->storeCallback(vm, 2);
    app->addBerryTimer(ms, cbId);

    be_return_nil(vm);
}

// =================================================================
// Container operations
// =================================================================

// ui.clear(handle)
static int ui_clear(bvm *vm)
{
    auto *app = berryCurrentApp();
    if (!app || be_top(vm) < 1)
        be_return_nil(vm);

    auto *h = app->getHandle(be_toint(vm, 1));
    if (!h)
        be_return_nil(vm);

    if (h->type == HandleType::SCROLLABLE)
    {
        static_cast<UI::ScrollableContainer *>(h->ptr)->getContent().clear();
        be_return_nil(vm);
    }

    if (h->type == HandleType::CONTAINER)
    {
        static_cast<UI::Container *>(h->ptr)->clear();
    }

    be_return_nil(vm);
}

// ui.remove_child(parent, child)
static int ui_remove_child(bvm *vm)
{
    auto *app = berryCurrentApp();
    if (!app || be_top(vm) < 2)
        be_return_nil(vm);

    int parentIdx = be_toint(vm, 1);
    int childIdx = be_toint(vm, 2);

    auto *ph = app->getHandle(parentIdx);
    auto *ch = app->getHandle(childIdx);
    if (!ph || !ch)
        be_return_nil(vm);

    if (ph->type == HandleType::SCROLLABLE)
    {
        static_cast<UI::ScrollableContainer *>(ph->ptr)->getContent().removeChild(ch->ptr);
        app->invalidateHandle(childIdx);
        be_return_nil(vm);
    }

    if (ph->type == HandleType::CONTAINER)
    {
        static_cast<UI::Container *>(ph->ptr)->removeChild(ch->ptr);
        app->invalidateHandle(childIdx);
    }

    be_return_nil(vm);
}

// =================================================================
// Canvas drawing
// =================================================================

#define GET_CANVAS(vm, app, minArgs)                \
    if (!app || be_top(vm) < minArgs)               \
        be_return_nil(vm);                          \
    auto *cv = asCanvas(app->getHandle(be_toint(vm, 1))); \
    if (!cv || !cv->_spriteOk)                      \
        be_return_nil(vm);

static int ui_canvas_fill(bvm *vm)
{
    auto *app = berryCurrentApp();
    GET_CANVAS(vm, app, 2);
    cv->sprite.fillSprite((uint16_t)be_toint(vm, 2));
    be_return_nil(vm);
}

static int ui_canvas_draw_pixel(bvm *vm)
{
    auto *app = berryCurrentApp();
    GET_CANVAS(vm, app, 4);
    cv->sprite.drawPixel(be_toint(vm, 2), be_toint(vm, 3), (uint16_t)be_toint(vm, 4));
    be_return_nil(vm);
}

static int ui_canvas_draw_line(bvm *vm)
{
    auto *app = berryCurrentApp();
    GET_CANVAS(vm, app, 6);
    cv->sprite.drawLine(be_toint(vm, 2), be_toint(vm, 3),
                        be_toint(vm, 4), be_toint(vm, 5),
                        (uint16_t)be_toint(vm, 6));
    be_return_nil(vm);
}

static int ui_canvas_draw_rect(bvm *vm)
{
    auto *app = berryCurrentApp();
    GET_CANVAS(vm, app, 6);
    cv->sprite.drawRect(be_toint(vm, 2), be_toint(vm, 3),
                        be_toint(vm, 4), be_toint(vm, 5),
                        (uint16_t)be_toint(vm, 6));
    be_return_nil(vm);
}

static int ui_canvas_fill_rect(bvm *vm)
{
    auto *app = berryCurrentApp();
    GET_CANVAS(vm, app, 6);
    cv->sprite.fillRect(be_toint(vm, 2), be_toint(vm, 3),
                        be_toint(vm, 4), be_toint(vm, 5),
                        (uint16_t)be_toint(vm, 6));
    be_return_nil(vm);
}

static int ui_canvas_draw_circle(bvm *vm)
{
    auto *app = berryCurrentApp();
    GET_CANVAS(vm, app, 5);
    cv->sprite.drawCircle(be_toint(vm, 2), be_toint(vm, 3),
                          be_toint(vm, 4), (uint16_t)be_toint(vm, 5));
    be_return_nil(vm);
}

static int ui_canvas_fill_circle(bvm *vm)
{
    auto *app = berryCurrentApp();
    GET_CANVAS(vm, app, 5);
    cv->sprite.fillCircle(be_toint(vm, 2), be_toint(vm, 3),
                          be_toint(vm, 4), (uint16_t)be_toint(vm, 5));
    be_return_nil(vm);
}

static int ui_canvas_draw_ellipse(bvm *vm)
{
    auto *app = berryCurrentApp();
    GET_CANVAS(vm, app, 6);
    cv->sprite.drawEllipse(be_toint(vm, 2), be_toint(vm, 3),
                           be_toint(vm, 4), be_toint(vm, 5),
                           (uint16_t)be_toint(vm, 6));
    be_return_nil(vm);
}

static int ui_canvas_read_pixel(bvm *vm)
{
    auto *app = berryCurrentApp();
    GET_CANVAS(vm, app, 3);
    uint32_t c = cv->sprite.readPixel(be_toint(vm, 2), be_toint(vm, 3));
    be_pushint(vm, (bint)c);
    be_return(vm);
}

static int ui_canvas_flood_fill(bvm *vm)
{
    auto *app = berryCurrentApp();
    GET_CANVAS(vm, app, 4);

    int fx = be_toint(vm, 2);
    int fy = be_toint(vm, 3);
    uint16_t fillColor = (uint16_t)be_toint(vm, 4);

    int bx, by, cvW, cvH;
    cv->getBounds(bx, by, cvW, cvH);

    if (fx < 0 || fx >= cvW || fy < 0 || fy >= cvH)
        be_return_nil(vm);

    uint32_t targetColor = cv->sprite.readPixel(fx, fy);
    cv->sprite.drawPixel(fx, fy, fillColor);
    uint32_t filledColor = cv->sprite.readPixel(fx, fy);
    if (filledColor == targetColor)
        be_return_nil(vm);

    struct Seed { int16_t x, y; };
    std::vector<Seed> stack;
    stack.reserve(256);

    if (fx > 0) stack.push_back({(int16_t)(fx - 1), (int16_t)fy});
    if (fx < cvW - 1) stack.push_back({(int16_t)(fx + 1), (int16_t)fy});
    if (fy > 0) stack.push_back({(int16_t)fx, (int16_t)(fy - 1)});
    if (fy < cvH - 1) stack.push_back({(int16_t)fx, (int16_t)(fy + 1)});

    while (!stack.empty())
    {
        Seed s = stack.back();
        stack.pop_back();
        if (s.x < 0 || s.x >= cvW || s.y < 0 || s.y >= cvH) continue;
        if (cv->sprite.readPixel(s.x, s.y) != targetColor) continue;

        int left = s.x;
        while (left > 0 && cv->sprite.readPixel(left - 1, s.y) == targetColor) left--;

        int right = left;
        bool aboveQ = false, belowQ = false;
        while (right < cvW && cv->sprite.readPixel(right, s.y) == targetColor)
        {
            cv->sprite.drawPixel(right, s.y, fillColor);
            if (s.y > 0)
            {
                bool m = (cv->sprite.readPixel(right, s.y - 1) == targetColor);
                if (m && !aboveQ) { stack.push_back({(int16_t)right, (int16_t)(s.y - 1)}); aboveQ = true; }
                else if (!m) aboveQ = false;
            }
            if (s.y < cvH - 1)
            {
                bool m = (cv->sprite.readPixel(right, s.y + 1) == targetColor);
                if (m && !belowQ) { stack.push_back({(int16_t)right, (int16_t)(s.y + 1)}); belowQ = true; }
                else if (!m) belowQ = false;
            }
            right++;
        }
        if (stack.size() > 2000) break;
    }

    be_return_nil(vm);
}

static int ui_canvas_set_palette(bvm *vm)
{
    auto *app = berryCurrentApp();
    GET_CANVAS(vm, app, 5);
    cv->sprite.setPaletteColor(be_toint(vm, 2),
                               (uint8_t)be_toint(vm, 3),
                               (uint8_t)be_toint(vm, 4),
                               (uint8_t)be_toint(vm, 5));
    be_return_nil(vm);
}

#undef GET_CANVAS

// =================================================================
// Utility
// =================================================================

static int ui_color565(bvm *vm)
{
    if (be_top(vm) < 3)
        be_return_nil(vm);
    uint8_t r = (uint8_t)be_toint(vm, 1);
    uint8_t g = (uint8_t)be_toint(vm, 2);
    uint8_t b = (uint8_t)be_toint(vm, 3);
    uint16_t c = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
    be_pushint(vm, c);
    be_return(vm);
}

static int ui_mark_dirty(bvm *vm)
{
    UI::markDirty();
    be_return_nil(vm);
}

// =================================================================
// Module registration
// =================================================================

void registerBerryUIModule(bvm *vm)
{
    be_newmodule(vm);
    be_setname(vm, -1, "ui");

    auto reg = [&](const char *name, bntvfunc f)
    {
        be_pushntvfunction(vm, f);
        be_setmember(vm, -2, name);
        be_pop(vm, 1);
    };

    // element creation
    reg("label", ui_label);
    reg("button", ui_button);
    reg("scrollable", ui_scrollable);
    reg("canvas", ui_canvas);

    // property setters
    reg("set_text", ui_set_text);
    reg("set_text_color", ui_set_text_color);
    reg("set_text_size", ui_set_text_size);
    reg("set_align", ui_set_align);
    reg("set_bg_color", ui_set_bg_color);
    reg("set_border_colors", ui_set_border_colors);
    reg("set_content_height", ui_set_content_height);
    reg("set_auto_content_height", ui_set_auto_content_height);
    reg("bounds", ui_bounds);

    // callbacks & timers
    reg("on_click", ui_on_click);
    reg("on_touch", ui_on_touch);
    reg("on_touch_end", ui_on_touch_end);
    reg("timer", ui_timer);

    // container operations
    reg("clear", ui_clear);
    reg("remove_child", ui_remove_child);

    // canvas drawing
    reg("canvas_fill", ui_canvas_fill);
    reg("canvas_draw_pixel", ui_canvas_draw_pixel);
    reg("canvas_draw_line", ui_canvas_draw_line);
    reg("canvas_draw_rect", ui_canvas_draw_rect);
    reg("canvas_fill_rect", ui_canvas_fill_rect);
    reg("canvas_draw_circle", ui_canvas_draw_circle);
    reg("canvas_fill_circle", ui_canvas_fill_circle);
    reg("canvas_draw_ellipse", ui_canvas_draw_ellipse);
    reg("canvas_read_pixel", ui_canvas_read_pixel);
    reg("canvas_flood_fill", ui_canvas_flood_fill);
    reg("canvas_set_palette", ui_canvas_set_palette);

    // utility
    reg("color565", ui_color565);
    reg("mark_dirty", ui_mark_dirty);

    // alignment constants
    auto regInt = [&](const char *name, int val)
    {
        be_pushint(vm, val);
        be_setmember(vm, -2, name);
        be_pop(vm, 1);
    };

    regInt("CENTER", 0);
    regInt("LEFT", 1);
    regInt("RIGHT", 2);

    // theme constants
    regInt("DESKTOP_BG", UI::Theme::DesktopBg);
    regInt("TITLE_BAR_ACTIVE", UI::Theme::TitleBarActive);
    regInt("TITLE_BAR_INACTIVE", UI::Theme::TitleBarInactive);
    regInt("TITLE_TEXT_ACTIVE", UI::Theme::TitleTextActive);
    regInt("TITLE_TEXT_INACTIVE", UI::Theme::TitleTextInactive);
    regInt("BUTTON_FACE", UI::Theme::ButtonFace);
    regInt("BUTTON_HIGHLIGHT", UI::Theme::ButtonHighlight);
    regInt("BUTTON_SHADOW", UI::Theme::ButtonShadow);
    regInt("BUTTON_DARK_SHADOW", UI::Theme::ButtonDarkShadow);
    regInt("WINDOW_BG", UI::Theme::WindowBg);
    regInt("WINDOW_BORDER", UI::Theme::WindowBorder);
    regInt("TEXT_COLOR", UI::Theme::TextColor);
    regInt("TASKBAR_BG", UI::Theme::TaskbarBg);
    regInt("MENU_BG", UI::Theme::MenuBg);
    regInt("MENU_HIGHLIGHT", UI::Theme::MenuHighlight);
    regInt("SCROLL_TRACK", UI::Theme::ScrollTrack);
    regInt("SCROLL_THUMB", UI::Theme::ScrollThumb);

    be_setglobal(vm, "ui");
    be_pop(vm, 1);
}

#endif // ENABLE_BERRY
