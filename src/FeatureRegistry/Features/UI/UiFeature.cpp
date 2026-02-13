#include "UiFeature.h"
#include "../Logging.h"
#include "ActionQueue.h"
#include "Renderer.h"
#include "Desktop.h"

const int screenWidth = 240;
const int screenHeight = 320;

Feature *UiFeature = new Feature("UI", []()
                                 {
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(1);
    tft.setCursor(0, 0);
    tft.println("UI Feature Initialized");

    readCalibrationData();

    if (!UI::initRenderer()) {
        LoggerInstance->Error(F("Failed to create sprite back-buffer"));
        return FeatureState::ERROR;
    }

    UI::desktop().init();

    CommandInterpreterInstance->RegisterCommand(screenCustomCommand);
    CommandInterpreterInstance->RegisterCommand(pageCustomCommand);

    LoggerInstance->Info(F("UI feature initialized (Win95 desktop)"));

    return FeatureState::RUNNING; }, []()
                                 {
    static bool prevTouched = false;
    static int prevX = 0, prevY = 0;
    int tx, ty;
    bool touched = tft.getTouch(&tx, &ty);

    if (touched) {
        UI::desktop().handleTouch(tx, ty);
    } else if (prevTouched) {
        UI::desktop().handleTouchEnd(prevX, prevY);
        UI::executeQueuedActions();
    }

    prevTouched = touched;
    if (touched) {
        prevX = tx;
        prevY = ty;
    }

    UI::desktop().loop();
    UI::desktop().draw(); });

void uiFeatureInit()
{
}
