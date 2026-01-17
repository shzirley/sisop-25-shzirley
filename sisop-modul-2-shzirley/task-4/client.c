#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define SHM_KEY 1234
#define MAX_MESSAGE 256

typedef struct {
    char message[MAX_MESSAGE];
    int count;
    int ready;
} SharedData;

int main() {
    char input[300];
    printf("input (format: Pesan;Jumlah): ");
    fgets(input, sizeof(input), stdin);
    char *token = strtok(input, ";");
    char *pesan = token;
    token = strtok(NULL, ";");
    int jumlah = atoi(token);
    int shmid = shmget(SHM_KEY, sizeof(SharedData), IPC_CREAT | 0666);
    SharedData *data = (SharedData *)shmat(shmid, NULL, 0);
    strcpy(data->message, pesan);
    data->count = jumlah;
    data->ready = 1;
    FILE *log = fopen("system.log", "a");
    fprintf(log, "Message from client: %s\n", data->message);
    fprintf(log, "Message count: %d\n", data->count);
    fclose(log);
    shmdt(data);
    return 0;
}
