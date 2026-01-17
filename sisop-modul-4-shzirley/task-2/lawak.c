#define FUSE_USE_VERSION 31

#include <fuse3/fuse.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <stdbool.h>
#include <pwd.h>
#include <limits.h>

#define MAX_FILTER_WORDS 100
#define CONFIG_FILE "lawak.conf"
#define LOG_PATH "/var/log/lawakfs.log"
#define MAX_READ_SIZE (1024 * 1024)
#define MAX_CONFIG_LINE 2048

char source_dir[PATH_MAX];

struct Config{
    char secret_basename[NAME_MAX];
    int access_start_hour, access_start_min;
    int access_end_hour, access_end_min;
    char *filter_words[MAX_FILTER_WORDS];
    int filter_word_count;
    bool config_loaded;
} config ={0};

static const char base64_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

size_t base64_encode(const unsigned char *src, size_t len, char *out, size_t out_max){
    if (!src || !out || len == 0 || out_max == 0) return 0;
    
    size_t out_len = 0;
    size_t required_size = ((len + 2) / 3) * 4 + 1;
    
    if (required_size > out_max){
        return 0;
    }
    
    for (size_t i = 0; i < len; i += 3){
        unsigned int val = 0;
        int padding = 0;
        
        for (int j = 0; j < 3; j++){
            val <<= 8;
            if (i + j < len){
                val |= src[i + j];
            } else{
                padding++;
            }
        }
        
        for (int j = 0; j < 4; j++){
            if (j > 1 && j <= padding){
                out[out_len++] = '=';
            } else{
                out[out_len++] = base64_table[(val >> (18 - j * 6)) & 0x3F];
            }
        }
    }
    
    out[out_len] = '\0';
    return out_len;
}

bool validate_time_format(int hour, int min){
    return (hour >= 0 && hour < 24 && min >= 0 && min < 60);
}

void cleanup_config(){
    for (int i = 0; i < config.filter_word_count; i++){
        if (config.filter_words[i]){
            free(config.filter_words[i]);
            config.filter_words[i] = NULL;
        }
    }
    config.filter_word_count = 0;
}

void load_config(){
    FILE *fp = fopen(CONFIG_FILE, "r");
    if (!fp){
        fprintf(stderr, "Warning: Cannot open config file %s\n", CONFIG_FILE);
        return;
    }
    
    cleanup_config();
    
    char line[MAX_CONFIG_LINE];
    int line_num = 0;
    
    while (fgets(line, sizeof(line), fp)){
        line_num++;
        
        if (line[0] == '\n' || line[0] == '#') continue;
        
        char *eq = strchr(line, '=');
        if (!eq){
            fprintf(stderr, "Warning: Invalid config line %d: missing '='\n", line_num);
            continue;
        }
        
        *eq = '\0';
        char *key = line;
        char *val = eq + 1;
        
        while (*key == ' ' || *key == '\t') key++;
        while (*val == ' ' || *val == '\t') val++;
        val[strcspn(val, "\n\r")] = '\0';
        
        if (strlen(key) == 0 || strlen(val) == 0){
            fprintf(stderr, "Warning: Empty key or value at line %d\n", line_num);
            continue;
        }

        if (strcmp(key, "SECRET_FILE_BASENAME") == 0){
            if (strlen(val) >= NAME_MAX){
                fprintf(stderr, "Warning: SECRET_FILE_BASENAME too long, truncating\n");
                val[NAME_MAX-1] = '\0';
            }
            strncpy(config.secret_basename, val, NAME_MAX-1);
            config.secret_basename[NAME_MAX-1] = '\0';
        } else if (strcmp(key, "ACCESS_START") == 0){
            int h, m;
            if (sscanf(val, "%d:%d", &h, &m) != 2 || !validate_time_format(h, m)){
                fprintf(stderr, "Warning: Invalid ACCESS_START format: %s\n", val);
                continue;
            }
            config.access_start_hour = h;
            config.access_start_min = m;
        } else if (strcmp(key, "ACCESS_END") == 0){
            int h, m;
            if (sscanf(val, "%d:%d", &h, &m) != 2 || !validate_time_format(h, m)){
                fprintf(stderr, "Warning: Invalid ACCESS_END format: %s\n", val);
                continue;
            }
            config.access_end_hour = h;
            config.access_end_min = m;
        } else if (strcmp(key, "FILTER_WORDS") == 0){
            char *val_copy = strdup(val);
            if (!val_copy){
                fprintf(stderr, "Warning: Memory allocation failed for filter words\n");
                continue;
            }
            
            char *token = strtok(val_copy, ",");
            while (token && config.filter_word_count < MAX_FILTER_WORDS){
                while (*token == ' ' || *token == '\t') token++;
                char *end = token + strlen(token) - 1;
                while (end > token && (*end == ' ' || *end == '\t')){
                    *end = '\0';
                    end--;
                }
                
                if (strlen(token) > 0){
                    config.filter_words[config.filter_word_count] = strdup(token);
                    if (config.filter_words[config.filter_word_count]){
                        config.filter_word_count++;
                    }
                }
                token = strtok(NULL, ",");
            }
            free(val_copy);
        }
    }
    fclose(fp);
    config.config_loaded = true;
    
    printf("Config loaded: secret_basename='%s', access_time=%02d:%02d-%02d:%02d, filter_words=%d\n",
           config.secret_basename, config.access_start_hour, config.access_start_min,
           config.access_end_hour, config.access_end_min, config.filter_word_count);
}

