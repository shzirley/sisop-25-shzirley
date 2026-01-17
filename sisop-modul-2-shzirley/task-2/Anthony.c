#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>

#define MAX_LINE 1024
#define INITIAL_CAPACITY 1000
#define GROWTH_FACTOR 2
#define LOG_FILE "log.txt"
#define ZIP_NAME "films.zip"
#define EXTRACTED_FOLDER "film"

struct Film {
    char title[256];
    char director[256];
    char country[128];
    int year;
};

struct Film *films = NULL;
int film_count = 0;
int film_capacity = 0;

void trim_newline(char *str) {
    char *pos;
    if ((pos = strchr(str, '\n')) != NULL)
        *pos = '\0';
}

void init_film_array() {
    film_capacity = INITIAL_CAPACITY;
    films = (struct Film *)malloc(film_capacity * sizeof(struct Film));
    if (!films) {
        fprintf(stderr, "Gagal alokasi memori film\n");
        exit(EXIT_FAILURE);
    }
}

void resize_film_array() {
    film_capacity *= GROWTH_FACTOR;
    films = (struct Film *)realloc(films, film_capacity * sizeof(struct Film));
    if (!films) {
        fprintf(stderr, "Gagal realloc memori film\n");
        exit(EXIT_FAILURE);
    }
}

void parse_csv(const char *filepath) {
    FILE *file = fopen(filepath, "r");
    if (!file) {
        perror("Gagal membuka file CSV");
        exit(EXIT_FAILURE);
    }

    char line[MAX_LINE];
    while (fgets(line, sizeof(line), file)) {
        if (film_count >= film_capacity)
            resize_film_array();

        struct Film film;
        trim_newline(line);
        sscanf(line, "%255[^,],%255[^,],%127[^,],%d", film.title, film.director, film.country, &film.year);

        films[film_count++] = film;
    }
    fclose(file);
}

void log_activity(const char *category, const char *title) {
    FILE *log = fopen(LOG_FILE, "a");
    if (!log) return;
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    fprintf(log, "[%02d:%02d:%02d] Proses mengelompokkan berdasarkan %s: sedang mengelompokkan untuk film %s\n",
            tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec, category, title);
    fclose(log);
}

void *group_by_title(void *arg) {
    mkdir("judul", 0755);
    for (int i = 0; i < film_count; i++) {
        char ch = toupper(films[i].title[0]);
        char fname[20];
        if (isalnum(ch)) snprintf(fname, sizeof(fname), "judul/%c.txt", ch);
        else strcpy(fname, "judul/#.txt");

        FILE *f = fopen(fname, "a");
        if (f) {
            fprintf(f, "%s - %d - %s\n", films[i].title, films[i].year, films[i].director);
            fclose(f);
        }
        log_activity("Abjad", films[i].title);
    }
    return NULL;
}

void *group_by_year(void *arg) {
    mkdir("tahun", 0755);
    for (int i = 0; i < film_count; i++) {
        char fname[30];
        snprintf(fname, sizeof(fname), "tahun/%d.txt", films[i].year);
        FILE *f = fopen(fname, "a");
        if (f) {
            fprintf(f, "%s - %d - %s\n", films[i].title, films[i].year, films[i].director);
            fclose(f);
        }
        log_activity("Tahun", films[i].title);
    }
    return NULL;
}

void handle_grouping() {
    pthread_t t1, t2;
    pthread_create(&t1, NULL, group_by_title, NULL);
    pthread_create(&t2, NULL, group_by_year, NULL);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    printf("Pengelompokan selesai!\n");
}

void handle_report_generation() {
    char report_name[64];
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    snprintf(report_name, sizeof(report_name), "report_%02d%02d%04d.txt",
             tm_info->tm_mday, tm_info->tm_mon + 1, tm_info->tm_year + 1900);

    FILE *report = fopen(report_name, "w");
    if (!report) return;

    struct Stat {
        char country[128];
        int before2000;
        int after2000;
    } stats[1000];
    int stat_count = 0;

    for (int i = 0; i < film_count; i++) {
        char normalized[128];
        int len = 0;
        for (int j = 0; films[i].country[j]; j++) {
            if (films[i].country[j] != ' ')
                normalized[len++] = toupper(films[i].country[j]);
        }
        normalized[len] = '\0';

        int found = 0;
        for (int j = 0; j < stat_count; j++) {
            if (strcmp(stats[j].country, normalized) == 0) {
                if (films[i].year < 2000)
                    stats[j].before2000++;
                else
                    stats[j].after2000++;
                found = 1;
                break;
            }
        }
        if (!found) {
            strcpy(stats[stat_count].country, normalized);
            stats[stat_count].before2000 = (films[i].year < 2000);
            stats[stat_count].after2000 = (films[i].year >= 2000);
            stat_count++;
        }
    }

    for (int i = 0; i < stat_count; i++) {
        fprintf(report, "%d. Negara: %s\n", i + 1, stats[i].country);
        fprintf(report, "Film sebelum 2000: %d\n", stats[i].before2000);
        fprintf(report, "Film setelah 2000: %d\n\n", stats[i].after2000);
    }
    fclose(report);
    printf("Report berhasil dibuat: %s\n", report_name);
}

void download_zip() {
    pid_t pid = fork();
    if (pid == 0) {
        char *argv[] = {
            "wget",
            "--no-check-certificate",
            "-O", ZIP_NAME,
            "https://drive.usercontent.google.com/download?id=12GWsZbSH858h2HExP3x4DfWZB1jLdV-J&export=download&authuser=0",
            NULL
        };
        execvp("wget", argv);
        perror("execvp gagal");
        exit(1);
    } else {
        wait(NULL);
    }
}

void unzip_file(const char *zipname, const char *output_dir) {
    pid_t pid = fork();
    if (pid == 0) {
        char *argv[] = { "unzip", (char *)zipname, "-d", (char *)output_dir, NULL };
        execvp("unzip", argv);
        perror("execvp gagal");
        exit(1);
    } else {
        wait(NULL);
        remove(zipname);
    }
}

int main() {
    int choice;
    char csv_path[256];

    init_film_array();

    while (1) {
        printf("\n=== MENU ===\n");
        printf("1. Download File\n");
        printf("2. Mengelompokkan Film\n");
        printf("3. Membuat Report\n");
        printf("0. Keluar\n> ");
        scanf("%d", &choice);
        getchar();

        switch (choice) {
            case 1:
                download_zip();
                unzip_file(ZIP_NAME, EXTRACTED_FOLDER);
                break;
            case 2:
                printf("Masukkan path file CSV (misal: film/netflix.csv): ");
                fgets(csv_path, sizeof(csv_path), stdin);
                trim_newline(csv_path);
                parse_csv(csv_path);
                handle_grouping();
                break;
            case 3:
                handle_report_generation();
                break;
            case 0:
                free(films);
                exit(0);
            default:
                printf("Pilihan tidak valid.\n");
        }
    }
    return 0;
}
