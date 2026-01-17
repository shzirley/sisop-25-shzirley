[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-22041afd0340ce965d47ae6ef1cefeee28c7c493a6346c4f15d667ab976d596c.svg)](https://classroom.github.com/a/V7fOtAk7)
|    NRP     |      Name      |
| :--------: | :------------: |
| 5025241223 | Muhammad Akhdan Alwaafy |
| 5025241226 | Angela Vania Sugiyono |

# Praktikum Modul 3 _(Module 4 Lab Work)_

### Laporan Resmi Praktikum Modul 4 _(Module 3 Lab Work Report)_

</div>

### Daftar Soal _(Task List)_

- [Task 1 - FUSecure](/task-1/)

## 🧠 **Penjelasan Kode (per bagian)**

---

### 🔰 `#define FUSE_USE_VERSION 31`

* **Menyatakan versi FUSE** yang akan digunakan.
* FUSE menyediakan versi API berbeda, dan di sini kita set versi 31 agar kompatibel dengan fitur terbaru `libfuse3`.

---

### 📂 `#include <...>`

* Memasukkan header library yang dibutuhkan:

  * `fuse3/fuse.h`: Library utama FUSE.
  * `pwd.h`: Untuk mendapatkan informasi user (`getpwuid()`).
  * `dirent.h`, `errno.h`, `fcntl.h`, `sys/stat.h`: Operasi file & direktori.
  * `unistd.h`, `stdlib.h`: Fungsi standar C.

---

### 🎯 `const char *source_dir = "/home/shared_files";`

* Direktori sumber fisik di dalam filesystem aslinya.
* Semua file dan folder FUSE akan diambil dari sini.

---

### 👤 `get_calling_uid()`

```c
uid_t get_calling_uid() {
    return fuse_get_context()->uid;
}
```

* Mengambil UID user asli (bukan root) yang memanggil filesystem FUSE.
* Digunakan untuk menentukan siapa usernya dan membatasi akses.

---

### 📜 `fuse_getattr()`

```c
// Membaca atribut file/direktori
// Mengecek path dan membatasi:
// - Jika path berisi /private_yuadi, hanya UID 1001 bisa akses
// - Jika path berisi /private_irwandi, hanya UID 1002 bisa akses
// Memanggil lstat() untuk ambil atribut file aslinya.
```

---

### 📂 `fuse_readdir()`

```c
// Membaca isi direktori
// - Menentukan username dari UID
// - Hanya menampilkan folder private milik user itu sendiri
// - Jika user masuk ke path /private_*, hanya menampilkan isinya sendiri
```

✅ **Ide Utama:**
Agar setiap user hanya melihat folder privat mereka sendiri dan `public`.
Menggunakan `getpwuid()` untuk membuat string `private_<username>`.

---

### 🔐 `fuse_open()` dan `fuse_read()`

```c
// fuse_open(): hanya izinkan pembukaan file read-only.
// Jika path adalah folder private, hanya user pemiliknya bisa open.
// fuse_read(): membaca isi file (pread()) dan kembalikan data.
```

---

### 🚫 Operasi Write dan Modify

```c
// Semua operasi tulis seperti:
// - mkdir, rmdir, write, unlink, rename, create, chmod, chown
// dikembalikan -EROFS (Read-Only File System Error)
```

👉 Artinya filesystem ini **hanya untuk baca**.

---

### 📝 `struct fuse_operations fuse_example_oper`

```c
// Struct ini memetakan setiap fungsi ke operasi FUSE.
```

* Contohnya `getattr = fuse_getattr`, `read = fuse_read`, dll.
* Semua operasi menulis diarahkan ke fungsi yang menolak (`-EROFS`).

---

### 🚀 `main()`

```c
int main(int argc, char *argv[]) {
    umask(0); // agar permission sesuai
    return fuse_main(argc, argv, &fuse_example_oper, NULL);
}
```

* Menjalankan filesystem FUSE menggunakan `fuse_main()`.

---

## 📸 **Output Contoh**

**a.**
![Soal 1A](https://github.com/Praktikum-NETICS-2025/sisop-modul-4-shzirley/blob/main/task1p4_a.png?raw=true)

**b.**
![Task 1B](https://github.com/Praktikum-NETICS-2025/sisop-modul-4-shzirley/blob/main/task1p4_b.png?raw=true)

**c.**
![Task 1C](https://github.com/Praktikum-NETICS-2025/sisop-modul-4-shzirley/blob/main/task1p4_c.png?raw=true)

**d.**
![Task 1D](https://github.com/Praktikum-NETICS-2025/sisop-modul-4-shzirley/blob/main/task1p4_d.png?raw=true)

**e.**
![Task 1E](https://github.com/Praktikum-NETICS-2025/sisop-modul-4-shzirley/blob/main/task1p4_e.png?raw=true)

---

```
$ ls /mnt/fuse
public  private_yuadi  private_irwandi

$ ls /mnt/fuse/private_yuadi
# Terlihat hanya bila Anda login sebagai user yuadi
```

---

💡 **Kesimpulan:**
Kode ini membuat filesystem FUSE *read-only* di atas `/home/shared_files`, di mana:

* Folder `private_yuadi` hanya bisa diakses `yuadi`.
* Folder `private_irwandi` hanya bisa diakses `irwandi`.
* Folder `public` bisa diakses siapa saja.

---

- [Task 2 - LawakFS++](/task-2/)

  # LawakFS++ - Laporan Lengkap Cara Kerja Filesystem Terkutuk

## 1. Deskripsi Umum

**LawakFS++** adalah filesystem berbasis FUSE (Filesystem in Userspace) yang didesain sebagai filesystem terkutuk dengan fitur:

* Penyembunyian ekstensi file dari tampilan pengguna.
* Kontrol akses terhadap file rahasia berdasarkan waktu.
* Penyaringan kata-kata tertentu dalam file teks (dynamic filtering).
* Konversi isi file biner ke Base64.
* Logging lengkap aktivitas akses file.
* Read-Only System: semua operasi tulis ditolak secara eksplisit.

LawakFS++ menggunakan file konfigurasi eksternal (`lawak.conf`) yang fleksibel, memungkinkan pengguna mengatur nama file rahasia, waktu akses, dan daftar kata yang ingin difilter.

---

## 2. File Konfigurasi `lawak.conf`

File ini dibaca saat filesystem dimulai dan digunakan untuk mengatur parameter perilaku.

### Contoh Isi File:

```ini
FILTER_WORDS=ducati,ferrari,mu,chelsea,prx,onic,sisop
SECRET_FILE_BASENAME=secret
ACCESS_START=08:00
ACCESS_END=18:00
```

### Penjelasan Parameter:

* `FILTER_WORDS`: daftar kata yang akan dicari dan diganti dalam file teks. Kata-kata ini akan disensor menjadi "lawak".
* `SECRET_FILE_BASENAME`: nama dasar file rahasia. File seperti `secret.txt`, `secret.jpg` akan dianggap sebagai file rahasia.
* `ACCESS_START`: jam mulai akses diizinkan untuk file rahasia. Format 24 jam (contoh: 08:00).
* `ACCESS_END`: jam akhir akses diizinkan untuk file rahasia. Di luar waktu ini, file dianggap tidak ada.

File konfigurasi ini memungkinkan perubahan kebijakan tanpa perlu mengubah kode sumber dan mengompilasi ulang.

---

## 3. Struktur dan Alur Program

### Fungsi `main`

```c
int main(int argc, char *argv[]) {
    realpath(argv[1], source_dir); // Simpan direktori sumber asli
    load_config();                 // Load konfigurasi dari file lawak.conf
    return fuse_main(argc, argv, &lawakfs_oper, NULL);
}
```

### Struktur Global `config`

Struktur `Config` menyimpan informasi penting yang berasal dari `lawak.conf`, seperti kata-kata filter dan batas waktu akses.

---

## 4. Fitur dan Implementasi

### A. Penyembunyian Ekstensi File

Pada fungsi `readdir`, nama file diproses untuk menghilangkan ekstensi sebelum ditampilkan:

```c
char *dot = strrchr(de->d_name, '.');
if (dot && de->d_type == DT_REG){
    size_t name_len = dot - de->d_name;
    strncpy(display_name, de->d_name, name_len);
    display_name[name_len] = '\0';
}
```

Namun saat membuka file (via `open`, `read`, dll.), nama file akan dicocokkan ke file sebenarnya dengan ekstensi melalui `find_real_file`.

### B. Akses Berbasis Waktu untuk File Rahasia

Fungsi `is_secret_file(path)` akan mengecek apakah nama file mengandung nama dasar `secret`. Fungsi `is_allowed_hour()` akan membandingkan waktu saat ini dengan rentang waktu konfigurasi.

Setiap operasi `getattr`, `access`, `open`, dan `read` akan memanggil kedua fungsi ini. Jika akses ke file `secret` dilakukan di luar jam yang ditentukan, maka akan mengembalikan `-ENOENT` (file tidak ditemukan).

### C. Filtering Konten Teks

Filtering dilakukan dalam `filter_text()`:

* Menggunakan pencarian substring tanpa case-sensitive.
* Mengganti setiap kemunculan kata terlarang dengan kata "lawak".

Contoh:

```c
memcpy(p, "lawak", word_len);
```

Jika `word_len >= 5`, maka seluruh kata diganti dengan "lawak" sepenuhnya. Jika kurang, hanya sebagian dari "lawak" yang digunakan.

### D. Konversi Base64 untuk File Biner

File biner dikenali berdasarkan ekstensi file yang bukan teks. Konten biner tidak ditampilkan mentah, tapi diencode ke format Base64:

```c
size_t encoded_len = base64_encode((unsigned char*)temp_buf, res, encoded_buf, sizeof(encoded_buf));
```

Hasil encoding disalin ke buffer output dan dikirim ke pengguna.

### E. Logging Akses File

Setiap operasi `read` dan `access` akan mencatat aktivitas ke `/var/log/lawakfs.log`. Format:

```
[YYYY-MM-DD HH:MM:SS] [UID] [ACTION] [PATH]
```

Contoh:

```
[2025-06-22 14:34:55] [1000] [READ] /readme
```

Fungsi logging:

```c
void log_action(const char *action, const char *path);
```

### F. Read-Only Enforcement

Semua fungsi write seperti `write`, `create`, `mkdir`, `unlink`, `rename`, `truncate`, `rmdir` akan mengembalikan `-EROFS` secara eksplisit:

```c
return -EROFS;
```

Hal ini memastikan filesystem bersifat read-only.

---

## 5. Fungsi-fungsi Penting pada `fuse_operations`

| Fungsi       | Deskripsi                                                            |
| ------------ | -------------------------------------------------------------------- |
| `getattr`    | Mendapatkan metadata file (ukuran, mode, waktu modifikasi, dll).     |
| `readdir`    | Menampilkan daftar isi direktori, menyembunyikan ekstensi file.      |
| `open`       | Membuka file, mengecek akses waktu jika file adalah `secret`.        |
| `read`       | Membaca isi file dengan filtering (teks) atau base64 (biner).        |
| `access`     | Mengecek apakah file dapat diakses, dengan log dan batasan waktu.    |
| `write` dll. | Semua fungsi tulis ditolak secara eksplisit, mengembalikan `-EROFS`. |

---

## 6. Contoh Kasus dan Output

### a. Menyembunyikan Ekstensi File

```bash
$ ls /mnt/lawakfs/
readme   secret   image
```

Padahal di direktori asli:

```bash
$ ls /source_dir/
readme.txt   secret.txt   image.png
```

### b. Akses di Luar Jam

```bash
$ cat /mnt/lawakfs/secret
cat: /mnt/lawakfs/secret: No such file or directory
```

### c. Filtering Konten

```
Isi asli: "Saya suka sisop dan PRX."
Hasil baca: "Saya suka lawak dan lawak."
```

### d. Konten File Biner

```bash
$ cat /mnt/lawakfs/image
U2FtcGxlIGJhc2U2NCBlbmNvZGVkIGltYWdlLg==
```

### e. Log Akses

```bash
$ sudo tail /var/log/lawakfs.log
[2025-06-22 10:15:00] [1000] [READ] /readme
[2025-06-22 10:15:01] [1000] [ACCESS] /readme
```

---

## 7. Kesimpulan

LawakFS++ mengintegrasikan keamanan, filtering, dan kontrol akses waktu dalam filesystem berbasis pengguna. Semua fitur diatur melalui file konfigurasi eksternal, sehingga fleksibel dan mudah diubah tanpa perlu recompilasi.

Filesystem ini cocok untuk:

* Membuat sistem file dengan pembatasan berbasis konten.
* Membatasi akses file berdasarkan jam kerja.
* Menyediakan perlindungan terhadap modifikasi data.

---

## 8. Pengembangan Lanjutan

* **Hot reload konfigurasi** tanpa unmount.
* **Penggunaan ACL** untuk membatasi pengguna tertentu.
* **UI sederhana** untuk mengedit `lawak.conf`.
* **Dukungan audit tambahan** untuk operasi selain `read` dan `access`.

---

## 🖼️ Hasil Output (Screenshot)

**a.**
![Task 2A](https://github.com/Praktikum-NETICS-2025/sisop-modul-4-shzirley/blob/main/task2a.png?raw=true)

**b.**
![Task 2B](https://github.com/Praktikum-NETICS-2025/sisop-modul-4-shzirley/blob/main/task2b.png?raw=true)

**c.**
![Task 2C](https://github.com/Praktikum-NETICS-2025/sisop-modul-4-shzirley/blob/main/task2c.png?raw=true)

**d.**
![Task 2D](https://github.com/Praktikum-NETICS-2025/sisop-modul-4-shzirley/blob/main/task2d.png?raw=true)

**e.**
![Task 2E](https://github.com/Praktikum-NETICS-2025/sisop-modul-4-shzirley/blob/main/task2e.png?raw=true)

---

- [Task 3 - Drama Troll](/task-3/)

## 📝 Tujuan

Membuat filesystem FUSE bernama **TrollFS** untuk menjebak user `DainTontas`.
Pada awalnya file `very_spicy_info.txt` menampilkan konten berbeda tergantung siapa pembacanya. Jika user `DainTontas` membaca file `upload.txt`, maka jebakan awal aktif. Jika kemudian DainTontas melakukan `echo upload > upload.txt`, jebakan final aktif dan semua file `.txt` di filesystem akan menampilkan ASCII art `"Fell for it again reward\n"` hingga filesystem di-restart.

---

## 🧑‍💻 Penjelasan Kode per Fungsi

### `is_trap_triggered()` dan `is_final_triggered()`

* Mengecek status jebakan berdasarkan file penanda `/tmp/trap_state` dan `/tmp/trap_final_state`.
* Jika file tersebut ada, jebakan aktif.

### `troll_getattr()`

* Memberikan atribut file untuk filesystem.
* Hanya mengenali dua file (`very_spicy_info.txt` dan `upload.txt`), membuatnya berstatus file biasa (`S_IFREG | 0444`), dan direktori root (`S_IFDIR | 0755`).

### `troll_readdir()`

* Menyediakan listing file dalam direktori `/mnt/troll`.
* Memasukkan `.` dan `..` serta dua file utama ke dalam buffer directory.

### `troll_open()`

* Membuka file tanpa pembatasan apapun.

### `troll_read()`

* Jika jebakan final aktif dan file berakhiran `.txt`, maka semua pembacaan file mengembalikan teks `"Fell for it again reward\n"`.
* Jika DainTontas membaca `upload.txt`, aktifkan jebakan awal (`/tmp/trap_state`).
* Jika membaca `very_spicy_info.txt`:

  * Jika jebakan awal aktif:

    * DainTontas → teks roadmap bocor.
    * User lain → teks personal secret.
  * Jika jebakan awal belum aktif → teks “tidak mencurigakan”.

### `troll_write()`

* Mengecek apakah DainTontas menulis `"upload\n"` ke `upload.txt`.
* Jika iya, buat file `/tmp/trap_final_state` untuk mengaktifkan jebakan final.

### `troll_create()` dan `troll_truncate()`

* Implementasi dummy untuk membuat file dan memotong file agar perintah `echo ... > file` berfungsi tanpa error.

### `troll_unlink()`

* Menolak upaya penghapusan file di filesystem troll.

### `main()`

* Menjalankan filesystem FUSE dan memetakan semua callback.

---

**Skenario Uji Coba:**

1. User `DainTontas` melihat `very_spicy_info.txt`:

```
$ cat /mnt/troll/very_spicy_info.txt
Very spicy internal developer information: leaked roadmap.docx
```

2. User `DainTontas` melihat `upload.txt` (jebakan awal aktif):

```
$ cat /mnt/troll/upload.txt
```

3. User `DainTontas` melakukan `echo upload > /mnt/troll/upload.txt` untuk memicu jebakan final:

```
$ echo upload > /mnt/troll/upload.txt
```

4. Setelah jebakan final aktif, semua file `.txt` menampilkan:

```
$ cat /mnt/troll/very_spicy_info.txt
Fell for it again reward

$ cat /mnt/troll/upload.txt
Fell for it again reward
```

---

## 🖼️ Hasil Output (Screenshot)

![Task 3](https://github.com/Praktikum-NETICS-2025/sisop-modul-4-shzirley/blob/main/task3p4.png?raw=true)

---

- [Task 4 - LilHabOS](/task-4/)

