#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

#define TARGET_FOLDER "ektrakdisini/film"

int main(){
    int pipefd[2];
    pid_t pid;
    DIR *folder;
    struct dirent *entry;
    char *jpgList[1000];
    int count = 0;

    if (pipe(pipefd) == -1){
        perror("Pipe gagal");
        return 1;
    }

    pid = fork();

    if (pid < 0) {
        perror("Fork gagal");
        return 1;
    }

    if (pid == 0) {
        close(pipefd[1]); // Tutup write end

        // Baca jumlah file
        int fileCount;
        read(pipefd[0], &fileCount, sizeof(int));

        if (fileCount == 0) {
            printf("Tidak ada file .jpg ditemukan.\n");
            close(pipefd[0]);
            return 1;
        }

        char buffer[256];
        char *filmList[1000];

        for (int i = 0; i < fileCount; i++) {
            int len;
            read(pipefd[0], &len, sizeof(int));
            read(pipefd[0], buffer, len);
            buffer[len] = '\0';
            filmList[i] = strdup(buffer);
        }

        close(pipefd[0]);

        srand(time(NULL));
        int acak = rand() % fileCount;
        printf("Film for Trabowo & Peddy: ‘%s’\n", filmList[acak]);

        for (int i = 0; i < fileCount; i++) {
            free(filmList[i]);
        }

    } else {// P proc
        close(pipefd[0]); // close read end

        folder = opendir(TARGET_FOLDER);
        if (folder == NULL) {
            perror("Gagal buka folder");
            return 1;
        }

        while ((entry = readdir(folder)) != NULL) {
            if (strstr(entry->d_name, ".jpg") != NULL){
                jpgList[count] = strdup(entry->d_name);
                count++;
            }
        }
        closedir(folder);

        // Kirim jumlah file
        write(pipefd[1], &count, sizeof(int));

        // Kirim tiap nama file
        for (int i = 0; i < count; i++){
            int len = strlen(jpgList[i]);
            write(pipefd[1], &len, sizeof(int));
            write(pipefd[1], jpgList[i], len);
            free(jpgList[i]);
        }

        close(pipefd[1]); // Tutup write end

        wait(NULL); // Tunggu child selesai
    }

    return 0;
}
