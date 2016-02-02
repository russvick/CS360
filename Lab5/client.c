/*
Lab 5
Ritche Long
Russ Vick
CS360
KC Wang
Lab 05
*/

#include "functions.h"

#define MAX 256

// Define variables
struct hostent *hp;
struct sockaddr_in  server_addr;

int sock, r;
int SERVER_IP, SERVER_PORT;


// client initialization code
int client_init(char *argv[])
{
  printf("======= clinet init ==========\n");

  printf("1 : get server info\n");
  hp = gethostbyname(argv[1]);
  if (hp==0){
     printf("unknown host %s\n", argv[1]);
     exit(1);
  }

  SERVER_IP   = *(long *)hp->h_addr;
  SERVER_PORT = atoi(argv[2]);

  printf("2 : create a TCP socket\n");
  sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock<0){
     printf("socket call failed\n");
     exit(2);
  }

  printf("3 : fill server_addr with server's IP and PORT#\n");
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = SERVER_IP;
  server_addr.sin_port = htons(SERVER_PORT);

  // Connect to server
  printf("4 : connecting to server ....\n");
  r = connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
  if (r < 0){
    printf("connect failed\n");
    exit(1);
  }

  printf("5 : connected OK to \007\n");
  printf("---------------------------------------------------------\n");
  printf("hostname=%s  IP=%s  PORT=%d\n",
          hp->h_name, (char *)inet_ntoa(SERVER_IP), (int)SERVER_PORT);
  printf("---------------------------------------------------------\n");

  printf("========= init done ==========\n");
}

main(int argc, char *argv[ ], char *env[])
{
  int n, i = 0, pid, childpid, status, fd;
  char line[MAX], ans[MAX];
  char *myargv[128];
  char tokens[100][256];
  char *token;
  char temp[MAX];
  char myargvCopy[128];
  int commandId = -1;
  char commands[16][10];
  char home[MAX];


  if (argc < 3){
    printf("Usage : client ServerName SeverPort\n");
    exit(1);
  }

  client_init(argv);
  InitilizeVars(commands, env, home);


  printf("********  processing loop  *********\n");
  while (1){
    printf("input a line : ");
    bzero(line, MAX);                // zero out line[ ]
    fgets(line, MAX, stdin);         // get a line (end with \n) from stdin

    line[strlen(line)-1] = 0;        // kill \n at end
    if (line[0]==0)                  // exit if NULL line
      exit(0);

    // Tokenize the string.
    Tokenize(tokens, line);
    CopyIntoMyArgv(myargv, tokens);
    commandId = GetCommandId(tokens[0], commands);


    switch(commandId)
    {
      case 0: // ls
        // Send ENTIRE line to server
        n = write(sock, line, 128);

        strcpy(line, "");
        while(strcmp(line, "END OF LS"))
        {
          n = 0;
          n = read(sock, line, 128);
          printf("%s\n", line);
          n = write(sock, "RECEIVED", 128);
        }
        printf("%s executed successfully.\n", myargv[0]);

        break;
      case 1: // pwd
        // Write entire line to server
        n = write(sock, line, 128);
        n = read(sock, ans, 128);
        printf("%s\n", ans);
        printf("%s executed successfully.\n", myargv[0]);
        break;
      case 2: // cd
      case 3: // mkdir
      case 4: // rmdir
      case 5: // rm
        // Send ENTIRE line to server
        n = write(sock, line, 128);

        // Read a line from sock and show it
        printf("%s executed successfully.\n", myargv[0]);
        break;
      case 6: // lcat
        // Fork a process for child and parent process
        pid = fork();
        if(pid)    // Parent does this
        {
          childpid = wait(&status);
          printf("%s executed successfully.\n", myargv[0]);
        }
        else      // Child does this
        {
          execve("/bin/cat", myargv, env);
        }

        break;
      case 7: // lpwd
        pid = fork();
        if(pid)
        {
          childpid = wait(&status);
          printf("%s executed successfully.\n", myargv[0]);
        }
        else
        {
          execve("/bin/pwd", myargv, env);
        }
        break;
      case 8: // lls
        pid = fork();
        if(pid)
        {
          childpid = wait(&status);
          printf("%s executed successfully.\n", myargv[0]);

        }
        else
        {
          execve("/bin/ls", myargv, env);
        }
        break;
      case 9: // lcd
        ChangeDirectories(tokens[1], home);
        printf("%s executed successfully.\n", myargv[0]);

        break;
      case 10: // lmkdir
        pid = fork();
        if(pid)
        {
          childpid = wait(&status);
          printf("%s executed successfully.\n", myargv[0]);
        }
        else
        {
          execve("/bin/mkdir", myargv, env);
        }

        break;
      case 11: // lrmdir
        pid = fork();
        if(pid)
        {
          childpid = wait(&status);
          printf("%s executed successfully.\n", myargv[0]);
        }
        else
        {
          execve("/bin/rmdir", myargv, env);
        }

        break;
      case 12: // lrm
        pid = fork();
        if(pid)
        {
          childpid = wait(&status);
          printf("%s executed successfully.\n", myargv[0]);
        }
        else
        {
          execve("/bin/rm", myargv, env);
        }

        break;
      case 13: // get
        // Send get [pathname] to server
        n = write(sock, line, 128);
        strcpy(myargvCopy, myargv[1]);
        get(sock, myargvCopy);
        printf("%s executed successfully.\n", myargv[0]);
        break;
      case 14: // put
        // Send put [pathname] to server
        n = write(sock, line, 128);
        strcpy(myargvCopy, myargv[1]);
        put(sock, myargvCopy);
        printf("%s executed successfully.\n", myargv[0]);
        break;
      case 15: // quit
        exit(1);
        break;
      default: // Not valid
        printf("Not a valid command! Please try again.\n");
        break;
    }
  }
}
