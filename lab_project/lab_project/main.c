/* main.c – Lab Management System: Full Simulation Driver */
#include "lab.h"

/* Forward declarations for init functions defined in other files */
void init_computers(void);

/* ═══════════════════════════════════════════════
   DEMO SIMULATION – showcases all OS concepts
   ═══════════════════════════════════════════════ */
static void run_simulation(void) {

    /* ── Phase 1: User Logins ── */
    print_separator("PHASE 1: USER LOGIN SIMULATION", GREEN);
    for (int i = 1; i <= MAX_USERS; i++) {
        user_login(i);
        sleep_ms(100);
    }
    display_users();

    /* ── Phase 2: Semaphore-based Computer Allocation ── */
    print_separator("PHASE 2: COMPUTER RESOURCE ALLOCATION (Semaphore)", CYAN);
    printf("  Only %d computers for %d users — semaphore controls access.\n\n",
           MAX_COMPUTERS, MAX_USERS);

    /* All users try to get a computer */
    for (int i = 1; i <= MAX_USERS; i++) {
        allocate_computer(i);
        sleep_ms(80);
    }
    display_computers();

    /* ── Phase 3: Print Queue ── */
    print_separator("PHASE 3: PRINT QUEUE SUBMISSION", MAGENTA);

    /* Submit jobs with varying priorities */
    submit_print_job(1, "thesis_final.pdf",   12, 1); /* High  */
    submit_print_job(2, "assignment3.docx",    4, 3); /* Low   */
    submit_print_job(3, "lab_report.txt",      7, 2); /* Med   */
    submit_print_job(4, "resume_v2.pdf",       2, 1); /* High  */
    submit_print_job(5, "notes.txt",           1, 3); /* Low   */
    submit_print_job(6, "project_slides.pptx", 20,2); /* Med   */
    submit_print_job(1, "appendix.pdf",        5, 2); /* Med   */
    submit_print_job(3, "budget.xlsx",         3, 1); /* High  */

    display_print_queue();
    printf("\n");
    process_print_queue();

    /* ── Phase 4: Shared File Access ── */
    print_separator("PHASE 4: SHARED FILE ACCESS (Mutex Lock)", BLUE);
    write_shared_file(1, "Alice's lab notes – experiment #7");
    write_shared_file(3, "Charlie's simulation results v1.2");
    write_shared_file(5, "Evan's dataset – processed OK");
    read_shared_file(2);

    /* ── Phase 5: Resource Allocation + Deadlock Detection ── */
    print_separator("PHASE 5: RESOURCE ALLOCATION (Banker's Algorithm)", RED);
    printf("  Total: R0=3 units, R1=4 units across all users\n\n");

    /* Normal allocations */
    request_resource(1, 0, 2);  /* Alice  wants 2 units of R0 */
    request_resource(2, 1, 2);  /* Bob    wants 2 units of R1 */
    request_resource(3, 0, 1);  /* Charlie wants 1 unit  of R0 */
    request_resource(4, 1, 1);  /* Diana  wants 1 unit  of R1 */

    display_resource_state();
    detect_deadlock();

    /* Intentionally unsafe request to trigger deadlock warning */
    print_separator("PHASE 5b: INDUCING UNSAFE STATE", RED);
    printf("  Pushing allocations to edge of safety...\n\n");
    request_resource(2, 0, 1);  /* Bob requests more R0 – may make system unsafe */
    request_resource(5, 1, 1);  /* Evan requests R1 */
    request_resource(6, 0, 1);  /* Fatima requests R0 – likely triggers unsafe */

    detect_deadlock(); /* Final deadlock check */

    /* ── Phase 6: Resource Release + User Logouts ── */
    print_separator("PHASE 6: CLEANUP – RELEASE & LOGOUT", YELLOW);

    release_resource(1, 0, 2);
    release_resource(2, 1, 2);
    release_resource(3, 0, 1);
    release_resource(4, 1, 1);
    release_resource(5, 1, 1);

    detect_deadlock(); /* Should be safe now */

    /* Release all computers */
    for (int i = 1; i <= MAX_USERS; i++)
        release_computer(i);

    display_computers();

    /* Logout all users */
    for (int i = 1; i <= MAX_USERS; i++) {
        user_logout(i);
        sleep_ms(60);
    }

    display_users();

    /* ── Final Summary ── */
    print_separator("SIMULATION COMPLETE", GREEN);
    printf(GREEN "  ✅ All modules demonstrated successfully:\n" RESET);
    printf("     • User Login/Logout Simulation\n");
    printf("     • Semaphore-based Computer Allocation\n");
    printf("     • Priority Print Queue Scheduling\n");
    printf("     • Mutex-protected Shared File Access\n");
    printf("     • Deadlock Detection via Banker's Algorithm\n");
    printf(CYAN "\n  📄 Activity log saved to: %s\n" RESET, LOG_FILE);
    printf(CYAN "  📄 Shared file saved to:   %s\n\n" RESET, SHARED_FILE);
}

