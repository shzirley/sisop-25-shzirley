#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define MSG_KEY 5678
#define MAX_TEXT 256

typedef struct {
    long mtype;
    char mtext[MAX_TEXT];
} Message;

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: ./worker <id>\n");
        return 1;
    }
    int worker_id = atoi(argv[1]);
    int msgid = msgget(MSG_KEY, 0666);
    if (msgid == -1) {
        perror("msgget failed");
        return 1;
    }

    Message msg;
    int count = 0;
    FILE *log = fopen("system.log", "a");
    if (log == NULL) {
        perror("Failed to open log file");
        return 1;
    }

    // Receive messages for this worker
    while (1) {
        if (msgrcv(msgid, &msg, sizeof(msg.mtext), worker_id, 0) != -1) {
            fprintf(log, "Worker%d: message received\n", worker_id);
            fflush(log);
            count++;
        }

        // Stop when the worker has received enough messages
        if (count >= 3 && worker_id == 1) break; // Worker 1 gets 3 messages
        if (count >= 3 && worker_id == 2) break; // Worker 2 gets 3 messages
        if (count >= 2 && worker_id == 3) break; // Worker 3 gets 2 messages
        if (count >= 2 && worker_id == 4) break; // Worker 4 gets 2 messages
        // Continue for more workers as needed
    }

    fprintf(log, "Worker %d: %d messages\n", worker_id, count);
    fclose(log);
    return 0;
}
