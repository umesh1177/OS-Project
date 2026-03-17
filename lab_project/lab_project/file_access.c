/* file_access.c – Mutex-protected shared file read/write */
#include "lab.h"

pthread_mutex_t file_mutex = PTHREAD_MUTEX_INITIALIZER;

void init_shared_file(void) {
    FILE *fp = fopen(SHARED_FILE, "w");
    if (!fp) { perror("shared file"); return; }
    fprintf(fp, "=== Shared Lab File ===\n");
    fprintf(fp, "This file is accessed by all lab users.\n\n");
    fclose(fp);
    printf(GREEN "  [✓] Shared file '%s' initialized.\n" RESET, SHARED_FILE);
}

void write_shared_file(int user_id, const char *data) {
    if (user_id < 1 || user_id > MAX_USERS) return;
    User *u = &users[user_id - 1];

    printf(BLUE "  [~] %s requesting write lock on shared file...\n" RESET, u->name);
    pthread_mutex_lock(&file_mutex);
    printf(GREEN "  [✓] %s acquired write lock.\n" RESET, u->name);

    FILE *fp = fopen(SHARED_FILE, "a");
    if (fp) {
        char tbuf[64];
        current_time_str(tbuf, sizeof(tbuf));
        fprintf(fp, "[%s] %s wrote: %s\n", tbuf, u->name, data);
        fclose(fp);
    }

    sleep_ms(150); /* simulate write duration */

    char msg[160];
    snprintf(msg, sizeof(msg), "FILE WRITE  | %-10s wrote \"%s\"", u->name, data);
    log_event(msg);
    printf(GREEN "  [✓] %s wrote to shared file.\n" RESET, u->name);

    pthread_mutex_unlock(&file_mutex);
    printf(YELLOW "  [←] %s released write lock.\n" RESET, u->name);
}

void read_shared_file(int user_id) {
    if (user_id < 1 || user_id > MAX_USERS) return;
    User *u = &users[user_id - 1];

    pthread_mutex_lock(&file_mutex);

    print_separator("SHARED FILE CONTENTS", CYAN);
    printf("  (Read by: %s)\n\n", u->name);

    FILE *fp = fopen(SHARED_FILE, "r");
    if (fp) {
        char line[256];
        while (fgets(line, sizeof(line), fp))
            printf("  %s", line);
        fclose(fp);
    } else {
        printf("  (File not found)\n");
    }

    char msg[128];
    snprintf(msg, sizeof(msg), "FILE READ   | %-10s read shared file", u->name);
    log_event(msg);

    pthread_mutex_unlock(&file_mutex);
}