bool is_secret_file(const char *path){
    if (!config.config_loaded || strlen(config.secret_basename) == 0){
        return false;
    }
    
    const char *base = strrchr(path, '/');
    base = base ? base + 1 : path;
    return strncmp(base, config.secret_basename, strlen(config.secret_basename)) == 0;
}

bool is_allowed_hour(){
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    if (!t) return false;
    
    int now_min = t->tm_hour * 60 + t->tm_min;
    int start = config.access_start_hour * 60 + config.access_start_min;
    int end = config.access_end_hour * 60 + config.access_end_min;
    
    if (end <= start){
        return now_min >= start || now_min < end;
    }
    return now_min >= start && now_min < end;
}

void log_action(const char *action, const char *path){
    FILE *logf = fopen(LOG_PATH, "a");
    if (!logf) return;
    
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    if (!t){
        fclose(logf);
        return;
    }
    
    char timestr[32];
    strftime(timestr, sizeof(timestr), "%Y-%m-%d %H:%M:%S", t);
    uid_t uid = getuid();
    
    const char *safe_path = path ? path : "(null)";
    
    fprintf(logf, "[%s] UID:%d %s %s\n", timestr, uid, action, safe_path);
    fclose(logf);
}

void filter_text(char *buf, size_t len){
    if (!buf || len == 0 || config.filter_word_count == 0) return;
    
    for (int i = 0; i < config.filter_word_count; i++){
        if (!config.filter_words[i]) continue;
        
        char *word = config.filter_words[i];
        size_t word_len = strlen(word);
        if (word_len == 0) continue;
        
        char *p = buf;
        while ((p = strcasestr(p, word)) && (size_t)(p - buf + word_len) <= len){
            size_t replace_len = (word_len < 5) ? word_len : 5;
            memcpy(p, "lawak", replace_len);
            if (word_len > 5){
                memset(p + 5, ' ', word_len - 5);
            }
            p += word_len;
        }
    }
}

bool is_text_file(const char *path){
    const char *ext = strrchr(path, '.');
    if (!ext) return false;
    return (strcmp(ext, ".txt") == 0 || strcmp(ext, ".md") == 0 || 
            strcmp(ext, ".log") == 0 || strcmp(ext, ".conf") == 0 ||
            strcmp(ext, ".c") == 0 || strcmp(ext, ".h") == 0 ||
            strcmp(ext, ".py") == 0 || strcmp(ext, ".js") == 0);
}

char *find_real_file(const char *filename){
    if (!filename || strlen(filename) == 0) return NULL;
    
    DIR *dp = opendir(source_dir);
    if (!dp) return NULL;
    
    struct dirent *de;
    char *result = NULL;
    
    while ((de = readdir(dp)) != NULL){
        if (de->d_type == DT_REG || de->d_type == DT_UNKNOWN){
            char *dot = strrchr(de->d_name, '.');
            if (dot){
                size_t base_len = dot - de->d_name;
                const char *fname = (filename[0] == '/') ? filename + 1 : filename;
                
                if (base_len == strlen(fname) && 
                    strncmp(de->d_name, fname, base_len) == 0){
                    result = malloc(PATH_MAX);
                    if (result){
                        int ret = snprintf(result, PATH_MAX, "%s/%s", source_dir, de->d_name);
                        if (ret >= PATH_MAX){
                            free(result);
                            result = NULL;
                        }
                    }
                    break;
                }
            }
        }
    }
    closedir(dp);
    return result;
}

