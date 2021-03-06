//586 lines (507 sloc)  13.2 KB
#include "types.h"
#include "util.c"

void init();
INODE *mount_root(char *p);
void ls(char *p, PROC *parent);
//unsigned long search(INODE *inodePtr, char *name);
void printdir(INODE *inodePtr);
void cd(char *pathname);
int mymkdir(MINODE *pip, char *name);
//MINODE *iget(int dev, unsigned long ino);
//MINODE *iput(MINODE *mip);
unsigned long ialloc(int dev);
unsigned long balloc(int dev);
int findCmd(char *command);

PROC *p0;
PROC *p1;
MINODE minode[100];
MINODE *root = 0;
int fd;
char buff[BLOCK_SIZE];
char path[64];
SUPER *sb;
GD    *gp;
INODE *in;


int main(int argc, char *argv[]) { /*{{{*/
    char line[128], command[64], path[64];
    int cmd;

    init();

    memcpy(&root->INODE,mount_root(argv[1]),sizeof(INODE));

    while(1){
        printf("input command: [ls|cd|mkdir|pwd|quit] ");
        fgets(line, 128, stdin);
        line[strlen(line)-1] = 0;
        if(line[0]==0)
            continue;
        path[0]=0;
        sscanf(line, "%s %s", command, path);
        cmd = findCmd(command);
        switch(cmd){
            case 0: //mkdir
                mymkdir(p0->cwd,path);
                break;
            case 1: //cd
                if(strlen(path)==0){
                    printf("must input cd path\n");
                    break;
                }
                cd(path);
                break;
            case 2: //pwd
                break;
            case 3: //ls
                ls(path,p0);
                break;
            case 99: //quit
                exit(0);
        }
    }


    return 0;
} /* end main */ /*}}}*/

void init() { /*{{{*/
    int i;
    p0 = malloc(sizeof(PROC));
    p0->cwd = malloc(sizeof(MINODE));
    //p0->cwd->INODE = malloc(sizeof(INODE));
    p1 = malloc(sizeof(PROC));
    p1->cwd = malloc(sizeof(MINODE));
    //p0->cwd->INODE = malloc(sizeof(INODE));
    root = malloc(sizeof(MINODE));
    p0->uid=0;
    //p0->cwd=0;
    p1->uid=1;
    //p0->cwd=0;
    sb = malloc(sizeof(SUPER));
    gp = malloc(sizeof(GD));
    in = malloc(sizeof(INODE));


    for (i=0; i<100; i++) {
        minode[i].refCount=0;
    }

    return;
} /*}}}*/

INODE *mount_root(char *path) { /*{{{*/
    fd = open(path,O_RDWR);
    lseek(fd, BLOCK_SIZE*SUPERBLOCK, SEEK_SET);
    read(fd, sb, BLOCK_SIZE);
    lseek(fd, BLOCK_SIZE*GDBLOCK, SEEK_SET);
    read(fd, gp, sizeof(GD));
    lseek(fd, BLOCK_SIZE*gp->bg_inode_table+sizeof(INODE), SEEK_SET);
    read(fd, in, sizeof(INODE));
    memcpy(&p0->cwd->INODE, in, sizeof(INODE));
    p0->cwd->ino=2;
    p0->cwd->dev=fd;
    memcpy(&p1->cwd->INODE, in, sizeof(INODE));
    p1->cwd->ino=2;
    p1->cwd->dev=fd;

    return in;
} /*}}}*/

void ls(char *pathname, PROC *parent) { /*{{{*/
    INODE *cwd = calloc(sizeof(INODE), 1);
    char path[128];
    strncpy(path, pathname, 128);
    int inodeIndex, seek;

    if(pathname[0] == '/') {
        strncpy(path, path+1, 127);
        char *token = strtok(path, "/");
        memcpy(cwd, &root->INODE, sizeof(INODE));

        while(token !=NULL) {
            inodeIndex = search(cwd, token);
            seek = ((inodeIndex-1)/8 + gp->bg_inode_table)*1024 +
                   (inodeIndex-1)%8 * 128;

            lseek(fd, seek,SEEK_SET);

            read(fd, cwd, sizeof(INODE));
            token = strtok(NULL, "/");

        }
        printdir(cwd);
        return;

    } else if(pathname[0] <= 0) {
        printdir(&parent->cwd->INODE);
        return;

    } else {


        char *token = strtok(path, "/");
        memcpy(cwd, &parent->cwd->INODE, sizeof(INODE));

        while(token !=NULL) {
            inodeIndex = search(cwd, token);
            seek = ((inodeIndex-1)/8 + gp->bg_inode_table)*1024 +
                   (inodeIndex-1)%8 * 128;
            lseek(fd, seek, SEEK_SET);
            read(fd, cwd, sizeof(INODE));
            token = strtok(NULL, "/");
        }
    }
} /*}}}*/

