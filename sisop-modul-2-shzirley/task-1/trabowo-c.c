#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>

#define SOURCE_FOLDER "ektrakdisini/film"
#define DEST_FOLDER "film"

pthread_mutex_t log_mutex;
int turn = 0;  // 0 for Trabowo, 1 for Peddy

// menyimpan info file
typedef struct {
    char filename[256];
    int nomor;
} Film;

// ambil genre dari nama file
const char *get_genre(const char *filename){
    static char genre[50];
    const char *underscore = strrchr(filename, '_');
    const char *dot = strrchr(filename, '.');

    if (underscore && dot && dot > underscore){
        size_t len = dot - underscore - 1;
        strncpy(genre, underscore + 1, len);
        genre[len] = '\0';
        return genre;
    }
    return NULL;
}

// kapitalisasi huruf pertama genre
void kapital_genre(const char *src, char *dest){
    if (!src || strlen(src) == 0) {
        dest[0] = '\0';
        return;
    }
    strcpy(dest, src);
    dest[0] = toupper(dest[0]);
}

// f mencatat log
void catat_log(const char *filename, const char *person, const char *genre, int my_turn){
    while (turn != my_turn) usleep(100);
    pthread_mutex_lock(&log_mutex);

    FILE *log = fopen("recap.txt", "a");
    if (!log) {
        pthread_mutex_unlock(&log_mutex);
        return;
    }

    time_t now = time(NULL);
    char time_str[100];
    strftime(time_str, sizeof(time_str), "%d-%m-%Y %H:%M:%S", localtime(&now));

    fprintf(log, "[%s] %s: %s telah dipindahkan ke Film%s\n", time_str, person, filename, genre);
    fclose(log);

    turn = 1 - my_turn;
    pthread_mutex_unlock(&log_mutex);
}

// move n catat log
void pindah_film(Film film, const char *person, int my_turn){
    const char *genre = get_genre(film.filename);
    if (!genre) return;

    char capital_genre[100];
    kapital_genre(genre, capital_genre);

    char source[512], folder[512], destination[1024];
    snprintf(source, sizeof(source), "%s/%s", SOURCE_FOLDER, film.filename);
    snprintf(folder, sizeof(folder), "%s/Film%s", DEST_FOLDER, capital_genre);
    mkdir(folder, 0755);
    snprintf(destination, sizeof(destination), "%s/%s", folder, film.filename);

    rename(source, destination);
    catat_log(film.filename, person, capital_genre, my_turn);
}

//compare untuk sorting berdasarkan nomor
int compare_film(const void *a, const void *b){
    Film *fa = (Film *)a;
    Film *fb = (Film *)b;
    return fa->nomor - fb->nomor;
}

// untuk thread
typedef struct{
    Film *films;
    int total;
} ThreadArg;

void *p_trabowo(void *arg){
    ThreadArg *args = (ThreadArg *)arg;
    for (int i = 0; i < args->total; i += 2){
        pindah_film(args->films[i], "Trabowo", 0);
    }
    return NULL;
}

void *p_peddy(void *arg){
    ThreadArg *args = (ThreadArg *)arg;
    for (int i = args->total - 1; i > 0; i -= 2){
        pindah_film(args->films[i], "Peddy", 1);
    }
    return NULL;
}

int hitung_file(const char *folder){
    DIR *dir = opendir(folder);
    if (!dir) return 0;
    int count = 0;
    struct dirent *entry;
    char path[600];
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
        snprintf(path, sizeof(path), "%s/%s", folder, entry->d_name);
        struct stat st;
        if (stat(path, &st) == 0 && S_ISREG(st.st_mode)) count++;
    }
    closedir(dir);
    return count;
}

int ambil_nomor(const char *filename){
    int num = 0;
    sscanf(filename, "%d_", &num);
    return num;
}

int main(){
    mkdir(DEST_FOLDER, 0755);

    DIR *dir = opendir(SOURCE_FOLDER);
    if (!dir){
        perror("cannot open source folder bro");
        return 1;
    }

    Film films[1000];
    int total = 0;
    struct dirent *entry;

    while ((entry = readdir(dir)) != NULL){
        if (strstr(entry->d_name, ".jpg")){
            strcpy(films[total].filename, entry->d_name);
            films[total].nomor = ambil_nomor(entry->d_name);
            total++;
        }
    }
    closedir(dir);

    qsort(films, total, sizeof(Film), compare_film);

    pthread_mutex_init(&log_mutex, NULL);

    pthread_t t1, t2;
    ThreadArg args = { .films = films, .total = total };

    pthread_create(&t1, NULL, p_trabowo, &args);
    pthread_create(&t2, NULL, p_peddy, &args);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    FILE *f = fopen("total.txt", "w");
    int h = hitung_file("film/FilmHorror");
    int a = hitung_file("film/FilmAnimasi");
    int d = hitung_file("film/FilmDrama");

    fprintf(f, "Jumlah film horror: %d\n", h);
    fprintf(f, "Jumlah film animasi: %d\n", a);
    fprintf(f, "Jumlah film drama: %d\n", d);

    const char *genre_terbanyak;

    if (h >= a && h >= d){
        genre_terbanyak = "horror";
    } else if (a >= h && a >= d){
        genre_terbanyak = "animasi";
    } else genre_terbanyak = "drama";

    fprintf(f, "Genre dengan jumlah film terbanyak: %s\n", genre_terbanyak);


    fclose(f);
    pthread_mutex_destroy(&log_mutex);
    return 0;
}
