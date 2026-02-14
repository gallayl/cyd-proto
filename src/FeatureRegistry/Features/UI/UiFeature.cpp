#include "../../../config.h"

#if ENABLE_UI

#include <esp_timer.h>
#include "UiFeature.h"
#include "../Logging.h"
#include "ActionQueue.h"
#include "Renderer.h"
#include "Desktop.h"

const int screenWidth = 240;
const int screenHeight = 320;

static volatile bool frameReady = true;
static esp_timer_handle_t frameTimer = nullptr;

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

    esp_timer_create_args_t args = {};
    args.callback = [](void *) { frameReady = true; };
    args.name = "ui_frame";
    esp_timer_create(&args, &frameTimer);
    esp_timer_start_periodic(frameTimer, 33000);

    LoggerInstance->Info(F("UI feature initialized (Win95 desktop)"));

    return FeatureState::RUNNING; }, []()
                                 {
    static bool prevTouched = false;
    static int prevX = 0, prevY = 0;
    int tx, ty;
    bool touched = tft.getTouch(&tx, &ty);

    if (touched) {
        UI::desktop().handleTouch(tx, ty);
        UI::markDirty();
    } else if (prevTouched) {
        UI::desktop().handleTouchEnd(prevX, prevY);
        UI::executeQueuedActions();
        UI::markDirty();
    }

    prevTouched = touched;
    if (touched) {
        prevX = tx;
        prevY = ty;
    }

    UI::desktop().tickTimers();

    if (UI::isDirty() && frameReady) {
        frameReady = false;
        UI::desktop().draw();
        UI::clearDirty();
    } }, []()
                                 {
    if (frameTimer) {
        esp_timer_stop(frameTimer);
        esp_timer_delete(frameTimer);
        frameTimer = nullptr;
    } });

void uiFeatureInit()
{
}

#endif
