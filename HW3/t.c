#include "theader.c"

#define NUM_CMDS 11

void initilize()
{
    extern NODE *root, *cwd;
    extern char *commandList[NUM_CMDS];

    // Initilize root
    root = (NODE *)malloc(sizeof(NODE));
    strcpy(root->name, "/");
    root->type = 'D';
    root->childPtr = NULL;
    root->siblingPtr = root;
    root->parentPtr = root;

    // Initilize Current Working Directory
    cwd = root;

    // Initilize Commands
    int i;
    for(i = 0; i < NUM_CMDS; i++)
    {
        commandList[i] = (char *)malloc(sizeof(char) * 8);
    }

    strcpy(commandList[0], "mkdir");
    strcpy(commandList[1], "rmdir");
    strcpy(commandList[2], "cd");
    strcpy(commandList[3], "ls");
    strcpy(commandList[4], "pwd");
    strcpy(commandList[5], "creat");
    strcpy(commandList[6], "rm");
    strcpy(commandList[7], "save");
    strcpy(commandList[8], "reload");
    strcpy(commandList[9], "menu");
    strcpy(commandList[10], "quit");
}

void GetLine()
{
    extern char line[128];
	  printf("Type 'menu' to see menu\n");
    printf("~ ");

    fgets(line, 64, stdin);
    TrimNewLine(line);
}

int FindCommand()
{
    extern char line[128];
    extern char command[64];
    extern char pathname[64];
    extern char *commandList[NUM_CMDS];
    char lineCopy[128];
    int i;

	// Reset command and pathnames
	strcpy(command, "");
	strcpy(pathname, "");


    // We have a command and a pathname
    // Make a copy of the line, so we don't lose it.
    strcpy(lineCopy, line);
    // Parse line for the command and the pathname
    strcpy(command, strtok(lineCopy, " "));

	if (StrContains(line, ' '))
	{
		strcpy(pathname, strtok(0, " "));
	}


    for(i = 0; i < NUM_CMDS; i++)
    {
        if(strcmp(command, commandList[i]) == 0)
        {
            break;
        }
    }


    return i;
}

void GetDirNameAndBaseName(char *pathname)
{
	int i, length, lastSlashIndex;
	extern char dirname[64], basename[64];
	extern NODE *cwd;
	char pathnameCopy[64];


	strcpy(pathnameCopy, pathname);

	if (StrContains(pathnameCopy, '/'))
	{
		length = strlen(pathnameCopy) - 1;

		for (i = length; i >= 0; i--)
		{
			if (pathnameCopy[i] == '/')
			{
				lastSlashIndex = i;
				break;
			}
		}

		pathnameCopy[lastSlashIndex] = '\0';

		if (lastSlashIndex == 0)
		{
			strcpy(dirname, "/");
		}
		else
		{
			strcpy(dirname, pathnameCopy);
		}

		strcpy(basename, (pathnameCopy + lastSlashIndex + 1));
	}
	else
	{
		strcpy(dirname, "");
		strcpy(basename, pathnameCopy);
	}

}

void GetSubDirectory(char *pathname)
{
	char *token;
	char pathnameCopy[64];

	strcpy(pathnameCopy, pathname);

	// Seperate cases for absolute vs relative paths
	if (pathname[0] == '/')
	{
		token = strtok(pathnameCopy, "/");
		if (strcmp(token, (pathname + 1)) == 0)
		{
			strcpy(pathname, "");
		}
		else
		{
			strcpy(pathname, pathname + strlen(token) + 2);
		}
	}
	else
	{
		token = strtok(pathnameCopy, "/");
		if (strcmp(token, pathname) == 0)
		{
			strcpy(pathname, "");
		}
		else
		{
			strcpy(pathname, pathname + strlen(token) + 1);
		}
	}

	return;
}

