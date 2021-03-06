#include "lab3.h"

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// STATIC FUNCTIONS
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void decomposePATH ()
{
    char* pathTmp = NULL;
    char* token = NULL;
    Path_t* tptr = NULL;
    Path_t* nptr = NULL;
    char* name = NULL;

    fprintf(stderr, "Decomposing PATH...\n");
    if (PATH)
    {
        pathTmp = (char*) malloc(strlen(PATH) + 1);
        strcpy(pathTmp, PATH);

        token = strtok(pathTmp, ":");
        fprintf(stderr, "%s\n", token);

        nptr = (Path_t*) malloc(sizeof(Path_t));
        nptr->name = (char*) malloc(strlen(token));
        strcpy(nptr->name, token);
        nptr->next = NULL;

        pathList = tptr = nptr;

        while (1)
        {
            token = strtok(NULL, ":");
            if (!token) { break; }
            fprintf(stderr, "%s\n", token);

            nptr = (Path_t*) malloc(sizeof(Path_t));
            nptr->name = (char*) malloc(strlen(token));
            strcpy(nptr->name, token);
            nptr->next = NULL;
            tptr->next = nptr;
            tptr = nptr;
        }

        fprintf(stderr, "[\033[032m  OK  \033[0m] PATH environment variable decomposed\n");
        free(pathTmp);
    }
    else
    {
        fprintf(stderr, "[\033[031m FAIL \033[0m] PATH environment variable decomposed\n");
        exit(-1);
    }
}



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// FUNCTION DEFINITIONS
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

extern Path_t* pathList;             /* Pointer to list of paths from PATH ENV */
extern char*   PATH;                 /* Pointer to PATH ENV variable */
extern char*   HOME;                 /* Pointer to HOME ENV variable */
extern char*   LOGNAME;              /* Pointer to LOGNAME ENV variable */
extern char    line[LINE_MAX];       /* Command user input */
extern char    command[64];          /* Command string */
extern char    head[512], tail[512]; /* Child process vars */

void __init__ (char** env)
{
    int i;
    int envLen = 0;  /* string length of entire ENV variables */

    system("clear");
    fprintf(stderr, "Initializing \033[033mBrown Yoda shell\033[0m...\n");

    PATH = HOME = LOGNAME = NULL;
    pathList = NULL;

    for (i = 0; NULL != env[i]; i++) { envLen += strlen(env[i]) + 1; }

    PATH = envz_get(*env, envLen, "PATH");
    if (PATH)
    {
        fprintf(stderr, "[\033[032m  OK  \033[0m] PATH environment variable exist\n");
        fprintf(stderr, "PATH=%s\n", PATH);
    }
    else
    {
        fprintf(stderr, "[\033[031m FAIL \033[0m] PATH environment variable exist\n");
        exit(-1);
    }

    HOME = envz_get(*env, envLen, "HOME");
    if (HOME)
    {

        fprintf(stderr, "[\033[032m  OK  \033[0m] HOME environment variable exist\n");
        fprintf(stderr, "HOME=%s\n", HOME);
    }
    else
    {
        fprintf(stderr, "[\033[031m FAIL \033[0m] HOME environment variable exist\n");
        exit(-1);
    }


    if (0 == chdir(HOME))
    {
        fprintf(stderr, "[\033[032m  OK  \033[0m] Change to current directory to HOME\n");
    }
    else
    {
        fprintf(stderr, "[\033[031m FAIL \033[0m] Change to current directory to HOME\n");
        exit(-1);
    }

    LOGNAME = envz_get(*env, envLen, "LOGNAME");
    if (LOGNAME)
    {

        fprintf(stderr, "[\033[032m  OK  \033[0m] LOGNAME environment variable exist\n");
        fprintf(stderr, "LOGNAME=%s\n", LOGNAME);
    }
    else
    {
        fprintf(stderr, "[\033[031m FAIL \033[0m] LOGNAME environment variable exist\n");
        exit(-1);
    }


    decomposePATH();

    fprintf(stderr, "\n[ Setup Completed ]\n");

    for (i = 3; i >= 0; i--)
    {
        fprintf(stderr, "%d\r", i);
        usleep(800000);
    }

}


