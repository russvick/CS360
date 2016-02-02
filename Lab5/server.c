// This is the echo SERVER server.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netdb.h>

#define  MAX 256

#define LS 0
#define CD 1
#define MKDIR 2
#define RMDIR 3
#define RM 4
#define GET 5
#define PUT 6
#define QUIT 7
#define ERROR 8
#define PWD 9

typedef unsigned short u16;
typedef unsigned long u32;

// Define variables:
struct sockaddr_in  server_addr, client_addr, name_addr;
struct hostent *hp;

int  sock, newsock;                  // socket descriptors
int  serverPort;                     // server port number
int  r, length, n;                   // help variables
char path[MAX][MAX];
char *myargv[MAX];
char cwd[MAX];
char home[MAX];

int put(int ns)
{
	char temp[128];
	char* token;
	char buf[128];
	int fd;
	int i = 0;

	// Find the last token of string, which is the file name
	strcpy(temp, myargv[1]);
	token = strtok(temp, "/");

	while(token)
	{
		strcpy(temp, token);
		token = strtok(NULL, "/");
	}

	fd = open(temp, O_WRONLY | O_TRUNC | O_CREAT, 0744);

	if ( fd == -1 )
	{
		return;
	}

	while(strcmp(buf, "EOF"))
	{
		n = 0;
		n = read(ns, buf, 128);
		//printf("n is: %d bytes\n", n);
		if(strcmp(buf, "EOF"))
		{
			write(fd, buf, 128);
		}

		printf("read: %s\n", buf);
		write(ns, "RECEIVED", 128);
		strcpy(buf, "");
	}
	close(fd);
}

int get(int ns)
{
	char p[128];
	char buf[128];
	char cc[128];  // client communitcation buffer
	int fd, i = 0;
	struct stat st;	//stat struct

	if ( !myargv[1] )
	{
		printf("no filename entered!\n");
		return;
	}
	else
	{
		if ( myargv[1][0] == '/' )
		{
			strcpy(p, myargv[1]);
			fd = open(p, O_RDONLY);
		}
		else
		{
			strcpy(p, cwd);
			strcat(p, "/");
			strcat(p, myargv[1]);

			fd = open(p, O_RDONLY);
		}

		lstat(p, &st);

		i = st.st_size;
		if ( i < 0 )
		{
			sprintf(buf, "-1\n");
			//write(fd, buf, 128);
			close(fd);
			return;
		}
		else
		{
			sprintf(buf, "%d", i);
			//write(fd, buf, 128);
		}

		printf("size: %s\n", buf);

		strcpy(cc, "");

		//read(ns, cc, 128);
		//printf("cc: %s\n", cc);
		while( strcmp(cc, "RECEIVED") )
		{
			write(ns, buf, 128);
			n = read(ns, cc, 128);
			printf("cc: %s\n", cc);
		}
		strcpy(cc, "");
		printf("n: %d\n", n);



		while ( i >= 128 )
		{
			read(fd, buf, 128);
			printf("read: %s\n", buf);

			while ( strcmp(cc, "RECEIVED") )
			{
				write(ns, buf, 128);
				n = read(ns, cc, 128);
				printf("n: %d\n", n);
			}
			i -= 128;

			strcpy(cc, "");

		}

		n = read(fd, buf, i);
		buf[i] = 0;
		printf("n: %d\n", n);
		printf("read: %s\n", buf);
		//while( strcmp(cc, "RECEIEVED") )
		//{
			write(ns, buf, i);
			printf("i: %d\n", i);

		//	printf("read: %s\n", buf);
		//	n = read(ns, cc, 128);
		//	printf("n: %d\n", n);
		//}
		i = 0;

		//write(ns, "EOF", 128);

		close(p);
	}
}

