#ifndef _SUPER_BLOCK_H_
#define _SUPER_BLOCK_H_

#define S_OFFSET  1024
#define I_OFFSET   2048
#define D_OFFSET    10485760
#define MAX_INODE      100
#define MAX_D_BLOCK   256000
#define BLK_SIZE   4096

#define MAX_COMMAND_LENGTH 50
#define MAX_FILE_SIZE BLK_SIZE*1024

typedef struct _super_block_
{
        int i_offset;
        int d_offset;
        int max_inode;
        int max_d_block;
        int next_available_inode;
        int next_available_d_block;
        int blk_size;
}superblock;

superblock* read_sb(int fd);
void print_sb_info(superblock* sb);
void print_sb_region(int fd);
#endif
