#include "shell.h"
#include "kernel.h"
#include "std_lib.h"
#include "filesystem.h"
#include <stddef.h> 

void shell() {
  char buf[64];
  char cmd[64];
  char arg[2][64];

  byte cwd = FS_NODE_P_ROOT;

  while (true) {
    printString("MengOS:");
    printCWD(cwd);
    printString("$ ");
    readString(buf);
    parseCommand(buf, cmd, arg);

    if (strcmp(cmd, "cd")) cd(&cwd, arg[0]);
    else if (strcmp(cmd, "ls")) ls(cwd, arg[0]);
    else if (strcmp(cmd, "mv")) mv(cwd, arg[0], arg[1]);
    else if (strcmp(cmd, "cp")) cp(cwd, arg[0], arg[1]);
    else if (strcmp(cmd, "cat")) cat(cwd, arg[0]);
    else if (strcmp(cmd, "mkdir")) mkdir(cwd, arg[0]);
    else if (strcmp(cmd, "clear")) clearScreen();
    else printString("Invalid command\n");
  }
}

// Task 4 - printCWD
void printCWD(byte cwd) {
  struct node_fs node_fs_buf;
  char* path_parts[8];
  int depth = 0;
  int i;

  if (cwd == FS_NODE_P_ROOT) {
    printString("/");
    return;
  }

  readSector(&node_fs_buf, FS_NODE_SECTOR_NUMBER);
  readSector(((byte*)&node_fs_buf) + SECTOR_SIZE, FS_NODE_SECTOR_NUMBER + 1);

  while (cwd != FS_NODE_P_ROOT && depth < 8) {
    path_parts[depth] = node_fs_buf.nodes[cwd].node_name;
    cwd = node_fs_buf.nodes[cwd].parent_index;
    depth++;
  }

  for (i = depth - 1; i >= 0; i--) {
    printString("/");
    printString(path_parts[i]);
  }
}

// Task 5 - parseCommand
void parseCommand(char* buf, char* cmd, char arg[2][64]) {
  int i = 0, j = 0, k = 0;
  bool reading_arg = false;
  clear((byte*)cmd, 64);
  clear((byte*)arg[0], 64);
  clear((byte*)arg[1], 64);

  while (buf[i] != '\0') {
    if (buf[i] == ' ') {
      if (!reading_arg) {
        cmd[j] = '\0';
        reading_arg = true;
        k = 0;
      } else {
        arg[0][k] = '\0';
        k = 0;
        break;
      }
      i++;
      continue;
    }
    if (!reading_arg) {
      cmd[j++] = buf[i++];
    } else {
      arg[0][k++] = buf[i++];
    }
  }

  if (reading_arg && buf[i] != '\0') {
    while (buf[i] == ' ') i++;
    k = 0;
    while (buf[i] != '\0' && buf[i] != ' ') {
      arg[1][k++] = buf[i++];
    }
    arg[1][k] = '\0';
  }
}

// Task 6 - cd
void cd(byte* cwd, char* dirname) {
  struct node_fs node_fs_buf;
  int i;

  // cd tanpa argumen
  if (strlen(dirname) == 0) {
    printString("cd: argumen tidak boleh kosong\n");
    return;
  }

  // cd / -> kembali ke root
  if (strcmp(dirname, "/")) {
    *cwd = FS_NODE_P_ROOT;
    return;
  }

  // cd .. -> naik ke parent
  if (strcmp(dirname, "..")) {
    if (*cwd == FS_NODE_P_ROOT) return; // Sudah di root
    readSector(&node_fs_buf, FS_NODE_SECTOR_NUMBER);
    readSector(((byte*)&node_fs_buf) + SECTOR_SIZE, FS_NODE_SECTOR_NUMBER + 1);
    *cwd = node_fs_buf.nodes[*cwd].parent_index;
    return;
  }

  // cd <dirname> -> pindah ke subdirektori
  readSector(&node_fs_buf, FS_NODE_SECTOR_NUMBER);
  readSector(((byte*)&node_fs_buf) + SECTOR_SIZE, FS_NODE_SECTOR_NUMBER + 1);

  for (i = 0; i < FS_MAX_NODE; i++) {
    if (
      node_fs_buf.nodes[i].parent_index == *cwd &&
      strcmp(node_fs_buf.nodes[i].node_name, dirname) &&
      node_fs_buf.nodes[i].data_index == FS_NODE_D_DIR
    ) {
      *cwd = i;
      return;
    }
  }

  printString("Folder tidak ditemukan\n");
}


