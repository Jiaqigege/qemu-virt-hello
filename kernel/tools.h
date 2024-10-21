#pragma once

#define COLOR_RESET   "\033[0m"
#define COLOR_BLACK   "\033[30m"
#define COLOR_RED     "\033[31m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_BLUE    "\033[34m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_CYAN    "\033[36m"
#define COLOR_WHITE   "\033[37m"

#define BG_BLACK   "\033[40m"
#define BG_RED     "\033[41m"
#define BG_GREEN   "\033[42m"
#define BG_YELLOW  "\033[43m"
#define BG_BLUE    "\033[44m"
#define BG_MAGENTA "\033[45m"
#define BG_CYAN    "\033[46m"
#define BG_WHITE   "\033[47m"

#define NEWLINE "\n"

// 定义打印宏
#define print_with_color(color, msg) printf(color msg COLOR_RESET NEWLINE)

// 定义各种颜色的打印宏
#define black_println(msg)   print_with_color(COLOR_BLACK, msg)
#define red_println(msg)     print_with_color(COLOR_RED, msg)
#define green_println(msg)   print_with_color(COLOR_GREEN, msg)
#define yellow_println(msg)  print_with_color(COLOR_YELLOW, msg)
#define blue_println(msg)    print_with_color(COLOR_BLUE, msg)
#define magenta_println(msg) print_with_color(COLOR_MAGENTA, msg)
#define cyan_println(msg)    print_with_color(COLOR_CYAN, msg)
#define white_println(msg)   print_with_color(COLOR_WHITE, msg)

extern void put32(unsigned long int addr, unsigned int ch);
extern unsigned int get32(unsigned long int addr);
extern void delay(unsigned long time);
