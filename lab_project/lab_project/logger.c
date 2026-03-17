/* logger.c – Thread-safe logging to file + console */
#include "lab.h"

FILE *log_fp = NULL;
pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

void init_logger(void) {
    log_fp = fopen(LOG_FILE, "w");
    if (!log_fp) { perror("log open"); exit(1); }
    fprintf(log_fp, "=== Lab Management System – Session Log ===\n\n");
    fflush(log_fp);
}

void log_event(const char *event) {
    char tbuf[64];
    current_time_str(tbuf, sizeof(tbuf));

    pthread_mutex_lock(&log_mutex);
    if (log_fp) {
        fprintf(log_fp, "[%s] %s\n", tbuf, event);
        fflush(log_fp);
    }
    printf(CYAN "[LOG %s]" RESET " %s\n", tbuf, event);
    pthread_mutex_unlock(&log_mutex);
}

void close_logger(void) {
    if (log_fp) {
        fprintf(log_fp, "\n=== Session Ended ===\n");
        fclose(log_fp);
        log_fp = NULL;
    }
}
