#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <ctype.h>
#include <pthread.h>
#include "cJSON.h"

//API, JSON, ZIP

size_t simpen_data(void *isi, size_t ukuran, size_t jumlah, char *penampung) {
    strcat(penampung, isi);
    return ukuran * jumlah;
}

void ambil_data_dari_api(const char *alamat_api, char *penampung_data) {
    CURL *curl;
    CURLcode hasil;

    memset(penampung_data, 0, 10000);

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, alamat_api);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, simpen_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, penampung_data);

        hasil = curl_easy_perform(curl);

        if (hasil != CURLE_OK) {
            fprintf(stderr, "Gagal ambil data: %s\n", curl_easy_strerror(hasil));
        }

        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();
}

void olah_dan_simpan(const char *nama_file, const char *data_json) {
    cJSON *root = cJSON_Parse(data_json);
    if (!root) {
        printf("Error parsing JSON\n");
        return;
    }

    cJSON *data = cJSON_GetObjectItem(root, "data");
    if (!data) {
        printf("Error: Tidak ada data field\n");
        cJSON_Delete(root);
        return;
    }

    const char *title = cJSON_GetObjectItem(data, "title")->valuestring;
    const char *status = cJSON_GetObjectItem(data, "status")->valuestring;

    cJSON *published = cJSON_GetObjectItem(data, "published");
    const char *release_date = published && cJSON_GetObjectItem(published, "from") ?
                               cJSON_GetObjectItem(published, "from")->valuestring :
                               "Unknown";

    cJSON *genres = cJSON_GetObjectItem(data, "genres");
    cJSON *themes = cJSON_GetObjectItem(data, "themes");
    cJSON *authors = cJSON_GetObjectItem(data, "authors");

    char genre_str[512] = "";
    char theme_str[512] = "";
    char author_str[512] = "";

    cJSON *item = NULL;

    if (genres) {
        cJSON_ArrayForEach(item, genres) {
            strcat(genre_str, cJSON_GetObjectItem(item, "name")->valuestring);
            if (item->next) strcat(genre_str, ", ");
        }
    }

    if (themes) {
        cJSON_ArrayForEach(item, themes) {
            strcat(theme_str, cJSON_GetObjectItem(item, "name")->valuestring);
            if (item->next) strcat(theme_str, ", ");
        }
    }

    if (authors) {
        cJSON_ArrayForEach(item, authors) {
            strcat(author_str, cJSON_GetObjectItem(item, "name")->valuestring);
            if (item->next) strcat(author_str, ", ");
        }
    }

    char path_file[256];
    snprintf(path_file, sizeof(path_file), "Manhwa/%s.txt", nama_file);

    FILE *f = fopen(path_file, "w");
    if (f != NULL) {
        fprintf(f, "Title: %s\n", title);
        fprintf(f, "Status: %s\n", status);
        fprintf(f, "Release: %s\n", release_date);
        fprintf(f, "Genre: %s\n", genre_str);
        fprintf(f, "Theme: %s\n", theme_str);
        fprintf(f, "Author: %s\n", author_str);
        fclose(f);
        printf("Data berhasil disimpan di %s\n", path_file);
    } else {
        perror("Gagal buka file");
    }

    cJSON_Delete(root);
}

void buat_folder(const char *nama_folder) {
    struct stat cek = {0};
    if (stat(nama_folder, &cek) == -1) {
        pid_t child = fork();
        if (child == 0) {
            execlp("mkdir", "mkdir", "-p", nama_folder, (char *)NULL);
            perror("Gagal exec mkdir");
            exit(1);
        } else {
            wait(NULL);
        }
    }
}

void buat_nama_zip(const char *nama_file, char *nama_zip) {
    int j = 0;
    for (int i = 0; nama_file[i] != '\0'; i++) {
        if (isupper(nama_file[i])) {
            nama_zip[j++] = nama_file[i];
        }
    }
    nama_zip[j] = '\0';
    strcat(nama_zip, ".zip");
}

void seal_the_scrolls() {
    buat_folder("Archive");

    DIR *dir = opendir("Manhwa");
    if (!dir) {
        perror("Gagal buka folder Manhwa");
        return;
    }

    struct dirent *ent;
    while ((ent = readdir(dir)) != NULL) {
        if (strstr(ent->d_name, ".txt") != NULL) {
            char path_file[512];
            snprintf(path_file, sizeof(path_file), "Manhwa/%s", ent->d_name);

            char nama_zip_kapital[256];
            buat_nama_zip(ent->d_name, nama_zip_kapital);

            char nama_zip[512];
            snprintf(nama_zip, sizeof(nama_zip), "Archive/%s", nama_zip_kapital);

            pid_t pid = fork();
            if (pid == 0) {
                execlp("zip", "zip", "-j", nama_zip, path_file, (char *)NULL);
                perror("Gagal exec zip");
                exit(1);
            } else if (pid < 0) {
                perror("Gagal fork saat zip");
            } else {
                wait(NULL);
            }
        }
    }

    closedir(dir);
}

// Download Heroine dengan Thread

typedef struct {
    char *heroine_name;
    char *link;
    int download_count;
} DownloadTask;


