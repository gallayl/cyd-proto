#include "UiFeature.h"
#include "../Logging.h"
#include "screens.h"            // page definitions and touch helpers
#include "ActionQueue.h"        // deferred callbacks from widgets

// physical display dimensions (ILI9341 rotated portrait)
const int screenWidth = 240;
const int screenHeight = 320;
// definition of the feature object
Feature *UiFeature = new Feature("UI", []()
                                 {
    tft.fillScreen(TFT_BLACK);

    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(1);
    tft.setCursor(0, 0);
    tft.println("UI Feature Initialized");

    readCalibrationData();

    // register our consolidated "screen" namespace command
    CommandInterpreterInstance->RegisterCommand(screenCustomCommand);
    CommandInterpreterInstance->RegisterCommand(pageCustomCommand);

    // start on the welcome page by default
    showWelcomeScreen();

    LoggerInstance->Info(F("UI feature initialized"));

    return FeatureState::RUNNING; }, []()
                                 {
                                     // poll touch and forward to current screen
                                     static bool prevTouched = false;
                                     static int prevX = 0, prevY = 0;
                                     int tx, ty;
                                     bool touched = tft.getTouch(&tx, &ty);
                                     if (touched) {
                                         uiHandleTouch(tx, ty);
                                     } else if (prevTouched) {
                                         uiHandleTouchEnd(prevX, prevY);                                         // execute any actions widgets queued during
                                         // touch processing (screen changes, etc.)
                                         UI::executeQueuedActions();                                     }
                                     prevTouched = touched;
                                     if (touched) {
                                         prevX = tx;
                                         prevY = ty;
                                     } });

// optional explicit init, currently no-op but kept for API
void uiFeatureInit()
{
    // nothing for now; feature registry will call Setup()
}
