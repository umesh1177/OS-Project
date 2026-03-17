/* printer.c – Priority-based print queue with semaphore */
#include "lab.h"

sem_t      printer_sem;
PrintQueue print_queue;
static int job_counter = 1;

void init_print_queue(void) {
    memset(&print_queue, 0, sizeof(print_queue));
    pthread_mutex_init(&print_queue.lock, NULL);
    sem_init(&printer_sem, 0, MAX_PRINTERS);
    printf(GREEN "  [✓] Print queue initialized (%d printers available)\n" RESET,
           MAX_PRINTERS);
}

/* Sort queue by priority (lower number = higher priority), then FCFS */
static void sort_queue(void) {
    for (int i = 0; i < print_queue.count - 1; i++) {
        for (int j = 0; j < print_queue.count - i - 1; j++) {
            PrintJob *a = &print_queue.jobs[j];
            PrintJob *b = &print_queue.jobs[j + 1];
            if (a->priority > b->priority ||
                (a->priority == b->priority && a->submit_time > b->submit_time)) {
                PrintJob tmp = *a; *a = *b; *b = tmp;
            }
        }
    }
}

void submit_print_job(int user_id, const char *filename, int pages, int priority) {
    if (user_id < 1 || user_id > MAX_USERS) return;
    User *u = &users[user_id - 1];

    pthread_mutex_lock(&print_queue.lock);

    if (print_queue.count >= MAX_QUEUE) {
        printf(RED "  [✗] Print queue full! Job from %s rejected.\n" RESET, u->name);
        pthread_mutex_unlock(&print_queue.lock);
        return;
    }

    PrintJob *job = &print_queue.jobs[print_queue.count];
    job->job_id      = job_counter++;
    job->user_id     = user_id;
    job->pages       = pages;
    job->priority    = priority;
    job->submit_time = time(NULL);
    strncpy(job->filename, filename, MAX_NAME_LEN - 1);

    print_queue.count++;
    sort_queue();
    pthread_mutex_unlock(&print_queue.lock);

    char msg[160];
    snprintf(msg, sizeof(msg),
             "PRINT SUBMIT| Job#%d by %-10s file=%-15s pages=%d priority=%d",
             job->job_id, u->name, filename, pages, priority);
    log_event(msg);
    printf(GREEN "  [✓] Print job #%d submitted by %s (%d pages, priority %d)\n" RESET,
           job->job_id, u->name, pages, priority);
}

void process_print_queue(void) {
    print_separator("PRINT QUEUE PROCESSING", MAGENTA);

    if (print_queue.count == 0) {
        printf("  No jobs in queue.\n");
        return;
    }

    printf("  Processing %d job(s) across %d printer(s)...\n\n",
           print_queue.count, MAX_PRINTERS);

    int printer_id = 1;

    while (print_queue.count > 0) {
        /* Acquire a printer (semaphore) */
        if (sem_trywait(&printer_sem) != 0) {
            printf(YELLOW "  [~] All printers busy, waiting...\n" RESET);
            sleep_ms(300);
            continue;
        }

        pthread_mutex_lock(&print_queue.lock);
        if (print_queue.count == 0) {
            sem_post(&printer_sem);
            pthread_mutex_unlock(&print_queue.lock);
            break;
        }

        /* Take highest-priority job (front after sort) */
        PrintJob job = print_queue.jobs[0];
        /* Shift queue */
        for (int i = 0; i < print_queue.count - 1; i++)
            print_queue.jobs[i] = print_queue.jobs[i + 1];
        print_queue.count--;
        pthread_mutex_unlock(&print_queue.lock);

        int pid = ((printer_id - 1) % MAX_PRINTERS) + 1;
        printf(MAGENTA "  🖨️  [Printer-%d]" RESET " Printing Job#%d | File: %-15s | "
               "User: %-10s | Pages: %d\n",
               pid, job.job_id, job.filename, users[job.user_id - 1].name, job.pages);

        sleep_ms(200 + job.pages * 50); /* simulate print time */

        char msg[160];
        snprintf(msg, sizeof(msg),
                 "PRINT DONE  | Job#%d  Printer-%d  User=%-10s  pages=%d",
                 job.job_id, pid, users[job.user_id - 1].name, job.pages);
        log_event(msg);
        printf(GREEN "  [✓] Job#%d completed on Printer-%d\n" RESET, job.job_id, pid);

        sem_post(&printer_sem);
        printer_id++;
    }
    printf(GREEN "\n  All print jobs completed!\n" RESET);
}

void display_print_queue(void) {
    print_separator("PRINT QUEUE STATUS", MAGENTA);
    pthread_mutex_lock(&print_queue.lock);
    if (print_queue.count == 0) {
        printf("  Queue is empty.\n");
    } else {
        printf("  %-6s %-12s %-18s %-8s %-8s\n",
               "Job#", "User", "Filename", "Pages", "Priority");
        printf("  %-6s %-12s %-18s %-8s %-8s\n",
               "────", "────────────", "──────────────────", "────────", "────────");
        for (int i = 0; i < print_queue.count; i++) {
            PrintJob *j = &print_queue.jobs[i];
            const char *pstr = j->priority == 1 ? RED "High  " RESET
                             : j->priority == 2 ? YELLOW "Medium" RESET
                             :                    GREEN "Low   " RESET;
            printf("  %-6d %-12s %-18s %-8d %s\n",
                   j->job_id, users[j->user_id - 1].name, j->filename, j->pages, pstr);
        }
    }
    pthread_mutex_unlock(&print_queue.lock);
}
