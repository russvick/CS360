#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#define DEC 10
#define OCT 8
#define HEX 16

typedef unsigned int u32;

char *table = "0123456789ABCDEF";

void mymain(int argc, char *argv[ ], char *env[ ]);
void myprintf(char *fmt, ...);

int prints(char *x);
int printu(u32 x);
int printd(int x);
int printo(u32 x);
int printx(u32 x);

int dec(u32 x);
int oct(u32 x);
int hex(u32 x);

void mymain(int argc, char *argv[ ], char *env[ ])
{
    int i;

    myprintf("in mymain(): argc=%d\n", argc);
    
    // We print the argv strings here
    for (i=0; i < argc; i++)
    {
        myprintf("argv[%d] = %s\n", i, argv[i]);
    }

    // We print the environment variable strings here
    myprintf("-------- env strings ----------\n");
    i = 0;
    while(env[i] != NULL)
    {
        myprintf("env[%d] = %s\n", i, env[i]);
        i++;
    }

    myprintf("---------- testing YOUR myprintf() ---------\n");
    myprintf("this is a test\n");
    myprintf("testing a=%d b=%x c=%c s=%s\n", 123, 123, 'a', "testing");
    myprintf("string=%s, a=%d  b=%u  c=%o  d=%x\n",
             "testing string", -1024, 1024, 1024, 1024);
    myprintf("mymain() return to main() in assembly\n"); 
}

void myprintf(char *fmt, ...)
{
    int i = 0;
    int *ip; //Variable used to point to next instruction
 
    int retPC;
    int *ebp; //Base pointer

    ebp = (int *)get_ebp(); //Base pointer
    ip = ebp; 
    retPC = *(ip++); //point return pointer to return address of the stack frame.
     
    ip += 2;// Must iterate by 2 bytes to make the IP point to the first argument [EBP+12].

    i = 0;
    // We loop through the string entirely
    while(fmt[i] != '\0')
    {
        if(fmt[i] == '\n')  // Case where character is a newline
        {
            putchar('\n');
            putchar('\r');
        }
        else if(fmt[i] == '%')  // User wants to print either a char, string, int, etc.
        {
            char *cPtr = NULL;
            switch(fmt[++i]) 
            {
                case 's':
                    cPtr = (char *)*ip; 
                    prints(cPtr);
                    break;
                case 'u':
                    printu(*ip);    
                    break;
                case 'd':
                    printd(*ip);
                    break;
                case 'o':
                    printo(*ip);
                    break;
                case 'x':
                    printx(*ip);
                    break;
                case 'c':
                    putchar(*ip);
                    break;
                default:
                    break;
            }
            ip++; //shift ip over 1 byte so it stays consistent with iterations of the variable (i)
        }
        else
        {
            putchar(fmt[i]); // Regular character
        }
        i++;
    }    
    return;
}


int printu(u32 x)
{
    if (x==0)
        putchar('0');
    else
        dec(x);
}

int dec(u32 x)
{
    char temp;
    if (x)
    {
        temp = table[x % DEC];
        dec(x / DEC);
        putchar(temp);
    } 
}

int printo(u32 x)
{
    if (x==0)
        putchar('0');
    else
        octo(x);
}

int octo(u32 x)
{
    char temp;
    if (x)
    {
        temp = table[x % OCT];
        octo(x / OCT);
        putchar(temp);
    } 
}

int printd(int x)
{
    if (x==0)
        putchar('0');
    else
    {
        if(x < 0)
        {
            putchar('-');
            // We change x to it's positive 
            x += (2 * x * -1);
        }	       
        dec(x);
    }
}

int printx(u32 x)
{
    if (x==0)
        putchar('0');
    else
        hex(x);
}

int hex(u32 x)
{
    char temp;
    if (x)
    {
        temp = table[x % HEX];
        hex(x / HEX);
        putchar(temp);
    } 
}

int prints(char *x)
{
    int i = 0;
    if (x==0)
        putchar('0');
    else
    {
        for(i = 0; i < strlen(x); i++)
        {
            putchar(x[i]);
            if(x[i] == '\n')
            {
                putchar('\r'); //Carriage return
            }
        }
    }
}
