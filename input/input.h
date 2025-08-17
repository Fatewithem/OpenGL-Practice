#pragma once

// 回调函数声明
void onResize(int width, int height);
void OnKey(int key, int scancode, int action, int mods);
void OnMouse(int button, int action, int mods);
void OnCursor(double xpos, double ypos);
void onScroll(double offset);