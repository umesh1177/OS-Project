/* utils.c – Banner, separator, time helpers */
#include "lab.h"

void print_banner(void) {
    printf(MAGENTA);
    printf("\n╔══════════════════════════════════════════════════════╗\n");
    printf("║     🖥️  MULTI-USER LINUX LAB MANAGEMENT SYSTEM 🖥️      ║\n");
    printf("║           OS Subject Project — C Language             ║\n");
    printf("╚══════════════════════════════════════════════════════╝\n");
    printf(RESET "\n");
}

void print_separator(const char *title, const char *color) {
    printf("\n%s", color);
    printf("══════════════════════════════════════════\n");
    printf("  %s\n", title);
    printf("══════════════════════════════════════════\n");
    printf(RESET);
}

void current_time_str(char *buf, size_t len) {
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    strftime(buf, len, "%H:%M:%S", tm_info);
}

void sleep_ms(int ms) {
    struct timespec ts;
    ts.tv_sec  = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000L;
    nanosleep(&ts, NULL);
}