static int lawakfs_getattr(const char *path, struct stat *st, struct fuse_file_info *fi){
    (void) fi;
    
    if (!path || !st) return -EINVAL;
    
    if (is_secret_file(path) && !is_allowed_hour()){
        log_action("BLOCKED_ACCESS", path);
        return -ENOENT;
    }
    
    char *real = find_real_file(path);
    if (!real) return -ENOENT;
    
    int res = lstat(real, st);
    int saved_errno = errno;
    free(real);
    
    return (res == -1) ? -saved_errno : 0;
}

static int lawakfs_open(const char *path, struct fuse_file_info *fi){
    if (!path) return -EINVAL;
    
    if (is_secret_file(path) && !is_allowed_hour()){
        log_action("BLOCKED_OPEN", path);
        return -ENOENT;
    }
    
    char *real = find_real_file(path);
    if (!real) return -ENOENT;
    
    int fd = open(real, O_RDONLY);
    int saved_errno = errno;
    free(real);
    
    if (fd == -1) return -saved_errno;
    
    close(fd);
    return 0;
}

static int lawakfs_read(const char *path, char *buf, size_t size, off_t offset, 
                       struct fuse_file_info *fi){
    (void) fi;
    
    if (!path || !buf || size == 0) return -EINVAL;
    
    if (is_secret_file(path) && !is_allowed_hour()){
        log_action("BLOCKED_READ", path);
        return -ENOENT;
    }
    
    char *real = find_real_file(path);
    if (!real) return -ENOENT;
    
    int fd = open(real, O_RDONLY);
    if (fd == -1){
        int saved_errno = errno;
        free(real);
        return -saved_errno;
    }
    
    struct stat st;
    if (fstat(fd, &st) == -1){
        int saved_errno = errno;
        close(fd);
        free(real);
        return -saved_errno;
    }
    
    size_t max_read = (st.st_size > MAX_READ_SIZE) ? MAX_READ_SIZE : st.st_size;
    if (offset >= st.st_size){
        close(fd);
        free(real);
        return 0;
    }
    
    if (is_text_file(real)){
        char *tmp = malloc(max_read + 1);
        if (!tmp){
            close(fd);
            free(real);
            return -ENOMEM;
        }
        
        ssize_t len = pread(fd, tmp, max_read, offset);
        close(fd);
        
        if (len == -1){
            int saved_errno = errno;
            free(tmp);
            free(real);
            return -saved_errno;
        }
        
        if (len == 0){
            free(tmp);
            free(real);
            return 0;
        }
        
        tmp[len] = '\0';
        filter_text(tmp, len);
        
        size_t copy_len = (len < (ssize_t)size) ? len : size;
        memcpy(buf, tmp, copy_len);
        
        free(tmp);
        free(real);
        log_action("READ_TEXT", path);
        return copy_len;
        
    } else{
        char *tmp = malloc(max_read);
        if (!tmp){
            close(fd);
            free(real);
            return -ENOMEM;
        }
        
        ssize_t len = pread(fd, tmp, max_read, 0);
        close(fd);
        
        if (len == -1){
            int saved_errno = errno;
            free(tmp);
            free(real);
            return -saved_errno;
        }
        
        if (len == 0){
            free(tmp);
            free(real);
            return 0;
        }
        
        size_t encoded_max_size = ((len + 2) / 3) * 4 + 1;
        char *encoded = malloc(encoded_max_size);
        if (!encoded){
            free(tmp);
            free(real);
            return -ENOMEM;
        }
        
        size_t enc_len = base64_encode((unsigned char *)tmp, len, encoded, encoded_max_size);
        free(tmp);
        
        if (enc_len == 0){
            free(encoded);
            free(real);
            return -EIO;
        }
        
        if (offset >= (off_t)enc_len){
            free(encoded);
            free(real);
            return 0;
        }
        
        size_t remaining = enc_len - offset;
        size_t copy_len = (remaining < size) ? remaining : size;
        memcpy(buf, encoded + offset, copy_len);
        
        free(encoded);
        free(real);
        log_action("READ_BINARY", path);
        return copy_len;
    }
}

static int lawakfs_access(const char *path, int mask){
    if (!path) return -EINVAL;
    
    if (is_secret_file(path) && !is_allowed_hour()){
        log_action("BLOCKED_ACCESS_CHECK", path);
        return -ENOENT;
    }
    
    char *real = find_real_file(path);
    if (!real) return -ENOENT;
    
    int res = access(real, mask);
    int saved_errno = errno;
    free(real);
    
    if (res != -1){
        log_action("ACCESS_CHECK", path);
    }
    
    return (res == -1) ? -saved_errno : 0;
}