int ls(int ns)
{
	printf("start of ls\n");
	struct stat st;	//stat struct
	struct tm *time; // time struct
	struct dirent *dp; // directory pointer
	char buf[128];
	char cc[128]; 		// client communication buffer
	char temp[128];
	char t[128];
	DIR *dir;

	int asdf=0;

	int stlen = 0;
	int k;

	u16 i = 0; // to read unsigned 16 and 32 bit integers
	u32 j = 0;
	short linkFlag = 0; // check for if there is a link to print at the end
	char linkBuf[128]; // string buffers
	char timeBuf[128];


	printf("before myargv, %s\n", myargv[1]);
	printf("home: %s\n", home);
	if ( myargv[1])
	{
		if ( myargv[1][0] ==  '/')
		{
			strcpy(temp, home);		// home directory (our root)
			strcat(temp, myargv[1]);		// cat myargv[1]
			//strcpy(temp, myargv[1]);	// dirName is absolute
			printf("abs:%s\n", temp);
		}
		else
		{
			getcwd(temp, 128);	// dirName is relative
			strcat(temp, "/");
			strcat(temp, myargv[1]);

			printf("rel:%s\n", temp);
		}
	}
	else
	{
		getcwd(temp, 128);
		printf("null argv\n");
	}

	dir = opendir(temp);

	while((dp = readdir(dir)) != NULL )
	{
		// subtract the lenth of the last file from length of temp
		// make everything after that point null
		// then concatenate the new string to that position. stlen

		k = strlen(temp) - stlen;
		//printf("temp:%s\nk:%d\n", temp, k);
		while ( temp[k] ) { temp[k] = 0; i++; }
		//printf("1temp now: %s\n", temp);
		strcat(temp, "/");
		strcat(temp, dp->d_name);
		//printf("2temp now: %s\n", temp);
		stlen = strlen(dp->d_name);
		//printf("stlen: %d\n", stlen);

		//printf("before lstat dp->d_name: %s\n", dp->d_name);
		if ( lstat(temp, &st) < 0 )	//read stats of file
		{
			printf("ERROR: could not open %s", cwd);
			break;
		}

		i = st.st_mode;
		//printf("st_mode = %d\n", i);

		//printf("0120000 & i = %d\n", i & 0120000);
		if ( (i & 0100000) == 0100000 && (i & 0020000) != 0020000 ) sprintf(buf, "-");
		else if ( (i & 0040000) == 0040000) sprintf(buf, "d");
		else if ( (i & 0120000) == 0120000)
		{
			sprintf(buf, "l");
			linkFlag = 1;
		}
		else printf("ERROR\n");

		//printf("  "); // for formatting

		// see if each type of user has read (r), write (w), or executable(x) access
		// if not place a - instead

		//owner
		if ( i & ( 1 << 8 ) ) strcat(buf, "r");
		else strcat(buf, "-");
		if ( i & ( 1 << 7 ) ) strcat(buf, "w");
		else strcat(buf, "-");
		if ( i & ( 1 << 6 ) ) strcat(buf, "x");
		else strcat(buf, "-");

		//group
		if ( i & ( 1 << 5 ) ) strcat(buf, "r");
		else strcat(buf, "-");
		if ( i & ( 1 << 4 ) ) strcat(buf, "w");
		else strcat(buf, "-");
		if ( i & ( 1 << 3 ) ) strcat(buf, "x");
		else strcat(buf, "-");

		//user
		if ( i & ( 1 << 2 ) ) strcat(buf, "r");
		else strcat(buf, "-");
		if ( i & ( 1 << 1 ) ) strcat(buf, "w");
		else strcat(buf, "-");
		if ( i & ( 1 << 0 ) ) strcat(buf, "x");
		else strcat(buf, "-");

		i = st.st_uid;		// print original creators UID integer
		sprintf(t, " %d", i);
		strcat(buf, t);

		i = st.st_gid;
		sprintf(t, " %d", i);
		strcat(buf, t);

		j = st.st_size;		// print size in bytes
		sprintf(t, " %d", j);
		strcat(buf, t);

		time = localtime(&(st.st_mtime));	// get time
		strftime(timeBuf, 128, "\t%b %d %H:%M", time); // put formatted time in a string
		//printf(" %s", timeBuf);		// print time
		strcat(buf, timeBuf);
		strcat(buf, " ");
		// if there is a link
		if ( linkFlag == 1 )
		{
			/*k = 0;
			while ( linkBuf[k] ) { linkBuf[k] = 0; k++; }*/
			strcpy(t, dp->d_name); // put name of the link into buf
			strcat(t, " -> ");  // concatenate arrow ( link points to actual file )
			k = readlink(temp, linkBuf, 128); // read link into linkBuf
			//printf("%s\n", t);
			linkBuf[k + 1] = 0; // remove garbage after ls link
			//strcat(t, "\t");
			strcat(t, linkBuf); // concatenate actual link name
			//buf[strlen(t) - 1] = 0;
			//printf("t: %s\nbuf %s\n", t, buf);
			t[strlen(t) - 1] = 0;
			//printf(" %s\n", t); // print whole string
			strcat(t, " ");
			strcat(buf, t);
		}
		else
		{
			//printf(" %s\n", dp->d_name);
			strcat(buf, dp->d_name);
		}
		linkFlag = 0;

		printf("buf: %s\n", buf);


		//n = read(ns, cc, 128);			// make sure client recieved line


		while ( strcmp(cc, "RECEIVED") )
		{
			write(ns, buf, 128);
			n = read(ns, cc, 128);
			printf("n: %d\n", n);
		}

		strcpy(cc, "");
		asdf++;
		printf("client recieved line %d\n", asdf);

	}// end while
	//}

	write(ns, "END OF LS", 128);

	/*else
	{
		printf("%s not a valid path", temp);
		return;
	}*/
}

