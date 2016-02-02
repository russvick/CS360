#include "theader.c"

void PrintAlreadyExistsError(char *name, char type)
{
	if (type == 'D')
	{
		printf("Error: %s directory already exists.\n\n", name);
	}
	else
	{
		printf("Error: %s file already exists.\n\n", name);
	}
	return;
}

void PrintDirectoryNotFoundError()
{
	extern char dirname[64];
	printf("Error: %s directory name doesn't exist.\n\n", dirname);
	return;
}

void PrintFileNotFoundError()
{
	extern char dirname[64];
	printf("Error: %s file name doesn't exist.\n\n", dirname);
	return;
}

void PrintNotDirectoryError(char *name)
{
	printf("Error: (%s) is a file, not a directory.\n\n", name);
	return;
}

void PrintNotFileError(char *name)
{
    printf("Error: (%s) is a directory, not a file.\n\n", name);
    return;
}

void PrintDirectoryHasChildrenError(char *name)
{
	printf("Error: Cannot delete %s directory, it has children.\n\n", name);
	return;
}

void PrintCouldNotOpenFileError(char *filename)
{
	printf("Error: Couldn't open file %s\n\n", filename);
	return;
}