/*
unsigned long search(INODE *inodePtr, char *name) {
    DIR *dp = (DIR *) buff;
    int i,j;
    char *cp;
    char tmp[256];
    for ( i =0; i <12; i ++) {
        cp = buff;
        dp = (DIR *) buff;
        lseek(fd, inodePtr->i_block[i]*1024, SEEK_SET);
        read(fd, buff, 1024);
        while(cp < buff + 1024) {
            for ( j = 0; j < dp->name_len; j++ ) {
                tmp[j]=(char)dp->name[j];
            }
            tmp[j] = 0;
            if( strcmp(name, tmp) == 0 ) {
                return dp->inode;
            } else {
                cp += dp->rec_len;
                dp = (DIR *)cp;
            }
        }
    }
    return 0;
} */


void printdir(INODE *inodePtr) { /*{{{*/
    int data_block = inodePtr->i_block[0];
    DIR *dp;
    lseek(fd, BLOCK_SIZE*data_block, SEEK_SET);
    read(fd, buff, BLOCK_SIZE);
    dp = (DIR *)buff;
    char *cp = buff;

    while(cp < buff + 1024) {
        char name[dp->name_len];
        memcpy(name, dp->name, dp->name_len+1);
        name[dp->name_len]='\0';

        printf("%d\t%d\t%d\t%s\n", dp->inode, dp->rec_len, dp->name_len, name);
        cp += dp->rec_len;
        dp = (DIR *)cp;
    }
    return;
} /*}}}*/

void cd(char *pathname) { /*{{{*/
    char path[128];
    strncpy(path,pathname,128);
    int seek;
    if( strcmp(path,"/")==0|| strcmp(path,"")==0) {
//        printf("cd to root\n");
        memcpy(&p0->cwd->INODE, &root->INODE, sizeof(INODE));
        return;
    } else {
        if( path[0]== '/') {
            memcpy(&p0->cwd->INODE, &root->INODE, sizeof(INODE));
            strncpy(path, path+1, 127);
        }
        char *token = strtok(path,"/");
        // printf("%s",token);
        int inodeIndex;

        INODE *cwd = calloc(sizeof(INODE), 1);
        memcpy(cwd, &p0->cwd->INODE, sizeof(INODE));
        while(token != NULL) {
            inodeIndex = search(cwd, token);
            seek = ((inodeIndex-1)/8 + gp->bg_inode_table)*1024 +
                   (inodeIndex-1)%8 * 128;
            lseek(fd, seek, SEEK_SET);

//            lseek(fd, (BLOCK_SIZE * gp->bg_inode_table + (128 *
//                       inodeIndex)), SEEK_SET);
            read(fd, cwd, sizeof(INODE));
            token = strtok(NULL, "/");
        }
        memcpy(&p0->cwd->INODE, cwd, sizeof(INODE));

        return;
    }
} /*}}}*/

/*
MINODE *iget(int dev, unsigned long ino) {
    int seek = ((ino-1)/8 + gp->bg_inode_table)*1024 +
               (ino-1)%8 * 128;
    lseek(fd, seek,SEEK_SET);

    INODE iptr;

    int i;
    read(dev, &iptr, sizeof(INODE));

    for(i=0; i<100; i++) {
        if(minode[i].INODE.i_block[0] == iptr.i_block[0]) {
            minode[i].refCount++;
            return &minode[i];
        }
    }
    for(i=0; i<100; i++) {
        if(minode[i].INODE.i_size==0) {
            break;
        }
    }
    memcpy(&minode[i].INODE, &iptr, sizeof(INODE));
    minode[i].dev = dev;
    minode[i].ino = ino;
    return &minode[i];
} 

MINODE *iput(MINODE *mip) { 
    mip->refCount--;
    if(mip->refCount>0) {
        return mip;
    }
    if(mip->dirty==0) {
        return mip;
    }
    if(mip->dirty>0) {
        mip->dirty=0;
        int seek = ((mip->ino-1)/8 + gp->bg_inode_table)*1024 +
                   (mip->ino-1)%8 * 128;
        lseek(fd, seek,SEEK_SET);
        write(mip->dev, &mip->INODE, sizeof(INODE));
        return mip;
    }
    return (MINODE *)-1;
} 
*/

unsigned long ialloc(int dev) { /*{{{*/
    int i;
    char data_buff[1024];
    lseek(dev, IBITMAP * BLOCK_SIZE, SEEK_SET);
    read(dev, data_buff, 1024);
    for (i = 1; i<1024*8; i++) {
        if (tstbit(data_buff,i-1) == 0) {
            setbit(data_buff,i-1);
            lseek(dev, IBITMAP * BLOCK_SIZE, SEEK_SET);
            write(dev, data_buff, 1024);
            return i;
        }


    }
    return -1;
} /*}}}*/

