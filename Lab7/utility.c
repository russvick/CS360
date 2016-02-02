#include "type.h"
#include "lab7.h"

//////////////////////////////////////////////////
/////////////// Utility Fucntions ////////////////
//////////////////////////////////////////////////

int get_block(int fd, int blk, char buf[ ])
{
  lseek(fd, (long)blk*BLKSIZE, 0);
  read(fd, buf, BLKSIZE);
}

int put_block(int block, char *buf)
{
   lseek(fd, (long)BLOCK_SIZE*block,0);
   write(fd, buf,BLOCK_SIZE);
}


void Initilize()
{
  int i;
  P0.cwd = 0;
  P0.uid = 0;

  P1.cwd = 0;
  P1.uid = 1;

  root = 0;

  for (i = 0; i < 100; i++)
  {
    minode[i].refCount = 0;
  }

  return;
}

void GetBlock(int dev, int blk, char buf[BLKSIZE])
{
    lseek(dev, (long)blk*BLKSIZE, 0);
    read(dev, buf, BLKSIZE);
}

void PutBlock(int dev, int blk, char buf[BLKSIZE])
{
    lseek(dev, (long)blk*BLKSIZE, 0);
    write(dev, buf, BLKSIZE);
}

void GetSuper()
{
    char buf[BLKSIZE];

    GetBlock(fd, 1, buf);

    sp = (SUPER *)buf;

    if (sp->s_magic != 0xEF53)
    {
        printf("s_magic: %x is not an EXT2 file system.\n", sp->s_magic);
        exit(1);
    }

    // Save number of blocks and inodes in globals
    nblocks = sp->s_blocks_count;
    ninodes = sp->s_inodes_count;
}

// Gets the GD block, and sets proper global variables.
void GetGD()
{
    char buf[BLKSIZE];
    // read GD to get block numbers of bmap,imap,InodeStartBlock;
    GetBlock(fd, 2, buf);
    gp = (GD *)buf;

    // save as globals
    imap = gp->bg_inode_bitmap;
    bmap = gp->bg_block_bitmap;
    InodeStartBlock = gp->bg_inode_table;
}


// MINODE *iget(in dev, u32 ino): This function returns a pointer to the in-
// memory INODE of (dev, ino). The returned minode is unique, i.e. only one copy of
// the INODE exists in memory. In addition, the minode is locked for exclusive use
// until it is either released or unlocked.

MINODE *iget(int dev, int InodeNum)
{
    char buf[BLKSIZE];
    int i, block, offset;
    INODE *LocalInoPtr;
    MINODE *LocalMinodePtr;

    // Case if there is already the same minode in the minode array.
    for(i = 0; i < NMINODES; i++)
    {
        LocalMinodePtr = &minode[i];
        if(LocalMinodePtr->dev == dev && LocalMinodePtr->ino == InodeNum)
        {
            LocalMinodePtr->refCount++;
            return LocalMinodePtr;
        }
    }

    // Case if we are adding the minode into the minode array.
    for(i = 0; i < NMINODES; i++)
    {
        LocalMinodePtr = &minode[i];
        if(LocalMinodePtr->refCount == 0)
        {
            LocalMinodePtr->refCount = 1;
            LocalMinodePtr->dev = dev;
            LocalMinodePtr->ino = InodeNum;

            // get INODE of ino to buf
            block = (InodeNum - 1)/8 + InodeStartBlock;
            offset = (InodeNum - 1) % 8;

            GetBlock(dev, block, buf);
            LocalInoPtr = (INODE *)buf + offset;

            // Set the inode to the minode's inode attribute
            // Not quite sure here ?
            LocalMinodePtr->INODE = *LocalInoPtr;

            return LocalMinodePtr;
        }
    }
    // We are out of space, so we return
    return NULL;
}

// iput(MINODE *mip): This function releases and unlocks a minode point-
// ed by mip. If the process is the last one to use the minode (refCount = 0), the INODE
// is written back to disk if it is dirty (modified).
int iput(MINODE *mip)
{
    char buf[BLOCK_SIZE];
    int i, block, offset;
    INODE *LocalInoPtr;
    MINODE *tempMINODE;

    // Decrement the refCount by 1.
    mip->refCount--;

    // We enter if statement only if we have to write the inode back to disk
    if(mip->refCount == 0 || mip->dirty  == 1)
    {
        // Mailman's Algorithm to get the block and offset of the inode
        // we are going to write back to disk.
        block = (mip->ino - 1) / 8 + InodeStartBlock;
        offset = (mip->ino - 1) % 8;

        GetBlock(mip->dev, block, buf);
        LocalInoPtr = (INODE *)buf + offset;

        // Safe route
        // Copies over all
        LocalInoPtr->i_mode = mip->INODE.i_mode;
        LocalInoPtr->i_uid = mip->INODE.i_uid;
        LocalInoPtr->i_size = mip->INODE.i_size;
        LocalInoPtr->i_atime = mip->INODE.i_atime;
        LocalInoPtr->i_ctime = mip->INODE.i_ctime;
        LocalInoPtr->i_mtime = mip->INODE.i_mtime;
        LocalInoPtr->i_dtime = mip->INODE.i_dtime;
        LocalInoPtr->i_gid = mip->INODE.i_gid;
        LocalInoPtr->i_links_count = mip->INODE.i_links_count;
        for(i = 0; i < 15; i++)
        {
            LocalInoPtr->i_block[i] = mip->INODE.i_block[i];
        }

        PutBlock(mip->dev, block, buf);
    }

    // We return the refCount of the minode
    return mip->refCount;
}

void mount_root()
{
    GetSuper();
    GetGD();

    if(sp->s_magic != SUPER_MAGIC)
    {
		printf("Not a valid EXT2 filesystem...Exiting\n");
		exit(-1);
	}
    // set root minode
    root = (MINODE *)iget(fd, 2);
    printf("Root ID %x\n",root->INODE.i_mode);
    // Let cwd of both P0 and P1 point at the root minode (refCount=3)
    P0.cwd = (MINODE *)iget(fd, 2);
    P1.cwd = (MINODE *)iget(fd, 2);

    root->refCount = 3;

    return;
}
// PROC* running           MINODE *root
//       |                          |
//       |                          |               ||*********************
//       V                          |  MINODE       ||
//     PROC[0]                      V minode[100]   ||         Disk dev
//  =============  |-pointerToCWD-> ============    ||   ==================
//  |nextProcPtr|  |                |  INODE   |    ||   |     INODEs
//  |pid = 1    |  |                | -------  |    ||   ==================
//  |uid = 0    |  |                | (dev,2)  |    ||
//  |cwd --------->|                | refCount |    ||*********************
//  |           |                   | dirty    |
//  |fd[10]     |                   |          |
//  | ------    |
//  | - ALL 0 - |                   |==========|
//  | ------    |                   |  INODE   |
//  | ------    |                   | -------  |
//  =============                   | (dev,ino)|
//                                  |  refCount|
//    PROC[1]          ^            |  dirty   |
//     pid=2           |
//     uid=1           |
//     cwd ----> root minode        |==========|
