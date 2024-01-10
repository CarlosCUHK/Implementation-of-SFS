#include "call.h"
#include "string.h"
#define min(a, b) ((a) < (b) ? (a) : (b))
const char *HD = "HD";

int get_data(int block, char* buf, int offset, int count, int fd){
    int blk_offset = block * BLK_SIZE + D_OFFSET + offset;
    if (lseek(fd, blk_offset, SEEK_SET) == -1){
        return -1;
    }
    int read_bytes = read(fd, buf, count);
    return read_bytes;
}

void get_dir_content(DIR_NODE* dir, inode* ip, int fd){
    int blk_num = ip->direct_blk[0];
    if (lseek(fd, blk_num*BLK_SIZE+D_OFFSET, SEEK_SET) == -1)
        return;
    int *address;
    int read_bytes = 0;
    read_bytes = read(fd, dir, min(BLK_SIZE, ip->f_size-read_bytes));

    if (ip->blk_number > 1)
    {
        blk_num = ip->direct_blk[1];
        lseek(fd, blk_num*BLK_SIZE+D_OFFSET, SEEK_SET);
        read_bytes += read(fd, dir+read_bytes, min(BLK_SIZE, ip->f_size-read_bytes));
    }
    if (ip->blk_number > 2){
        for (int i = 0; i < ip->blk_number-2; i++)
        {   
            blk_num = ip->indirect_blk;
            lseek(fd, blk_num*BLK_SIZE+D_OFFSET+i*4, SEEK_SET);
            read(fd, address, sizeof(int));
            lseek(fd, (*address)*BLK_SIZE+D_OFFSET, SEEK_SET);
            read_bytes += read(fd, dir+read_bytes, min(BLK_SIZE, ip->f_size-read_bytes));
        }
    }
}

int compare_name(DIR_NODE* dir, char* name, inode* ip){
    for (int i = 0; i < ip->f_size/sizeof(DIR_NODE); i++)
    {
        if (strcmp(dir[i].f_name, name) == 0){
            return dir[i].i_number;
        }
    }
    return -1;
}

int search_for_inode(inode* ip, char* child_name, int fd){
    int inode_number;
    if (ip->f_type != DIR){
        inode_number = -1;
        return inode_number;
    }
    DIR_NODE* dir = (DIR_NODE*)malloc(ip->f_size);
    get_dir_content(dir, ip, fd);
    inode_number = compare_name(dir, child_name, ip);
    free(dir);
    return inode_number;
}


inode* get_inode(int i_number, int fd){
	inode* ip = malloc(sizeof(inode));
	int currpos=lseek(fd, I_OFFSET + i_number * sizeof(inode), SEEK_SET);
	if(currpos<0){
		printf("Error: lseek()\n");
		return NULL;
	}
	//read inode from disk
	int ret = read(fd, ip, sizeof(inode));
	if(ret != sizeof (inode) ){
		printf("Error: read()\n");
		return NULL;
	}
	return ip;
}

int open_t(char *pathname)
{   
    int fd = open (HD, O_RDWR);
	if(fd<0){       
		printf("Error: open()\n");
		return -1;
	}
	int inode_number;
	// write your code here.
    inode_number = 0;
    char tmp_name[MAX_COMMAND_LENGTH];
    strcpy(tmp_name, pathname);
    char* piece[MAX_COMMAND_LENGTH];
    piece[1] = strtok(tmp_name, "/");
    int level = 1;
    while(piece[level] != NULL){
        level++;
        piece[level] = strtok(NULL,"/");
    }
    level--;
    inode* ip;
    if (level == 0){
        return 0;
    }
    int count = 0;
    while(level > 0){
        level--;
        count++;
        ip = get_inode(inode_number, fd);
        inode_number = search_for_inode(ip, piece[count], fd);
        if(inode_number == -1)
            return inode_number;
    }
    close(fd);
	return inode_number;
}


int read_t(int i_number, int offset, void *buf, int count)
{
	int read_bytes;
	// write your code here.
    int file_size;
    int fd = open (HD, O_RDWR);
	if(fd<0){       
		printf("Error: open()\n");
		return -1;
	}
    inode* ip;
    ip = get_inode(i_number, fd);
    int a = offset/BLK_SIZE;
    int b = offset%BLK_SIZE;
    int a_end = (offset+count-1)/BLK_SIZE;
    int b_end = (offset+count-1)%BLK_SIZE;
    int end_of_file = ip->blk_number-1;
    int blk_num;
    int* address;
    if (a_end > end_of_file){
        a_end = min(end_of_file, a_end);
        b_end = BLK_SIZE-1;
    }
    if (a > a_end){
        return 0;
    }
    read_bytes = 0;
    file_size = ip->f_size;
    for (int i = a; i < a_end+1; i++){
        if (i < 2){
            if (i == a){
                read_bytes = get_data(ip->direct_blk[i], buf, b, (min(BLK_SIZE-b, (a_end-i)*BLK_SIZE+b_end-b)),fd);
            }
            else{
                read_bytes += get_data(ip->direct_blk[i], buf+read_bytes, 0, (min(BLK_SIZE, (a_end-i)*BLK_SIZE+b_end)),fd);
            }
        }
        else{
            blk_num = ip->indirect_blk;
            lseek(fd, blk_num*BLK_SIZE+D_OFFSET+(i-2)*4, SEEK_SET);
            read(fd, address, sizeof(int));
            if (i == a){
                read_bytes = get_data((*address), buf, b, (min(BLK_SIZE-b, (a_end-i)*BLK_SIZE+b_end-b)),fd);
            }
            else {
                read_bytes += get_data((*address), buf+read_bytes, 0, (min(BLK_SIZE, (a_end-i)*BLK_SIZE+b_end))),fd);
            }
        }
    }
	return read_bytes; 
}

