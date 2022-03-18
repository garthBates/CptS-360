#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef struct Node {
	char name[64];
	char type;
	struct Node *childptr, *siblingptr, *parenteptr;
} Node;

typedef struct fileStack {
	struct fileStack *prev;
	char name[64];
}fileStack;

Node *root, *cwd, *start;
char line[128];
char command[16], pathname[64];
char dirName[64], baseName[64];
char *name[100];
FILE *fp;
char *cmd[] = { "mkdir", "rmdir", "ls", "cd", "pwd", "creat", "rm",
"reload", "save", "menu", "quit", NULL };

int tokenize(char *pathname) {
	char *s;
	s = strtok(pathname, "/"); //first call to strtok()
	while (s) {
		puts(s);
		s = strtok(0, "/"); //call strtok() until it returns NULL
	}
}

int findCmd(char *command) {
	int i = 0;
	while (cmd[i]) {
		//printf("command: %s cmd[%d]: %s\n", command, i, cmd[i]);
		if (!strcmp(command, cmd[i])) {
			return i; // found command: return index i
		}
		i = i + 1;
	}
	return -1;
}

void initialize(void) {
	root = (Node *)malloc(sizeof(Node));
	strcpy(root->name, "/");
	root->type = 'D';
	root->childptr = NULL;
	root->parenteptr = NULL;
	root->siblingptr = NULL;
	cwd = root;
}

int dividePath(char *path) {
	char temp[64]; //same size as pathname? 
	strcpy(temp, path);

	char *split = strtok(temp, "/");
	if ((*split) == NULL) {
		*dirName = '/';
	}
	while (split != NULL) {
		strcpy(baseName, split);
		split = strtok(NULL, "/");
	}

	if (strlen(path) - strlen(baseName) == 0) strncpy(dirName, path, strlen(path) - strlen(baseName));
	else strncpy(dirName, path, strlen(path) - strlen(baseName) - 1);
}

int preOrderTree(Node* node) {
	if (node == NULL) {
		return 0;
	}
	fprintf(fp, "%c\t", node->type);
	Node *temp = node;
	fileStack *top = malloc(sizeof(fileStack)), *pushptr;
	strcpy(top->name, temp->name);
	top->prev = NULL;

	while (temp->parenteptr != NULL) {
		temp = temp->parenteptr;
		if (temp != NULL) {
			pushptr = malloc(sizeof(fileStack));
			strcpy(pushptr->name, temp->name);
			pushptr->prev = top;
			top = pushptr;
		}
	}
	top = top->prev;
	while (top != NULL) {
		fprintf(fp, "/%s", top->name);
		top = top->prev;
	}
	fprintf(fp, "\n");

	if (node->childptr != NULL) {
		preOrderTree(node->childptr);
	}
	if (node->siblingptr != NULL) {
		preOrderTree(node->siblingptr);
	}
}

int mkdir(char path[]) {
	if (strcmp(path, "") == 0) {
		puts("No path name mkdir\n");
		return 0;
	}
	// dont need to add / when loading
	else if (strcmp(path, "/") == 0) {
		return 0;
	}

	Node *child, *parent;
	if (strcmp(path, "/") == 0) {
		parent = root;
	}
	else {
		parent = cwd;
	}

	dividePath(path);

	char *split = strtok(dirName, "/");

	while (split != NULL) {
		child = parent->childptr;
		while (1) {
			if (child == NULL) {
				puts("No path name of child exists.\n");
				return 0;
			}
			if (strcmp(child->name, split) == 0 && 'D' == child->type) {
				parent = child;
				break;
			}
			child = child->siblingptr;
		}
		split = strtok(NULL, "/");
	}

	Node *sibling = child = parent->childptr;
	Node *prevSibling;
	while (sibling != NULL) {
		if (strcmp(sibling->name, baseName) == 0) {
			puts("Path name already exists\n");
			return 0;
		}
		prevSibling = sibling;
		sibling = sibling->siblingptr;
	}

	if (child == NULL) {
		child = malloc(sizeof(Node));
		child->parenteptr = parent;
		strcpy(child->name, baseName);
		child->type = 'D';
		child->childptr = NULL;
		child->siblingptr = NULL;
		parent->childptr = child;
	}
	else {
		sibling = malloc(sizeof(Node));
		sibling->parenteptr = parent;
		strcpy(sibling->name, baseName);
		sibling->type = 'D';
		sibling->childptr = NULL;
		sibling->siblingptr = NULL;
		prevSibling->siblingptr = sibling;
	}
}

