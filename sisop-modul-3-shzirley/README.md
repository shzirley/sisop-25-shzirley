[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-22041afd0340ce965d47ae6ef1cefeee28c7c493a6346c4f15d667ab976d596c.svg)](https://classroom.github.com/a/Eu-CByJh)
|    NRP     |      Name      |
| :--------: | :------------: |
| 5025241223 | Muhammad Akhdan Alwaafy |
| 5025241226 | Angela Vania Sugiyono |

# Praktikum Modul 3 _(Module 3 Lab Work)_

### Laporan Resmi Praktikum Modul 3 _(Module 3 Lab Work Report)_

Di suatu pagi hari yang cerah, Budiman salah satu mahasiswa Informatika ditugaskan oleh dosennya untuk membuat suatu sistem operasi sederhana. Akan tetapi karena Budiman memiliki keterbatasan, Ia meminta tolong kepadamu untuk membantunya dalam mengerjakan tugasnya. Bantulah Budiman untuk membuat sistem operasi sederhana!

_One sunny morning, Budiman, an Informatics student, was assigned by his lecturer to create a simple operating system. However, due to Budiman's limitations, he asks for your help to assist him in completing his assignment. Help Budiman create a simple operating system!_

### Soal 1

> Sebelum membuat sistem operasi, Budiman diberitahu dosennya bahwa Ia harus melakukan beberapa tahap terlebih dahulu. Tahap-tahapan yang dimaksud adalah untuk **mempersiapkan seluruh prasyarat** dan **melakukan instalasi-instalasi** sebelum membuat sistem operasi. Lakukan seluruh tahapan prasyarat hingga [perintah ini](https://github.com/arsitektur-jaringan-komputer/Modul-Sisop/blob/master/Modul3/README-ID.md#:~:text=sudo%20apt%20install%20%2Dy%20busybox%2Dstatic) pada modul!

> _Before creating the OS, Budiman was informed by his lecturer that he must complete several steps first. The steps include **preparing all prerequisites** and **installing** before creating the OS. Complete all the prerequisite steps up to [this command](https://github.com/arsitektur-jaringan-komputer/Modul-Sisop/blob/master/Modul3/README-ID.md#:~:text=sudo%20apt%20install%20%2Dy%20busybox%2Dstatic) in the module!_

---

**Answer:**

### 🔧 Code:
````
  sudo apt -y update
  sudo apt -y install qemu-system build-essential bison flex libelf-dev libssl-dev bc grub-common grub-pc libncurses-dev libssl-dev mtools grub-pc-bin xorriso tmux

  mkdir -p osboot
  cd osboot

  wget https://cdn.kernel.org/pub/linux/kernel/v6.x/linux-6.1.1.tar.xz
  tar -xvf linux-6.1.1.tar.xz
  cd linux-6.1.1

  make tinyconfig
  make menuconfig

  64-Bit Kernel
  General Setup > Configure standard kernel features > Enable support for printk
  General Setup > Configure standard kernel features > Enable futex support
  General Setup > Initial RAM filesystem and RAM disk (initramfs/initrd) support
  General Setup > Control Group Support
  General setup > Configure standard kernel features > Multiple users, groups and capabilities support
  Enable the block layer > Legacy autoloading support
  Enable the block layer > Partition type > Advanced Partition Selection
  Device Drivers > Character devices > Enable TTY
  Device Drivers > Character devices > Virtio console
  Device Drivers > Character devices > /dev/mem virtual device support
  Device Drivers > Network device support > Virtio Network Driver
  Device Drivers > Serial Ata / Paralel ATA
  Device Drivers > Block Devices > Virtio block driver
  Device Drivers > Block Devices > loopback device support
  Device Drivers > Block Devices > RAM block device support
  Device Drivers > Virtio drivers
  Device Drivers > Virtualization Drivers
  Device Drivers > Generic  Driver Options > Maintain a devtmpfs at filesystems
  Device Drivers > Generic Driver Options > Automount devtmpfs at /dev
  Executable file formats > Kernel Support for ELF binaries
  Executable file formats > Kernel Support scripts starting with #!
  File Systems > FUSE support
  File Systems > The extended 3 filesystem
  File Systems > The extended 4 filesystem
  File Systems > Second extended fs support
  File Systems > Virtio Filesystem
  File Systems > Kernel automounter support
  File Systems > Pseudo File Systems > /proc file system support
  File Systems > Pseudo File Systems > sysctl support
  File Systems > Pseudo File Systems > sysfs file system support
  Networking Support > Networking options > Unix domain sockets
  Networking Support > Networking options > TCP/IP Networking

  make -j$(nproc)

  cp arch/x86/boot/bzImage ..
````

---

### 📖 Explanation:

### **1. Persiapan: Install tools untuk build kernel dan emulasi**

```bash
sudo apt -y update
sudo apt -y install qemu-system build-essential bison flex libelf-dev libssl-dev bc grub-common grub-pc libncurses-dev libssl-dev mtools grub-pc-bin xorriso tmux
```

* Update daftar paket.
* Install dependensi penting untuk:

  * **Build kernel**: `build-essential`, `libelf-dev`, `libssl-dev`, `bison`, `flex`, `bc`, `libncurses-dev`
  * **GRUB & ISO**: `grub-common`, `grub-pc`, `grub-pc-bin`, `mtools`, `xorriso`
  * **Emulator**: `qemu-system`
  * **Multiplexer terminal (opsional)**: `tmux`

---

### **2. Buat folder kerja dan masuk ke dalamnya**

```bash
mkdir -p osboot
cd osboot
```

* `mkdir -p osboot`: Membuat folder bernama `osboot`.
* `cd osboot`: Masuk ke folder kerja untuk proyek build kernel Budiman.

---

### **3. Unduh dan ekstrak source code kernel Linux**

```bash
wget https://cdn.kernel.org/pub/linux/kernel/v6.x/linux-6.1.1.tar.xz
tar -xvf linux-6.1.1.tar.xz
cd linux-6.1.1
```

* `wget ...`: Mengunduh kernel versi 6.1.1.
* `tar -xvf ...`: Mengekstrak file `.tar.xz`.
* `cd linux-6.1.1`: Masuk ke direktori kernel hasil ekstrak.

---

### **4. Konfigurasi minimal kernel menggunakan `make tinyconfig`**

```bash
make tinyconfig
```

* Menghasilkan konfigurasi kernel yang **super minimal** (`.config`) untuk sistem embedded atau eksperimen.
* Titik awal yang ringan, cocok untuk membangun kernel QEMU.

---

### **5. Lakukan konfigurasi lanjutan secara interaktif**

```bash
make menuconfig
```

* Membuka **tampilan antarmuka berbasis teks (TUI)** untuk konfigurasi kernel.
* Di sinilah Budiman **mengaktifkan fitur-fitur penting** agar kernel dapat *boot*, mengenali perangkat dasar, mendukung virtualisasi QEMU, dan memuat file sistem seperti ext3/ext4.

---

### **6. Fitur-fitur penting yang harus diaktifkan**

> **(Berikut fitur-fitur yang WAJIB diaktifkan agar kernel bisa digunakan di QEMU)**

* **\[✔] 64-Bit Kernel**
* **\[✔] printk**, **futex**, **initramfs support**, **cgroup**
* **\[✔] /proc, /sys, devtmpfs, ELF, #! script**
* **\[✔] TTY, virtio console, loopback, virtio-net, virtio block**
* **\[✔] ext3/ext4/second fs, virtio-fs, automount, FUSE**
* **\[✔] TCP/IP, Unix domain socket**

Semua fitur ini disesuaikan melalui menu seperti:

```
General Setup
Device Drivers
File Systems
Networking Support
Executable file formats
```

---

### **7. Build kernel**

```bash
make -j$(nproc)
```

* Menyusun (compile) kernel.
* `-j$(nproc)` → Gunakan seluruh core CPU agar proses lebih cepat.
* Hasil akhir berupa kernel image `bzImage` akan berada di `arch/x86/boot/`.

---

### **8. Salin kernel image ke direktori luar**

```bash
cp arch/x86/boot/bzImage ..
```

* Menyalin hasil build kernel (`bzImage`) ke direktori `osboot/` agar mudah digunakan untuk pembuatan ISO nanti.

---

### 🖼 Screenshot:

![Soal 1](https://github.com/Praktikum-NETICS-2025/sisop-modul-3-shzirley/blob/main/soal1.png?raw=true)


### Soal 2

> Setelah seluruh prasyarat siap, Budiman siap untuk membuat sistem operasinya. Dosen meminta untuk sistem operasi Budiman harus memiliki directory **bin, dev, proc, sys, tmp,** dan **sisop**. Lagi-lagi Budiman meminta bantuanmu. Bantulah Ia dalam membuat directory tersebut!

> _Once all prerequisites are ready, Budiman is ready to create his OS. The lecturer asks that the OS should contain the directories **bin, dev, proc, sys, tmp,** and **sisop**. Help Budiman create these directories!_

---

**Answer:**

### 🔧 Code:
````
sudo apt install -y busybox-static

sudo bash

mkdir -p myramdisk/{bin,dev,proc,sys,etc,tmp,root,sisop}

cp -a /dev/null myramdisk/dev
cp -a /dev/tty* myramdisk/dev
cp -a /dev/zero myramdisk/dev
cp -a /dev/console myramdisk/dev

cp /usr/bin/busybox myramdisk/bin
cd myramdisk/bin
./busybox --install .
````
---

### 📖 Explanation:

### **1. Instalasi `busybox-static`**

```bash
sudo apt install -y busybox-static
```

* Menginstal versi **statis** dari `busybox` agar dapat dijalankan **tanpa ketergantungan library eksternal**.
* `busybox` adalah tools all-in-one yang mencakup banyak perintah Linux dalam satu file biner kecil (misalnya: `ls`, `sh`, `mount`, dll).
* Wajib ada di initramfs karena ringan dan mendukung fungsi dasar sistem.

---

### **2. Masuk ke root shell (opsional tergantung sistem)**

```bash
sudo bash
```

* Memastikan shell berada dalam konteks **superuser penuh**.
* Dibutuhkan karena banyak perintah berikutnya menyangkut manipulasi device node (`/dev`) dan direktori sistem root (`/`).

---

### **3. Buat struktur direktori initramfs**

```bash
mkdir -p myramdisk/{bin,dev,proc,sys,etc,tmp,root,sisop}
```

* Membuat folder untuk struktur root filesystem.
* Penjelasan tiap folder:

  * `bin/` : perintah dasar (seperti `sh`, `ls`, dll)
  * `dev/` : device file (console, tty, null, zero)
  * `proc/`, `sys/` : pseudo-filesystems wajib
  * `etc/`, `tmp/`, `root/` : direktori sistem umum
  * `sisop/` : folder khusus sesuai instruksi/tujuan custom Budiman

---

### **4. Salin device file penting ke ramdisk**

```bash
cp -a /dev/null myramdisk/dev
cp -a /dev/tty* myramdisk/dev
cp -a /dev/zero myramdisk/dev
cp -a /dev/console myramdisk/dev
```

* `cp -a` → mempertahankan permission, ownership, dan tipe file (termasuk device node)
* File-file ini sangat penting karena:

  * `/dev/null` → untuk buangan output
  * `/dev/tty*` dan `/dev/console` → untuk input/output terminal kernel
  * `/dev/zero` → menghasilkan aliran nol biner tak terbatas (berguna di program)

---

### **5. Salin binary `busybox` ke direktori bin**

```bash
cp /usr/bin/busybox myramdisk/bin
```

* Salin `busybox` ke dalam ramdisk sebagai satu-satunya utilitas sistem yang digunakan saat boot.

---

### **6. Install semua perintah `busybox` secara otomatis**

```bash
cd myramdisk/bin
./busybox --install .
```

* `--install .` → membuat **symlink** dari `busybox` ke nama-nama perintah yang didukung (misalnya: `sh`, `ls`, `echo`, `cat`, `mount`, dsb)
* Jadi setelah ini, kamu akan punya:

  * `ls` → symlink ke `busybox`
  * `sh` → symlink ke `busybox`
  * dan banyak lainnya
* Ini membuat sistem bisa menjalankan shell dan perintah dasar saat boot dengan initramfs.

---

### 🖼 Screenshot:

![Soal 2](https://github.com/Praktikum-NETICS-2025/sisop-modul-3-shzirley/blob/main/soal2.png?raw=true)

### Soal 3

> Budiman lupa, Ia harus membuat sistem operasi ini dengan sistem **Multi User** sesuai permintaan Dosennya. Ia meminta kembali kepadamu untuk membantunya membuat beberapa user beserta directory tiap usernya dibawah directory `home`. Buat pula password tiap user-usernya dan aplikasikan dalam sistem operasi tersebut!

> _Budiman forgot that he needs to create a **Multi User** system as requested by the lecturer. He asks your help again to create several users and their corresponding home directories under the `home` directory. Also set each user's password and apply them in the OS!_

**Format:** `user:pass`

```
root:Iniroot
Budiman:PassBudi
guest:guest
praktikan1:praktikan1
praktikan2:praktikan2
```

---

**Answer:**

### 🔧 Code:
````
mkdir -p home/Budiman
mkdir -p home/guest
mkdir -p home/praktikan1
mkdir -p home/praktikan2

openssl passwd -1 Iniroot
openssl passwd -1 PassBudi
openssl passwd -1 guest
openssl passwd -1 praktikan1
openssl passwd -1 praktikan2

mkdir -p etc
nano etc/passwd

root:$1$SEGe8exX$qW9GKvalmAS8kM375OldB0:0:0:root:/root:/bin/sh
Budiman:$1$eE6.AoMh$ZZxXhRDgnlmkx4nga1uze0:1001:100:Budiman:/home/Budiman:/bin/sh
guest:$1$qhjpaFL5$j5NCj3VeyQZQQ5ySk0c5E.:1002:100:guest:/home/guest:/bin/sh
praktikan1:$1$ZssiXJOo$dOXgibzv9MoJ/ufSFExQH/:1003:100:praktikan1:/home/praktikan1:/bin/sh
praktikan2:$1$P9pSOy06$MpCDjom8rr4Mfabw7YUPv.:1004:100:praktikan2:/home/praktikan2:/bin/sh

mkdir -p etc
nano etc/group
root:x:0:
bin:x:1:root
sys:x:2:root
tty:x:5:root,Budiman,guest,praktikan1,praktikan2
disk:x:6:root
wheel:x:10:root,Budiman
users:x:100:Budiman,guest,praktikan1,praktikan2

nano init

#!/bin/sh
/bin/mount -t proc none /proc
/bin/mount -t sysfs none /sys

while true
do
    /bin/getty -L tty1 115200 vt100
    sleep 1
done

chmod +x init

find . | cpio -oHnewc | gzip > ../myramdisk.gz
````

### 📖 Explanation:

### **1. Buat home directory untuk tiap user**

```bash
mkdir -p home/Budiman
mkdir -p home/guest
mkdir -p home/praktikan1
mkdir -p home/praktikan2
```

* Membuat direktori `home` beserta subdirektori pengguna.
* Diperlukan agar shell pengguna memiliki **home path** sesuai yang didefinisikan di `/etc/passwd`.

---

### **2. Buat password hash untuk tiap user**

```bash
openssl passwd -1 Iniroot
openssl passwd -1 PassBudi
openssl passwd -1 guest
openssl passwd -1 praktikan1
openssl passwd -1 praktikan2
```

* Perintah ini menghasilkan password **hash menggunakan MD5** (`-1` artinya MD5-based).
* Hash ini yang nanti dimasukkan ke file `/etc/passwd`.
* Ini membuat sistem initramfs bisa mengenali password login.

---

### **3. Buat file `/etc/passwd` secara manual**

```bash
mkdir -p etc
nano etc/passwd
```

> Isi contoh:

```
root:$1$SEGe8exX$qW9GKvalmAS8kM375OldB0:0:0:root:/root:/bin/sh
Budiman:$1$eE6.AoMh$ZZxXhRDgnlmkx4nga1uze0:1001:100:Budiman:/home/Budiman:/bin/sh
guest:$1$qhjpaFL5$j5NCj3VeyQZQQ5ySk0c5E.:1002:100:guest:/home/guest:/bin/sh
praktikan1:$1$ZssiXJOo$dOXgibzv9MoJ/ufSFExQH/:1003:100:praktikan1:/home/praktikan1:/bin/sh
praktikan2:$1$P9pSOy06$MpCDjom8rr4Mfabw7YUPv.:1004:100:praktikan2:/home/praktikan2:/bin/sh
```

* Format:

  ```
  username:password_hash:UID:GID:fullname:homedir:shell
  ```
* Setiap pengguna diberikan UID dan GID berbeda.
* Shell yang digunakan `/bin/sh` karena sistem hanya pakai busybox.

---

### **4. Buat file `/etc/group` untuk manajemen grup**

```bash
nano etc/group
```

> Isi contoh:

```
root:x:0:
bin:x:1:root
sys:x:2:root
tty:x:5:root,Budiman,guest,praktikan1,praktikan2
disk:x:6:root
wheel:x:10:root,Budiman
users:x:100:Budiman,guest,praktikan1,praktikan2
```

* Format:

  ```
  groupname:x:GID:members
  ```
* Grup `tty` → mengizinkan akses terminal (dibutuhkan untuk login)
* Grup `wheel` → biasanya digunakan untuk akses sudo (opsional di initramfs)
* Grup `users` → grup umum untuk semua user biasa

---

### **5. Buat file `init` untuk script booting sistem**

```bash
nano init
```

> Isi script:

```sh
#!/bin/sh
/bin/mount -t proc none /proc
/bin/mount -t sysfs none /sys

while true
do
    /bin/getty -L tty1 115200 vt100
    sleep 1
done
```

* `mount -t proc` dan `mount -t sysfs` → diperlukan agar beberapa fitur kernel berjalan
* `/bin/getty` → memunculkan prompt login di terminal `tty1`
* `-L` → Line mode
* `tty1` → terminal pertama
* `115200` → baudrate
* `vt100` → tipe terminal
* `sleep 1` → jeda antar panggilan `getty` agar tidak crash saat keluar

---

### **6. Jadikan file `init` bisa dieksekusi**

```bash
chmod +x init
```

* Supaya kernel bisa mengeksekusi file `init` saat initramfs dimuat.

---

### **7. Buat file initramfs gzip hasil kompresi dari direktori `myramdisk`**

```bash
find . | cpio -oHnewc | gzip > ../myramdisk.gz
```

* `find .` → mengambil semua file dan folder
* `cpio -oHnewc` → membuat archive cpio dengan format `newc` (yang bisa digunakan kernel)
* `gzip` → kompresi hasil ke file `myramdisk.gz`
* File `myramdisk.gz` ini akan digunakan nanti saat booting dengan kernel

---

### 🖼 Screenshot:

![Soal 3](https://github.com/Praktikum-NETICS-2025/sisop-modul-3-shzirley/blob/main/root.png?raw=true)

### Soal 4

> Dosen meminta Budiman membuat sistem operasi ini memilki **superuser** layaknya sistem operasi pada umumnya. User root yang sudah kamu buat sebelumnya akan digunakan sebagai superuser dalam sistem operasi milik Budiman. Superuser yang dimaksud adalah user dengan otoritas penuh yang dapat mengakses seluruhnya. Akan tetapi user lain tidak boleh memiliki otoritas yang sama. Dengan begitu user-user selain root tidak boleh mengakses `./root`. Buatlah sehingga tiap user selain superuser tidak dapat mengakses `./root`!

> _The lecturer requests that the OS must have a **superuser** just like other operating systems. The root user created earlier will serve as the superuser in Budiman's OS. The superuser should have full authority to access everything. However, other users should not have the same authority. Therefore, users other than root should not be able to access `./root`. Implement this so that non-superuser accounts cannot access `./root`!_

---

**Answer:**

### 🔧 Code:
````
```bash
chmod 700 root
chown root:root root
````

### 📖 Explanation:

* `chmod 700 root`
  Memberikan hak akses penuh (read, write, execute) **hanya untuk user root**. User lain tidak akan memiliki akses apapun ke folder ini.

* `chown root:root root`
  Menetapkan bahwa **user dan group owner** dari folder `root` adalah **root**. Ini penting agar proses login sebagai root atau script yang dijalankan sebagai root memiliki akses ke folder home-nya secara eksklusif.

---
### 🖼 Screenshot:

![Soal 4](https://github.com/Praktikum-NETICS-2025/sisop-modul-3-shzirley/blob/main/soalke4.png?raw=true)

### Soal 5

> Setiap user rencananya akan digunakan oleh satu orang tertentu. **Privasi dan otoritas tiap user** merupakan hal penting. Oleh karena itu, Budiman ingin membuat setiap user hanya bisa mengakses dirinya sendiri dan tidak bisa mengakses user lain. Buatlah sehingga sistem operasi Budiman dalam melakukan hal tersebut!

> _Each user is intended for an individual. **Privacy and authority** for each user are important. Therefore, Budiman wants to ensure that each user can only access their own files and not those of others. Implement this in Budiman's OS!_

---

**Answer:**

### 🔧 Code:
````
```bash
chmod 700 Budiman
chown 1001:100 Budiman

chmod 700 guest
chown 1002:100 guest

chmod 700 praktikan1
chown 1003:100 praktikan1

chmod 700 praktikan2
chown 1004:100 praktikan2

ls -ld home/*
````

### 📖 Explanation:

##### 🔧 Perintah `chmod 700`

* Memberikan hak akses penuh (read, write, execute) **hanya untuk pemilik direktori**.
* Grup dan user lain **tidak mendapat akses sama sekali**.
* Cocok digunakan agar **setiap user hanya bisa mengakses folder miliknya sendiri**.

##### 👤 Perintah `chown UID:GID`

* Mengubah kepemilikan direktori:

  * `UID` adalah **user ID**.
  * `GID` adalah **group ID**.
* Misalnya:

  * `chown 1001:100 Budiman` berarti direktori `Budiman` dimiliki oleh user dengan UID `1001` dan grup dengan GID `100`.

##### 📂 Perintah `ls -ld home/*`

* Menampilkan **izin akses, pemilik, dan grup** untuk masing-masing folder user di `/home`.
* Output contoh:

  ```
  drwx------ 2 1001 100 4096 Mei 28 10:00 Budiman
  drwx------ 2 1002 100 4096 Mei 28 10:00 guest
  drwx------ 2 1003 100 4096 Mei 28 10:00 praktikan1
  drwx------ 2 1004 100 4096 Mei 28 10:00 praktikan2
  ```
---

### 🖼 Screenshot:
![Soal 5](https://github.com/Praktikum-NETICS-2025/sisop-modul-3-shzirley/blob/main/soalkelima.png?raw=true)

### Soal 6

> Dosen Budiman menginginkan sistem operasi yang **stylish**. Budiman memiliki ide untuk membuat sistem operasinya menjadi stylish. Ia meminta kamu untuk menambahkan tampilan sebuah banner yang ditampilkan setelah suatu user login ke dalam sistem operasi Budiman. Banner yang diinginkan Budiman adalah tulisan `"Welcome to OS'25"` dalam bentuk **ASCII Art**. Buatkanlah banner tersebut supaya Budiman senang! (Hint: gunakan text to ASCII Art Generator)

> _Budiman wants a **stylish** operating system. Budiman has an idea to make his OS stylish. He asks you to add a banner that appears after a user logs in. The banner should say `"Welcome to OS'25"` in **ASCII Art**. Use a text to ASCII Art generator to make Budiman happy!_ (Hint: use a text to ASCII Art generator)

---

**Answer:**

### 🔧 Code:

```bash
nano /etc/motd
```

```text
  _      __      __                        __          ____   ____ _  ___   ____
 | | /| / /___  / /____ ___   __ _  ___   / /_ ___    / __ \ / __/( )|_  | / __/
 | |/ |/ // -_)/ // __// _ \ /  ' \/ -_) / __// _ \  / /_/ /_\ \  |// __/ /__ \ 
 |__/|__/ \__//_/ \__/ \___//_/_/_/\__/  \__/ \___/  \____//___/   /____//____/
```

```bash
nano /etc/profile
```

```bash
#!/bin/sh
```

---

### 📖 Explanation:

#### 1. Pemahaman Soal

Soal ini meminta kita untuk membuat sistem operasi "Budiman" terlihat **stylish** dengan cara:

* Menambahkan sebuah banner **ASCII Art** bertuliskan **"Welcome to OS'25"**.
* Banner tersebut harus tampil **setiap kali user login** ke terminal.

#### 2. Penjelasan Jawaban

```bash
nano /etc/motd
```

* File `/etc/motd` (Message of The Day) adalah file sistem pada Linux yang isinya akan ditampilkan **setelah user login** ke terminal.
* Dengan mengedit file ini dan menambahkan **ASCII Art**, kita bisa menyapa pengguna dengan tampilan yang menarik.

#### 3. ASCII Art Banner

```text
  _      __      __                        __          ____   ____ _  ___   ____
 | | /| / /___  / /____ ___   __ _  ___   / /_ ___    / __ \ / __/( )|_  | / __/
 | |/ |/ // -_)/ // __// _ \ /  ' \/ -_) / __// _ \  / /_/ /_\ \  |// __/ /__ \ 
 |__/|__/ \__//_/ \__/ \___//_/_/_/\__/  \__/ \___/  \____//___/   /____//____/
```

* Ini merupakan **banner ASCII Art** yang bertuliskan "Welcome to OS'25".
* Dibuat menggunakan [patorjk.com/software/taag](https://patorjk.com/software/taag) seperti yang disarankan dalam soal.
* Tujuannya adalah untuk membuat tampilan sistem operasi menjadi lebih **keren dan profesional**, sesuai dengan harapan Dosen Budiman.

---

### 🖼 Screenshot:

![Soal 6](https://github.com/Praktikum-NETICS-2025/sisop-modul-3-shzirley/blob/main/soal6.png?raw=true)


### Soal 7

> Melihat perkembangan sistem operasi milik Budiman, Dosen kagum dengan adanya banner yang telah kamu buat sebelumnya. Kemudian Dosen juga menginginkan sistem operasi Budiman untuk dapat menampilkan **kata sambutan** dengan menyebut nama user yang login. Sambutan yang dimaksud berupa kalimat `"Helloo %USER"` dengan `%USER` merupakan nama user yang sedang menggunakan sistem operasi. Kalimat sambutan ini ditampilkan setelah user login dan setelah banner. Budiman kembali lagi meminta bantuanmu dalam menambahkan fitur ini.

> _Seeing the progress of Budiman's OS, the lecturer is impressed with the banner you created. The lecturer also wants the OS to display a **greeting message** that includes the name of the user who logs in. The greeting should say `"Helloo %USER"` where `%USER` is the name of the user currently using the OS. This greeting should be displayed after user login and after the banner. Budiman asks for your help again to add this feature._

---

**Answer:**

### 🔧 Code:

```bash
nano /etc/profile
```

```bash
#!/bin/sh

case "$-" in
    i) ;;
    *) return;;
esac

if [ -z "$LOGIN_SH_DONE" ]; then
    export LOGIN_SH_DONE=1
    echo "Helloo $USER"
fi
```

---

### 📖 Explanation:

#### 1. Pemahaman Soal

Tujuan dari soal ini adalah:

* Menampilkan pesan **sambutan personal** setelah user login ke terminal.
* Pesan berbunyi:

```bash
Helloo <nama_user>
```

Contoh:

```bash
Helloo budiman
```

Pesan ini harus muncul **setelah banner ASCII Art** yang ditampilkan melalui `/etc/motd`.

---

#### 2. Penjelasan Jawaban

##### 1. Membuka dan Mengedit `/etc/profile`

```bash
nano /etc/profile
```

* File `/etc/profile` adalah skrip inisialisasi global yang dieksekusi setiap kali user login dengan **shell login** (misalnya menggunakan terminal).
* Cocok untuk menambahkan perintah sambutan setelah login.

---

##### 2. Isi Tambahan: Pemeriksaan Shell Interaktif

```bash
case "$-" in
    i) ;;
    *) return;;
esac
```

* Baris ini memastikan skrip hanya dijalankan pada **shell interaktif**.

  * `$-` menyimpan flag dari shell.
  * Jika bukan shell interaktif, maka skrip `return` dan tidak menampilkan apapun.
* Hal ini penting agar pesan sambutan **tidak muncul saat skrip dijalankan dari proses otomatis seperti cron atau SSH non-login**.

---

##### 3. Menampilkan Pesan Sambutan Sekali Saja

```bash
if [ -z "$LOGIN_SH_DONE" ]; then
    export LOGIN_SH_DONE=1
    echo "Helloo $USER"
fi
```

* **Penjelasan:**

  * Mengecek apakah variabel `LOGIN_SH_DONE` belum diset.
  * Jika belum, maka:

    * Variabel `LOGIN_SH_DONE` diset menjadi 1.
    * Cetak pesan sambutan dengan `echo`.
  * Variabel ini mencegah pesan muncul **berulang kali dalam satu sesi** login.

---

#### 3. Integrasi dengan ASCII Banner

* **Banner** ditambahkan di `/etc/motd`, dan otomatis muncul saat user login.
* **Pesan sambutan** ditambahkan di `/etc/profile`, yang dieksekusi **setelah** login dan **setelah** `/etc/motd` dibaca.
* Maka urutannya adalah:

  1. Banner muncul dari `/etc/motd`
  2. Pesan sambutan muncul dari `/etc/profile`

---

### 🖼 Screenshot:

![Soal 7](https://github.com/Praktikum-NETICS-2025/sisop-modul-3-shzirley/blob/main/soal7.png?raw=true)


### Soal 8

> Dosen Budiman sudah tua sekali, sehingga beliau memiliki kesulitan untuk melihat tampilan terminal default. Budiman menginisiatif untuk membuat tampilan sistem operasi menjadi seperti terminal milikmu. Modifikasilah sistem operasi Budiman menjadi menggunakan tampilan terminal kalian.

> _Budiman's lecturer is quite old and has difficulty seeing the default terminal display. Budiman takes the initiative to make the OS look like your terminal. Modify Budiman's OS to use your terminal appearance!_

**Answer:**

---

**Answer:**

### 🔧 Code:

```bash
nano /etc/profile
```

Kemudian tambahkan skrip berikut di dalam `/etc/profile`:

```sh
#!/bin/sh

case "$-" in
    *i*) ;;       # hanya lanjutkan jika shell interaktif
    *) return;;   # keluar jika bukan shell interaktif
esac

if [ -z "$LOGIN_SH_DONE" ]; then
    export LOGIN_SH_DONE=1
    echo "Helloo $USER"

    export PS1="\033[1;32m\u@\h:\033[1;34m\w\033[0m\$ "
    alias ls='ls --color=auto'
fi
```

---

### 📖 Explanation:

#### 1. Pemahaman Soal

* Dosen Budiman mengalami kesulitan melihat tampilan terminal default yang kurang kontras.
* Diminta mengubah tampilan terminal agar lebih nyaman dilihat, dengan warna prompt yang lebih mencolok dan warna output perintah `ls`.
* Hasil tampilan diharapkan lebih menarik dan mudah dibaca, seperti prompt warna hijau dan biru yang jelas.

---

#### 2. Penjelasan Jawaban

##### a. Mengedit `/etc/profile`

* File ini adalah skrip global yang dieksekusi setiap kali shell login untuk semua user.
* Dengan modifikasi ini, seluruh user termasuk Budiman akan mendapatkan perubahan tampilan.

---

##### b. Menjalankan Skrip Hanya di Shell Interaktif

```sh
case "$-" in
    *i*) ;; 
    *) return;;
esac
```

* Skrip hanya jalan di shell interaktif (terminal langsung), agar tidak mengganggu proses non-interaktif seperti cron, SSH non-login, dll.

---

##### c. Cegah Eksekusi Berulang

```sh
if [ -z "$LOGIN_SH_DONE" ]; then
    export LOGIN_SH_DONE=1
```

* Variabel ini memastikan skrip ini hanya dijalankan sekali per sesi, mencegah spam output atau alias duplikat jika membuka beberapa terminal sekaligus.

---

##### d. Menampilkan Pesan Sambutan

```sh
echo "Helloo $USER"
```

* Menyapa user dengan pesan yang personal setelah login.

---

##### e. Mengubah Prompt Terminal (`PS1`)

```sh
export PS1="\033[1;32m\u@\h:\033[1;34m\w\033[0m\$ "
```

* Warna hijau terang untuk `username@hostname`.
* Warna biru terang untuk direktori kerja saat ini.
* Reset warna di akhir supaya teks lain normal.
* Membuat prompt lebih jelas dan nyaman dibaca.

---

##### f. Alias `ls` Berwarna

```sh
alias ls='ls --color=auto'
```

* Output perintah `ls` diberi warna sesuai tipe file (folder, executable, link, dll).
* Memudahkan membedakan jenis file secara visual.

---

### 🖼 Screenshot:

![Soal 8](https://github.com/Praktikum-NETICS-2025/sisop-modul-3-shzirley/blob/main/soal8.png?raw=true)

---

### Soal 9

> Ketika mencoba sistem operasi buatanmu, Budiman tidak bisa mengubah text file menggunakan text editor. Budiman pun menyadari bahwa dalam sistem operasi yang kamu buat tidak memiliki text editor. Budimanpun menyuruhmu untuk menambahkan **binary** yang telah disiapkan sebelumnya ke dalam sistem operasinya. Buatlah sehingga sistem operasi Budiman memiliki **binary text editor** yang telah disiapkan!

> _When trying your OS, Budiman cannot edit text files using a text editor. He realizes that the OS you created does not have a text editor. Budiman asks you to add the prepared **binary** into his OS. Make sure Budiman's OS has the prepared **text editor binary**!_

---

**Answer:**

### 🔧 Code:

```bash
git clone https://github.com/morisab/budiman-text-editor.git
cd budiman-text-editor
g++ -static -o budiman main.cpp
chmod +x budiman
mv budiman ../bin
```

---

### 📖 Explanation:

#### 1. Pemahaman Soal

* Budiman tidak bisa menggunakan text editor karena sistem tidak memilikinya.
* Kita diminta menambahkan binary text editor ke sistem operasi Budiman.
* Source code text editor sudah disediakan dalam repository Git.
* Tujuan akhirnya adalah agar text editor dapat dijalankan secara langsung dari sistem.

---

#### 2. Penjelasan Jawaban

##### a. Clone Source Code

```bash
git clone https://github.com/morisab/budiman-text-editor.git
```

* Mengambil seluruh kode sumber text editor dari GitHub.

---

##### b. Masuk ke Direktori Source Code

```bash
cd budiman-text-editor
```

* Berpindah ke folder yang berisi source code untuk dikompilasi.

---

##### c. Kompilasi Source Code dengan `g++`

```bash
g++ -static -o budiman main.cpp
```

* Mengompilasi `main.cpp` menjadi executable bernama `budiman`.
* Opsi `-static` membuat binary statis:

  * Semua library yang dibutuhkan disertakan di dalam binary.
  * Sangat berguna untuk sistem minimalis tanpa library dinamis.

---

##### d. Berikan Permission Eksekusi

```bash
chmod +x budiman
```

* Membuat file executable dapat dijalankan.

---

##### e. Pindahkan Binary ke Folder `bin`

```bash
mv budiman ../bin
```

* Memindahkan executable ke direktori `bin` yang biasanya sudah ada di `$PATH`.
* Sehingga text editor dapat dijalankan dari mana saja dalam terminal.

---

### 🖼 Screenshot:

![Soal 9](https://github.com/Praktikum-NETICS-2025/sisop-modul-3-shzirley/blob/main/soal9.png?raw=true)

---


### Soal 10

> Setelah seluruh fitur yang diminta Dosen dipenuhi dalam sistem operasi Budiman, sudah waktunya Budiman mengumpulkan tugasnya ini ke Dosen. Akan tetapi, Dosen Budiman tidak mau menerima pengumpulan selain dalam bentuk **.iso**. Untuk terakhir kalinya, Budiman meminta tolong kepadamu untuk mengubah seluruh konfigurasi sistem operasi yang telah kamu buat menjadi sebuah **file .iso**.

> After all the features requested by the lecturer have been implemented in Budiman's OS, it's time for Budiman to submit his assignment. However, Budiman's lecturer only accepts submissions in the form of **.iso** files. For the last time, Budiman asks for your help to convert the entire configuration of the OS you created into a **.iso file**.

Berikut adalah jawaban lengkap dan terstruktur untuk **soal nomor 10**, sudah disesuaikan dengan format penjelasan sebelumnya:

---

**Answer:**

### 🔧 Code:

```bash
cd osboot

mkdir -p mylinuxiso/boot/grub

cp bzImage mylinuxiso/boot
cp myramdisk.gz mylinuxiso/boot

cat > mylinuxiso/boot/grub/grub.cfg << EOF
set timeout=5
set default=0

menuentry "MyLinux" {
  linux /boot/bzImage
  initrd /boot/myramdisk.gz
}
EOF

grub-mkrescue -o mylinux.iso mylinuxiso

qemu-system-x86_64 \
  -smp 2 \
  -m 256 \
  -display curses \
  -vga std \
  -cdrom mylinux.iso
```

---

### 📖 Explanation:

#### 1. Pemahaman Soal

* Budiman ingin mengubah sistem operasinya menjadi file `.iso` agar dapat diserahkan ke dosen.
* File `.iso` tersebut harus:

  * Memiliki kernel (`bzImage`) dan ramdisk (`myramdisk.gz`)
  * Bisa di-*boot* menggunakan GRUB
  * Bisa dijalankan menggunakan emulator seperti QEMU

---

#### 2. Penjelasan Baris per Baris

##### a. Masuk ke direktori kerja

```bash
cd osboot
```

* Masuk ke direktori utama sistem operasi Budiman.

---

##### b. Membuat struktur direktori ISO bootable

```bash
mkdir -p mylinuxiso/boot/grub
```

* Membuat folder untuk menyimpan kernel, initrd, dan konfigurasi GRUB.

---

##### c. Menyalin file kernel dan initrd

```bash
cp bzImage mylinuxiso/boot
cp myramdisk.gz mylinuxiso/boot
```

* Menyalin file kernel (`bzImage`) dan file RAM disk (`myramdisk.gz`) ke folder boot ISO.

---

##### d. Menyusun konfigurasi GRUB (grub.cfg)

```bash
cat > mylinuxiso/boot/grub/grub.cfg << EOF
set timeout=5
set default=0

menuentry "MyLinux" {
  linux /boot/bzImage
  initrd /boot/myramdisk.gz
}
EOF
```

* Membuat file konfigurasi GRUB (`grub.cfg`) secara otomatis.
* Konfigurasi ini menentukan menu boot dengan entry `MyLinux`, yang akan menjalankan kernel dan ramdisk.

---

##### e. Membuat file ISO menggunakan GRUB

```bash
grub-mkrescue -o mylinux.iso mylinuxiso
```

* Membuat file `.iso` bernama `mylinux.iso` dari folder `mylinuxiso/`.
* Perintah ini akan menghasilkan ISO yang bisa di-*boot* melalui BIOS/UEFI.

---

##### f. Menjalankan ISO dengan QEMU

```bash
qemu-system-x86_64 \
  -smp 2 \
  -m 256 \
  -display curses \
  -vga std \
  -cdrom mylinux.iso
```

* Menguji hasil file `.iso` menggunakan emulator QEMU.
* Konfigurasi:

  * 2 core CPU (`-smp 2`)
  * 256 MB RAM (`-m 256`)
  * Output berbasis teks (`-display curses`)
  * Menggunakan ISO buatan (`-cdrom mylinux.iso`)

---

### 🖼 Screenshot Contoh:

![Soal 10](https://github.com/Praktikum-NETICS-2025/sisop-modul-3-shzirley/blob/main/soal10.png?raw=true)

---

Pada akhirnya sistem operasi Budiman yang telah kamu buat dengan susah payah dikumpulkan ke Dosen mengatasnamakan Budiman. Kamu tidak diberikan credit apapun. Budiman pun tidak memberikan kata terimakasih kepadamu. Kamupun kecewa tetapi setidaknya kamu telah belajar untuk menjadi pembuat sistem operasi sederhana yang andal. Selamat!

_At last, the OS you painstakingly created was submitted to the lecturer under Budiman's name. You received no credit. Budiman didn't even thank you. You feel disappointed, but at least you've learned to become a reliable creator of simple operating systems. Congratulations!_