int findcmd(char* line)
{
	if ( !strncmp(line, "ls", 2) )
		return LS;
	else if ( !strncmp(line, "cd", 2) )
		return CD;
	else if ( !strncmp(line, "mkdir", 5) )
		return MKDIR;
	else if ( !strncmp(line, "rmdir", 5) )
		return RMDIR;
	else if ( !strncmp(line, "rm", 2) )
		return RM;
	else if ( !strncmp(line, "get", 3) )
		return GET;
	else if ( !strncmp(line, "put", 3) )
		return PUT;
	else if ( !strcmp(line, "quit") )
		return QUIT;
	else if ( !strcmp(line, "pwd") )
		return PWD;
	else
		return ERROR;
}

/*int runcmd(char* env[], char* buf)
{
	int i = 0, pid = 0;

	char filename[MAX];
	char *cmd;
	char *temp = "temp";

	pid = fork();

	if ( pid )
	{
		waitpid(pid,0,0);
		printf("child %d died", pid);
	}
	else
	{

		while( strcmp(path[i], "") ) // while path has an entry
		{
			printf("path[i]=%s\n", path[i]);
			strcpy(filename, path[i]); 	// copy path[i] to filename
			strcat(filename, "/");		// concatenate /
			strcat(filename, myargv[0]);	// concatenate command name

			//printf("filename=%s\n", filename);

			close(1);
			open("temp", O_WRONLY | O_CREAT, 744 );
			execve(filename, myargv, env); // execute filename
			close("temp");
			//fgets(buf, MAX, stdout);
			i++;

		}
		fprintf(stderr, "Command not found\n");
	}
}*/

// cd changes directory to home if nothing is entered or to a relative or absolute path
int cd()
{
	char dir[MAX];
	//char *home;
	int i = 0;

	if ( myargv[1] ) // if an argument exists
	{
		if(myargv[1][0] == '/') // absolute case
		{
			printf("abs\n");
			if ( chdir(myargv[1]) < 0 )
			{
				fprintf(stderr, "could not change directory %s\n", myargv[1]);
			}
			strcpy(cwd, myargv[1]);
		}
		else if ( strcmp(myargv[1], "..") == 0 )
		{
			if ( strcmp(cwd, home) == 0 )
			{
				return;
			}
			else
			{
				if ( chdir(myargv[1]) < 0 )
					fprintf(stderr, "could not change directory %s\n", myargv[1]);
				getcwd(dir, MAX);
				strcpy(cwd, dir);
			}
		}
		else // relative case
		{
			printf("rel\n");
			getcwd(dir, MAX); // get current directory
			strcat(dir, "/"); // concatenate a /
			strcat(dir, myargv[1]); // concatenate the argument

			if ( chdir(dir) < 0 ) // change dir
			{
				printf("could not change directory %s\n", dir);
			}

			strcpy(cwd, dir);
		}
	}
	else // no argument
	{
		printf("null\n");
		if ( chdir(home) < 0 ) // change dir to server home
		{
			printf("ERROR: could not change to home directory!");
		}
	}
}

void pwd(int ns)
{
	char temp[128];
	int stlen = strlen(home) + 1;

	strcpy ( temp, "~/");
	printf("home: %s\n", home);

	if ( strcmp(home, cwd) == 0 )
	{
		write(ns, temp, 128);
	}
	else
	{
		strcat(temp, &cwd[stlen]);
		write(ns, temp, 128);
	}

	//strcpy(temp, tok);
	printf("%s\n", temp);
	//write(ns, temp, 128);
}

// setmyargs creates myargv
int mkcmd(char *entry)
{
	int i = 1;
	char* cmd;

	cmd = strtok(entry, " "); // first item is the command
	myargv[0] = cmd;
	//printf("myargv[0]=%s\n", myargv[0]);
	myargv[i] = strtok(NULL, " "); // everything else is an argument
	//printf("myargv[1]=%s\n",myargv[i]);

	while ( myargv[i] ) // stops when strtok returns null
	{
		i++;
		myargv[i] = strtok(NULL, " ");
		//printf("myargv[%d]=%s\n", i, myargv[i]);
	}

	//myargv[i+1] = 0; // last argument is null
}