/* Display software credits */
void credits ()
{
    time_t timer;
    char buffer[25];
    struct tm* tm_info;

    time(&timer);
    tm_info = localtime(&timer);

    system("clear");
    strftime(buffer, 25, "%Y-%m-%d %H:%M:%S", tm_info);
    printf("\033[033mBrown Yoda shell 1.0.0\033[0m \033[2m(%s, %s)\n\033[0m", LOGNAME, buffer);
}


 /* Custom tokenizer for redirection <, >, >> */
int redirectTok(char* string, char* cmdArgs, char* file)
{
    /*
     * 0 - no redirect filename
     * 1 - < takes input from file
     * 2 - > send out to outfile
     * 3 - >> append output to outfile
     * -1 - failed
     */
    int state = -1; // initial state is failed

    string = removePreSpace(string);

    // extract string before redirect symbol
    while ('\0' != *string)
    {
        if (('>' != *string) && ('<' != *string))
        {
            *cmdArgs = *string;
            cmdArgs++;
        }
        else { break; }
        string++;
    }

    *cmdArgs = '\0';
    cmdArgs = removePostSpace(cmdArgs);

    if ('\0' == *string) { return 0; }

    // check for redirect symbol
    if ('>' == *string)
    {
        string++;

        // >> append output to file
        if ('>' == *string) { state = 3; }
        // > send output to file
        else if (isspace(*string)) { state = 2; }
        // failed wrong format
        else { return -1; }
    }
    else if ('<' == *string)
    {
        string++;
        // < take input from file
        if (isspace(*string)) { state = 1; }
        // failed wrong format
        else { return -1; }
    }
    // failed wrong format
    else { return -1; }

    string++;

    // remove whitespace
    string = removePreSpace(string);

    // extract string after the redirect symbol
    while ('\0' != *string)
    {
        if (!isspace(*string))
        {
            *file = *string;
            file++;
        }
        else { break; }
        string++;
    }

    *file = '\0';

    return state;
}

/* Reverses any string */
char* reverseString(char* string)
{
    char temp;
    int start = 0;
    int end = strlen(string) - 1;

    while (start < end)
    {
        temp = string[start];
        string[start] = string[end];
        string[end] = temp;
        ++start;
        --end;
    }
}

/* Remove whitespaces before a string */
char* removePreSpace(char* string)
{
    // remove pre-whitespace
    while ('\0' != *string)
    {
        if (isspace(*string)) { string++; }
        else { return string; }
    }
}

/* Remove whitespaces after a string */
char* removePostSpace(char* string)
{
    char end = strlen(string) - 1;
    // remove post-whitespace
    while (isspace(string[end--]));
    string[end + 2] = '\0';
    return string;
}

