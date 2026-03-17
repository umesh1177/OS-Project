/* deadlock.c – Deadlock detection via Banker's Algorithm (Safety Check) */
#include "lab.h"

ResourceGraph rg;
static pthread_mutex_t rg_mutex = PTHREAD_MUTEX_INITIALIZER;

void init_resource_graph(void) {
    memset(&rg, 0, sizeof(rg));

    /* Total resources: Resource-0 = 3 units, Resource-1 = 4 units */
    rg.available[0] = 3;
    rg.available[1] = 4;

    /* Max needs per user (what they could ever need) */
    int max_table[MAX_USERS][RESOURCES] = {
        {2, 3},  /* Alice   */
        {3, 2},  /* Bob     */
        {1, 4},  /* Charlie */
        {2, 2},  /* Diana   */
        {3, 1},  /* Evan    */
        {1, 3},  /* Fatima  */
    };
    for (int i = 0; i < MAX_USERS; i++)
        for (int j = 0; j < RESOURCES; j++)
            rg.max_need[i][j] = max_table[i][j];

    printf(GREEN "  [✓] Resource graph initialized. Available: [R0=%d, R1=%d]\n" RESET,
           rg.available[0], rg.available[1]);
}

/* Returns 1 if the system is in a SAFE state (Banker's safety algorithm) */
static int is_safe_state(void) {
    int work[RESOURCES];
    int finish[MAX_USERS];
    int safe_seq[MAX_USERS];
    int count = 0;

    for (int i = 0; i < RESOURCES; i++)
        work[i] = rg.available[i];
    memset(finish, 0, sizeof(finish));

    int progress = 1;
    while (progress) {
        progress = 0;
        for (int i = 0; i < MAX_USERS; i++) {
            if (finish[i]) continue;
            /* Check if request can be satisfied with current work */
            int can = 1;
            for (int j = 0; j < RESOURCES; j++) {
                int need = rg.max_need[i][j] - rg.allocation[i][j];
                if (need > work[j]) { can = 0; break; }
            }
            if (can) {
                for (int j = 0; j < RESOURCES; j++)
                    work[j] += rg.allocation[i][j];
                finish[i] = 1;
                safe_seq[count++] = i + 1; /* user IDs */
                progress = 1;
            }
        }
    }

    /* All finished? Safe state */
    for (int i = 0; i < MAX_USERS; i++)
        if (!finish[i]) return 0;

    /* Print safe sequence */
    printf(GREEN "  ✅ Safe Sequence: " RESET);
    for (int i = 0; i < count; i++) {
        printf("%s%s", users[safe_seq[i] - 1].name, (i < count - 1) ? " → " : "");
    }
    printf("\n");
    return 1;
}

void request_resource(int user_id, int res_id, int amount) {
    if (user_id < 1 || user_id > MAX_USERS || res_id < 0 || res_id >= RESOURCES) return;
    User *u = &users[user_id - 1];

    pthread_mutex_lock(&rg_mutex);

    /* Check if request exceeds max need */
    int need = rg.max_need[user_id - 1][res_id] - rg.allocation[user_id - 1][res_id];
    if (amount > need) {
        printf(RED "  [✗] %s: Request exceeds max need for R%d!\n" RESET, u->name, res_id);
        pthread_mutex_unlock(&rg_mutex);
        return;
    }
    if (amount > rg.available[res_id]) {
        printf(YELLOW "  [!] %s: R%d not available (%d requested, %d available) – WAITING\n"
               RESET, u->name, res_id, amount, rg.available[res_id]);
        rg.request[user_id - 1][res_id] += amount;
        pthread_mutex_unlock(&rg_mutex);
        return;
    }

    /* Pretend-allocate and check safety */
    rg.available[res_id]              -= amount;
    rg.allocation[user_id - 1][res_id]+= amount;

    if (!is_safe_state()) {
        /* UNSAFE – rollback */
        rg.available[res_id]              += amount;
        rg.allocation[user_id - 1][res_id]-= amount;

        printf(RED "  ⚠️  UNSAFE STATE DETECTED! Rolling back allocation for %s on R%d.\n"
               RESET, u->name, res_id);
        rg.request[user_id - 1][res_id] += amount;

        char msg[128];
        snprintf(msg, sizeof(msg),
                 "DEADLOCK !  | Unsafe alloc rolled back – User=%s R%d amount=%d",
                 u->name, res_id, amount);
        log_event(msg);
    } else {
        char msg[128];
        snprintf(msg, sizeof(msg), "RES ALLOC   | %-10s got %d unit(s) of R%d",
                 u->name, amount, res_id);
        log_event(msg);
        printf(GREEN "  [✓] %s allocated %d unit(s) of Resource-%d\n" RESET,
               u->name, amount, res_id);
    }

    pthread_mutex_unlock(&rg_mutex);
}