/* ═══════════════════════════════════════════
   INTERACTIVE MENU
   ═══════════════════════════════════════════ */
static void interactive_menu(void) {
    int choice, uid, rid, amt;
    char buf[64];

    while (1) {
        printf(MAGENTA "\n╔════════════════════════════════════╗\n" RESET);
        printf(MAGENTA "║         INTERACTIVE MENU           ║\n" RESET);
        printf(MAGENTA "╠════════════════════════════════════╣\n" RESET);
        printf("║  1. Display all users              ║\n");
        printf("║  2. Login a user                   ║\n");
        printf("║  3. Logout a user                  ║\n");
        printf("║  4. Allocate computer               ║\n");
        printf("║  5. Release computer               ║\n");
        printf("║  6. Display computers              ║\n");
        printf("║  7. Submit print job               ║\n");
        printf("║  8. Process print queue            ║\n");
        printf("║  9. Write to shared file           ║\n");
        printf("║ 10. Read shared file               ║\n");
        printf("║ 11. Request resource               ║\n");
        printf("║ 12. Release resource               ║\n");
        printf("║ 13. Detect deadlock                ║\n");
        printf("║ 14. Display resource state         ║\n");
        printf("║  0. Exit                           ║\n");
        printf(MAGENTA "╚════════════════════════════════════╝\n" RESET);
        printf("  Enter choice: ");
        if (scanf("%d", &choice) != 1) break;

        switch (choice) {
            case 0:
                printf(GREEN "  Goodbye!\n" RESET);
                return;
            case 1: display_users(); break;
            case 2:
                printf("  User ID (1-%d): ", MAX_USERS); scanf("%d", &uid);
                user_login(uid); break;
            case 3:
                printf("  User ID (1-%d): ", MAX_USERS); scanf("%d", &uid);
                user_logout(uid); break;
            case 4:
                printf("  User ID (1-%d): ", MAX_USERS); scanf("%d", &uid);
                allocate_computer(uid); break;
            case 5:
                printf("  User ID (1-%d): ", MAX_USERS); scanf("%d", &uid);
                release_computer(uid); break;
            case 6: display_computers(); break;
            case 7:
                printf("  User ID (1-%d): ", MAX_USERS); scanf("%d", &uid);
                printf("  Filename: "); scanf("%63s", buf);
                int pages, pri;
                printf("  Pages: "); scanf("%d", &pages);
                printf("  Priority (1=High,2=Med,3=Low): "); scanf("%d", &pri);
                submit_print_job(uid, buf, pages, pri); break;
            case 8: process_print_queue(); break;
            case 9:
                printf("  User ID (1-%d): ", MAX_USERS); scanf("%d", &uid);
                printf("  Data to write: "); scanf(" %63[^\n]", buf);
                write_shared_file(uid, buf); break;
            case 10:
                printf("  User ID (1-%d): ", MAX_USERS); scanf("%d", &uid);
                read_shared_file(uid); break;
            case 11:
                printf("  User ID (1-%d): ", MAX_USERS); scanf("%d", &uid);
                printf("  Resource ID (0 or 1): "); scanf("%d", &rid);
                printf("  Amount: "); scanf("%d", &amt);
                request_resource(uid, rid, amt); break;
            case 12:
                printf("  User ID (1-%d): ", MAX_USERS); scanf("%d", &uid);
                printf("  Resource ID (0 or 1): "); scanf("%d", &rid);
                printf("  Amount: "); scanf("%d", &amt);
                release_resource(uid, rid, amt); break;
            case 13: detect_deadlock(); break;
            case 14: display_resource_state(); break;
            default: printf(RED "  Invalid choice.\n" RESET);
        }
    }
}

/* ═══════════════════════════════════════════
   MAIN ENTRY POINT
   ═══════════════════════════════════════════ */
int main(int argc, char *argv[]) {
    print_banner();

    /* Initialize all subsystems */
    init_logger();
    init_users();
    init_computers();
    init_print_queue();
    init_shared_file();
    init_resource_graph();

    if (argc > 1 && strcmp(argv[1], "-i") == 0) {
        /* Interactive mode */
        printf(CYAN "\n  Interactive mode enabled.\n" RESET);
        interactive_menu();
    } else {
        /* Automatic demo simulation */
        printf(CYAN "\n  Running full simulation demo...\n"
               "  (Use './lab_mgmt -i' for interactive mode)\n" RESET);
        sleep_ms(400);
        run_simulation();
    }

    close_logger();

    /* Cleanup semaphores */
    sem_destroy(&computer_sem);
    sem_destroy(&printer_sem);

    return 0;
}
