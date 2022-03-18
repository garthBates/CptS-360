/*********** util.c file ****************/

int get_block(int dev, int blk, char *buf)
{
   lseek(dev, (long)blk*BLKSIZE, 0);
   read(dev, buf, BLKSIZE);
}   
int put_block(int dev, int blk, char *buf)
{
   lseek(dev, (long)blk*BLKSIZE, 0);
   write(dev, buf, BLKSIZE);
}   

int tokenize(char *pathname)
{
  char *s;
  strcpy(gline, pathname);
  nname = 0;
  s = strtok(gline, "/");
  while(s){
    name[nname++] = s;
  s = strtok(0, "/");
  }
}


MINODE *iget(int dev, int ino)
{
  MINODE *mip;
  MTABLE *mp;
  INODE *ip;
  int i, block, offset;
  char buf[BLKSIZE]

  for (i=0, iNMINODES; i++){
    MINODE *mip = &MINODE[i];
    if (mip->refCount && (mip->dev==dev) && (mip->ino==ino)){
      mip->refCount++
      return mip;
    }
  }
  mip = malloc();
  mip->dev; mip->ino = ino;
  block = (ino-1)/8 + iblock;
  offset= (ino-1)%8;
  get_block(dev, block, buf);
  ip = (INODE *)buf + offset;
  mip->INODE = *ip;
  // initialize minode
  mip->refCount = 1;
  mip->mounted = 0;
  mip->dirty = 0;
  mip->mountptr = 0;
  return mip;
}

void iput(MINODE *mip)
{
  INODE *ip;
  int i, block, offset;
  char buf[BLKSIZE];

  if (mip==0) return;
  mip->refCount--;
  if (mip->refCount > 0) return;
  if (mip->dirty == 0) return;

  // write INODE back to disk
  block = (mip->ino - 1) / 8 + iblock;
  offset = (mip->ino - 1) % 8;  

  // get block containing this inode
  get_block(mip->dev, block, buf);
  ip = (INODE *)buf + offset;
  *ip = mip->INODE;
  put_block(mip->dev, block, buf);
  midalloc(mip);
} 

int search(MINODE *mip, char *name)
{
  int i;
  char *cp, temp[256], sbuf[BLKSIZE];
  DIR *dp;
  for (i=0; i<12; i++){ // search DIR direct blocks only
    if (mip->INODE.i_block[i] == 0)
      return 0;
    get_block(mip->dev, mip->INODE.i_block[i], sbuf);
    dp = (DIR *)sbuf;
    cp = sbuf;
    while (cp < sbuf + BLKSIZE){
      strncpy(temp, dp->name, dp->name_len);
      temp[dp->name_len] = 0;
      printf("%8d%8d%8u %s\n", dp->inode, dp->rec_len, dp->name_len, temp);
      if (strcmp(name, temp)==0){
        printf("found %s : inumber = %d\n", name, dp->inode);
        return dp->inode;
      }
      cp += dp->rec_len;
      dp = (DIR *)cp;
    }
  }
  return 0;
}

int getino(char *pathname)
{
  MINODE *mip;
  int i, ino;
  if (strcmp(pathname, "/")==0){
    return 2;       // return root
  }   
  if (pathname[0] == "/")
    mip = root; // if absolute
  else
    mip = p0->cwd;     // if relative
  mip->refCount++;        // in order to
    
  tokenize(pathname); // assume: name[ ], nname are globals

  for (i=0; i<nname; i++){    // search for each component string
      if (!S_ISDIR(mip->INODE.i_mode)){ // check DIR type
        printf("%s is not a directory\n", name[i]);
        iput(mip);
        return 0;
      }
      ino = search(mip, name[i]);
      if (!ino){
        printf("no such component name %s\n", name[i]);
        iput(mip);
        return 0;
      }
      iput(mip);   // release current minode
      mip = iget(dev, ino);   // switch to new minode
    }
    iput(mip);
    return ino;
}

int findmyname(MINODE *parent, u32 myino, char *myname) 
{
  int i;
    char buf[BLOCK_SIZE], namebuf[256], *cp;

    for(i = 0; i <= 11 ; i ++)
    {
        if(parent->INODE.i_block[i] != 0)
        {
            get_block(parent->dev, parent->INODE.i_block[i], buf);
            dp = (DIR *)buf;
            cp = buf;

            while(cp < &buf[BLOCK_SIZE])
            {
                strncpy(namebuf,dp->name,dp->name_len);
                namebuf[dp->name_len] = 0;

                if(dp->inode == myino)
                {
                    strcpy(name, namebuf);
                    return 1; // success!
                }
                cp +=dp->rec_len;
                dp = (DIR *)cp;
            }
        }
    }
    return -1; // no success
}

int findino(MINODE *mip, u32 *myino) // myino = ino of . return ino of ..
{
  INODE *ip;
  char buf[1024];
  char *cp;
  DIR *dp;

  //check if exists
  if(!mip)
  {
    printf("ERROR: ino does not exist!\n");
    return 1;
  }

  //point ip to minode's inode
  ip = &mip->INODE;

  //check if directory
  if(!S_ISDIR(ip->i_mode))
  {
    printf("ERROR: ino not a directory\n");
    return 1;
  }

  get_block(dev, ip->i_block[0], buf);
  dp = (DIR*)buf;
  cp = buf;

  //.
  *myino = dp->inode;

  cp += dp->rec_len;
  dp = (DIR*)cp;

  //..
  *parentino = dp->inode;

  return 0;
}