void release_resource(int user_id, int res_id, int amount) {
    if (user_id < 1 || user_id > MAX_USERS || res_id < 0 || res_id >= RESOURCES) return;
    User *u = &users[user_id - 1];

    pthread_mutex_lock(&rg_mutex);

    if (rg.allocation[user_id - 1][res_id] < amount) {
        printf(RED "  [✗] %s: Cannot release more than allocated!\n" RESET, u->name);
        pthread_mutex_unlock(&rg_mutex);
        return;
    }
    rg.allocation[user_id - 1][res_id] -= amount;
    rg.available[res_id]               += amount;

    char msg[128];
    snprintf(msg, sizeof(msg), "RES RELEASE | %-10s released %d unit(s) of R%d",
             u->name, amount, res_id);
    log_event(msg);
    printf(YELLOW "  [←] %s released %d unit(s) of Resource-%d\n" RESET,
           u->name, amount, res_id);

    pthread_mutex_unlock(&rg_mutex);
}

int detect_deadlock(void) {
    print_separator("DEADLOCK DETECTION (Banker's Algorithm)", RED);

    pthread_mutex_lock(&rg_mutex);

    printf("  Available Resources: R0=%d, R1=%d\n\n",
           rg.available[0], rg.available[1]);

    printf("  %-10s %-16s %-16s %-16s\n",
           "User", "Allocated", "Max Need", "Still Need");
    printf("  %-10s %-16s %-16s %-16s\n",
           "──────────", "────────────────", "────────────────", "────────────────");

    for (int i = 0; i < MAX_USERS; i++) {
        int need0 = rg.max_need[i][0] - rg.allocation[i][0];
        int need1 = rg.max_need[i][1] - rg.allocation[i][1];
        printf("  %-10s [R0=%d R1=%d]     [R0=%d R1=%d]     [R0=%d R1=%d]\n",
               users[i].name,
               rg.allocation[i][0], rg.allocation[i][1],
               rg.max_need[i][0],   rg.max_need[i][1],
               need0, need1);
    }

    printf("\n  Running Safety Algorithm...\n");
    int safe = is_safe_state();

    pthread_mutex_unlock(&rg_mutex);

    if (safe) {
        printf(GREEN "\n  ✅ SYSTEM IS IN A SAFE STATE — No Deadlock Detected.\n" RESET);
        log_event("DEADLOCK CHK| System is in SAFE state – no deadlock");
    } else {
        printf(RED "\n  ❌ DEADLOCK DETECTED! System is in UNSAFE state!\n" RESET);
        printf(RED "     Some processes are waiting for resources that will never be freed.\n" RESET);
        log_event("DEADLOCK CHK| DEADLOCK DETECTED – system in UNSAFE state!");
    }
    return !safe;
}

void display_resource_state(void) {
    print_separator("RESOURCE ALLOCATION TABLE", YELLOW);
    printf("  Available: R0=%d, R1=%d\n\n", rg.available[0], rg.available[1]);
    for (int i = 0; i < MAX_USERS; i++) {
        printf("  %-10s  Allocated:[R0=%d R1=%d]  Waiting:[R0=%d R1=%d]\n",
               users[i].name,
               rg.allocation[i][0], rg.allocation[i][1],
               rg.request[i][0],    rg.request[i][1]);
    }
}
