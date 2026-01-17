#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

int file_exists(const char *filename){
    return access(filename, F_OK) == 0;
}

int folder_exists(const char *foldername){
    struct stat st;
    return stat(foldername, &st) == 0 && S_ISDIR(st.st_mode);
}


void donlotfile(const char *file_key, const char *output_name){
    pid_t pid = fork();
    if (pid == 0){
        // child proses: jalankan wget
        execlp("wget", "wget",
               "--no-check-certificate", //biar gk gagal karena sertif
               "--content-disposition",
               "-O", output_name,
               ("https://drive.google.com/uc?export=download&id=1nP5kjCi9ReDk5ILgnM7UCnrQwFH67Z9B"),
               NULL);
        perror("gagal menjalankan wget");
        exit(1);
    } else if (pid > 0){
        wait(NULL);//tunggu sampai selesai
        printf("succeed download: %s\n", output_name);
    } else perror("fork gagal");
}

void extract_zip(const char *namafile, const char *dest_folder){
    pid_t pid = fork();
    if (pid == 0){
        // child proses: unzip file
        execl("/usr/bin/unzip", "unzip", namafile, "-d", dest_folder, NULL);
        perror("gagal menjalankan unzip");
        exit(1);
    } else if (pid > 0){
        wait(NULL); // tunggu unzip selesai
        printf("ekstraksi berhasil ke folder: %s\n", dest_folder);
    } else perror("fork gagal");
    
}

int main(){
    const char *file_key = "1nP5kjCi9ReDk5ILgnM7UCnrQwFH67Z9B"; 
    const char *namafile = "film.zip";
    const char *output_dir = "ektrakdisini";

//bwat donlot
    donlotfile(file_key, namafile);


//janlup diekstrak yyy
    extract_zip(namafile, output_dir);

    return 0;
}