// get path from server's env
int getpath(char* env[])
{
	int i = 0;
	char temp[MAX];
	char *tok;

	while(strncmp(env[i],"PATH=", 5)) { i++; } // find PATH= in env

	strcpy(temp, env[i]); 		// copy env to temp to prevent messing up env

	tok = strtok(temp, "PATH="); 	// get PATH out of temp
	//printf("tok:%s\n", tok);
	tok = strtok(tok, ":");		// get first token before :
	//printf("tok:%s\n", tok);
	strcpy(path[0], tok);		// copy first token into path[0] gbl
	//printf("path[0]=%s\n",path[0]);

	i = 0;				// need i to be 0 again

	while ( 1 )
	{
		i++;
		tok = strtok(NULL, ":");	// get next token
		if ( !tok )			// if null break
		{
			path[i][0] = 0;		// set next path var to null
			break;
		}
		strcpy(path[i], tok);		// copy into path
	}
}

// Server initialization code:

int server_init(char *name)
{
   printf("==================== server init ======================\n");
   // get DOT name and IP address of this host

   printf("1 : get and show server host info\n");
   hp = gethostbyname(name);
   if (hp == 0){
      printf("unknown host\n");
      exit(1);
   }
   printf("    hostname=%s  IP=%s\n",
               hp->h_name,  inet_ntoa(*(long *)hp->h_addr));

   //  create a TCP socket by socket() syscall
   printf("2 : create a socket\n");
   sock = socket(AF_INET, SOCK_STREAM, 0);
   if (sock < 0){
      printf("socket call failed\n");
      exit(2);
   }

   printf("3 : fill server_addr with host IP and PORT# info\n");
   // initialize the server_addr structure
   server_addr.sin_family = AF_INET;                  // for TCP/IP
   server_addr.sin_addr.s_addr = htonl(INADDR_ANY);   // THIS HOST IP address
   server_addr.sin_port = 0;   // let kernel assign port

   printf("4 : bind socket to host info\n");
   // bind syscall: bind the socket to server_addr info
   r = bind(sock,(struct sockaddr *)&server_addr, sizeof(server_addr));
   if (r < 0){
       printf("bind failed\n");
       exit(3);
   }

   printf("5 : find out Kernel assigned PORT# and show it\n");
   // find out socket port number (assigned by kernel)
   length = sizeof(name_addr);
   r = getsockname(sock, (struct sockaddr *)&name_addr, &length);
   if (r < 0){
      printf("get socketname error\n");
      exit(4);
   }

   // show port number
   serverPort = ntohs(name_addr.sin_port);   // convert to host ushort
   printf("    Port=%d\n", serverPort);

   // listen at port with a max. queue of 5 (waiting clients)
   printf("5 : server is listening ....\n");
   listen(sock, 5);
   printf("===================== init done =======================\n");
}


int rf(int ns)
{
	char p[128];
	char buf[128];
	if (myargv[1][0] == '/')
	{
		printf("inif\n");
		strcpy(p, home);
		strcat(p, myargv[1]);
		printf("p: %s\n", p);
		if ( (unlink(p)) < 0 )
		{
			sprintf(buf, "could not remove directory\n" );
			write(ns, buf, 128);
			return;
		}
		sprintf(buf, "successfully removed directory\n");
		write(ns, buf, 128);
	}
	else
	{
		printf("inelse\n");
		strcpy(p, cwd);
		strcat(p, "/");
		strcat(p, myargv[1]);
		printf("p: %s\n", p);
		if ( (unlink(p)) < 0 )
		{
			sprintf(buf, "could not remove directory\n" );
			write(ns, buf, 128);
			return;
		}
		sprintf(buf, "successfully removed directory\n");
		write(ns, buf, 128);
	}
}

int rd(int ns)
{
	char p[128];
	char buf[128];
	if (myargv[1][0] == '/')
	{
		printf("inif\n");
		strcpy(p, home);
		strcat(p, myargv[1]);
		printf("p: %s\n", p);
		if ( (rmdir(p)) < 0 )
		{
			sprintf(buf, "could not remove directory\n" );
			write(ns, buf, 128);
			return;
		}
		sprintf(buf, "successfully removed directory\n");
		write(ns, buf, 128);
	}
	else
	{
		printf("inelse\n");
		strcpy(p, cwd);
		strcat(p, "/");
		strcat(p, myargv[1]);
		printf("p: %s\n", p);
		if ( (rmdir(p)) < 0 )
		{
			sprintf(buf, "could not remove directory\n" );
			write(ns, buf, 128);
			return;
		}
		sprintf(buf, "successfully removed directory\n");
		write(ns, buf, 128);
	}
}