void *download_heroine(void *arg) {
    DownloadTask *task = (DownloadTask *)arg;
    CURL *curl;
    CURLcode res;
    char folder_path[256];
    char file_path[512];

    snprintf(folder_path, sizeof(folder_path), "Heroines/%s", task->heroine_name);
    mkdir("Heroines", 0755);
    mkdir(folder_path, 0755);

    for (int i = 1; i <= task->download_count; i++) {
        curl = curl_easy_init();
        if (curl) {
            snprintf(file_path, sizeof(file_path), "%s/%s_%d.jpg", folder_path, task->heroine_name, i);

            FILE *fp = fopen(file_path, "wb");
            if (!fp) {
                perror("fopen");
                curl_easy_cleanup(curl);
                continue;
            }

            curl_easy_setopt(curl, CURLOPT_URL, task->link);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL); // Default write
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
            curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0");

            res = curl_easy_perform(curl);

            if (res != CURLE_OK) {
                fprintf(stderr, "Failed to download %s: %s\n", file_path, curl_easy_strerror(res));
                fclose(fp);
                remove(file_path); // Hapus file rusak
            } else {
                fclose(fp);

                // Validasi ukuran file
                struct stat st;
                if (stat(file_path, &st) == 0 && st.st_size > 0) {
                    printf("Downloaded: %s (%ld bytes)\n", file_path, st.st_size);
                } else {
                    printf("File corrupted or empty: %s\n", file_path);
                    remove(file_path);
                }
            }

            curl_easy_cleanup(curl);
        }
    }

    pthread_exit(NULL);
}



void zip_and_cleanup() {
    char *manhwa_codes[] = {"MMDW", "TVLA", "NIOCTP", "DWCWD"};
    char *heroines[] = {"Lia", "Artezia", "Adelia", "Roselia"};
    int count = 4;

    // 1. Buat folder Archive/Images
    pid_t pid = fork();
    if (pid == 0) {
        char *args[] = {"mkdir", "-p", "Archive/Images", NULL};
        execvp("mkdir", args);
        perror("exec mkdir");
        exit(EXIT_FAILURE);
    } else {
        wait(NULL);
    }

    // 2. Zip setiap folder heroine
    for (int i = 0; i < count; i++) {
        pid_t zip_pid = fork();
        if (zip_pid == 0) {
            char zipname[256];
            snprintf(zipname, sizeof(zipname), "Archive/Images/%s_%s.zip", manhwa_codes[i], heroines[i]);

            char foldername[256];
            snprintf(foldername, sizeof(foldername), "Heroines/%s", heroines[i]);

            char *args[] = {"zip", "-r", zipname, foldername, NULL};
            execvp("zip", args);
            perror("exec zip");
            exit(EXIT_FAILURE);
        } else {
            wait(NULL);
        }
    }

    // hapus folder heroine berdasarkan abjad
    char *to_delete[] = {"Adelia", "Artezia", "Lia", "Roselia"};
    for (int i = 0; i < count; i++) {
        pid_t del_pid = fork();
        if (del_pid == 0) {
            char folderpath[256];
            snprintf(folderpath, sizeof(folderpath), "Heroines/%s", to_delete[i]);

            char *args[] = {"rm", "-r", folderpath, NULL};
            execvp("rm", args);
            perror("exec rm");
            exit(EXIT_FAILURE);
        } else {
            wait(NULL);
        }
    }
}

int main() {

    curl_global_init(CURL_GLOBAL_DEFAULT);
    // === Bagian 1: Proses API dan JSON ===
    const char *daftar_api[] = {
        "https://api.jikan.moe/v4/manga/168827",
        "https://api.jikan.moe/v4/manga/147205",
        "https://api.jikan.moe/v4/manga/169731",
        "https://api.jikan.moe/v4/manga/175521"
    };

    const char *judul_manhwa[] = {
        "Mistaken_as_the_Monster_Dukes_Wife",
        "The_Villainess_Lives_Again",
        "No_I_Only_Charmed_the_Princess",
        "Darling_Why_Cant_We_Divorce"
    };

    char tempat_data[10000];
    buat_folder("Manhwa");

    for (int i = 0; i < 4; i++) {
        printf("Ngambil data buat manhwa ke-%d: %s...\n", i + 1, judul_manhwa[i]);
        ambil_data_dari_api(daftar_api[i], tempat_data);
        olah_dan_simpan(judul_manhwa[i], tempat_data);
    }

    seal_the_scrolls();

    // === Bagian 2: Download gambar heroine pakai thread ===
    char *heroine_names[] = {"Lia", "Artezia", "Adelia", "Roselia"};
    char *links[] = {
        "https://i.pinimg.com/736x/11/d9/ad/11d9ad85a47892f5fd979a0209162377.jpg",
        "https://i.pinimg.com/736x/28/4e/98/284e982667609520530e40bc4e13e3a9.jpg",
        "https://i.pinimg.com/736x/21/21/62/212162a0e1264d20b749715dee60fda7.jpg",
        "https://i.pinimg.com/736x/3d/fd/6d/3dfd6dd4f002583585cd821e38734b13.jpg"
    };

    int downloads[] = {3, 6, 4, 10}; // sesuai bulan rilis

    int heroine_count = sizeof(heroine_names) / sizeof(heroine_names[0]);
    pthread_t threads[heroine_count];
    DownloadTask tasks[heroine_count];

    for (int i = 0; i < 4; i++) {
        tasks[i].heroine_name = heroine_names[i];
        tasks[i].link = links[i];
        tasks[i].download_count = downloads[i];
    

        if(pthread_create(&threads[i], NULL, download_heroine, (void *)&tasks[i]));
        
        // tunggu dulu thread selesai baru lanjut heroine berikutnya
        pthread_join(threads[i], NULL);
    }

    printf("Semua download selesai!\n");

    // setelah semua download thread selesai
    zip_and_cleanup();


    return 0;
}