bool AlreadyExists(NODE *node, char type)
{
	NODE *temp;
	bool found = false;
	extern char basename[64];

	temp = node->childPtr;

	// Check all sibling pointers of the parent's childPtr to see if the node already exists
	while (temp != NULL)
	{
		if (strcmp(temp->name, basename) == 0)
		{
			found = true;
			break;
		}

		temp = temp->siblingPtr;
	}

	return found;
}

bool SearchAndCreate(NODE **node, char *dirname, char type)
{
	char *token;
	NODE *sibling = NULL;
	char dirnameCopy[64];
	extern char basename[64];

	strcpy(dirnameCopy, dirname);
	token = strtok(dirnameCopy, "/");

	// BASE CASES
	if ((token == NULL) && ((*node)->type == 'D')) // The correct directory exists, so we create the node and return true.
	{
		if (!AlreadyExists(*node, type))
		{
			NODE *temp;
			NODE *newNode = (NODE *)malloc(sizeof(NODE));
			strcpy(newNode->name, basename);
			newNode->type = type;
			newNode->childPtr = NULL;
			newNode->siblingPtr = NULL;
			newNode->parentPtr = (*node);

			if ((*node)->childPtr == NULL)
			{
				(*node)->childPtr = newNode;
			}
			else
			{
				temp = (*node)->childPtr;
				while (temp->siblingPtr != NULL)
				{
					temp = temp->siblingPtr;
				}

				temp->siblingPtr = newNode;
			}

			return true;
		}
		else
		{
			// Error: name for file/directory already exists
			PrintAlreadyExistsError(basename, type);
			return false;
		}

	}
	sibling = (*node)->childPtr;

	while (sibling != NULL)
	{
		if ((strcmp(sibling->name, token) == 0) && (sibling->type == 'D'))
		{
			// We change the dirnameCopy variable into the subdirectory
			GetSubDirectory(dirname);
			SearchAndCreate(&sibling, dirname, type);
			break;
		}
		else if ((strcmp(sibling->name, token) == 0) && (sibling->type == 'F'))
		{
			// Error: we have a name that matches, but it isn't a directory
			PrintNotDirectoryError(sibling->name);

			// We return false here.
			return false;
		}
		else
		{
			sibling = sibling->siblingPtr;
		}


	}

	// Error: Couldn't find directory
	// We get here if we have looped through all sibling nodes and
	//	none of them matched the next token in the directory
	if (sibling == NULL)
	{
		PrintDirectoryNotFoundError();
		return false;
	}
}

bool SearchAndSetCwd(NODE **node, char *dirname)
{
	char *token;
	NODE *sibling = NULL;
	char dirnameCopy[64];
	extern char basename[64];
	extern NODE *cwd;

	strcpy(dirnameCopy, dirname);
	token = strtok(dirnameCopy, "/");

	// BASE CASES
	if ((token == NULL) && ((*node)->type == 'D')) // The correct directory exists, so we set cwd and return true
	{
		cwd = (*node);
		return true;

	}
	else if ((strcmp(token, "..") == 0) && ((*node)->type == 'D'))
	{
		cwd = (*node)->parentPtr;
		return true;
	}

	sibling = (*node)->childPtr;

	while (sibling != NULL)
	{
		if ((strcmp(sibling->name, token) == 0) && (sibling->type == 'D'))
		{
			// We change the dirnameCopy variable into the subdirectory
			GetSubDirectory(dirnameCopy);
			SearchAndSetCwd(&sibling, dirnameCopy);
			break;
		}
		else if ((strcmp(sibling->name, token) == 0) && (sibling->type == 'F'))
		{
			// Error: we have a name that matches, but it isn't a directory
			PrintNotDirectoryError(sibling->name);

			// We return false here.
			return false;
		}
		else
		{
			sibling = sibling->siblingPtr;
		}
	}

	// Error: Couldn't find directory
	// We get here if we have looped through all sibling nodes and
	//	none of them matched the next token in the directory
	if (sibling == NULL)
	{
		PrintDirectoryNotFoundError();
		return false;
	}
}