static int lawakfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                          off_t offset, struct fuse_file_info *fi, 
                          enum fuse_readdir_flags flags){
    (void) offset;
    (void) fi;
    (void) flags;
    
    if (!path || !buf || !filler) return -EINVAL;
    
    char fpath[PATH_MAX];
    int ret = snprintf(fpath, sizeof(fpath), "%s%s", source_dir, path);
    if (ret >= PATH_MAX) return -ENAMETOOLONG;
    
    DIR *dp = opendir(fpath);
    if (!dp) return -errno;
    
    filler(buf, ".", NULL, 0, 0);
    filler(buf, "..", NULL, 0, 0);
    
    struct dirent *de;
    while ((de = readdir(dp)) != NULL){
        if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0){
            continue;
        }
        
        if (is_secret_file(de->d_name) && !is_allowed_hour()){
            continue;
        }
        
        char name[NAME_MAX];
        char *dot = strrchr(de->d_name, '.');
        if (dot && de->d_type == DT_REG){
            size_t len = dot - de->d_name;
            if (len < NAME_MAX){
                strncpy(name, de->d_name, len);
                name[len] = '\0';
            } else{
                strncpy(name, de->d_name, NAME_MAX-1);
                name[NAME_MAX-1] = '\0';
            }
        } else{
            strncpy(name, de->d_name, NAME_MAX-1);
            name[NAME_MAX-1] = '\0';
        }
        
        filler(buf, name, NULL, 0, 0);
    }
    
    closedir(dp);
    return 0;
}

static int lawakfs_write(const char *path, const char *buf, size_t size, 
                        off_t offset, struct fuse_file_info *fi){
    (void) path; (void) buf; (void) size; (void) offset; (void) fi;
    return -EROFS;
}

static int lawakfs_mkdir(const char *path, mode_t mode){
    (void) path; (void) mode;
    return -EROFS;
}

static int lawakfs_unlink(const char *path){
    (void) path;
    return -EROFS;
}

static int lawakfs_rmdir(const char *path){
    (void) path;
    return -EROFS;
}

static int lawakfs_rename(const char *from, const char *to, unsigned int flags){
    (void) from; (void) to; (void) flags;
    return -EROFS;
}

static int lawakfs_create(const char *path, mode_t mode, struct fuse_file_info *fi){
    (void) path; (void) mode; (void) fi;
    return -EROFS;
}

static int lawakfs_truncate(const char *path, off_t size, struct fuse_file_info *fi){
    (void) path; (void) size; (void) fi;
    return -EROFS;
}

static const struct fuse_operations lawakfs_oper ={
    .getattr  = lawakfs_getattr,
    .readdir  = lawakfs_readdir,
    .open     = lawakfs_open,
    .read     = lawakfs_read,
    .access   = lawakfs_access,
    .write    = lawakfs_write,
    .mkdir    = lawakfs_mkdir,
    .unlink   = lawakfs_unlink,
    .rmdir    = lawakfs_rmdir,
    .rename   = lawakfs_rename,
    .create   = lawakfs_create,
    .truncate = lawakfs_truncate,
};

int main(int argc, char *argv[]){
    umask(0022);
    
    if (argc < 3){
        fprintf(stderr, "Usage: %s <source_dir> <mountpoint> [fuse_options]\n", argv[0]);
        fprintf(stderr, "Example: %s /home/user/data /mnt/lawakfs -f\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    
    struct stat st;
    if (stat(argv[1], &st) != 0){
        perror("Source directory");
        exit(EXIT_FAILURE);
    }
    
    if (!S_ISDIR(st.st_mode)){
        fprintf(stderr, "Error: %s is not a directory\n", argv[1]);
        exit(EXIT_FAILURE);
    }
    
    if (!realpath(argv[1], source_dir)){
        perror("realpath");
        exit(EXIT_FAILURE);
    }
    
    printf("LawakFS++ starting...\n");
    printf("Source directory: %s\n", source_dir);
    printf("Mount point: %s\n", argv[2]);
    
    load_config();
    
    argv[1] = argv[2];
    argc--;
    argv++;
    
    int ret = fuse_main(argc, argv, &lawakfs_oper, NULL);
    
    // Cleanup
    cleanup_config();
    
    return ret;
}
