#pragma once

// 宏 替换函数为func
#ifdef DEBUG
#define GL_CALL(func) func;checkerror();
#else
#define GL_CALL(func) func;
#endif

void checkerror();