/* process.h
 * Public API for the Process Management Module
 */

#ifndef PROCESS_H
#define PROCESS_H

#include <stddef.h>

typedef struct Process {
    int   pid;          // Process ID
    char  user[32];     // Name of the user
    char  state;        /* R, S, Z, T, ... */
    long  mem_kb;       /* Memory usage (VmRSS) in kB */
    char  cmd[256];     /* Command / Process name */
    struct Process *next;
} Process;

/* Reads all processes from /proc and returns a linked list */
Process *read_processes(void);

/* Sorts processes by memory usage (descending order) */
Process *sort_by_mem(Process *head);

/* Prints the first ~50 processes (debug use) */
void print_processes(Process *head, const char *machine_name);

/* Frees the entire process list */
void free_processes(Process *head);

/* Serializes process list into a text buffer:
   Format: pid;user;mem;state;cmd\n
*/
size_t serialize_processes(Process *head, char *buffer, size_t bufsize);

/* Deserializes a text buffer into a linked list of Process */
Process *deserialize_processes(const char *buffer);

int kill_process_soft(int pid);  // avec SIGTERM
int kill_process_hard(int pid); // avec SIGKILL
int pause_process(int pid); // avec SIGSTOP
int continue_process(int pid); // avec SIGCONT


#endif /* PROCESS_H */