// Task 7 - ls
void ls(byte cwd, char* dirname) {
  struct node_fs node_fs_buf;
  int i;
  byte target_idx = cwd;

  readSector(&node_fs_buf, FS_NODE_SECTOR_NUMBER);
  readSector(((byte*)&node_fs_buf) + SECTOR_SIZE, FS_NODE_SECTOR_NUMBER + 1);

  // handle ls dirname
  if (strlen(dirname) > 0 && !strcmp(dirname, ".")) {
    target_idx = cwd;
  } else if (strlen(dirname) > 0) {
    for (i = 0; i < FS_MAX_NODE; i++) {
      if (node_fs_buf.nodes[i].parent_index == cwd &&
          strcmp(node_fs_buf.nodes[i].node_name, dirname) &&
          node_fs_buf.nodes[i].data_index == FS_NODE_D_DIR) {
        target_idx = i;
        break;
      }
    }
    if (i == FS_MAX_NODE) {
      printString("Direktori tidak ditemukan\n");
      return;
    }
  }

  // Tampilkan isi direktori
  for (i = 0; i < FS_MAX_NODE; i++) {
    if (node_fs_buf.nodes[i].parent_index == target_idx &&
        node_fs_buf.nodes[i].node_name[0] != '\0') {
      printString(node_fs_buf.nodes[i].node_name);
      printString("\n");
    }
  }
}


// Task 8 - mv (rename saja)
void mv(byte cwd, char* src, char* dst) {
  struct node_fs node_fs_buf;
  int i;
  readSector(&node_fs_buf, FS_NODE_SECTOR_NUMBER);
  readSector(((byte*)&node_fs_buf) + SECTOR_SIZE, FS_NODE_SECTOR_NUMBER + 1);

  for (i = 0; i < FS_MAX_NODE; i++) {
    if (node_fs_buf.nodes[i].parent_index == cwd &&
        strcmp(node_fs_buf.nodes[i].node_name, src)) {
      strcpy(node_fs_buf.nodes[i].node_name, dst);
      writeSector(&node_fs_buf, FS_NODE_SECTOR_NUMBER);
      writeSector(((byte*)&node_fs_buf) + SECTOR_SIZE, FS_NODE_SECTOR_NUMBER + 1);
      return;
    }
  }

  printString("File/folder tidak ditemukan\n");
}

// Task 9 - cp
void cp(byte cwd, char* src, char* dst) {
  struct file_metadata metadata;
  enum fs_return status;

  metadata.parent_index = cwd;
  strcpy(metadata.node_name, src);
  fsRead(&metadata, &status);

  if (status == FS_SUCCESS || status == FS_R_SUCCESS) {
    struct file_metadata new_file;
    new_file.parent_index = cwd;
    new_file.filesize = metadata.filesize;
    strcpy(new_file.node_name, dst);
    memcpy(new_file.buffer, metadata.buffer, metadata.filesize);
    fsWrite(&new_file, &status);

    if (status != FS_SUCCESS && status != FS_W_SUCCESS)
      printString("Gagal menyalin file\n");
  } else {
    printString("File tidak ditemukan\n");
  }
}

// Task 10 - cat
void cat(byte cwd, char* filename) {
  struct file_metadata metadata;
  enum fs_return status;

  metadata.parent_index = cwd;
  strcpy(metadata.node_name, filename);
  fsRead(&metadata, &status);

  if (status == FS_SUCCESS || status == FS_R_SUCCESS) {
    int i;
    char c[2]; // Pindahkan inisialisasi ke luar loop
    for (i = 0; i < metadata.filesize; i++) {
      c[0] = metadata.buffer[i];
      c[1] = '\0';
      printString(c);
    }
    printString("\n");
  } else {
    printString("File tidak ditemukan atau bukan file biasa\n");
  }
}


// Task 11 - mkdir
void mkdir(byte cwd, char* dirname) {
  struct file_metadata metadata;
  enum fs_return status;

  metadata.parent_index = cwd;
  metadata.filesize = 0;
  strcpy(metadata.node_name, dirname);
  clear(metadata.buffer, FS_MAX_SECTOR * SECTOR_SIZE);
  fsWrite(&metadata, &status);

  if (status != FS_SUCCESS && status != FS_W_SUCCESS)
    printString("Gagal membuat folder\n");
}
