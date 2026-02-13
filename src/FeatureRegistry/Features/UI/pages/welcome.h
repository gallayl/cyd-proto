#include <lvgl.h>

void showWelcomeScreen()
{
    // Create a new screen
    lv_obj_t *welcomeScreen = lv_obj_create(NULL);
    lv_scr_load(welcomeScreen);

    // Create a label for the welcome message
    lv_obj_t *welcomeLabel = lv_label_create(welcomeScreen);
    lv_label_set_text(welcomeLabel, "Welcome to CYD!");
    lv_obj_center(welcomeLabel);
}