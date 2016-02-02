#include "theader.c"

#define NUM_CMDS 11

NODE *root, *cwd;
char line[128];
char command[16], pathname[64];
char dirname[64], basename[64];
char *commandList[11];


main()
{
    initilize();

    while(1)
    {
        int id;

		// Read line from user, and get the function id from it
        GetLine();
        id = FindCommand();

        switch(id)
        {
            case 0:
                mkdir(pathname);
                break;
            case 1:
                rmdir(pathname);
                break;
            case 2:
                cd(pathname);
                break;
            case 3:
                ls();
                break;
            case 4:
                pwd();
                break;
            case 5:
                creat(pathname);
                break;
            case 6:
                rm(pathname);
                break;
            case 7:
                save();
                break;
            case 8:
                reload();
                break;
            case 9:
                menu();
                break;
            case 10:
                quit();
                break;
            default:
                printf("Command not recognized! Please try again.\n\n");
                break;
        }
    }




	return 0;
}
