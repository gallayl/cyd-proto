#include <lvgl.h>
#include "../../../../hw/RgbLed.h" // adjust relative include path

// keep slider pointers and state in file scope
static lv_obj_t *sliderR;
static lv_obj_t *sliderG;
static lv_obj_t *sliderB;
static lv_obj_t *colorPreview;
static uint8_t curR = 0;
static uint8_t curG = 0;
static uint8_t curB = 0;

static void rgb_slider_event_cb(lv_event_t *e)
{
    lv_obj_t *slider = (lv_obj_t *)lv_event_get_target(e);
    int val = lv_slider_get_value(slider);

    if (slider == sliderR)
        curR = val;
    else if (slider == sliderG)
        curG = val;
    else if (slider == sliderB)
        curB = val;

    // apply to hardware
    setRgbLedColor(curR, curG, curB);

    // update preview box color
    lv_obj_set_style_bg_color(colorPreview, lv_color_make(curR, curG, curB), 0);
}

void rgbLedPage()
{
    // create new screen and load
    lv_obj_t *screen = lv_obj_create(NULL);
    lv_scr_load(screen);

    // title label
    lv_obj_t *title = lv_label_create(screen);
    lv_label_set_text(title, "RGB LED Control");
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);

    // color preview box
    colorPreview = lv_obj_create(screen);
    lv_obj_set_size(colorPreview, 100, 100);
    lv_obj_align(colorPreview, LV_ALIGN_CENTER, 0, -20);
    lv_obj_set_style_radius(colorPreview, 8, 0);
    lv_obj_set_style_bg_color(colorPreview, lv_color_make(curR, curG, curB), 0);

    // red slider
    sliderR = lv_slider_create(screen);
    lv_slider_set_range(sliderR, 0, 255);
    lv_slider_set_value(sliderR, curR, LV_ANIM_OFF);
    lv_obj_align(sliderR, LV_ALIGN_BOTTOM_MID, 0, -60);
    lv_obj_add_event_cb(sliderR, rgb_slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_t *lblR = lv_label_create(screen);
    lv_label_set_text(lblR, "R");
    lv_obj_align_to(lblR, sliderR, LV_ALIGN_OUT_LEFT_MID, -10, 0);

    // green slider
    sliderG = lv_slider_create(screen);
    lv_slider_set_range(sliderG, 0, 255);
    lv_slider_set_value(sliderG, curG, LV_ANIM_OFF);
    lv_obj_align(sliderG, LV_ALIGN_BOTTOM_MID, 0, -30);
    lv_obj_add_event_cb(sliderG, rgb_slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_t *lblG = lv_label_create(screen);
    lv_label_set_text(lblG, "G");
    lv_obj_align_to(lblG, sliderG, LV_ALIGN_OUT_LEFT_MID, -10, 0);

    // blue slider
    sliderB = lv_slider_create(screen);
    lv_slider_set_range(sliderB, 0, 255);
    lv_slider_set_value(sliderB, curB, LV_ANIM_OFF);
    lv_obj_align(sliderB, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_add_event_cb(sliderB, rgb_slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_t *lblB = lv_label_create(screen);
    lv_label_set_text(lblB, "B");
    lv_obj_align_to(lblB, sliderB, LV_ALIGN_OUT_LEFT_MID, -10, 0);
}
