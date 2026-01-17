[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-22041afd0340ce965d47ae6ef1cefeee28c7c493a6346c4f15d667ab976d596c.svg)](https://classroom.github.com/a/9LcL5VTQ)
|    NRP     |      Name      |
| :--------: | :------------: |
| 5025241223 | Muhammad Akhdan Alwaafy |
| 5025241226 | Angela Vania Sugiyono |

# Praktikum Modul 2 _(Module 2 Lab Work)_

</div>

### Daftar Soal _(Task List)_

- [Task 1 - Trabowo & Peddy Movie Night](/task-1/)

- [Task 2 - Organize and Analyze Anthony's Favorite Films](/task-2/)

- [Task 3 - Cella’s Manhwa](/task-3/)

- [Task 4 - Pipip's Load Balancer](/task-4/)

### Laporan Resmi Praktikum Modul 2 _(Module 2 Lab Work Report)_

---

# **A. Ekstraksi File ZIP (trabowo-a.c)**

## **1. Tujuan**
Tujuan dari program ini adalah untuk:
- Mengunduh file dari Google Drive secara otomatis menggunakan perintah `wget`.
- Mengekstrak file ZIP hasil unduhan ke dalam folder tujuan menggunakan perintah `unzip`.
- Menggunakan proses child (`fork`) agar proses berjalan terpisah dari proses utama.
- Memberikan solusi otomatisasi untuk pengelolaan file dari sumber eksternal berbasis command-line.

---

## **2. Code**
Berikut adalah kode program lengkap yang digunakan:

```c
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
               "--no-check-certificate",
               "--content-disposition",
               "-O", output_name,
               ("https://drive.google.com/uc?export=download&id=1nP5kjCi9ReDk5ILgnM7UCnrQwFH67Z9B"),
               NULL);
        perror("gagal menjalankan wget");
        exit(1);
    } else if (pid > 0){
        wait(NULL); // tunggu sampai selesai
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

    // Download file
    donlotfile(file_key, namafile);

    // Ekstrak file zip
    extract_zip(namafile, output_dir);

    return 0;
}
```

---
## **3. Cara Pengerjaan (Penjelasan Tiap Fungsi)**

Program ini terdiri dari beberapa fungsi utama yang memiliki peran spesifik. Berikut adalah penjelasan masing-masing fungsi:

### **a. `int file_exists(const char *filename)`**
- **Tujuan**: Mengecek apakah sebuah file tertentu sudah ada di sistem.
- **Cara kerja**: Menggunakan fungsi `access()` dengan parameter `F_OK`, yang akan mengembalikan nilai `0` jika file ditemukan.
- **Kegunaan**: Dapat digunakan sebelum proses download agar tidak mengunduh ulang file yang sudah ada. Namun, fungsi ini **belum dipanggil di dalam fungsi `main()`**.

---

### **b. `int folder_exists(const char *foldername)`**
- **Tujuan**: Mengecek apakah sebuah path adalah direktori yang valid.
- **Cara kerja**:
  - Memanggil fungsi `stat()` untuk mendapatkan informasi file.
  - Menggunakan makro `S_ISDIR()` untuk memastikan bahwa path tersebut adalah direktori.
- **Kegunaan**: Berguna untuk mengecek apakah folder tujuan ekstraksi sudah ada. Juga belum digunakan dalam `main()`.

---

### **c. `void donlotfile(const char *file_key, const char *output_name)`**
- **Tujuan**: Mengunduh file dari Google Drive.
- **Cara kerja**:
  1. Proses baru dibuat menggunakan `fork()`.
  2. Di dalam child process, perintah `wget` dijalankan dengan `execlp()`:
     - `--no-check-certificate` agar tetap bisa mengunduh meskipun ada masalah SSL.
     - `--content-disposition` agar file bisa dinamai sesuai informasi header.
     - `-O` digunakan untuk menentukan nama file output (`output_name`).
     - URL Google Drive masih ditulis **secara langsung (hardcoded)**, tidak menggunakan `file_key` secara dinamis.
  3. Parent process akan menunggu hingga proses anak selesai menggunakan `wait()`.

---

### **d. `void extract_zip(const char *namafile, const char *dest_folder)`**
- **Tujuan**: Mengekstrak file ZIP hasil unduhan ke dalam folder tertentu.
- **Cara kerja**:
  1. Membuat proses anak dengan `fork()`.
  2. Proses anak menjalankan perintah `unzip` dengan `execl()` untuk mengekstrak file `namafile` ke dalam `dest_folder`.
  3. Parent process kembali menunggu proses unzip selesai dengan `wait()`.
- **Catatan**: Pastikan direktori tujuan (`dest_folder`) sudah ada atau dibuat sebelum ekstraksi jika ingin lebih aman.

---

### **e. `int main()`**
- **Fungsi utama program**:
  1. Mendefinisikan nilai awal:
     - `file_key`: ID file dari Google Drive.
     - `namafile`: Nama file ZIP hasil download.
     - `output_dir`: Nama folder tempat ekstraksi dilakukan.
  2. Menjalankan proses pengunduhan melalui `donlotfile()`.
  3. Setelah file berhasil diunduh, langsung mengekstraknya dengan `extract_zip()`.

---

Dengan struktur ini, program secara otomatis akan:
- **Mengunduh file dari URL Google Drive**
- **Mengekstraknya ke folder tujuan**
- Menggunakan **fork()** dan **exec()** untuk memisahkan proses dan menjalankan perintah sistem

---
## **4. Output**

![trabowo-a-1](https://github.com/Praktikum-NETICS-2025/sisop-modul-2-shzirley/blob/main/trabowo-a-1.png?raw=true)
![trabowo-a-2](https://github.com/Praktikum-NETICS-2025/sisop-modul-2-shzirley/blob/main/trabowo-a-2.png?raw=true)

---

# B. **Pemilihan Film Secara Acak (trabowo-b.c)**

## **1. Tujuan**
Program ini bertujuan untuk:
- Membaca semua file `.jpg` dari folder target (`ektrakdisini/film`).
- Mengirim daftar file tersebut dari parent process ke child process menggunakan pipe (interprocess communication).
- Memilih satu file `.jpg` secara acak di child process dan menampilkannya sebagai hasil “film terpilih”.
- Menerapkan konsep proses ganda (fork), pipe, dan penggunaan direktori dalam sistem operasi Linux/Unix.

---

## **2. Code**
Berikut adalah kode lengkap program:

```c
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

    } else {
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

        write(pipefd[1], &count, sizeof(int));

        for (int i = 0; i < count; i++){
            int len = strlen(jpgList[i]);
            write(pipefd[1], &len, sizeof(int));
            write(pipefd[1], jpgList[i], len);
            free(jpgList[i]);
        }

        close(pipefd[1]);

        wait(NULL);
    }

    return 0;
}
```

---

## **3. Cara Pengerjaan**

### **a. `#define TARGET_FOLDER`**
- Mendefinisikan lokasi folder tempat program mencari file `.jpg`. Folder ini harus sudah ada dan berisi file `.jpg`.

---

### **b. `pipe()` dan `fork()`**
- Membuat saluran komunikasi (pipe) antar proses.
- `fork()` digunakan untuk membuat child process. Parent dan child berbagi pipe untuk pertukaran data.

---

### **c. Proses Parent**
- **Tugas**: Membaca isi direktori `TARGET_FOLDER`.
- Menggunakan `opendir()` dan `readdir()` untuk membaca semua file di folder.
- Jika nama file mengandung ekstensi `.jpg`, disimpan dalam array `jpgList[]`.
- Jumlah file `.jpg` ditulis ke pipe, diikuti dengan nama masing-masing file (panjang dan kontennya).
- Menutup sisi write dari pipe setelah pengiriman selesai dan menunggu child process dengan `wait()`.

---

### **d. Proses Child**
- **Tugas**: Menerima daftar nama file dari pipe.
- Membaca jumlah file, lalu membaca nama file satu per satu.
- File disimpan dalam array `filmList[]`.
- Menggunakan `rand()` untuk memilih salah satu nama file secara acak.
- Menampilkan nama file terpilih sebagai “Film for Trabowo & Peddy”.
- Semua memori yang dialokasikan dengan `strdup()` dibebaskan menggunakan `free()`.

---

### **e. Penanganan Error**
- Program menangani kesalahan dalam `pipe()`, `fork()`, dan `opendir()` dengan `perror()`, lalu keluar dengan kode `1`.

---

### **f. Fitur Unik**
- **Komunikasi antar proses menggunakan pipe.**
- **Pemilihan acak nama file untuk simulasi pemilihan film.**
- **Penggunaan `dirent.h` untuk mengakses isi folder.**

---
## **4. Output**

![trabowo-b](https://github.com/Praktikum-NETICS-2025/sisop-modul-2-shzirley/blob/main/trabowo-b.png?raw=true)

---

# **C. Memilah Film Berdasarkan Genre (trabowo-c.c)**

## **1. Tujuan**
Program ini bertujuan untuk:
- Mengelompokkan file film `.jpg` berdasarkan genre yang tertulis di nama filenya.
- Memindahkan file ke folder tujuan sesuai genre (`FilmHorror`, `FilmAnimasi`, `FilmDrama`).
- Mencatat setiap pemindahan ke file log `recap.txt` oleh dua thread berbeda (Trabowo & Peddy) secara bergiliran.
- Menghitung jumlah film per genre dan mencatat hasilnya di file `total.txt`.
- Menerapkan sinkronisasi thread dengan mutex dan giliran (turn-based access).

---

## **2. Code**
Berikut adalah kode lengkap dari program:

```c
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
```

---

## **3. Cara Pengerjaan (Penjelasan Tiap Fungsi)**

### **a. Struktur Data `Film`**
```c
typedef struct {
    char filename[256];
    int nomor;
} Film;
```
Menyimpan nama file dan nomor urut dari nama file (sebelum tanda `_`).

---

### **b. Fungsi `get_genre`**
```c
const char *get_genre(const char *filename)
```
Mengambil genre dari nama file, yang berada di antara `_` dan `.`. Contoh: `123_Film_horror.jpg` → `horror`.

---

### **c. Fungsi `kapital_genre`**
```c
void kapital_genre(const char *src, char *dest)
```
Mengubah huruf pertama genre menjadi kapital untuk penamaan folder (e.g., `horror` → `Horror`).

---

### **d. Fungsi `catat_log`**
```c
void catat_log(const char *filename, const char *person, const char *genre, int my_turn)
```
- Mencatat waktu dan nama file yang dipindahkan ke `recap.txt`.
- Sinkronisasi dilakukan dengan mutex dan variabel global `turn` agar log dicatat bergantian oleh thread Trabowo dan Peddy.

---

### **e. Fungsi `pindah_film`**
```c
void pindah_film(Film film, const char *person, int my_turn)
```
- Menentukan folder tujuan berdasarkan genre.
- Membuat folder jika belum ada, lalu memindahkan file dengan `rename()`.
- Memanggil `catat_log` untuk mencatat log pemindahan.

---

### **f. Fungsi `compare_film`**
```c
int compare_film(const void *a, const void *b)
```
Menyorting array film berdasarkan nomor file secara ascending untuk penjadwalan thread.

---

### **g. Fungsi Thread: `p_trabowo` dan `p_peddy`**
```c
void *p_trabowo(void *arg)
void *p_peddy(void *arg)
```
- Trabowo memproses file dari indeks genap (0, 2, 4...).
- Peddy memproses file dari indeks ganjil dari belakang (total-1, total-3...).
- Keduanya memanggil `pindah_film()` dengan argumen berbeda.

---

### **h. Fungsi `hitung_file`**
```c
int hitung_file(const char *folder)
```
Menghitung jumlah file reguler dalam folder yang diberikan (digunakan untuk `total.txt`).

---

### **i. Fungsi `ambil_nomor`**
```c
int ambil_nomor(const char *filename)
```
Mengambil angka di depan nama file sebelum karakter `_` sebagai nomor film untuk keperluan sorting.

---

### **j. Fungsi `main()`**
Langkah-langkah utama program:
1. Membuat folder tujuan (`film/`).
2. Membaca file dari `ektrakdisini/film`, memfilter file `.jpg`, dan menyimpan ke array `Film`.
3. Menyorting array berdasarkan nomor.
4. Membuat dua thread (`Trabowo` dan `Peddy`) untuk memproses film.
5. Menunggu kedua thread selesai (`pthread_join`).
6. Menghitung jumlah file tiap genre dan menuliskannya ke `total.txt` bersama genre terbanyak.
7. Membersihkan mutex (`pthread_mutex_destroy`).

---
## **4. Output**

![trabowo-c-1](https://github.com/Praktikum-NETICS-2025/sisop-modul-2-shzirley/blob/main/trabowo-c-1.png?raw=true)
![trabowo-c-2](https://github.com/Praktikum-NETICS-2025/sisop-modul-2-shzirley/blob/main/trabowo-c-2.png?raw=true)

---

# **D. Pengarsipan Film (trabowo-d.c)**

## **1. Tujuan**
Program ini bertujuan untuk:
- Mengarsipkan tiga folder genre film (`FilmAnimasi`, `FilmDrama`, dan `FilmHorror`) ke dalam file `.zip`.
- Menghapus folder setelah berhasil diarsipkan.
- Menampilkan pesan konfirmasi setelah setiap proses.

---

## **2. Code**
Berikut adalah kode lengkap program:

```c
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
```

---

## **3. Cara Pengerjaan**

### **a. Fungsi `arsipdel(const char *foldernem)`**
- Tujuan: Mengarsipkan folder dan menghapusnya setelah selesai diarsipkan.
- Langkah-langkah:
  1. **Fork proses** untuk membuat child process.
  2. **Child process pertama** menjalankan perintah `zip -r folder.zip folder/` menggunakan `execlp()` untuk mengarsipkan folder.
  3. Setelah proses zip selesai (`wait()`), **parent process** menampilkan pesan keberhasilan.
  4. Kemudian, **fork lagi** untuk membuat child kedua.
  5. **Child kedua** menjalankan perintah `rm -rf folder/` untuk menghapus folder asli.
  6. Parent process kedua kembali menunggu dan menampilkan pesan bahwa folder berhasil dihapus.

### **b. Fungsi `main()`**
- Memanggil `arsipdel()` tiga kali untuk folder:
  - `film/FilmAnimasi`
  - `film/FilmDrama`
  - `film/FilmHorror`

---
## **4. Output**

![trabowo-d](https://github.com/Praktikum-NETICS-2025/sisop-modul-2-shzirley/blob/main/trabowo-d.png?raw=true)

---


# **CELLA'S MANHWA**


## **Code**

```c
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
```

# **A. Summoning the Manhwa Stats**
## 🔶 **1. Pengambilan Data API & Pemrosesan JSON**
### 🔹 Fungsi Terkait:
- `ambil_data_dari_api()`
- `simpen_data()`
- `olah_dan_simpan()`

### 🔹 Alur:
1. `ambil_data_dari_api()` menggunakan **libcurl** untuk melakukan HTTP GET request ke URL API Jikan.
2. Response (dalam bentuk JSON) disimpan ke buffer `penampung_data` menggunakan callback `simpen_data()`.
3. `olah_dan_simpan()` mem-parsing JSON dengan **cJSON**, mengambil field:
   - `title`, `status`, `published.from`
   - Array `genres`, `themes`, `authors`
4. Semua data ditulis ke file `Manhwa/NamaManhwa.txt`.

## ✨ **Output**

### DWCWD
![DWCWD](https://github.com/Praktikum-NETICS-2025/sisop-modul-2-shzirley/blob/main/DWCWD.txt.png?raw=true)

### MMDW
![MMDW](https://github.com/Praktikum-NETICS-2025/sisop-modul-2-shzirley/blob/main/MMDW.txt.png?raw=true)

### NIOCP
![NIOCP](https://github.com/Praktikum-NETICS-2025/sisop-modul-2-shzirley/blob/main/NIOCP.txt.png?raw=true)

### TVLA
![TVLA](https://github.com/Praktikum-NETICS-2025/sisop-modul-2-shzirley/blob/main/TVLA.txt.png?raw=true)

---

# **B. Seal the Scrolls**
## 🔶 **2. Manajemen Folder**
### 🔹 Fungsi Terkait:
- `buat_folder()`
- `seal_the_scrolls()`

### 🔹 Alur:
- `buat_folder()` memanggil `mkdir -p` menggunakan `fork()` dan `execlp()` untuk membuat folder (jika belum ada).
- `seal_the_scrolls()`:
  1. Membuka direktori `Manhwa/`.
  2. Untuk setiap file `.txt`:
     - Mengambil huruf kapital dari nama file sebagai nama `.zip`.
     - Menjalankan perintah `zip -j Archive/NAMA.zip Manhwa/file.txt` via fork.

## ✨ **Output**

![task-3-c-1](https://github.com/Praktikum-NETICS-2025/sisop-modul-2-shzirley/blob/main/task-3-c-1.png?raw=true)

---

# **C. Making the Waifu Gallery**
## 🔶 **3. Multithreaded Download Gambar**
### 🔹 Struktur:
```c
typedef struct {
    char *heroine_name;
    char *link;
    int download_count;
} DownloadTask;
```

### 🔹 Fungsi Terkait:
- `download_heroine()` – Fungsi thread
- `pthread_create()` dan `pthread_join()`

### 🔹 Alur:
1. Untuk setiap heroine:
   - Buat thread baru `download_heroine()`.
   - Folder `Heroines/<Nama>` dibuat.
   - Gambar dari link yang sama didownload beberapa kali (jumlah tergantung `download_count`).
   - Nama file seperti: `Lia_1.jpg`, `Lia_2.jpg`, dst.
   - Validasi ukuran file dilakukan (`stat`) untuk menghindari korupsi.
2. Thread akan selesai (`pthread_exit`) setelah download selesai.

### ❗ Catatan Penting:
```c
if(pthread_create(&threads[i], NULL, download_heroine, (void *)&tasks[i]));
pthread_join(threads[i], NULL);
```
- `pthread_create()` tidak dicek hasilnya, dan tanda `;` di akhir if menyebabkan thread dijalankan **berurutan**, bukan paralel.
- Harusnya semua `pthread_create()` dulu, lalu `pthread_join()` dalam loop terpisah agar **benar-benar paralel**.


## ✨ **Output**

![task-3-c-2](https://github.com/Praktikum-NETICS-2025/sisop-modul-2-shzirley/blob/main/task-3-c-2.png?raw=true)

---

# **D. Zip. Save. Goodbye**
## 🔶 **4. Kompresi dan Pembersihan**
### 🔹 Fungsi:
- `zip_and_cleanup()`

### 🔹 Alur:
1. Membuat folder `Archive/Images/` via `fork() + execvp("mkdir", ...)`.
2. Untuk setiap heroine:
   - Jalankan `zip -r Archive/Images/<kode>_<nama>.zip Heroines/<nama>`.
3. Setelah itu, jalankan `rm -r Heroines/<nama>` untuk menghapus folder heroine.

---

## 🔷 Ringkasan Komponen Utama
| Bagian               | Teknologi/Metode yang Digunakan |
|----------------------|-------------------------------|
| HTTP Request         | libcurl                       |
| JSON Parsing         | cJSON                         |
| Proses File/Folder   | `fork()`, `execvp()`, `mkdir`, `zip`, `rm` |
| Multithread Download | `pthread_create`, `pthread_join` |
| Validasi File Gambar | `stat()` dan size > 0 check    |

## ✨ **Output**

![task-3-d](https://github.com/Praktikum-NETICS-2025/sisop-modul-2-shzirley/blob/main/task-3-d.png?raw=true)

---

Berikut adalah **laporan lengkap** yang telah dirapikan **beserta seluruh kode utuh** untuk masing-masing komponen: `client.c`, `loadbalancer.c`, dan `worker.c`.

---

# **PIPIP'S LOAD BALANCER**

## **Tujuan**

Membangun sistem distribusi pesan yang terdiri dari:

- **Client**: Mengirim pesan ke *Load Balancer* melalui *Shared Memory*.
- **Load Balancer**: Meneruskan pesan ke beberapa *Worker* melalui *Message Queue* menggunakan metode *round-robin*.
- **Worker**: Menerima dan mencatat pesan yang diproses.

Semua proses mencatat aktivitas ke dalam file log `system.log`.

---

## **Diagram Alur**

```
Client
   |
[ Shared Memory ]
   |
Load Balancer
   |
[ Message Queue ]
   |
Worker 1, Worker 2, ..., Worker N
```

---

## **Penjelasan Komponen**

### 1. **client.c**

#### Fungsi:
- Mengirim pesan dan jumlahnya ke shared memory.
- Menandai bahwa data siap dibaca oleh Load Balancer.
- Mencatat log aktivitas ke `system.log`.

#### Kode:

```c
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
```

---

## **Cara Pengerjaan**

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
```
Import pustaka:
- `stdio.h`, `stdlib.h`, `string.h`: Operasi dasar I/O, alokasi memori, dan manipulasi string.
- `sys/ipc.h`, `sys/shm.h`: Untuk IPC (Inter-Process Communication) dengan **shared memory**.

```c
#define SHM_KEY 1234
#define MAX_MESSAGE 256
```
- Konstanta `SHM_KEY`: ID unik untuk shared memory.
- `MAX_MESSAGE`: Panjang maksimal pesan.

```c
typedef struct {
    char message[MAX_MESSAGE];
    int count;
    int ready;
} SharedData;
```
Struct `SharedData` untuk ditaruh di shared memory:
- `message`: isi pesan.
- `count`: jumlah pesan yang ingin dikirim.
- `ready`: sinyal ke `loadbalancer` bahwa data siap dibaca.

```c
char input[300];
printf("input (format: Pesan;Jumlah): ");
fgets(input, sizeof(input), stdin);
```
- Menerima input user dengan format: `Pesan;Jumlah`.

```c
char *token = strtok(input, ";");
char *pesan = token;
token = strtok(NULL, ";");
int jumlah = atoi(token);
```
- Memecah string input berdasarkan `;`, ambil `pesan` dan `jumlah`.

```c
int shmid = shmget(SHM_KEY, sizeof(SharedData), IPC_CREAT | 0666);
SharedData *data = (SharedData *)shmat(shmid, NULL, 0);
```
- Buat/mendapatkan shared memory (`shmget`) lalu *attach* ke pointer (`shmat`).

```c
strcpy(data->message, pesan);
data->count = jumlah;
data->ready = 1;
```
- Tulis data ke shared memory dan beri tanda bahwa data siap (`ready = 1`).

```c
FILE *log = fopen("system.log", "a");
fprintf(log, "Message from client: %s\n", data->message);
fprintf(log, "Message count: %d\n", data->count);
fclose(log);
```
- Tulis aktivitas client ke log `system.log`.

```c
shmdt(data);
```
- *Detach* dari shared memory.

---

### 2. **loadbalancer.c**

#### Fungsi:
- Membaca pesan dari shared memory.
- Mendistribusikan pesan ke worker menggunakan message queue secara bergiliran (round-robin).
- Mencatat aktivitas ke `system.log`.

#### Kode:

```c
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

    int shmid = shmget(SHM_KEY, sizeof(SharedData), 0666);
    SharedData *data = (SharedData *)shmat(shmid, NULL, 0);

    while (data->ready != 1) usleep(10000);

    int msgid = msgget(MSG_KEY, IPC_CREAT | 0666);

    FILE *log = fopen("system.log", "a");
    if (log == NULL) {
        perror("Failed to open log file");
        return 1;
    }

    for (int i = 0; i < data->count; i++) {
        int worker_id = (i % NUM_WORKERS) + 1;

        fprintf(log, "Received at lb: %s (#message %d)\n", data->message, i + 1);

        Message msg;
        msg.mtype = worker_id;
        strcpy(msg.mtext, data->message);
        msgsnd(msgid, &msg, sizeof(msg.mtext), 0);

        usleep(10000);
    }

    fclose(log);
    shmdt(data);
    return 0;
}
```
---
## **Cara Pengerjaan**

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
```
- Tambahan `unistd.h` dan `sys/msg.h` untuk menggunakan `usleep()` dan **message queue**.

```c
#define SHM_KEY 1234
#define MSG_KEY 5678
#define MAX_MESSAGE 256
#define MAX_TEXT 256
```
- Konstanta kunci shared memory dan message queue.

```c
typedef struct {
    char message[MAX_MESSAGE];
    int count;
    int ready;
} SharedData;

typedef struct {
    long mtype;
    char mtext[MAX_TEXT];
} Message;
```
- `SharedData`: sama dengan di `client`.
- `Message`: untuk message queue.

```c
if (argc != 2) {
    printf("Usage: ./loadbalancer <n>\n");
    return 1;
}
```
- Harus menerima 1 argumen: jumlah worker.

```c
int NUM_WORKERS = atoi(argv[1]);
if (NUM_WORKERS <= 0) {
    printf("Jumlah worker harus lebih dari 0\n");
    return 1;
}
```
- Validasi jumlah worker.

```c
int shmid = shmget(SHM_KEY, sizeof(SharedData), 0666);
SharedData *data = (SharedData *)shmat(shmid, NULL, 0);
```
- Akses shared memory dan *attach*.

```c
while (data->ready != 1) usleep(10000);
```
- Tunggu sampai client memberi tanda siap (`ready == 1`).

```c
int msgid = msgget(MSG_KEY, IPC_CREAT | 0666);
```
- Dapatkan/membuat message queue.

```c
FILE *log = fopen("system.log", "a");
for (int i = 0; i < data->count; i++) {
    fprintf(log, "Received at lb: %s (#message %d)\n", data->message, i + 1);
    Message msg;
    msg.mtype = (i % NUM_WORKERS) + 1;
    strcpy(msg.mtext, data->message);
    msgsnd(msgid, &msg, sizeof(msg.mtext), 0);
}
fclose(log);
```
- Kirim `count` pesan ke worker secara bergilir (round-robin) menggunakan message queue (`msgsnd`).
- Log setiap pesan.

```c
shmdt(data);
```
- *Detach* dari shared memory.

---

### 3. **worker.c**

#### Fungsi:
- Menerima pesan dari message queue berdasarkan ID worker.
- Mencatat setiap pesan yang diterima ke log.
- Menghentikan proses setelah menerima sejumlah pesan tertentu.

#### Kode:

```c
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

    while (1) {
        if (msgrcv(msgid, &msg, sizeof(msg.mtext), worker_id, IPC_NOWAIT) != -1) {
            fprintf(log, "Worker%d: message received\n", worker_id);
            fflush(log);
            count++;
        } else {
            usleep(10000);
            if (count > 0) break;
        }
    }

    fprintf(log, "Worker %d: %d messages\n", worker_id, count);
    fclose(log);
    return 0;
}
```

---

## **Cara Pengerjaan**

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
```
- Untuk message queue dan sleep.

```c
#define MSG_KEY 5678
#define MAX_TEXT 256
```
- Kunci dan batas pesan.

```c
typedef struct {
    long mtype;
    char mtext[MAX_TEXT];
} Message;
```
- Struct message queue.

```c
if (argc != 2) {
    printf("Usage: ./worker <id>\n");
    return 1;
}
int worker_id = atoi(argv[1]);
```
- Terima ID worker sebagai argumen.

```c
int msgid = msgget(MSG_KEY, 0666);
Message msg;
int count = 0;
FILE *log = fopen("system.log", "a");
```
- Buka message queue dan log.

```c
while (1) {
    if (msgrcv(msgid, &msg, sizeof(msg.mtext), worker_id, IPC_NOWAIT) != -1) {
        fprintf(log, "Worker%d: message received\n", worker_id);
        fflush(log);
        count++;
    } else {
        usleep(10000);
        if (count > 0) break;
    }
}
```
- Terima pesan yang sesuai `mtype == worker_id`.
- Jika tidak ada pesan dan sudah pernah terima sebelumnya, keluar.

```c
fprintf(log, "Worker %d: %d messages\n", worker_id, count);
fclose(log);
return 0;
```
- Log jumlah pesan yang diterima dan keluar.

---

## **Kesimpulan**

Sistem ini mengimplementasikan komunikasi antar proses menggunakan shared memory dan message queue secara efisien:

- **Client** → Shared Memory → **Load Balancer** → Message Queue → **Workers**
- Pembagian pesan dilakukan secara adil melalui mekanisme round-robin.
- Semua komponen mencatat aktivitas masing-masing dalam `system.log`.

---

## **Output**

![Distribusi Pesan IPC](https://github.com/Praktikum-NETICS-2025/sisop-modul-2-shzirley/blob/main/task-4.png?raw=true)

---

# **Organize and Analyze Anthony's Favorite Films**

### **a. One Click and Done!**

- Download file ZIP
- Ekstrak ke folder
- Hapus ZIP
- Semua dalam satu perintah

---

### **b. Sorting Like a Pro**

- Mengelompokkan berdasarkan huruf awal judul → folder `judul/*.txt`
- Mengelompokkan berdasarkan tahun → folder `tahun/*.txt`
- Format: `Judul Film - Tahun Rilis - Sutradara`
- Logging ke `log.txt` dengan format dan waktu
- Dikerjakan paralel menggunakan **thread**

---

### **c. The Ultimate Movie Report**

- Hitung film sebelum dan setelah 2000 untuk setiap negara
- Format laporan seperti contoh
- Disimpan di file `report_ddmmyyyy.txt`
- Negara harus dibersihkan spasi dan kapitalisasi
- Dikerjakan secara otomatis saat pilih menu

## Bagian Header dan Definisi Konstanta

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
```

### Penjelasan:
- `stdio.h`: fungsi I/O seperti `printf`, `scanf`, `fopen`.
- `stdlib.h`: fungsi alokasi memori (`malloc`, `realloc`, `exit`).
- `string.h`: manipulasi string (`strcpy`, `strcmp`, `strchr`).
- `ctype.h`: manipulasi karakter (`toupper`, `isalnum`).
- `time.h`: waktu sistem (`time`, `localtime`, `strftime`).
- `pthread.h`: untuk multithreading.
- `unistd.h`: fungsi POSIX seperti `fork`, `exec`.
- `sys/stat.h`: membuat direktori (`mkdir`).
- `sys/wait.h`: tunggu child process selesai (`wait`).

```c
#define MAX_LINE 1024
#define INITIAL_CAPACITY 1000
#define GROWTH_FACTOR 2
#define LOG_FILE "log.txt"
#define ZIP_NAME "films.zip"
#define EXTRACTED_FOLDER "film"
```

### Penjelasan:
- `MAX_LINE`: buffer maksimal per baris CSV.
- `INITIAL_CAPACITY`: kapasitas awal array film.
- `GROWTH_FACTOR`: faktor penggandaan ketika kapasitas penuh.
- `LOG_FILE`: nama file log aktivitas.
- `ZIP_NAME`: nama file ZIP hasil unduh.
- `EXTRACTED_FOLDER`: folder hasil ekstrak file ZIP.

---

## Struktur Data

```c
struct Film {
    char title[256];
    char director[256];
    char country[128];
    int year;
};
```

### Penjelasan:
Struktur `Film` mewakili satu entri film, dengan:
- `title`: judul film.
- `director`: nama sutradara.
- `country`: negara produksi.
- `year`: tahun rilis.

```c
struct Film *films = NULL;
int film_count = 0;
int film_capacity = 0;
```

### Penjelasan:
- `films`: pointer ke array film yang dialokasikan dinamis.
- `film_count`: jumlah film saat ini.
- `film_capacity`: kapasitas maksimal saat ini dari array.

---

## Fungsi Utilitas

### Menghapus newline dari string

```c
void trim_newline(char *str) {
    char *pos;
    if ((pos = strchr(str, '\n')) != NULL)
        *pos = '\0';
}
```

### Penjelasan:
- Cari karakter newline (`\n`) dalam `str`.
- Jika ada, ganti dengan null terminator (`\0`) agar string tidak memuat newline.

---

### Alokasi Memori Awal untuk Array Film

```c
void init_film_array() {
    film_capacity = INITIAL_CAPACITY;
    films = (struct Film *)malloc(film_capacity * sizeof(struct Film));
    if (!films) {
        fprintf(stderr, "Gagal alokasi memori film\n");
        exit(EXIT_FAILURE);
    }
}
```

### Penjelasan:
- Alokasikan memori sebanyak `INITIAL_CAPACITY` untuk array `films`.
- Jika `malloc` gagal, cetak error dan keluar program.

---

### Resize Array jika Penuh

```c
void resize_film_array() {
    film_capacity *= GROWTH_FACTOR;
    films = (struct Film *)realloc(films, film_capacity * sizeof(struct Film));
    if (!films) {
        fprintf(stderr, "Gagal realloc memori film\n");
        exit(EXIT_FAILURE);
    }
}
```

### Penjelasan:
- Gandakan kapasitas array `films`.
- Gunakan `realloc` untuk menyesuaikan memori.
- Error handling bila `realloc` gagal.

---

## Parsing CSV

```c
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
```

### Penjelasan:
- Buka file CSV untuk dibaca.
- Baca per baris.
- Jika jumlah film melebihi kapasitas, lakukan resize.
- Gunakan `sscanf` untuk memisahkan kolom berdasarkan koma.
- Simpan ke array `films`.

---

## Logging Aktivitas Pengelompokan

```c
void log_activity(const char *category, const char *title) {
    FILE *log = fopen(LOG_FILE, "a");
    if (!log) return;
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    fprintf(log, "[%02d:%02d:%02d] Proses mengelompokkan berdasarkan %s: sedang mengelompokkan untuk film %s\n",
            tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec, category, title);
    fclose(log);
}
```

### Penjelasan:
- Tulis ke `log.txt` informasi waktu dan aktivitas.
- Contoh log: `[14:05:32] Proses mengelompokkan berdasarkan Tahun: sedang mengelompokkan untuk film Titanic`

---

## Thread 1: Kelompokkan Berdasarkan Judul

```c
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
```

### Penjelasan:
- Buat folder `judul/`.
- Kelompokkan berdasarkan huruf pertama judul:
  - Jika huruf/angka: `judul/A.txt`, `judul/B.txt`, dst.
  - Jika simbol: simpan ke `judul/#.txt`.
- Tambahkan log aktivitas.

---

## Thread 2: Kelompokkan Berdasarkan Tahun

```c
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
```

### Penjelasan:
- Buat folder `tahun/`.
- Buat file untuk setiap tahun rilis (`tahun/1997.txt`, `tahun/2020.txt`).
- Tambahkan log aktivitas.

---

## Menjalankan Kedua Thread

```c
void handle_grouping() {
    pthread_t t1, t2;
    pthread_create(&t1, NULL, group_by_title, NULL);
    pthread_create(&t2, NULL, group_by_year, NULL);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    printf("Pengelompokan selesai!\n");
}
```

### Penjelasan:
- Buat dua thread:
  - Thread 1: kelompok judul.
  - Thread 2: kelompok tahun.
- Tunggu keduanya selesai.

---

## Laporan Statistik Berdasarkan Negara

```c
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
```

### Penjelasan:
- Buat laporan `report_DDMMYYYY.txt`.
- Normalisasi nama negara: hapus spasi dan kapitalisasi.
- Hitung jumlah film sebelum/sesudah 2000 per negara.
- Simpan ke file.

---

## Download dan Ekstrak File

### Unduh File ZIP

```c
void download_zip() {
    pid_t pid = fork();
    if (pid == 0) {
        char *argv[] = {
            "wget",
            "--no-check-certificate",
            "-O", ZIP_NAME,
            "https://drive.usercontent.google.com/download?id=12GWsZbSH858h2HExP3x4DfWZB1jLdV-J&export=download",
            NULL
        };
        execvp("wget", argv);
        perror("execvp gagal");
        exit(1);
    } else {
        wait(NULL);
    }
}
```

### Ekstrak File ZIP

```c
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
```

---

## Fungsi Utama `main`

```c
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
```

---

### Penjelasan:
- Tampilkan menu.
- Pilihan:
  1. Unduh dan ekstrak file ZIP.
  2. Masukkan file CSV, parsing, dan kelompokkan.
  3. Buat laporan statistik.
  0. Keluar program.
 
---

## Masalah

masalah pada kode saya adalah ia tidak dapat membuat report dengan benar, sebelum nya mencapai 56 negara bukan 77 setelah menggunakan fork menjadi 200 ke atas lebih

negara germany diatas tahun 2000 seharusnya 13, bukan 62

judul berawal kan 0 masuk ke folder tahun bukan judul

---

## **Outpu**


## Judul
![Judul](https://github.com/Praktikum-NETICS-2025/sisop-modul-2-shzirley/blob/main/judul.png?raw=true)

## Tahun
![Tahun](https://github.com/Praktikum-NETICS-2025/sisop-modul-2-shzirley/blob/main/tahun.png?raw=true)

## Diagram Sistem
Diagram sistem distribusi pesan menggunakan IPC:

![Diagram Sistem](https://github.com/Praktikum-NETICS-2025/sisop-modul-2-shzirley/blob/main/task-4.png?raw=true)

## Log Sistem
Cuplikan log dari proses Client, Load Balancer, dan Worker:

![Log](https://github.com/Praktikum-NETICS-2025/sisop-modul-2-shzirley/blob/main/log.png?raw=true)

## Cuplikan Laporan
Gambaran isi laporan akhir:

![Report](https://github.com/Praktikum-NETICS-2025/sisop-modul-2-shzirley/blob/main/report.png?raw=true)

---
