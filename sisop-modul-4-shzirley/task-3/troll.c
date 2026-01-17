#define _GNU_SOURCE
#define FUSE_USE_VERSION 26
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <pwd.h>

static const char *spicy_name = "very_spicy_info.txt";
static const char *upload_name = "upload.txt";
static const char *state_path = "/tmp/trap_state";
static const char *final_state_path = "/tmp/trap_final_state";

// cek apakah jebakan sudah aktif
static int is_trap_triggered() {
    return access(state_path, F_OK) == 0;
}

// cek apakah jebakan final (echo sudah di-upload)
static int is_final_triggered() {
    return access(final_state_path, F_OK) == 0;
}

// helper membuat file
static void create_file(const char *path) {
    FILE *fp = fopen(path, "w");
    if (fp) fclose(fp);
}

// getattr
static int troll_getattr(const char *path, struct stat *stbuf) {
    memset(stbuf, 0, sizeof(struct stat));
    if (strcmp(path, "/") == 0) {
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
    } else if (strcmp(path+1, spicy_name) == 0 || strcmp(path+1, upload_name) == 0) {
        stbuf->st_mode = S_IFREG | 0666;
        stbuf->st_nlink = 1;
        stbuf->st_size = 1024;
    } else {
        return -ENOENT;
    }
    return 0;
}

// readdir
static int troll_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                         off_t offset, struct fuse_file_info *fi) {
    (void) offset; (void) fi;
    if (strcmp(path, "/") != 0) return -ENOENT;
    filler(buf, ".", NULL, 0);
    filler(buf, "..", NULL, 0);
    filler(buf, spicy_name, NULL, 0);
    filler(buf, upload_name, NULL, 0);
    return 0;
}

// open
static int troll_open(const char *path, struct fuse_file_info *fi) {
    return 0;
}

// read
static int troll_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    struct fuse_context *ctx = fuse_get_context();
    uid_t caller_uid = ctx->uid;
    struct passwd *pw = getpwuid(caller_uid);

    // Kalau jebakan final → semua txt fell for it
    if (is_final_triggered() && strstr(path, ".txt")) {
        const char *ascii_art = "Fell for it again reward\n";
        size_t len = strlen(ascii_art);
        if (offset < len) {
            if (offset + size > len) size = len - offset;
            memcpy(buf, ascii_art + offset, size);
            return size;
        }
        return 0;
    }

    // Kalau baca upload → aktifkan jebakan
    if (strcmp(path+1, upload_name) == 0) {
        if (pw && strcmp(pw->pw_name, "DainTontas") == 0 && !is_trap_triggered()) {
            create_file(state_path);
        }
        return 0;
    }

    // Kalau baca spicy
    if (strcmp(path+1, spicy_name) == 0) {
        const char *content;
        if (is_trap_triggered()) {
            // jebakan aktif
            if (pw && strcmp(pw->pw_name, "DainTontas") == 0) {
                content = "Very spicy internal developer information: leaked roadmap.docx\n";
            } else {
                content = "DainTontas' personal secret!!.txt\n";
            }
        } else {
            content = "This is very_spicy_info.txt — nothing suspicious here.\n";
        }
        size_t len = strlen(content);
        if (offset < len) {
            if (offset + size > len) size = len - offset;
            memcpy(buf, content + offset, size);
            return size;
        }
        return 0;
    }

    return -ENOENT;
}

// write (mengaktifkan jebakan final kalau DainTontas echo upload ke upload.txt)
static int troll_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    struct fuse_context *ctx = fuse_get_context();
    uid_t caller_uid = ctx->uid;
    struct passwd *pw = getpwuid(caller_uid);

    if (strcmp(path+1, upload_name) == 0) {
        // kalau user DainTontas
        if (pw && strcmp(pw->pw_name, "DainTontas") == 0 && is_trap_triggered()) {
            // buat final state
            create_file(final_state_path);
        }
        return size;
    }

    if (strcmp(path+1, spicy_name) == 0) {
        return size;
    }

    return -EACCES;
}

// create
static int troll_create(const char *path, mode_t mode, struct fuse_file_info *fi) {
    return 0;
}

// truncate
static int troll_truncate(const char *path, off_t size) {
    return 0;
}

// jangan dihapus
static int troll_unlink(const char *path) {
    return -EACCES;
}

// fuse ops
static struct fuse_operations troll_ops = {
    .getattr = troll_getattr,
    .readdir = troll_readdir,
    .open    = troll_open,
    .read    = troll_read,
    .write   = troll_write,
    .create  = troll_create,
    .truncate= troll_truncate,
    .unlink  = troll_unlink,
};

int main(int argc, char *argv[]) {
    return fuse_main(argc, argv, &troll_ops, NULL);
}