unsigned long balloc(int dev) { /*{{{*/
    int i;
    char data_buff[1024];
    lseek(dev, BBITMAP * BLOCK_SIZE, SEEK_SET);
    read(dev, data_buff, 1024);
    for (i = 1; i<1024*8; i++) {
        if (tstbit(data_buff,i) == 0) {
            setbit(data_buff,i);
            lseek(dev, BBITMAP * BLOCK_SIZE, SEEK_SET);
            write(dev, data_buff, 1024);
            return i;
        }

    }
    return -1;
} /*}}}*/

int mymkdir(MINODE *pip, char *name) { /*{{{*/
    MINODE *mip;
    char *cp;
    memset(buff, 0, 1024);
    int dev = fd;
    int i;
    unsigned long inumber = ialloc(dev);
    unsigned long bnumber = balloc(dev);
    mip = iget(dev, inumber);
    mip->INODE.i_block[0] = bnumber;

    for(i=1; i<16; i++) {
        mip->INODE.i_block[i]=0;
    }

    mip->dirty=1;
    mip->ino = inumber;
    mip->INODE.i_mode=0x41ED;
    mip->INODE.i_uid=p0->uid;
    mip->INODE.i_gid=p0->gid;
    mip->INODE.i_size=1024;
    mip->INODE.i_links_count=2;
    mip->INODE.i_atime=mip->INODE.i_ctime=mip->INODE.i_mtime = time(0L);
    mip->INODE.i_blocks = 2;
    iput(mip);
    memset(buff, 0, 1024);
    dp = (DIR *)buff;

    dp->inode = inumber;		/* Inode number */
    strncpy(dp->name, ".", 1);    /* File name */
    dp->name_len = 1;		/* Name length */
    dp->rec_len = 12;		/* Directory entry length */

    cp = buff;
    cp += dp->rec_len;            /* advance by rec_len */
    dp = (DIR *)cp;


    dp->inode = pip->ino;      /* Inode number of parent DIR */
    dp->name_len = 2;		/* Name length */
    strncpy(dp->name, "..", 2);   /* File name */
    dp->rec_len = BLOCK_SIZE - 12;/* last DIR entry length to end of block */

    lseek(fd,BLOCK_SIZE*(bnumber),SEEK_SET);
    write(fd,buff,BLOCK_SIZE);
    lseek(fd, pip->INODE.i_block[0]*BLOCK_SIZE,SEEK_SET);
    read(fd, buff, 1024);

    cp = buff;
    dp = (DIR *) buff;

    while(cp+dp->rec_len < buff + 1024) {

        cp += dp->rec_len;
        dp = (DIR *)cp;

    }
    int need_length = 4*((8+dp->name_len+3)/4);

    int tmp = dp->rec_len;
    dp->rec_len = need_length;
    tmp = tmp - dp->rec_len;
    cp += dp->rec_len;
    dp = (DIR *)cp;
    dp->rec_len = tmp;
    dp->name_len = strlen(name);
    dp->inode = mip->ino;

    strcpy(dp->name, name);
//	for(i = 0;i<dp->name_len;i++)
//		dp->name[i]=name[i];
    lseek(fd, pip->INODE.i_block[0]*BLOCK_SIZE,SEEK_SET);
    write(fd,buff,BLOCK_SIZE);
    pip->dirty =1;
    pip->refCount++;

    return 0;

} /*}}}*/


void rpwd(MINODE *wd){
    char my_name[256];
    unsigned long my_ino, parent_ino;

    //If cwd is root, return root
    if (wd == root){
        return;
    }

    //from wd->INODE.i_block[0], get my_ino and parent_ino
    getino(wd, &my_ino, &parent_ino);
    wd = iget(wd->dev, parent_ino);
    rpwd(wd);
    findmyname(wd, my_ino, my_name);
    printf("%s", my_name);

}

int pwd(){
    if (p0->cwd == root){
        printf("/");
    }
    else {
        rpwd(p0->cwd);
    }

    printf("\n");

    return 0;
}

int findCmd(char *cname){ /*{{{*/
    if(strcmp(cname, "mkdir")==0){
        return 0;
    }
    if(strcmp(cname, "cd")==0){
        return 1;
    }
    if(strcmp(cname, "pwd")==0){
        return 2;
    }
    if(strcmp(cname, "ls")==0){
        return 3;
    }
    if(strcmp(cname, "quit")==0){
        return 99;
    }
    return -1;
} 


