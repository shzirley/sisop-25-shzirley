#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

void arsipdel(const char *foldernem){
    pid_t pid = fork();

    if (pid == -1){
        perror("Fork failed");
        return;
    }

    if(pid == 0){  // child proccess
        char zip_filename[512]; 
        snprintf(zip_filename, sizeof(zip_filename), "%s.zip", foldernem); 
        
        execlp("zip", "zip", "-r", zip_filename, foldernem, (char *)NULL);
        perror("exec failed");
        exit(1); 
    }else{  // parent proccess
        wait(NULL);
        printf("Folder %s berhasil diarsipkan, yay selesai praktikumnya.\n", foldernem);

        pid_t pid2 = fork();  // fork lagi tuk hps folder

        if(pid2 == -1){
            perror("Fork failed");
            return;
        }

        if(pid2 == 0){  // proses anak kedua untuk menghapus folder
            execlp("rm", "rm", "-rf", foldernem, (char *)NULL); 
            perror("exec failed");
            exit(1); 
        }else{  // proses induk
            wait(NULL); 
            printf("Folder %s dihapus setelah diarsipkan.\n", foldernem);
        }
    }
}

int main(){
    arsipdel("film/FilmAnimasi");
    arsipdel("film/FilmDrama");
    arsipdel("film/FilmHorror");

    return 0;
}
