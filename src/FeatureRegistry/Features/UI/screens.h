#pragma once

// declarations for UI screens and touch event forwarding

void showWelcomeScreen();
void showRgbLedScreen();

// touch event helpers that UI feature loop will call
void uiHandleTouch(int x, int y);
void uiHandleTouchEnd(int x, int y);
