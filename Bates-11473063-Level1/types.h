#ifndef MOUNT_ROUTE_C_TYPES_H
#define MOUNT_ROUTE_C_TYPES_H
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ext2fs/ext2_fs.h>
#include <libgen.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

// define shorter TYPES, save typing efforts
typedef struct ext2_group_desc  GD;
typedef struct ext2_super_block SUPER;
typedef struct ext2_inode       INODE;
typedef struct ext2_dir_entry_2 DIR;    // need this for new version of e2fs

GD    *gp;
SUPER *sp;
INODE *ip;
DIR   *dp;

#define BLOCK_SIZE        1024
#define BITS_PER_BLOCK    (8*BLOCK_SIZE)
#define INODES_PER_BLOCK  (BLOCK_SIZE/sizeof(INODE))

// Block number of EXT2 FS on FD
#define SUPERBLOCK        1
#define GDBLOCK           2
#define BBITMAP           3
#define IBITMAP           4
#define INODEBLOCK        5
#define ROOT_INODE        2

// Default dir and regulsr file modes
#define DIR_MODE          0040777
#define FILE_MODE         0100644
#define SUPER_MAGIC       0xEF53
#define SUPER_USER        0

// Proc status
#define FREE              0
#define BUSY              1
#define KILLED            2

// Table sizes
#define NMINODES         100
#define NMOUNT            10
#define NPROC             10
#define NFD               10
#define NOFT             100

// Open File Table
typedef struct Oft {
    int   mode;
    int   refCount;
    struct Minode *inodeptr;
    long  offset;
} OFT;

// PROC structure
typedef struct Proc {
    int   uid;
    int   pid;
    int   gid;
    int   ppid;
    struct Proc *parent;
    int   status;

    struct Minode *cwd;
    OFT   *fd[NFD];
} PROC;

// In-memory inodes structure
typedef struct Minode {
    INODE    INODE;               // disk inode
    ushort   dev;
    unsigned long ino;
    ushort   refCount;
    ushort   dirty;
    ushort   mounted;
    struct Mount *mountptr;
} MINODE;

// Mount Table structure
typedef struct Mount {
    int    ninodes;
    int    nblocks;
    int    dev, busy;
    struct Minode *mounted_inode;
    char   name[256];
    char   mount_name[64];
} MOUNT;

int tstbit(char *buf, int BIT) {
    int i, j;
    i = BIT / 8;
    j = BIT % 8;
    return buf[i] & (1 << j);
}

int setbit(char *buf, int BIT) {
    int i, j;
    i = BIT / 8;
    j = BIT % 8;
    buf[i] |= (1 << j);
    return 1;
}

int clearbit(char *buf, int BIT) {
    int i, j;
    i = BIT / 8;
    j = BIT % 8;
    buf[i] &= ~(1 << j);
    return 1;
}
#endif //MOUNT_ROUTE_C_TYPES_H