bool SearchAndRmDir(NODE **node, char *pathname)
{
	NODE *child, *temp;
	char *token;
	char pathnameCopy[64];
	extern char basename[64];
	extern NODE *cwd;

	strcpy(pathnameCopy, pathname);
	token = strtok(pathnameCopy, "/");


	// BASE CASES
	if ((token == NULL) && ((*node)->type == 'D')) // We found the correct directory to delete
	{
		child = (*node)->childPtr;

		if (child == NULL) // child has to be null, otherwise we wouldn't be able to delete the directory
		{
			// Set temp to the oldest sibling of the node to delete
			temp = (*node)->parentPtr->childPtr;

			while (temp->siblingPtr != child)
			{
				temp = temp->siblingPtr;
			}

			if (temp == (*node)->parentPtr->childPtr)
			{
				(*node)->parentPtr->childPtr = NULL;
			}

			temp->siblingPtr = (*node)->siblingPtr;

			// Free all memory allocated to the child pointer
			free((*node));

			return true;
		}
		else
		{
			// We can't delete a directory with child nodes
			PrintDirectoryHasChildrenError((*node)->name);

			return false;
		}



	}

	temp = (*node)->childPtr;

	// Search the sibling pointers to find the next directory to do a recursive call on.
	while (temp != NULL)
	{
    // We found the directory to do a recursive call on
		if ((strcmp(temp->name, token) == 0) && (temp->type == 'D'))
		{
			GetSubDirectory(pathname);
			return SearchAndRmDir(&temp, pathname);

		}
		else if ((strcmp(temp->name, token) == 0) && (temp->type == 'F'))
		{
			// Error: we have a name that matches, but it isn't a directory
			PrintNotDirectoryError(temp->name);

			// We return false here.
			return false;
		}
		else
		{
			temp = temp->siblingPtr;
		}


	}



	// Error: Couldn't find directory
	// We get here if we have looped through all sibling nodes and
	//	none of them matched the next token in the directory
	if (temp == NULL)
	{
		PrintDirectoryNotFoundError();
		return false;
	}


}

bool SearchAndRm(NODE **node, char *pathname)
{
	NODE *child, *temp;
	char *token;
	char pathnameCopy[64];
	extern char basename[64];
	extern NODE *cwd;

	strcpy(pathnameCopy, pathname);
	token = strtok(pathnameCopy, "/");


	// BASE CASES
	if ((token == NULL) && ((*node)->type == 'F')) // We found the correct file to delete
	{
		// Set temp to the oldest sibling of the node to delete
		temp = (*node)->parentPtr->childPtr;

		while ((temp->siblingPtr != (*node)) && (temp->siblingPtr != NULL))
		{
			temp = temp->siblingPtr;
		}

		if (temp == (*node)->parentPtr->childPtr)
		{
			(*node)->parentPtr->childPtr = NULL;
		}

		temp->siblingPtr = (*node)->siblingPtr;

		// Free all memory allocated to the child pointer
		free((*node));

		return true;



	}
    else if((token == NULL) && ((*node)->type == 'D'))
    {
        PrintNotFileError((*node));
        return false;
    }

	temp = (*node)->childPtr;

	// Search the sibling pointers to find the next directory to do a recursive call on.
	while (temp != NULL)
	{
    // We found the directory or the file to do a recursive call on
    if (strcmp(temp->name, token) == 0)
		{
			GetSubDirectory(pathname);
			return SearchAndRm(&temp, pathname);

		}
		else
		{
			temp = temp->siblingPtr;
		}


	}



	// Error: Couldn't find directory
	// We get here if we have looped through all sibling nodes and
	//	none of them matched the next token in the directory
	if (temp == NULL)
	{
		PrintFileNotFoundError();
		return false;
	}
}


