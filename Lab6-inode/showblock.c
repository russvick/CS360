#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <ext2fs/ext2_fs.h>
#include <sys/stat.h>

typedef unsigned int u32;

// define shorter TYPES, save typing efforts
typedef struct ext2_group_desc  GD;
typedef struct ext2_super_block SUPER;
typedef struct ext2_inode       INODE;
typedef struct ext2_dir_entry_2 DIR;    // need this for new version of e2fs

GD    *gp;
SUPER *sp;
INODE *ip;
DIR   *dp;

#define BLKSIZE 1024
#define NUMPARAMS 64
#define TOKENLENGTH 128

int inodeBeginBlock = 0;
int fd = 0;

char *path;
char name[NUMPARAMS][TOKENLENGTH]; 	// to hold filename tokens
char *disk = "diskimage";

void Initilize()
{
	int i = 0;
	fd = open(disk, O_RDONLY);
	for(i = 0; i < NUMPARAMS; i++)
	{
		strcpy(name[i],"");
	}
}

void get_block(int fd, int blk, char buf[BLKSIZE])
{
	lseek(fd, (long)(blk*BLKSIZE), 0);
	read(fd, buf, BLKSIZE);
}

void printSuperInfo(SUPER *s)
{
	printf("*********** super block info ***********\n");
	printf("inode_count: %15d\n", s->s_inodes_count);
  printf("blocks_count: %14d\n",s->s_blocks_count);
  printf("r_blocks_count: %12d\n", s->s_r_blocks_count);
	printf("free_blocks_count: %9d\n", s->s_free_blocks_count);
	printf("free_inodes_count: %9d\n", s->s_free_inodes_count);
  printf("log_blk_size: %14d\n", s->s_log_block_size);
  printf("first_data_block: %10d\n", s->s_first_data_block);
	printf("Magic: %4x\n", s->s_magic);
	printf("rev_level: %17d\n", s->s_rev_level);
	printf("inode_size: %16d\n", s->s_inode_size);
  printf("block_group_nr: %12d\n", 0);
  printf("blksize: %19d\n", 1024);
  printf("indode_per_group: %10d\n", 184);

	printf("------------------------------------\n");

  printf("desc_per_block: %12d\n", 32);
  printf("inodes_per_block: %10d\n", 8);
  printf("inode_size_ration: %9d\n", 1);
}

void printGroupDescriptor(GD* g)
{
		printf("*********** group 0 info ***********\n");
		printf("Blocks bitmap block: %7d\n", g->bg_block_bitmap);
		printf("Inodes bitmap block: %7d\n", g->bg_inode_bitmap);
		printf("Inodes table block: %8d\n", g->bg_inode_table);
		printf("Free blocks count: %9d\n", g->bg_free_blocks_count);
		printf("Free inodes count: %9d\n", g->bg_free_inodes_count);
		printf("Directories count: %9d\n", g->bg_used_dirs_count);
		printf("inodes_start: %14d\n", 10);
}

void ParsePath(char *path)
{
	int i = 0;

	char *token = strtok(path, "/");

	while(token != NULL)
	{
		strcpy(name[i], token);
		token = strtok(NULL, "/");
		i++;
	}
}

int GetParamCount()
{
	int i = 0;
	int count = 0;
	while(strcmp(name[i], "") != 0)
	{
		count++;
		i++;
	}
	return i;
}

void printInode(INODE *ip)
{
	printf("\n=======Inode info=======\n");

	printf("Mode: %d, uid: %d, size: %d\n", ip->i_mode, ip->i_uid, ip->i_size);
	printf("gid: %d, links: %d, blocks: %d", ip->i_gid, ip->i_links_count, ip->i_blocks);

	printf("\n=========================\n\n");
}

