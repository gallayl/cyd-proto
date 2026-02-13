#include "UiFeature.h"
#include "../Logging.h"

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
    CommandInterpreterInstance->RegisterCommand(*screenCustomCommand);
    CommandInterpreterInstance->RegisterCommand(*pageCustomCommand);

    LoggerInstance->Info("UI feature initialized");


    return FeatureState::RUNNING; }, []()
                                 {
                                     // TODO: UI handlers
                                 });

// optional explicit init, currently no-op but kept for API
void uiFeatureInit()
{
    // nothing for now; feature registry will call Setup()
}
