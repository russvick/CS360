#include "type.h"
#include "lab7.h"

int main(int argc, char *argv[])
{
  char Line[BLOCK_SIZE], Command[64], Pathname[BLOCK_SIZE], Parameter[64];
  char* line;

  // open device into read write mode
  char* Disk = "bigdisk";

  Initilize();
  //Open up disk to be read
  fd = open(Disk, O_RDWR);


  mount_root();

  running = &proc[0];
  running->status = READY;
  running->cwd = (MINODE *)iget(fd, 2);

  // GetUserInput(Line, Command, Pathname, Parameter);
  //
  // if (!strcmp(Command, "ls"))
  // {
  //     ls(Pathname);
  // }

  return 0;
}