int search(INODE *ip, char *token)
{
	/*
	1KB      0    1    2    3    4  | 5 . ........... 27| 28 ..................|
BLOCK: |Boot|Super| Gd |Bmap|Imap|Inodes blocks .....|....  data blocks ....|
                                 |    INODEs         |
                              ino|1,2,3,4 .......... |*/
/*
		1. Read block
		2. Scans through each of the files in the block to see if's name matches
		3. if it matches and is a file, and it's the file we are looking for return it's ino
		4. else its a match, but it's a file and it's not the file we are looking for (dirsRemaining == 0)
			 return 0
		5. else if it's a directory and it's the next one on the path
		6. If we get out of the loop, the path is invalid return 0
*/

	int i;
	char *cp;
	char dbuf[BLKSIZE];

	printf("Searching for: %s\n", token);

	for(i = 0; i < 12; i++)
	{
		if(ip->i_block[i] == 0)
		{
			printf("File not found in directory.\n\n");
			return 0;
		}

		get_block(fd, ip->i_block[i], dbuf);
		cp = dbuf;
		dp = (DIR *)dbuf;

		printf("**********************************************************\n");

		printf("\ti_number rec_len name_len\tname\n");
		//cycle through directories for the entirty of dbuf and blksize(1024)
		while(cp < dbuf + BLKSIZE)
		{
			printf("%10d %10d %8d\t\t%s\n", dp->inode, dp->rec_len, dp->name_len, dp->name);

			if(strcmp(dp->name, token) == 0)
			{
						printInode(ip);
				return dp->inode;
			}

  		//advance cp the entry length of dp in bytes
			cp+=dp->rec_len;

			dp = (DIR *)cp;
			//no null byte so we must add one
			dp->name[dp->name_len] = '\0';
		}
	}
	return 0;
}

main(int argc, char *argv[], char *env[])
{
	int i = 0, n = 0;
	int ino = 0;
	char buf[BLKSIZE];

	if(argc != 3) 	// Change diskname to what user specified.
	{
		printf("Usage: ./a.out [diskname] [path]\n");
		exit(1);
	}
	disk = argv[1];
	path = argv[2];

	Initilize();

	// Get the super block.
	get_block(fd, 1, buf);
	// Cast it to a SUPER structure
	sp = (SUPER *)buf;
	// Check if it is a EXT2 file system
	if(sp->s_magic != 0xEF53)
	{
		printf("Not an EXT2 File System.\n");
		exit(1);
	}
	printSuperInfo(sp);

	// get the GD block
	get_block(fd, 2, buf);

	gp = (GD *)buf;

	printGroupDescriptor(gp);
  //Grabs the block number of the start of the inode table of the current block group
	inodeBeginBlock = gp->bg_inode_table;

	get_block(fd, inodeBeginBlock, buf);
	ip = (INODE *)buf + 1; 	// Because inode number counts from 1
	printf("*********** root inode info ***********\n");
  printf("File mode: \t\t%4x\n", ip->i_mode);
	printf("Size in bytes: %13d\n", ip->i_size);
	printf("Blocks count: %14d\n", ip->i_blocks);
	printf("hit a key to continue : ");
	getchar();
	fflush(stdin);

	printf("block[0] = %d\n", ip->i_block[0]);
	printf("*********** root dir entries ***********\n");
  printf("block = %d\n", ip->i_block[0]);
	// Put tokens into name[][] char array
	ParsePath(argv[2]);

	i = 0;

	n = GetParamCount();

	for(i = 0; i < n; i++)
	{
		ino = search(ip, name[i]);

		if(ino == 0)
		{
			printf("Couldn't find the correct file.\n");
			break;
		}

		int block_num = (ino - 1) / 8 + inodeBeginBlock;
		int offset = (ino - 1) % 8;

		get_block(fd, block_num, buf);

		ip = (INODE *)buf + offset;


		if(S_ISREG(ip->i_mode) && strcmp(name[i + 1], "") != 0)
		{
			ino = 0;
			printf("Can't look for a file in a file.\n");
			break;
		}
	}

	getchar();

	printf("size = %d blocks  = %d\n", ip->i_size, ip->i_blocks/2);

	printf("*********** DISK BLOCKS  ***********\n");
	for(i = 0; i < 14; i++)
	{
		printf("Block[%d] = %d\n", i , ip->i_block[i]);
	}


	printf("*********** DIRECT BLOCKS  ***********\n");
	for(i = 0; i < 12; i++)
	{
		printf("%d ", ip->i_block[i]);
	}
	printf("\n\n");

	printf("*********** INDIRECT BLOCKS  ***********\n");

	get_block(fd, ip->i_block[12], buf);
	for(i = 0;  i <= 256; i++)
	{
		//int val = (int)buf[i];
		printf("%d ", ip->i_block[12]+i);
		if((i % 10 == 0) && (i >= 10))
		{
			printf("\n");
		}
	}
	getchar();

  printf("\n\n");
	printf("*********** DOUBLE INDIRECT BLOCKS  ***********\n");

	for(i = 0; i < 166; i++)
	{
		printf("%d ", (ip->i_block[13]+i));

		if((i % 10 == 0) && (i >= 10))
		{
			printf("\n");
		}
	}
	printf("\n");
	printf("Done!\n");


	return 0;
}
