#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h> 
#include <netinet/in.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
#include <arpa/inet.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <libgen.h>     // for dirname()/basename()
#include <time.h> 

#define MAX 256
#define BLK 1024

struct sockaddr_in saddr; 
char *serverIP   = "127.0.0.1";
int   serverPORT = 1234;
int   sock;

struct stat mystat, *sp;           // for ls command
char *t1 = "xwrxwrxwr--------";
char *t2 = "-----------------";

int ls_file(char *fname){
    struct stat fstat, *sp;
    int r, i;
    char info[1024]; //each line will be read into here
    char ftime[64];
    char buf[1024];  //for linkname
    ssize_t len;     //for linkname
    sp = &fstat;
    if ((r = lstat(fname, &fstat)) < 0){
        printf("can't stat %s\n", fname);
        exit(1);
    }
    if ((sp->st_mode & 0xF000) == 0x8000)  //if (S_ISREG())
        printf("%c", '-');
    if ((sp->st_mode & 0xF000) == 0x4000)  //if (S_ISDIR())
        printf("%c", 'd');
    if ((sp->st_mode & 0xF000) == 0xA000)  //if (S_ISLNK())
        printf("%c", '1');
    for (i = 8; i >= 0; i--){
        if (sp->st_mode & (1 << i))        //print r|w|x
            printf("%c", t1[i]);
        else
            printf("%c", t2[i]);           //or print -
    }
    printf("%4d ", sp->st_nlink);          //link count
    printf("%4d ", sp->st_gid);            //gid
    printf("%4d ", sp->st_uid);            //uid
    printf("%4d ", sp->st_size);           //file size

    //print time
    strcpy(ftime, ctime(&sp->st_ctime));    //print time in calendar form
    ftime[strlen(ftime) - 1] = 0;          //kill \n at end
    printf("%s  ", ftime);

    //print name
    printf("%s", basename(fname));         //print file basename

    // print -> linkname if symbolic file
    if ((sp->st_mode & 0xF000) == 0xA000){
        char linkname[MAX];

        if((len = readlink(fname, buf, sizeof(buf)-1)) != -1)
            buf[len] = '\0';

        //char linkname[MAX] = buf[len];
        strcpy(linkname, buf);
        printf(" -> %s", linkname);       //print out linked name
    }
    printf("\n");
}

int ls_dir(char *dname){
    //use opendir(), readdir(); then call ls_file(name)
    struct dirent *pDirent;
    DIR *pDir;

    pDir = opendir(dname);
    if (pDir == NULL){
        printf("Cannot open directory '%s'\n", dname);
        return 1;
    }

    while ((pDirent = readdir(pDir)) != NULL){
        ls_file(pDirent->d_name);
    }

    close(pDir);
    return 0;
}

int init()
{
    int n; 

    printf("1. create a socket\n");
    sock = socket(AF_INET, SOCK_STREAM, 0); 
    if (sock < 0) { 
        printf("socket creation failed\n"); 
        exit(0); 
    }
    
    printf("2. fill in server IP=%s, port number=%d\n", serverIP, serverPORT);
    bzero(&saddr, sizeof(saddr)); 
    saddr.sin_family = AF_INET; 
    saddr.sin_addr.s_addr = inet_addr(serverIP); 
    saddr.sin_port = htons(serverPORT); 
  
    printf("3. connect to server\n");
    if (connect(sock, (struct sockaddr *)&saddr, sizeof(saddr)) != 0) { 
        printf("connection with the server failed...\n"); 
        exit(0); 
    }
    printf("4. connected to server OK\n");
}
  
int main(int argc, char *argv[], char *env[]) 
{ 
    int  n;
    char line[MAX], ans[MAX];

    init();
  
    while (1){
      printf("\n");
      printf("****************** menu *********************\n");
      printf("* get  put  ls   cd   pwd  mkdir  rmdir  rm *\n");
      printf("* lcat     lls  lcd  lpwd lmkdir lrmdir lrm *\n");
      printf("*********************************************\n");
      printf("input a command: ");
      fgets(line, MAX, stdin);
      line[strlen(line)-1] = 0;        // kill \n at end
      if (line[0]==0)                  // exit if NULL line
         exit(0);
    

      //Checks for local commands
      char cmd[MAX];   
      char pathname[MAX];
      sscanf(line, "%s %s", &cmd, &pathname);
      if(strcmp(cmd, "lmkdir") == 0){
          int r = mkdir(pathname, 0755);
      }
      else if (strcmp(cmd, "lrmdir") == 0){
          int r = rmdir(pathname);
      }
      else if (strcmp(cmd, "lrm") == 0){
          int r = unlink(pathname);
      }
      else if (strcmp(cmd, "lcd") == 0){
          int r = chdir(pathname);
      }
      else if (strcmp(cmd, "lpwd") == 0){
          char buf[MAX];
          getcwd(buf, MAX);
          printf("%s\n", buf);

      }
      else if (strcmp(cmd, "get") == 0){
          // Sends command to server to open file for read
          n = write(sock, line, BLK);

          bzero(ans, BLK);

      }
      else if(strcmp(cmd, "lls") == 0){
          //system("ls -l");
          struct stat mystat, *sp = &mystat;
          int r;
          char *filename, path[1024], cwd[256];
          filename = "./"; // default to CWD
          if (argc > 1){
            filename = argv[1]; //if specified filename
          }
         if (r = lstat(filename, sp) < 0){
             printf("no such file %s\n", filename);
             exit(1);
         }
         strcpy(path, filename);
         if (path[0] != '/'){ //filename is relative : get cwd
            getcwd(cwd, 256);
            strcpy(path, cwd);
            strcat(path, "/");
            strcat(path, filename);
         }
         if (S_ISDIR(sp->st_mode)){
             ls_dir(path);
         } else {
             ls_file(path);
         }
      }
      else if (strcmp(cmd, "lcat") == 0){
          char lcat[MAX];
          strcat(lcat, "cat ");
          strcat(lcat, pathname);
          system(lcat); 
      }

      else {
        // Send ENTIRE line to server
        n = write(sock, line, MAX);
        //printf("client: wrote n=%d bytes; line=(%s)\n", n, line);
        if (strcmp(pathname[0], 'h') != 0){
            printf("cmd = %s  pathname = %s\n", cmd, pathname);
        } else {
            printf("cmd = %s  pathname =  \n", cmd);
        }

        // Read a line from sock and show it
        bzero(ans, MAX);
        n = read(sock, ans, MAX);
        printf("client: read  n=%d bytes; echo=(%s)\n",n, ans);
        //printf("%s\n", ans);
      }
    }
}


