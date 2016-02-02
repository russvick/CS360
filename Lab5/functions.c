

#include "functions.h"

#define MAX 256

// This is called in the beginning of program to initilize variables
// that we will be using throughout the duration of the program.
void InitilizeVars(char cmd[16][10], char *env[], char home[MAX])
{
  char *temp = (char *)malloc(sizeof(char) * MAX);
  int i = 0;
  // Initilizes all cmd into cmd array
  strcpy(cmd[0], "ls");
  strcpy(cmd[1], "pwd");
  strcpy(cmd[2], "cd");
  strcpy(cmd[3], "mkdir");
  strcpy(cmd[4], "rmdir");
  strcpy(cmd[5], "rm");
  strcpy(cmd[6], "lcat");
  strcpy(cmd[7], "lpwd");
  strcpy(cmd[8], "lls");
  strcpy(cmd[9], "lcd");
  strcpy(cmd[10], "lmkdir");
  strcpy(cmd[11], "lrmdir");
  strcpy(cmd[12], "lrm");
  strcpy(cmd[13], "get");
  strcpy(cmd[14], "put");
  strcpy(cmd[15], "quit");


  // HARD CODED, will change later.
  i = 0;
  while(env[i] != NULL)
  {
    if(IsHomeIndex(env[i]))
    {
      break;
    }
    i++;
  }

  strcpy(temp, env[i]);
  temp += 5;
  strcpy(home, temp);
}

// Returns TRUE if HOME is first 4 characters of string passed,
//  FALSE if not.
Boolean IsHomeIndex(char *envStr)
{
  Boolean correct = FALSE;
  if(envStr[0] == 'H' &&
    envStr[1] == 'O' &&
    envStr[2] == 'M' &&
    envStr[3] == 'E')
  {
    correct = TRUE;
  }

  return correct;
}

// Iterates through the cmd string array, and returns the index
//    that contains the same string as cmdtr.
int GetCommandId(char *cmdtr, char cmd[16][10])
{
  int i = 0, index = -1;
  for(i = 0; i < 16; i++)
  {
    if(strcmp(cmdtr, cmd[i]) == 0)  // They are equal
    {
      index = i;
      break;
    }
  }

  return index;
}

// Resets the tokens array, tokenizes the input line by spaces,
//    And stores those lines in the lines array
void Tokenize(char tokens[100][MAX], char line[MAX])
{
  int i = 0;
  char *token = NULL;
  char tempbuf[MAX];

  strcpy(tempbuf, line);

  // We reset the tokens array to "", because you can't set token[i] to NULL
  for(i = 0; i < 100; i++)
  {
    strcpy(tokens[i], "");
  }

  token = strtok(tempbuf, " ");

  i = 0;
  while(token != NULL)    // Keep copying into tokens if token has stuff
  {
    strcpy(tokens[i], token);
    token = strtok(NULL, " ");
    i++;
  }

  return;
}

// Changes the directory that the C program executes system calls in.
void ChangeDirectories(char *dirpath, char home[MAX])
{
  int status = 0;

  if(strcmp(dirpath, "") == 0) // Case if there's no argument, default to $HOME
  {
    strcpy(dirpath, home);
  }
  status = chdir(dirpath);

  if(status != 0) // Directory is non-existent.
  {
    printf("Directory not found.\n");
  }

  return;
}

// Resets the myargv array, and assigns it pointers to
//  the tokens array for every token in it.
void CopyIntoMyArgv(char *myargv[50], char tokens[100][MAX])
{
  int i = 0;
  for(i = 0; i < 50; i++)
  {
    myargv[i] = NULL;
  }

  i = 0;
  while(strcmp(tokens[i], "") != 0)
  {
    myargv[i] = tokens[i];
    i++;
  }
}

// Sends a file from the client to the server.
// File can be anywhere on the system.
// Sends to the current working directory of the server.
void put(int sock, char filename[128])
{
  char path[128];
  struct stat st;
  char buf[128];
  char cc[128];  // client communication buffer
  int fd, i = 0;
  char cwd[128]; // current working directory
  int n = 0;
  int size = 0;

  getcwd(cwd, 128);

  if ( !filename ) // Return if no filename.
  {
    printf("no filename entered!\n");
    return;
  }
  else
  {
    if ( filename[0] == '/' ) // absolute path is given for the file
    {
      fd = open(filename, O_RDONLY);
    }
    else    // Relative directory
    {
      strcpy(path, cwd );  // copy the current working directory to path
      strcat(path, "/");    // Append a slash to end
      strcat(path, filename); // Append the relative directory

      fd = open(path, O_RDONLY);
    }

    lstat(path, &st);
    size = st.st_size; // We get the size of the file we are transferring.

    sprintf(buf, "%d", size);

    n = write(sock, buf, 128); // Send the file size in bytes to the server.
    while(n != 128)
    {
      n = write(sock, buf, 128); // We keep writing until the server reads it.
    }

    // Size is the file size, and it gets decremented every loop
    while ( size >= 128 )
    {
      read(fd, buf, 128);

      while ( strcmp(cc, "RECEIVED") )
      {
        // Keep trying to write to the server until the server says RECEIVED back
        write(sock, buf, 128);
        n = read(sock, cc, 128);
      }

      strcpy(cc, ""); // Reset cc for next iteration

      // Decrement size by 128, so we know how much to send for the last write
      size -= 128; // Decrement size.
    }
  }

  n = read(fd, buf, size);

  buf[size] = 0;
  // Only send the exact size remaining of the file to server.
  write(sock, buf, size);
  close(path);

}

// Gets a file from the server and copies it's contents to the client.
void get(int sock, char myargvCopy[128])
{
  char temp[128];
  char buf[128];
  char line[MAX];
  int fd, i = 0;
  char *token;
  int n = 0;
  int size = 0;
  int count = 0;

  // Find the last token of string, which is the file name
  token = strtok(myargvCopy, "/");
  while(token != NULL)
  {
    strcpy(temp, token);
    token = strtok(NULL, "/");
  }
  // Open in readonly mode, get rid of contents in file if already exists
  fd = open(temp, O_WRONLY | O_TRUNC | O_CREAT, 0744);

  n = read(sock, buf, 128);
  // size is the size of the file that will be transferred
  size = strtol(buf, NULL, 10);
  // Keep writing until we know we sent 128 bytes, and the server got it.
  n = write(sock, "RECEIVED", 128);
  while(n != 128)
  {
    n = write(sock, "RECEIVED", 128);
  }

  if(size < 0)
  {
    close(fd);
    printf("Didn't read size correctly.\n");
    return;
  }


  strcpy(line, "");
  while(1)
  {
    n = 0;
    n = read(sock, line, 128);
    if(n < 128) // Break if we only have a little bit left
    {
      break;
    }
    write(fd, line, 128);

    n = write(sock, "RECEIVED", 128);
    while(n != 128)
    {
      n = write(sock, "RECEIVED", 128);
    }
    strcpy(line, "");
  }

  write(fd, line, n); // Write only the last remaining n bytes
  n = write(sock, "RECEIVED", 128);
  while(n != 128)
  {
    n = write(sock, "RECEIVED", 128);
  }
  close(fd);
  return;
}
