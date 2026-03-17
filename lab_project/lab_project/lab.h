#ifndef LAB_H
#define LAB_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

/* ─── Lab Configuration ─── */
#define MAX_USERS         6
#define MAX_COMPUTERS     3
#define MAX_PRINTERS      2
#define MAX_QUEUE         10
#define MAX_NAME_LEN      32
#define MAX_FILES         5
#define SHARED_FILE       "shared_lab_file.txt"
#define LOG_FILE          "lab_activity.log"
#define RESOURCES         2   /* for deadlock detection */

/* ─── ANSI Color Codes ─── */
#define RED     "\033[1;31m"
#define GREEN   "\033[1;32m"
#define YELLOW  "\033[1;33m"
#define BLUE    "\033[1;34m"
#define MAGENTA "\033[1;35m"
#define CYAN    "\033[1;36m"
#define WHITE   "\033[1;37m"
#define RESET   "\033[0m"

/* ─── User Structure ─── */
typedef struct {
    int  id;
    char name[MAX_NAME_LEN];
    int  logged_in;
    int  computer_id;   /* -1 if none */
    int  priority;      /* 1=High, 2=Med, 3=Low */
    time_t login_time;
} User;

/* ─── Print Job Structure ─── */
typedef struct {
    int  job_id;
    int  user_id;
    char filename[MAX_NAME_LEN];
    int  pages;
    int  priority;
    time_t submit_time;
} PrintJob;

/* ─── Print Queue ─── */
typedef struct {
    PrintJob jobs[MAX_QUEUE];
    int      count;
    pthread_mutex_t lock;
} PrintQueue;

/* ─── Resource Allocation for Deadlock ─── */
typedef struct {
    int allocation[MAX_USERS][RESOURCES];
    int request[MAX_USERS][RESOURCES];
    int available[RESOURCES];
    int max_need[MAX_USERS][RESOURCES];
} ResourceGraph;

/* ─── Globals (declared in main.c) ─── */
extern User        users[MAX_USERS];
extern int         computer_status[MAX_COMPUTERS]; /* 0=free, user_id if occupied */
extern sem_t       computer_sem;
extern sem_t       printer_sem;
extern PrintQueue  print_queue;
extern pthread_mutex_t file_mutex;
extern pthread_mutex_t log_mutex;
extern ResourceGraph rg;
extern FILE       *log_fp;

/* ─── Function Prototypes ─── */

/* login.c */
void  init_users(void);
int   user_login(int user_id);
void  user_logout(int user_id);
void  display_users(void);

/* computer.c */
int   allocate_computer(int user_id);
void  release_computer(int user_id);
void  display_computers(void);

/* printer.c */
void  init_print_queue(void);
void  submit_print_job(int user_id, const char *filename, int pages, int priority);
void  process_print_queue(void);
void  display_print_queue(void);

/* file_access.c */
void  init_shared_file(void);
void  write_shared_file(int user_id, const char *data);
void  read_shared_file(int user_id);

/* deadlock.c */
void  init_resource_graph(void);
void  request_resource(int user_id, int res_id, int amount);
void  release_resource(int user_id, int res_id, int amount);
int   detect_deadlock(void);
void  display_resource_state(void);

/* logger.c */
void  init_logger(void);
void  log_event(const char *event);
void  close_logger(void);

/* utils.c */
void  print_banner(void);
void  print_separator(const char *title, const char *color);
void  current_time_str(char *buf, size_t len);
void  sleep_ms(int ms);

#endif /* LAB_H */
