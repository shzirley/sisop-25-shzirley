#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>

#define SHM_KEY 1234
#define MSG_KEY 5678
#define MAX_MESSAGE 256
#define MAX_TEXT 256

typedef struct {
    char message[MAX_MESSAGE];
    int count;
    int ready;
} SharedData;

typedef struct {
    long mtype;
    char mtext[MAX_TEXT];
} Message;

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: ./loadbalancer <n>\n");
        return 1;
    }
    int NUM_WORKERS = atoi(argv[1]);
    if (NUM_WORKERS <= 0) {
        printf("Jumlah worker harus lebih dari 0\n");
        return 1;
    }

    // Get shared memory and message queue
    int shmid = shmget(SHM_KEY, sizeof(SharedData), 0666);
    SharedData *data = (SharedData *)shmat(shmid, NULL, 0);
    while (data->ready != 1) usleep(10000);

    int msgid = msgget(MSG_KEY, IPC_CREAT | 0666);

    FILE *log = fopen("system.log", "a");
    if (log == NULL) {
        perror("Failed to open log file");
        return 1;
    }

    int messages_per_worker = data->count / NUM_WORKERS;  // Number of messages each worker should process
    int remaining_messages = data->count % NUM_WORKERS;   // Handle leftover messages

    for (int i = 0; i < data->count; i++) {
        // Round-robin message distribution
        int worker_id = (i % NUM_WORKERS) + 1;

        fprintf(log, "Received at lb: %s (#message %d)\n", data->message, i + 1);

        Message msg;
        msg.mtype = worker_id;
        strcpy(msg.mtext, data->message);
        msgsnd(msgid, &msg, sizeof(msg.mtext), 0);

        usleep(10000); // Small delay to allow workers time to process the message
    }

    fclose(log);
    shmdt(data);
    return 0;
}
