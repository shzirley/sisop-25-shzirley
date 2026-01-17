#include "kernel.h"
#include "std_lib.h"
#include "filesystem.h"

void fsInit() {
  struct map_fs map_fs_buf;
  int i = 0;

  readSector(&map_fs_buf, FS_MAP_SECTOR_NUMBER);
  for (i = 0; i < 16; i++) map_fs_buf.is_used[i] = true;
  for (i = 256; i < 512; i++) map_fs_buf.is_used[i] = true;
  writeSector(&map_fs_buf, FS_MAP_SECTOR_NUMBER);
}

// TODO: 2. Implement fsRead function
/*void fsWrite(struct file_metadata* metadata, enum fs_return* status) {
  struct map_fs map_fs_buf;
  struct node_fs node_fs_buf;
  struct data_fs data_fs_buf;

  int i, j;
  int node_empty_idx = -1;
  int data_empty_idx = -1;
  int sector_count_needed = (metadata->filesize + SECTOR_SIZE - 1) / SECTOR_SIZE;
  int found = 0, empty_sector_count = 0;

  // Baca semua sektor yang diperlukan
  readSector(&map_fs_buf, FS_MAP_SECTOR_NUMBER);
  readSector(&(node_fs_buf.nodes[0]), FS_NODE_SECTOR_NUMBER);
  readSector(&(node_fs_buf.nodes[32]), FS_NODE_SECTOR_NUMBER);
  readSector(&data_fs_buf, FS_DATA_SECTOR_NUMBER);

  // 1. Cek apakah file dengan nama sama dan parent index sudah ada
  for (i = 0; i < FS_MAX_NODE; i++) {
    if (
      node_fs_buf.nodes[i].parent_index == metadata->parent_index &&
      strncmp(node_fs_buf.nodes[i].node_name, metadata->node_name, MAX_FILENAME) == 0
    ) {
      *status = FS_W_NODE_ALREADY_EXISTS;
      return;
    }
  }

  // 2. Cari node kosong
  for (i = 0; i < FS_MAX_NODE; i++) {
    if (node_fs_buf.nodes[i].node_name[0] == '\0') {
      node_empty_idx = i;
      break;
    }
  }
  if (node_empty_idx == -1) {
    *status = FS_W_NO_FREE_NODE;
    return;
  }

  // 3. Cari data kosong
  for (i = 0; i < FS_MAX_DATA; i++) {
    if (data_fs_buf.datas[i].sectors[0] == 0x00) {
      data_empty_idx = i;
      break;
    }
  }
  if (data_empty_idx == -1) {
    *status = FS_W_NO_FREE_DATA;
    return;
  }

  // 4. Hitung sektor kosong di map
  for (i = 0; i < SECTOR_SIZE; i++) {
    if (!map_fs_buf.is_used[i]) empty_sector_count++;
  }
  if (empty_sector_count < sector_count_needed) {
    *status = FS_W_NOT_ENOUGH_SPACE;
    return;
  }

  // 5. Simpan informasi file ke node
  struct node_item* new_node = &node_fs_buf.nodes[node_empty_idx];
  new_node->parent_index = metadata->parent_index;
  new_node->data_index = data_empty_idx;
  memcpy(new_node->node_name, metadata->node_name, MAX_FILENAME);

  // 6. Tulis data ke sektor-sektor kosong
  for (i = 0, j = 0; j < sector_count_needed && i < SECTOR_SIZE; i++) {
    if (!map_fs_buf.is_used[i]) {
      map_fs_buf.is_used[i] = true;
      data_fs_buf.datas[data_empty_idx].sectors[j] = i;
      writeSector(metadata->buffer + j * SECTOR_SIZE, i);
      j++;
    }
  }

  // 7. Tulis balik hasil modifikasi ke disk
  writeSector(&map_fs_buf, FS_MAP_SECTOR_NUMBER);
  writeSector(&(node_fs_buf.nodes[0]), FS_NODE_SECTOR_NUMBER);
  writeSector(&(node_fs_buf.nodes[32]), FS_NODE_SECTOR_NUMBER);
  writeSector(&data_fs_buf, FS_DATA_SECTOR_NUMBER);

  *status = FS_SUCCESS;
} */


void fsRead(struct file_metadata* metadata, enum fs_return* status) {
  struct node_fs node_fs_buf;
  struct data_fs data_fs_buf;
  int i, j;
  int node_idx = -1;
  int data_idx;

  readSector(&node_fs_buf, FS_NODE_SECTOR_NUMBER);
  readSector(((byte*)(&node_fs_buf)) + SECTOR_SIZE, FS_NODE_SECTOR_NUMBER + 1);
  readSector(&data_fs_buf, FS_DATA_SECTOR_NUMBER);

  for (i = 0; i < FS_MAX_NODE; i++) {
    struct node_item *node = &node_fs_buf.nodes[i];
    if (node->parent_index == metadata->parent_index &&
        strcmp(node->node_name, metadata->node_name) == 0) {
      node_idx = i;
      break;
    }
  }

  if (node_idx == -1) {
    *status = FS_R_NODE_NOT_FOUND;
    return;
  }

  if (node_fs_buf.nodes[node_idx].data_index == FS_NODE_D_DIR) {
    *status = FS_R_TYPE_IS_DIRECTORY;
    return;
  }

  data_idx = node_fs_buf.nodes[node_idx].data_index;
  metadata->filesize = 0;
  for (j = 0; j < FS_MAX_SECTOR; j++) {
    int sector_number = data_fs_buf.datas[data_idx].sectors[j];
    if (sector_number == 0) break;

    readSector(metadata->buffer + (j * SECTOR_SIZE), sector_number);
    metadata->filesize += SECTOR_SIZE;
  }

  *status = FS_R_SUCCESS;
}


