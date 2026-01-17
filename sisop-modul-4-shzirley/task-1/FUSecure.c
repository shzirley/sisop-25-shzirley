#define FUSE_USE_VERSION 31

#include <pwd.h>
#include <fuse3/fuse.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdlib.h>
#include <sys/stat.h>

const char *source_dir = "/home/shared_files";

// ngambil UID pemanggil (bukan UID proses root)
uid_t get_calling_uid() {
    return fuse_get_context()->uid;
}

static int fuse_getattr(const char *path, struct stat *stbuf,
                        struct fuse_file_info *fi) {
    (void) fi;
    uid_t uid = get_calling_uid();

    // membatasi akses ke folder dan isinya jika bukan pemilik
    if (strstr(path, "/private_yuadi") && uid != 1001)
        return -EACCES;
    if (strstr(path, "/private_irwandi") && uid != 1002)
        return -EACCES;

    char full_path[1024];
    snprintf(full_path, sizeof(full_path), "%s%s", source_dir, path);
    return lstat(full_path, stbuf);
}

static int fuse_readdir(const char *path, void *buf,
                        fuse_fill_dir_t filler, off_t offset,
                        struct fuse_file_info *fi,
                        enum fuse_readdir_flags flags){
    (void) offset;
    (void) fi;
    (void) flags;

    DIR *dp;
    struct dirent *de;
    char full_path[1024];

    // ngambil UID & nama user yang mengakses
    uid_t uid = fuse_get_context()->uid;
    struct passwd *pw = getpwuid(uid);
    if (!pw) return -EACCES;

    const char *username = pw->pw_name;
    char private_dirname[64];
    snprintf(private_dirname, sizeof(private_dirname), "private_%s", username);

    snprintf(full_path, sizeof(full_path), "%s%s", source_dir, path);
    dp = opendir(full_path);
    if (dp == NULL)
        return -errno;

    while ((de = readdir(dp)) != NULL){
        if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0)
            continue;

        if (strcmp(path, "/") == 0) {
            if (strncmp(de->d_name, "private_", 8) == 0 &&
                strcmp(de->d_name, private_dirname) != 0)
                continue;
        }

        if (strstr(path, "/private_") != NULL &&
            strstr(path, private_dirname) == NULL)
            continue;

        filler(buf, de->d_name, NULL, 0, 0);
    }

    closedir(dp);
    return 0;
}

static int fuse_open(const char *path, struct fuse_file_info *fi){
    char full_path[1024];
    uid_t uid = get_calling_uid();
    snprintf(full_path, sizeof(full_path), "%s%s", source_dir, path);

    // nolak akses tulis
    if ((fi->flags & O_WRONLY) || (fi->flags & O_RDWR)){
        return -EROFS;
    }

    if (strstr(path, "/private_yuadi") && uid != 1001)
        return -EACCES;
    if (strstr(path, "/private_irwandi") && uid != 1002)
        return -EACCES;

    int fd = open(full_path, O_RDONLY);
    if (fd == -1) return -errno;
    close(fd);
    return 0;
}

static int fuse_read(const char *path, char *buf, size_t size,
                     off_t offset, struct fuse_file_info *fi){
    char full_path[1024];
    snprintf(full_path, sizeof(full_path), "%s%s", source_dir, path);

    int fd = open(full_path, O_RDONLY);
    if (fd == -1) return -errno;

    int res = pread(fd, buf, size, offset);
    if (res == -1) res = -errno;
    close(fd);
    return res;
}

// semua operasi write/modifikasi ditolak dengan -EROFS (Read-only file system)
static int fuse_mkdir(const char *path, mode_t mode){
    return -EROFS;
}
static int fuse_rmdir(const char *path){
    return -EROFS;
}
static int fuse_write(const char *path, const char *buf, size_t size,
                      off_t offset, struct fuse_file_info *fi){
    return -EROFS;
}
static int fuse_unlink(const char *path){
    return -EROFS;
}
static int fuse_rename(const char *from, const char *to,
                       unsigned int flags){
    return -EROFS;
}
static int fuse_create(const char *path, mode_t mode,
                       struct fuse_file_info *fi){
    return -EROFS;
}
static int fuse_chmod(const char *path, mode_t mode,
                      struct fuse_file_info *fi){
    return -EROFS;
}
static int fuse_chown(const char *path, uid_t uid, gid_t gid,
                      struct fuse_file_info *fi){
    return -EROFS;
}

static const struct fuse_operations fuse_example_oper ={
    .getattr = fuse_getattr,
    .readdir = fuse_readdir,
    .open    = fuse_open,
    .read    = fuse_read,

    // blokir semua write ops
    .mkdir   = fuse_mkdir,
    .rmdir   = fuse_rmdir,
    .write   = fuse_write,
    .unlink  = fuse_unlink,
    .rename  = fuse_rename,
    .create  = fuse_create,
    .chmod   = fuse_chmod,
    .chown   = fuse_chown,
};

int main(int argc, char *argv[]){
    umask(0);
    return fuse_main(argc, argv, &fuse_example_oper, NULL);
  
}
