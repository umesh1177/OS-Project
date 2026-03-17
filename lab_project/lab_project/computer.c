/* computer.c – Semaphore-based computer resource allocation */
#include "lab.h"

int  computer_status[MAX_COMPUTERS]; /* 0=free, else user_id */
sem_t computer_sem;

void init_computers(void) {
    memset(computer_status, 0, sizeof(computer_status));
    sem_init(&computer_sem, 0, MAX_COMPUTERS);
}

int allocate_computer(int user_id) {
    if (user_id < 1 || user_id > MAX_USERS) return -1;
    User *u = &users[user_id - 1];
    if (!u->logged_in) {
        printf(RED "  [✗] %s is not logged in!\n" RESET, u->name);
        return -1;
    }
    if (u->computer_id != -1) {
        printf(YELLOW "  [!] %s already has PC-%d\n" RESET, u->name, u->computer_id);
        return u->computer_id;
    }

    printf(BLUE "  [~] %s waiting for a computer...\n" RESET, u->name);

    /* sem_trywait – non-blocking attempt */
    if (sem_trywait(&computer_sem) != 0) {
        printf(RED "  [✗] No computers available! %s must wait.\n" RESET, u->name);
        char msg[128];
        snprintf(msg, sizeof(msg), "COMP WAIT   | %s – all %d computers busy",
                 u->name, MAX_COMPUTERS);
        log_event(msg);
        return -1;
    }

    /* Find a free slot */
    for (int i = 0; i < MAX_COMPUTERS; i++) {
        if (computer_status[i] == 0) {
            computer_status[i] = user_id;
            u->computer_id     = i + 1;

            char msg[128];
            snprintf(msg, sizeof(msg), "COMP ALLOC  | %-10s → PC-%d  (sem_value now %d)",
                     u->name, i + 1, MAX_COMPUTERS - i - 1);
            log_event(msg);
            printf(GREEN "  [✓] PC-%d allocated to %s\n" RESET, i + 1, u->name);
            return i + 1;
        }
    }

    sem_post(&computer_sem); /* rollback */
    return -1;
}

void release_computer(int user_id) {
    if (user_id < 1 || user_id > MAX_USERS) return;
    User *u = &users[user_id - 1];
    if (u->computer_id == -1) return;

    int pc = u->computer_id - 1;
    computer_status[pc] = 0;
    u->computer_id      = -1;
    sem_post(&computer_sem);

    char msg[128];
    snprintf(msg, sizeof(msg), "COMP RELEASE| %-10s released PC-%d", u->name, pc + 1);
    log_event(msg);
    printf(YELLOW "  [←] %s released PC-%d\n" RESET, u->name, pc + 1);
}

void display_computers(void) {
    print_separator("COMPUTER RESOURCE STATUS", CYAN);
    int val;
    sem_getvalue(&computer_sem, &val);
    printf("  Semaphore value: " GREEN "%d" RESET " / %d computers free\n\n", val, MAX_COMPUTERS);

    for (int i = 0; i < MAX_COMPUTERS; i++) {
        if (computer_status[i] == 0) {
            printf("  [PC-%d]  " GREEN "● FREE\n" RESET, i + 1);
        } else {
            int uid = computer_status[i];
            printf("  [PC-%d]  " RED "● OCCUPIED" RESET " by %s (ID:%d)\n",
                   i + 1, users[uid - 1].name, uid);
        }
    }
}
