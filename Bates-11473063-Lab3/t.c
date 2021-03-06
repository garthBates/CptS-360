#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

int main (int argc, char * argv[], char * env[])
{                                                
   char user_input[1000], *pipe_cmd[50] = {NULL}, *constant_input = "GBshell>";

   printf("Type '-help' for help. \n");
   printf("%s", constant_input);
   while (1)
   {                                   
      fgets ( user_input, 1000, stdin);             
      user_input [ strlen ( user_input) - 1] = '\0';
                                                    
      // Split into pipe_commands terminated by NULL
      pipe_cmd[0] = strtok ( user_input, "|");                              
      for(auto index = 1; pipe_cmd[index] = strtok( NULL, "|"); index++);  
      determine_command(pipe_cmd, env);
      memset(user_input, 0, sizeof(user_input));
      printf("%s", constant_input);
   }       

   return 0;
}


void look_for_path(char * source, char * destination)
{
	char * path = getenv("PATH");
	char * check = strtok(path, ":");
	while(check)
	{
		strcpy(destination, check);
		strcat(destination, "/");
		strcat(destination, source);
		if(access(destination, F_OK) != -1)
			return;
		check = strtok(NULL, ":");
	}		
}

void execute_process(char * args[], char * env[])
{	
	int pid = fork();

	if(pid)
	{
		int status;
		waitpid(pid, &status, 0);
		fprintf(stderr, "Exit status of %s: %d\n", args[0], WEXITSTATUS(status));
	}
	else
	{
		char fullpath[100];
		look_for_path(args[0], fullpath);
		execve(fullpath, args, env);
	}
}

void determine_command_helper(char* cmd[], char * env[])
{
	char* root[20];
	//redirect the current root if 
	{
		char * cur = cmd[0];
		int size = strlen(cur);
		for(int i = 0; i < size; i++)
			if(cur[i] == '>')
			{
				if(i < size - 1 && cur[i + 1] == '>')
				{
					cur[i] = cur[i + 1] = '\0';
					char * file = strtok(cur + i + 2, " ");
               close(1);
					open(file, O_WRONLY|O_APPEND|O_CREAT);
				}
				else
				{
					cur[i] = '\0';
					char * file = strtok(cur + i + 1, " ");
					fclose(fopen(file, "w"));
					close(1);
					open(file, O_WRONLY|O_CREAT, 0644);
						
				}
			}
			else if(cur[i] == '<')
			{
				cur[i] = '\0';
				char * file = strtok(cur + i + 1, " ");
				close(0);
				open(file, O_RDONLY);
			}
	}
	root[0] = strtok(cmd[0], " ");
	for(int i = 1; root[i] = strtok(NULL, " "); i++);
	
	char ** child = cmd + 1;
	//base case. just run root
	if(child[0] == NULL)
	{
		execute_process(root, env);
		exit(1);
	}
	else
	{
		//make a pipe between root and child
		int pd[2];
		pipe(pd);
		int pid = fork();
		if(pid < 0)
		{
			exit(1);
		}
		else if(pid)
		{
			close(STDOUT_FILENO);
			dup(pd[1]);
			close(pd[0]);
			close(pd[1]);
			execute_process(root, env);
			close(STDOUT_FILENO);
			waitpid(pid, NULL, 0);
			exit(1);
		}
		else
		{
			close(STDIN_FILENO);
			dup(pd[0]);
			close(pd[1]);
			close(pd[0]);
			determine_command_helper(child, env);
		}
	}
}

void determine_command(char *cmd[], char * env[])
{
	//handlers here for cd and exit
	char copy[100];
	strcpy(copy, cmd[0]);
	char * tokin = strtok(copy, " ");
	if(strcmp(tokin, "cd") == 0)
	{
		tokin = strtok(NULL, " <>");
		if(!tokin || strcmp(tokin, "") == 0)
			tokin = getenv("HOME");
		chdir(tokin);
	}
	else if(strcmp(tokin, "exit") == 0)
	{
		exit(1);
	}
	else
	{
		int pid = fork();
		if(pid < 0)
		{
			perror("Could not fork child. Exiting\n");
			exit(1);
		}
		else if(pid)
		{
			int status;
			pid = wait(&status);
		}
		else
		{
			determine_command_helper(cmd, env);
			printf("End run cmd helper");
		}
	}
}
