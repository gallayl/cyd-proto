#include "screens.h"
#include <memory>
#include "screens/welcome.h"
#include "screens/rgbled.h"

#include "elements/container.h"
#include "elements/label.h"
#include "elements/button.h"
#include "../Logging.h"
#include "../../../hw/RgbLed.h" // for setRgbLedColor

extern LGFX tft;
extern const int screenWidth;
extern const int screenHeight;

namespace
{
    std::unique_ptr<UI::Container> currentPage;

    void clearCurrent()
    {
        if (currentPage)
        {
            if (currentPage->isMounted())
                currentPage->unmount();
            currentPage.reset();
        }
        // always clear display when switching
        tft.fillScreen(TFT_BLACK);
    }
}

void showWelcomeScreen()
{
    clearCurrent();
    std::unique_ptr<UI::Container> cont(new UI::Container());
    cont->setBounds(0, 0, screenWidth, screenHeight);

    std::unique_ptr<UI::Label> lbl(new UI::Label("Welcome to CYD", 10, 10, 220, 20));
    cont->addChild(std::move(lbl));

    std::unique_ptr<UI::Button> btn(new UI::Button("RGB Led settings", 10, 40, 220, 30));
    // clicking transitions to rgb screen
    btn->setCallback([]()
                     { showRgbLedScreen(); });
    cont->addChild(std::move(btn));

    cont->mount();
    currentPage = std::move(cont);
}

void showRgbLedScreen()
{
    clearCurrent();
    std::unique_ptr<UI::Container> cont(new UI::Container());
    cont->setBounds(0, 0, screenWidth, screenHeight);

    std::unique_ptr<UI::Label> lbl(new UI::Label("RGB LED Page", 10, 10, 220, 20));
    cont->addChild(std::move(lbl));

    // create colour buttons with callbacks
    auto makeColorButton = [&](const String &text, int y, uint8_t r, uint8_t g, uint8_t b)
    {
        std::unique_ptr<UI::Button> bptr(new UI::Button(text, 10, y, 100, 30));
        bptr->setCallback([r, g, b]()
                          { setRgbLedColor(r, g, b); });
        cont->addChild(std::move(bptr));
    };

    makeColorButton("Red", 40, 255, 0, 0);
    makeColorButton("Green", 80, 0, 255, 0);
    makeColorButton("Blue", 120, 0, 0, 255);

    std::unique_ptr<UI::Button> backBtn(new UI::Button("Back", 10, 160, 100, 30));
    backBtn->setCallback([]()
                         { showWelcomeScreen(); });
    cont->addChild(std::move(backBtn));

    cont->mount();
    currentPage = std::move(cont);
}

void uiHandleTouch(int x, int y)
{
    if (currentPage && currentPage->isMounted())
        currentPage->handleTouch(x, y);
}

void uiHandleTouchEnd(int x, int y)
{
    if (currentPage && currentPage->isMounted())
        currentPage->handleTouchEnd(x, y);
}