int rmdir(char *path) {
	if (strcmp(path, "/") == 0) {
		puts(" no path name rmdir\n");
		return 0;
	}

	if ((*dirName) == NULL) {
		dirName[0] = '/';
	}

	dividePath(path);
	puts(dirName);

	Node *child = NULL, *parent = NULL;
	if (dirName[0] == '/') {
		parent = root;
	}
	else {
		parent = cwd;
	}

  if(parent->type != 'D')
  {
    puts("Error! trying to delete a none directory type");
    return 0;
  }

	char *split = strtok(dirName, "/");
	while (split != NULL) {
		child = parent->childptr;
		while (1) {
			if (child == NULL) {
				puts("No path name of child exists.rmdir\n");
				return 0;
			}
			if (strcmp(child->name, split) == 0 && child->type == 'D') {
				parent = child;
				break;
			}
			child = child->siblingptr;
		}
		split = strtok(NULL, "/");
	}

	Node *sibling = child;
	child = parent->childptr;
	if (child != NULL) {
		if (strcmp(child->name, baseName) == 0) {
			if ('F' == child->type) { //file node check
				puts("Cannot delete file. Use rm to delete file. rmdir\n");
			}
			if (child->childptr == NULL) { //is directory empty check
				parent->childptr = child->siblingptr; //parents child becomes child's sibling
			}
			else {
				puts("Directory is not empty rmdir\n");
			}
			return 0;
		}
		else {
			while (sibling != NULL) {
				if (strcmp(sibling->name, baseName) == 0 && sibling->type == 'D') {
					if (sibling->childptr == NULL) {
						child->siblingptr = sibling->siblingptr;
					}
					else {
						puts("Directory is not empty rmdir\n");
					}
					return 0;
				}
				child = sibling;
				sibling = sibling->siblingptr;
			}
		}
	}
	puts(" Directory not found rmdir\n");
}

int cd() {
	if (strcmp(pathname, "") == 0 || strcmp(pathname, "/") == 0) {
		cwd = root;
		return 0;
	}

	Node *parent, *child;
	if (pathname[0] == '/') { //initizaling where the parent is
		parent = root;
	}
	else {
		parent = cwd;
	}
	child = parent->childptr;

	char *split = strtok(pathname, "/");
	while (split != NULL) {
		while (1) {
			if (child == NULL) {
				puts("No pathname exists cd\n");
				return 0;
			}
			if (strcmp(child->name, split) == 0 && child->type == 'D') {
				parent = child;
				break;
			}
			child = child->siblingptr;
		}
		child = parent->childptr;
		split = strtok(NULL, "/");
	}
	cwd = parent;
}

int ls() {
	Node *child = NULL, *parent = NULL;

	if ((strcmp(pathname, "") == 0) || pathname[0] != '/') {
		parent = cwd;
	}
	else {
		parent = root;
	}
	if (root->childptr == NULL) {
		puts("/");
		return 0;
	}

	char *split = strtok(pathname, "/");
	child = parent->childptr;

	while (split != NULL) {
		while (1) {
			if (child == NULL) {
				puts("No pathname exists ls\n");
				return 0;
			}
			if (strcmp(child->name, split) == 0) {
				parent = child;
				break;
			}
			child = child->siblingptr;
		}
		child = parent->childptr;
		split = strtok(NULL, "/");
	}
	child = parent->childptr;
	if (child == NULL) {
		puts("The child is NULL\n");
	}
	while (child != NULL)
	{
		fprintf(" %c\t%s\n", child->type, child->name);
		child = child->siblingptr;
	}
}

int pwd(pathname) {
	puts("   ");
	readpwd(cwd);
	puts("\n");
}
int readpwd(Node *node)
{
	if (strcmp(node->name, "/"))
	{
		readpwd(node->parenteptr);
		puts(node->name);
	}
	else puts(node->name);
}

int creat(char path[]) {
	if (strcmp(path, "") == 0) {
		puts(" No file name\n");
		return 0;
	}
	Node *child, *parent;
	if (strcmp(path, "/") == 0) {
		parent = root;
	}
	else {
		parent = cwd;
	}

	dividePath(path);

	char *split = strtok(dirName, "/");
	while (split != NULL) {
		child = parent->childptr;
		while (1) {
			if (child == NULL) {
				puts(" pathname does not exists\n");
				return 0;
			}
			if (strcmp(child->name, split) == 0 && child->type == 'D') {
				parent = child;
				break;
			}
			child = child->siblingptr;
		}
		split = strtok(NULL, "/");
	}
	child = parent->childptr;
	if (child == NULL) {
		child = malloc(sizeof(Node));
		strcpy(child->name, baseName);
		child->type = 'F';
		child->parenteptr = parent;
		child->childptr = NULL;
		child->siblingptr = NULL;
		parent->childptr = child;
		return 0;
	}
	Node *sibling;
	while (child != NULL) {
		if (strcmp(child->name, baseName) == 0) {
			puts("file already exists");
			return 0;
		}
		sibling = child;
		child = child->siblingptr;
	}
	//file node
	child = malloc(sizeof(Node));
	strcpy(child->name, baseName);
	child->type = 'F';
	child->parenteptr = parent;
	child->siblingptr = NULL;
	child->childptr = NULL;
	sibling->siblingptr = child;
}

