/* login.c – User login/logout simulation */
#include "lab.h"

User users[MAX_USERS];

static const char *user_names[MAX_USERS] = {
    "Alice", "Bob", "Charlie", "Diana", "Evan", "Fatima"
};

static pthread_mutex_t login_mutex = PTHREAD_MUTEX_INITIALIZER;

void init_users(void) {
    for (int i = 0; i < MAX_USERS; i++) {
        users[i].id          = i + 1;
        strncpy(users[i].name, user_names[i], MAX_NAME_LEN - 1);
        users[i].logged_in   = 0;
        users[i].computer_id = -1;
        users[i].priority    = (i % 3) + 1;  /* 1,2,3,1,2,3 */
        users[i].login_time  = 0;
    }
}

int user_login(int user_id) {
    if (user_id < 1 || user_id > MAX_USERS) return -1;
    User *u = &users[user_id - 1];

    pthread_mutex_lock(&login_mutex);
    if (u->logged_in) {
        printf(YELLOW "  [!] %s is already logged in.\n" RESET, u->name);
        pthread_mutex_unlock(&login_mutex);
        return 0;
    }
    u->logged_in  = 1;
    u->login_time = time(NULL);
    pthread_mutex_unlock(&login_mutex);

    char msg[128];
    snprintf(msg, sizeof(msg), "USER LOGIN  | %-10s (ID:%d  Priority:%d)",
             u->name, u->id, u->priority);
    log_event(msg);
    printf(GREEN "  [✓] %s logged in successfully (Priority: %d)\n" RESET,
           u->name, u->priority);
    return 1;
}

void user_logout(int user_id) {
    if (user_id < 1 || user_id > MAX_USERS) return;
    User *u = &users[user_id - 1];

    pthread_mutex_lock(&login_mutex);
    if (!u->logged_in) {
        pthread_mutex_unlock(&login_mutex);
        return;
    }
    /* Release computer if still held */
    if (u->computer_id != -1)
        release_computer(user_id);

    u->logged_in  = 0;
    u->login_time = 0;
    pthread_mutex_unlock(&login_mutex);

    char msg[128];
    snprintf(msg, sizeof(msg), "USER LOGOUT | %-10s (ID:%d)", u->name, u->id);
    log_event(msg);
    printf(YELLOW "  [←] %s logged out.\n" RESET, u->name);
}

void display_users(void) {
    print_separator("USER STATUS TABLE", BLUE);
    printf("  %-4s %-12s %-10s %-12s %-8s\n",
           "ID", "Name", "Status", "Computer", "Priority");
    printf("  %-4s %-12s %-10s %-12s %-8s\n",
           "──", "────────────", "──────────", "────────────", "────────");
    for (int i = 0; i < MAX_USERS; i++) {
        User *u = &users[i];
        const char *status  = u->logged_in ? GREEN "Online " RESET : RED "Offline" RESET;
        char comp[16];
        if (u->computer_id != -1)
            snprintf(comp, sizeof(comp), "PC-%d", u->computer_id);
        else
            strcpy(comp, "None");

        const char *pstr = u->priority == 1 ? RED "High" RESET
                         : u->priority == 2 ? YELLOW "Med " RESET
                         :                    GREEN "Low " RESET;
        printf("  %-4d %-12s %s      %-12s %s\n",
               u->id, u->name, status, comp, pstr);
    }
}
