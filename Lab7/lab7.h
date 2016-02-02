#ifndef LAB7_H
#define LAB7_H

#include "type.h"

int fd;
int imap, bmap;
int ninodes, nblocks, nFreeInodes, nFreeBlocks, InodeStartBlock;

PROC P0, P1;
PROC proc[NPROC];
PROC *running;
MINODE *root;
MINODE minode[NMINODES];

// Utility functions
void GetBlock(int dev, int blk, char buf[BLKSIZE]);
int iput(MINODE *mip);
int GetIno(int dev, char *pathname);
MINODE *iget(int dev, int InodeNum);
void PutBlock(int dev, int blk, char buf[BLKSIZE]);


// File System Functions
void Initilize();
void GetUserInput(char *line, char command[64], char pathname[128], char parameter[64]);
void GetSuper();
void GetGD();
void MountRoot();
void Ls(char *pathname);
int ListFile(MINODE *mip, char *filename);
int ListDir(MINODE *mip);



#endif