int rm(char*path) {
	if (strcmp(path, "") == 0) {
		puts("no pathname found rm\n");
		return 0;
	}

	path;
	Node *temp_root = NULL, *temp_holder = NULL, *will_delete = NULL;
	dividePath(path);

	temp_root = (cwd);


	if (strcmp(temp_root->name, "/") == 0) {
		if (temp_root->childptr->siblingptr == NULL) {
			will_delete = temp_root;
			free(will_delete);
			root->childptr = NULL;
			root->siblingptr = NULL;
			root->parenteptr = NULL;
			strcpy(root->name, "/");
			return 0;
		}
		else {
			temp_root = root->childptr;
			while (strcmp(temp_root->name, baseName) != 0) {
				temp_root = temp_root->siblingptr;
			}
			will_delete = temp_root;
			temp_root->parenteptr->childptr = temp_root->siblingptr;
			if (temp_root->parenteptr == NULL) {
				strcpy(temp_root->parenteptr->name, "/");
			}
			else {
				temp_root->siblingptr->parenteptr = temp_root->parenteptr;
			}

			root = temp_root;
			free(will_delete);
			return 0;
		}
	}

	temp_root = temp_root->childptr;
	while (strcmp(temp_root->name, baseName) != 0) {
		temp_root = temp_root->siblingptr;
	}

	if (temp_root == NULL) {
		puts(" Error! Can't delete home.");
		return 0;
	}
  
  if(temp_root->type != 'F')
  {
    puts("Error! Can not remove a non-file type");
    return 0;
  }
	else if (temp_root->siblingptr == NULL) {
		will_delete = temp_root;
		temp_root->parenteptr->childptr = NULL;
	}
	else {
		will_delete = temp_root;
		temp_root->parenteptr->childptr = temp_root->siblingptr;
		temp_root->siblingptr->parenteptr = temp_root->parenteptr;
	}
	root = temp_root;
	free(will_delete);
}

int save(char filename[]) {
	if (strcmp(filename, "") == 0) {
		puts(" no file name. save\n");
		return 0;
	}


	printf(" saving %s ---\n", filename);
	fp = fopen(filename, "w+");

	if (fp == NULL) {
		puts("Could not open file save\n");
		return 0;
	}

	preOrderTree(root->childptr);
	fclose(fp);
}

int reload() {
	if (strcmp(pathname, "") == 0) {
		puts(" no file name specified relaod\n");
		return 0;
	}
	printf("reloadng %s  \n", pathname);
	fp = fopen(pathname, "r");

	if (fp == NULL) {
		puts(" could not open file reload\n");
		return 0;
	}

	char type, line[100], path[100];
	while (1) {
		fgets(line, 100, fp);
		if (feof(fp)) {
			break;
		}
		printf("%s\n", line);
		line[strlen(line)] = 0; line[strlen(line)-1] = 0;
		if (line != NULL || strcmp(line, "") != 0) {
			char *splt = strtok(line, "\t");
			type = *splt;
			splt = strtok(NULL, "\t");
			strcpy(path, splt);
			if (type == 'F') {
				creat(path);
			}
			if (type == 'D')
			{
				mkdir(path);
			}

		}
		resetGloablVars();
	}
	fclose(fp);
}

int menu() {
	printf("========================= Menu =========================\n");
	printf("    mkdir rmdir ls cd pwd creat rm save reload quit\n");
	printf("========================================================\n");
}

int quit() {
	char path[100];
	printf("Enter file name in order to quit: ");
	fgets(path, 100, stdin);
	path[strlen(path)] = NULL;
	path[strlen(path) - 1] = NULL;
	save(path);
	printf("Quiting complete\n");
	exit(0);
}

int resetGloablVars() {
	memset(dirName, 0, sizeof(dirName));
	memset(pathname, 0, sizeof(pathname));
	memset(baseName, 0, sizeof(baseName));
}

int(*fptr[])(char*) = { (int(*)())mkdir, rmdir, ls, cd, pwd, creat, rm, reload, save, menu, quit };


int main(int argc, char *argv[], char *env[]) {
	int index;
	int r;
	initialize();
	menu();
	while (1) {
		printf("Input a command ");
		*pathname = NULL;
		fgets(line, 128, stdin);
		//line[strlen(line)] = 0;
		sscanf(line, " %s %s", command, pathname);


		index = findCmd(command);
		//printf("index: %d\n", index);
		if (index != -1) {
			r = fptr[index](pathname);
		}
		else {
			printf("Invlaid command\n");
			resetGloablVars();
		}
	}
	return 0;
}
