#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

#define MAX 10000
typedef struct {
    char *name;
    char *value;
} ENTRY;

ENTRY entry[MAX];

main(int argc, char *argv[])
{
  int i, m, r;
  char cwd[128], buf[128];
  char c;
  FILE *file, *file2, *fp;
  DIR *dir;
  struct dirent* d;

  m = getinputs();    // get user inputs name=value into entry[ ]                                                                               
  getcwd(cwd, 128);   // get CWD pathname                                                                                                       

  printf("Content-type: text/html\n\n");
  printf("<p>pid=%d uid=%d cwd=%s\n", getpid(), getuid(), cwd);

  printf("<H1>Echo Your Inputs</H1>");
  printf("You submitted the following name/value pairs:<p>");

  for(i=0; i <= m; i++)
     printf("%s = %s<p>", entry[i].name, entry[i].value);
  printf("<p>");
/*****************************************************************                                                                            
   Write YOUR C code here to processs the command                                                                                               
         mkdir dirname                                                                                                                          
         rmdir dirname                                                                                                                          
         rm    filename                                                                                                                         
         cat   filename                                                                                                                         
         cp    file1 file2                                                                                                                      
         ls    [dirname] <== ls CWD if no dirname                                                                                               
  *****************************************************************/
  if(strcmp(entry[0].value, "mkdir")==0)
    {
      r = mkdir(entry[1].value, 0755);
      if(r ==0)
        printf("Successfully added: %s<br>",entry[1].value);
      else
        printf("Failed to add: %s<br>",entry[1].value);
    }
  else if(strcmp(entry[0].value, "rmdir")==0)
    {
      r = rmdir(entry[1].value);
      if(r ==0)
          printf("Successfully removed %s <br>",entry[1].value);
      else
        printf("Failed to remove %s <br>",entry[1].value);
    }
  else if(strcmp(entry[0].value, "rm")==0)
    {
      r = remove(entry[1].value);
      if(r == 0)
        printf("Successfully removed %s<br>",entry[1].value);
      else
        printf("Failed to remove %s<brb>",entry[1].value);
    }
  else if(strcmp(entry[0].value, "cat")==0)
    {
      fp = fopen(entry[1].value,"r");

      while((c = fgetc(fp)) != EOF)
       putchar(c);
      }
      printf("<br>");
      close(fp);
    }
  else if(strcmp(entry[0].value,"cp")==0)
    {
      file = fopen(entry[1].value,"r");
      file2 = fopen(entry[2].value, "w");
      while(fgets(buf,128,file) != NULL)
        {
          fprintf(file2, "%s",buf);
        }
      printf("Copy Successful<br>");
      close(file);
      close(file2);
    }
  else if(strcmp(entry[0].value,"ls")==0)
    {
      if(strcmp(entry[1].value, "") == 0)
	{
          dir = opendir(".");
          printf("Contents of:<br>");
        }

      else
        {
          dir = opendir(entry[1].value);
          printf("Contents of %s:<br>",entry[1].value);
        }
      if(dir)
	{
          while((d = readdir(dir))!=NULL)
            {
              printf("%s<br>",d->d_name);
            }
          closedir(dir);
          printf("<br>");
	}
    }
 // create a FORM webpage for user to submit again                                                                                             
  printf("</title>");
  printf("</head>");
  printf("<body bgcolor=\"#FF0000\" link=\"#330033\" leftmargin=8 topmargin=8");
  printf("<p>------------------ DO IT AGAIN ----------------\n");

  printf("<FORM METHOD=\"POST\" ACTION=\"http://cs360.eecs.wsu.edu/~vick/cgi-bin/mycgi\">");

  //------ NOTE : CHANGE ACTION to YOUR login name ----------------------------                                                                 
  //printf("<FORM METHOD=\"POST\" ACTION=\"http://cs360.eecs.wsu.edu/~YOURNAME/cgi-bin/mycgi\">");                                              

  printf("Enter command : <INPUT NAME=\"command\"> <P>");
  printf("Enter filename1: <INPUT NAME=\"filename1\"> <P>");
  printf("Enter filename2: <INPUT NAME=\"filename2\"> <P>");
  printf("Submit command: <INPUT TYPE=\"submit\" VALUE=\"Click to Submit\"><P>");
  printf("</form>");
  printf("------------------------------------------------<p>");

  printf("</body>");
  printf("</html>");
}