void mkdir(char *pathname)
{
	bool dirFound = false;
	extern NODE *root, *cwd;
	extern char dirname[64], basename[64];

	// Set the dirname and basename global variables, based off the pathname
	GetDirNameAndBaseName(pathname);

	// Search for the dirname node to see if non-existent
	if (dirname[0] == '/')
	{
		dirFound = SearchAndCreate(&root, dirname, 'D');
	}
	else
	{
		dirFound = SearchAndCreate(&cwd, dirname, 'D');
	}

	if (dirFound)
	{
		printf("--- mkdir completed successfully ---\n\n");
	}


    return;
}

void rmdir(char *pathname)
{
	bool dirFound = false;
	extern NODE *root, *cwd;
	extern char dirname[64], basename[64];

	// Set the dirname and basename global variables, based off the pathname
	GetDirNameAndBaseName(pathname);

	// Search for the dirname node. If found, then we remove it's child if
  // it is empty
	if (dirname[0] == '/')
	{
		dirFound = SearchAndRmDir(&root, pathname);
	}
	else
	{
		dirFound = SearchAndRmDir(&cwd, pathname);
	}

	if (dirFound)
	{
		printf("--- rmdir completed successfully ---\n\n");
	}


	return;
}

void cd(char *pathname)
{
	bool dirFound = false;
	extern NODE *root, *cwd;
	extern char dirname[64], basename[64];

	// Set the dirname and basename global variables, based off the pathname
	GetDirNameAndBaseName(pathname);

	// Search for the dirname node to see if non-existent
	if (dirname[0] == '/')
	{
		dirFound = SearchAndSetCwd(&root, pathname);
	}
	else
	{
		dirFound = SearchAndSetCwd(&cwd, pathname);
	}

	if (dirFound)
	{
		printf("--- cd completed successfully ---\n\n");
	}


	return;
}

void ls()
{
	extern NODE *cwd;
	NODE *temp;

	temp = cwd->childPtr;

	if (temp == NULL)
	{
		printf("(empty)\n");
	}
	else
	{
		while (temp != NULL)
		{
			printf("%c %s\n", temp->type, temp->name);
			temp = temp->siblingPtr;
		}
	}
	printf("\n");



	return;
}

void rpwd(NODE *node, char *directoryName)
{
	char *temp;
	char *directoryNameCopy;
	extern NODE *root;

	// Allocate space for temp and a copy of the directory name
	temp = (char *)malloc(sizeof(char)* 32);
	directoryNameCopy = (char *)malloc(sizeof(char)* 32);

	// Make the copy of the directory name
	strcpy(directoryNameCopy, directoryName);

	if (node == root)		// Case for the root node
	{
		// Adds a slash to beginning of directory name
		strcpy(directoryName, "/");
		strcat(directoryName, directoryNameCopy);

		free(temp);
		free(directoryNameCopy);
		return;
	}

	// We add nodes with the slash at the end, until the root node.
	// For example, dir1/ will be added to the beginning of subfldr/subsubfldr/
	strcpy(temp, node->name);
	// The pwd can end with a slash, it doesn't matter if it's there or not
	strcat(temp, "/");
	strcpy(directoryName, temp);
	strcat(directoryName, directoryNameCopy);

	rpwd(node->parentPtr, directoryName);


	free(temp);
	free(directoryNameCopy);
	return;


}


void pwd()
{
	extern NODE *cwd;
	char *directoryName;
	directoryName = (char *)malloc(sizeof(char)* 64);
	strcpy(directoryName, "\0");
	rpwd(cwd, directoryName);


	printf("pwd: %s\n\n", directoryName);

	free(directoryName);
	return;
}


void creat(char *pathname)
{
	bool dirFound = false;
	extern NODE *root, *cwd;
	extern char dirname[64], basename[64];

	// Set the dirname and basename global variables, based off the pathname
	GetDirNameAndBaseName(pathname);

	// Search for the dirname node to see if non-existent
	if (dirname[0] == '/')
	{
		dirFound = SearchAndCreate(&root, dirname, 'F');
	}
	else
	{
		dirFound = SearchAndCreate(&cwd, dirname, 'F');
	}

	if (dirFound)
	{
		printf("--- creat completed successfully ---\n\n");
	}


	return;
}

