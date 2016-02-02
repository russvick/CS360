#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define NUM_CMDS 11


typedef struct node
{
    char name[64];
    char type;
    struct node *childPtr, *siblingPtr, *parentPtr;
} NODE;

typedef enum boolean
{
    false, true
} bool;

// Program Functions
void initilize();
void GetLine();
void ProcessLine();
int FindCommand();
void GetDirNameAndBaseName(char *pathname);
void GetSubDirectory(char *pathname);
bool AlreadyExists(NODE *node, char type);
bool SearchAndCreate(NODE **node, char *dirname, char type);
bool SearchAndSetCwd(NODE **node, char *dirname);
bool SearchAndRmDir(NODE **node, char *pathname);
bool SearchAndRm(NODE **node, char *pathname);



// Command Functions
void mkdir(char *pathname);
void rmdir(char *pathname);
void cd(char *pathname);
void ls();
void pwd();
void rpwd(NODE *node, char *directoryName);
void creat(char *pathname);
void rm(char *pathname);
void save();
void rSave(NODE *node, FILE *logFile, char directory[128]);
void reload();
void menu();
void quit();




// Utility Functions
void TrimNewLine(char *str);
bool StrContains(char *str, char character);


// Error Message Functions
void PrintAlreadyExistsError(char *name, char type);
void PrintDirectoryNotFoundError();
void PrintFileNotFoundError();
void PrintNotDirectoryError(char *name);
void PrintDirectoryHasChildrenError(char *name);
void PrintCouldNotOpenFileError(char *filename);