int md(int ns)
{
	char p[128];
	char buf[128];
	if (myargv[1][0] == '/')
	{
		strcpy(p, home);
		strcat(p, myargv[1]);
		//printf("p: %s\n", p);
		if ( (mkdir(p, 0755)) < 0 )
		{
			sprintf(buf, "could not create directory\n" );
			write(ns, buf, 128);
			printf("%s", buf);
			return;
		}
		sprintf(buf, "successfully created directory\n");
		write(ns, buf, 128);
		printf("%s", buf);
	}
	else
	{
		strcpy(p, cwd);
		strcat(p, "/");
		strcat(p, myargv[1]);
		//printf("p: %s\n", p);
		if ( (mkdir(p, 0755)) < 0 )
		{
			sprintf(buf, "could not create directory\n" );
			write(ns, buf, 128);
			printf("%s", buf);
			return;
		}
		sprintf(buf, "successfully created directory\n");
		write(ns, buf, 128);
		printf("%s", buf);
	}
\

}

main(int argc, char *argv[], char *env[])
{
	char *hostname;
	char line[MAX];
	char nl[MAX];
	char *end;
	int num1, num2;
	int pid, cmd;
	//int i = 0;

	if (argc < 2)
		hostname = "localhost";
	else
		hostname = argv[1];

	server_init(hostname);

	getcwd(cwd, MAX);
	strcpy(home, cwd);
	chroot(cwd);

	// Try to accept a client request
	while(1)
	{
		printf("server: accepting new connection ....\n");

		// Try to accept a client connection as descriptor newsock
		length = sizeof(client_addr);
		newsock = accept(sock, (struct sockaddr *)&client_addr, &length);
		// after newsock accepts client fork a new process for that client
		pid = fork();
		if ( pid == 0 )
		{
			if (newsock < 0){
				printf("server: accept error\n");
				exit(1);
			}
			printf("server: accepted a client connection from\n");
			printf("-----------------------------------------------\n");
			printf("        IP=%s  port=%d\n", inet_ntoa(client_addr.sin_addr.s_addr),
							ntohs(client_addr.sin_port));
			printf("-----------------------------------------------\n");

			// Processing loop
			while(1){
				getpath(env);
				/*while(strcmp(path[i],""))
				{
					printf("childpath:%s\n",path[i++]);
				}*/
				n = read(newsock, line, MAX);
				cmd = findcmd(line);
				mkcmd(line);
				switch(cmd)
				{
					case LS:
						printf("in ls\n");
						//runcmd(env, nl);
						ls(newsock);
						printf("before break\n");
						break;
					case CD:
						printf("in cd\n");
						cd();
						break;
					case MKDIR:
						printf("in mkdir\n");
						printf("line %s\n", line);
						printf("myargv:%s\n", myargv[1]);
						md(newsock);
						break;
					case RMDIR:
						printf("in rmdir\n");
						rd(newsock);
						break;
					case RM:
						printf("in rm\n");
						rf(newsock);
						break;
					case GET:
						printf("in get\n");
						get(newsock);
						break;
					case PUT:
						printf("in put\n");
						put(newsock);
						break;
					case PWD:
						pwd(newsock);
						break;
					case QUIT:
						printf("in quit\n");
						//exit(1); // does not work freezes program
						// server needs to kill the client
						break;
					case ERROR:
						printf("Command %s not recognized\n", line);
						break;
				}
				//printf("before if\n");
				if (n==0){
					printf("server: client died, server loops\n");
					close(newsock);
					break;
				}

			// show the line string
			printf("server: read  n=%d bytes; line=[%s]\n", n, line);

			//strcat(line, " ECHO");

			/*num1 = strtol(line, &end, 10);
			printf("num1: %d\n", num1);
			num2 = strtol(end, NULL, 10);
			printf("num2: %d\n", num2);

			num1 += num2;
			printf("after add num1: %d\n", num1);*/

			//sprintf(line, "%d", num1);

			// send the echo line to client
			//n = write(newsock, line, MAX);

			//printf("line: %s\n", line);

			//printf("server: wrote n=%d bytes ECHO=[%s]\n", sizeof(nl), nl);
			printf("server: ready for next request\n");
			} // end process loop
		} // end if
	}
}