/* Recursively execute commands in a pipe */
void executeCommand(char** env)
{
    char *token;                /* Pointer for tokens during toeknizing */
    char tmpString[LINE_MAX];   /* Temp string for tokenizing */
    char cmdPath[512];          /* Path + command concatenated string */
    char cmdArgs[512];          /* Command + arguments string */
    char doCommand[512];        /* Command to be executed */
    char redirectOutput[64];    /* Redirection output string */

    char **myargv;              /* Array of pointers for execve argv */
    char myargc = 0;            /* Counter for execve argv */

    int redirectState = 0;      /* Redirection type or not */
    int pid = 1;                /* Process ID for piping */
    int pd[2];                  /* Create pipe [0]=READ, [1]=WRITE */
    int success = 0;            /* Flag variable for command success */
    int hasPipe = 0;            /* Bool variable for doing pipe */

    Path_t* nptr = NULL;        /* Pointer to traverse path list */


    // Reverse string to start the commands from the end
    reverseString(head);
    token = strtok(head, "|");

    // Base case: if tail is null, return
    if (0 == strcmp(tail, "(null)")) return;

    // Reverses token string back to readable string, and
    // remove whitespaces and assign as tail
    reverseString(token);
    token = removePreSpace(token);
    token = removePreSpace(token);
    strcpy(tail, token);

    // Tokenize rest of the string and assign as head
    token = strtok(NULL, "");
    // If token is NULL, no string left, assign tail as head
    if (NULL == token)
    {
        hasPipe = 0;
        strcpy(head, tail);
        strcpy(tail, "(null)");
    }
    else
    {
        hasPipe = 1;
        reverseString(token);
        token = removePreSpace(token);
        token = removePreSpace(token);
        strcpy(head, token);
    }

    // Fork if pipe is used
    if (hasPipe)
    {
        pipe(pd);
        pid = fork();

        // Ensure the it was able to fork
        if (pid < 0)
        {
            fprintf(stderr, "Fork failed!");
            exit(-1);
        }
    }

    // Parent Proecess:
    // Execute command starting from tail, as Reader
    if (pid)
    {
        // Set reader if it's piping
        if (hasPipe)
        {
            close(pd[1]);
            close(0);
            dup(pd[0]);
        }

        fprintf(stderr, "%d scan: head=%s   tail=%s\n", getpid(), head, tail);

        if (0 != strcmp(tail, "(null)")) { strcpy(doCommand, tail); }
        else { strcpy(doCommand, head); }

        // Do Command
        fprintf(stderr, "%d do_command: line=%s\n", getpid(), doCommand);

        // Check if redirection symbols is used
        redirectState = redirectTok(doCommand, cmdArgs, redirectOutput);
        if (redirectState)
        {
            if (-1 == redirectState)
            {
                strcpy(cmdArgs, doCommand);
            }
            // < takes input from file
            else if (1 == redirectState)
            {
                close(0);
                open(redirectOutput, O_RDONLY);
                fprintf(stderr, "%d redirect input from %s\n", getpid(), redirectOutput);
            }
            // > send out to file
            else if (2 == redirectState)
            {
                close(1);
                open(redirectOutput, O_WRONLY | O_CREAT, 0644);
                fprintf(stderr, "%d redirect output to %s\n", getpid(), redirectOutput);
            }
            // >> append output to outfile
            else if (3 == redirectState)
            {
                close(1);
                open(redirectOutput, O_RDWR | O_APPEND | O_CREAT, 0644);
                fprintf(stderr, "%d redirect output to %s\n", getpid(), redirectOutput);
            }
        }

        // Extract command
        strcpy(tmpString, cmdArgs);
        token = strtok(tmpString, " ");
        strcpy(command, token);

        // Extract arguments:
        // Add command and arguments to myargv and
        // ensure that last item is NULL pointer
        myargc = 2;
        myargv = calloc(myargc, sizeof(char*));
        myargv[0] = token;

        token = strtok(NULL, " ");
        myargv[1] = token;

        while (NULL != token)
        {
            token = strtok(NULL, " ");
            myargc++;
            myargv = realloc(myargv, sizeof(char*) * myargc);
            myargv[myargc - 1] = token;
        }

        // Try every path list if command executes successfully
        fprintf(stderr, "%d tries %s in each PATH dir\n", getpid(), command);
        nptr = pathList;
        success = 0;
        while (NULL != nptr)
        {
            strcpy(cmdPath, "");
            strcat(cmdPath, nptr->name);
            strcat(cmdPath, "/");
            strcat(cmdPath, command);
            fprintf(stderr, "cmd=%s\n", cmdPath);
            if (-1 != execve(cmdPath, myargv, env))
            {
                success = 1;
                break;
            }
            nptr = nptr->next;
        }

        if (!success)
        {
            fprintf(stderr, "\033[031minvalid command %s\033[0m\n", command);
        }
    }
    // Child process:
    // Recursively call next command in the head until one command left
    // and child process serves as the Writer for the piping
    else
    {
        close(pd[0]);
        close(1);
        dup(pd[1]);

        executeCommand(env);
    }
}
