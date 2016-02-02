
#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netdb.h>
#include <fcntl.h>

#define MAX 256

typedef enum boolean
{
  FALSE, TRUE
} Boolean;

void InitilizeVars(char cmd[16][10], char *env[], char home[MAX]);
int GetCommandId(char *commandStr, char cmd[16][10]);
void Tokenize(char tokens[100][MAX], char line[MAX]);
void CopyIntoMyArgv(char *myargv[50], char tokens[100][MAX]);
Boolean IsHomeIndex(char *envStr);
void put(int sock, char filename[128]);
void get(int sock, char myargvCopy[128]);






#endif