// TODO: 3. Implement fsWrite function
void fsWrite(struct file_metadata* metadata, enum fs_return* status) {
  struct map_fs map_fs_buf;
  struct node_fs node_fs_buf;
  struct data_fs data_fs_buf;

  int i, j;
  int node_empty_idx = -1;
  int data_empty_idx = -1;
  int empty_sector_count = 0;
  int sector_needed = (metadata->filesize + SECTOR_SIZE - 1) / SECTOR_SIZE;

  // 1. Load data dari disk
  readSector(&map_fs_buf, FS_MAP_SECTOR_NUMBER);
  readSector(&(node_fs_buf.nodes[0]), FS_NODE_SECTOR_NUMBER);
  readSector(((byte*)&node_fs_buf) + SECTOR_SIZE, FS_NODE_SECTOR_NUMBER + 1);
  readSector(&data_fs_buf, FS_DATA_SECTOR_NUMBER);

  // 2. Cek apakah file sudah ada
  for (i = 0; i < FS_MAX_NODE; i++) {
    if (
      node_fs_buf.nodes[i].parent_index == metadata->parent_index &&
      strncmp(node_fs_buf.nodes[i].node_name, metadata->node_name, MAX_FILENAME) == 0
    ) {
      *status = FS_W_NODE_ALREADY_EXISTS;
      return;
    }
  }

  // 3. Cari node kosong
  for (i = 0; i < FS_MAX_NODE; i++) {
    if (node_fs_buf.nodes[i].node_name[0] == '\0') {
      node_empty_idx = i;
      break;
    }
  }
  if (node_empty_idx == -1) {
    *status = FS_W_NO_FREE_NODE;
    return;
  }

  // 4. Jika direktori, tulis langsung
  if (metadata->filesize == 0) {
    node_fs_buf.nodes[node_empty_idx].parent_index = metadata->parent_index;
    node_fs_buf.nodes[node_empty_idx].data_index = FS_NODE_D_DIR;
    memcpy(node_fs_buf.nodes[node_empty_idx].node_name, metadata->node_name, MAX_FILENAME);

    writeSector(&(node_fs_buf.nodes[0]), FS_NODE_SECTOR_NUMBER);
    writeSector(((byte*)&node_fs_buf) + SECTOR_SIZE, FS_NODE_SECTOR_NUMBER + 1);

    *status = FS_W_SUCCESS;
    return;
  }

  // 5. Cari data kosong
  for (i = 0; i < FS_MAX_DATA; i++) {
    if (data_fs_buf.datas[i].sectors[0] == 0x00) {
      data_empty_idx = i;
      break;
    }
  }
  if (data_empty_idx == -1) {
    *status = FS_W_NO_FREE_DATA;
    return;
  }

  // 6. Cek jumlah sektor kosong
  for (i = 0; i < SECTOR_SIZE; i++) {
    if (!map_fs_buf.is_used[i]) empty_sector_count++;
  }
  if (empty_sector_count < sector_needed) {
    *status = FS_W_NOT_ENOUGH_SPACE;
    return;
  }

  // 7. Simpan informasi node
  node_fs_buf.nodes[node_empty_idx].parent_index = metadata->parent_index;
  node_fs_buf.nodes[node_empty_idx].data_index = data_empty_idx;
  memcpy(node_fs_buf.nodes[node_empty_idx].node_name, metadata->node_name, MAX_FILENAME);

  // 8. Alokasikan sektor dan tulis data
  for (i = 0, j = 0; j < sector_needed && i < SECTOR_SIZE; i++) {
    if (!map_fs_buf.is_used[i]) {
      map_fs_buf.is_used[i] = true;
      data_fs_buf.datas[data_empty_idx].sectors[j] = i;
      writeSector(metadata->buffer + (j * SECTOR_SIZE), i);
      j++;
    }
  }

  // 9. Tulis ulang ke disk
  writeSector(&map_fs_buf, FS_MAP_SECTOR_NUMBER);
  writeSector(&(node_fs_buf.nodes[0]), FS_NODE_SECTOR_NUMBER);
  writeSector(((byte*)&node_fs_buf) + SECTOR_SIZE, FS_NODE_SECTOR_NUMBER + 1);
  writeSector(&data_fs_buf, FS_DATA_SECTOR_NUMBER);

  *status = FS_W_SUCCESS;
}
