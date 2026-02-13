#include "UiFeature.h"
#include "../Logging.h"

// instantiate constants and buffer
const int screenWidth = 240;
const int screenHeight = 320;
uint16_t drawBuffer[screenWidth * 10]; // Partial buffer size matches previous header

// LVGL display flush callback
static void my_disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
    uint32_t w = lv_area_get_width(area);
    uint32_t h = lv_area_get_height(area);
    tft.startWrite();
    tft.setAddrWindow(area->x1, area->y1, w, h);
    tft.pushPixels((uint16_t *)px_map, w * h);
    tft.endWrite();
    lv_display_flush_ready(disp);
}

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

    lv_init();
    static lv_display_t *disp = lv_display_create(screenWidth, screenHeight);
    lv_display_set_flush_cb(disp, my_disp_flush);
    lv_display_set_buffers(disp, drawBuffer, nullptr, sizeof(drawBuffer), LV_DISPLAY_RENDER_MODE_PARTIAL);

    return FeatureState::RUNNING; }, []()
                                 {
    lv_timer_handler();
    lv_tick_inc(millis()); });

// optional explicit init, currently no-op but kept for API
void uiFeatureInit()
{
    // nothing for now; feature registry will call Setup()
}