void rm(char *pathname)
{
	bool fileFound = false;
	extern NODE *root, *cwd;
	extern char dirname[64], basename[64];

	// Set the dirname and basename global variables, based off the pathname
	GetDirNameAndBaseName(pathname);

	// Search for the dirname node. If found, then we remove it's child if it
  // is empty
	if (dirname[0] == '/')
	{
		fileFound = SearchAndRm(&root, pathname);
	}
	else
	{
		fileFound = SearchAndRm(&cwd, pathname);
	}

	if (fileFound)
	{
		printf("--- rm completed successfully ---\n\n");
	}


	return;
}

void save()
{
	FILE *logFile = NULL;
	extern NODE *root;

	// Reset the file if it has any contents
	logFile = fopen("myfile", "w");
	fclose(logFile);
	// Open file into append mode, in which we append all the directories/files
  // to the end of the file
	logFile = fopen("myfile", "a");

	fprintf(logFile, "D\t/\n");

	rSave(root->childPtr, logFile, "/");


	fclose(logFile);

	printf("--- save completed successfully ---\n\n");

	return;
}

void rSave(NODE *node, FILE *logFile, char directory[128])
{
	char line[64];
	char nodeDirectory[64];
	extern NODE *root;

	// BASE CASE: node is null
	if (node == NULL)
	{
		return;
	}

	strcpy(nodeDirectory, directory);

	if (node != root && node != root->childPtr)
	{
		strcat(nodeDirectory, "/");
	}

	// concatenate on node name onto the directory name
	strcat(nodeDirectory, node->name);

	// Create string to log to file
	sprintf(line, "%c\t%s\n", node->type, nodeDirectory);

	// Write to file
	fprintf(logFile, "%s", line);

	// Recursive call to child
	rSave(node->childPtr, logFile, nodeDirectory);

	// Recursive call to sibling
	rSave(node->siblingPtr, logFile, nodeDirectory);

	return;

}

void reload()
{
	FILE *file;
	char filename[64];
	char line[64];
	char *token;

	printf("Type name of file to load from: ");
	scanf("%s", filename);
	// Open file into read mode
	file = fopen(filename, "r");

	if (file == NULL)
	{
		PrintCouldNotOpenFileError(filename);
		return;
	}

	while (fgets(line, 64, file) != NULL)
	{
		// We don't want to recreate the root node, as it's been already created
    // at the start of the program
		if (strcmp(line, "D\t/\n") == 0)
		{
			continue;
		}
		token = strtok(line, "\t");

		if (strcmp(token, "D") == 0)
		{
			printf("Creating directory ");
			token = strtok(NULL, "\t");
			TrimNewLine(token);
			printf("%s... \n", token);

			mkdir(token);
		}
		else
		{
			printf("Creating file ");
			token = strtok(NULL, "\t");
			TrimNewLine(token);
			printf("%s... \n", token);

			creat(token);
		}

	}

	// Get rid of newline characters sitting in stdin
	fflush(stdin);
	fclose(file);

	return;

}

void menu()
{
	printf("==================== MENU =====================\n");
	printf("mkdir rmdir ls cd pwd creat rm save reload quit\n");
	printf("===============================================\n\n");
}

void quit()
{
    save();
	printf("Bye!\n\n");
    exit(1);
}

void TrimNewLine(char *str)
{
    int endIndex = strlen(str) - 1;

    str[endIndex] = '\0';

    return;
}

bool StrContains(char *str, char character)
{
    int i;
    bool found = false;
    for(i = 0; i < strlen(str); i++)
    {
        if(str[i] == character)
        {
            found = true;
        }
    }

    return found;
}
