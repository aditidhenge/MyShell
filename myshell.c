/********************************************************************************************
This is a template for assignment on writing a custom Shell.

Students may change the return types and arguments of the functions given in this template,
but do not change the names of these functions.

Though use of any extra functions is not recommended, students may use new functions if they need to,
but that should not make code unnecessorily complex to read.

Students should keep names of declared variable (and any new functions) self explanatory,
and add proper comments for every logical step.

Students need to be careful while forking a new process (no unnecessory process creations)
or while inserting the single handler code (should be added at the correct places).

Finally, keep your filename as myshell.c, do not change this name (not even myshell.cpp,
as you not need to use any features for this assignment that are supported by C++ but not by C).
*********************************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>			// exit()
#include <unistd.h>			// fork(), getpid(), exec()
#include <sys/wait.h>		// wait()
#include <signal.h>			// signal()
#include <fcntl.h>			// close(), open()
#define SIZE 100000

void parseInput(char *input, char **parsedArray, char *delimeter)
{
	// This function will parse the input string into multiple commands or a single command with arguments depending on the delimiter (&&, ##, >, or spaces).
	char *tempInput = strdup(input);   //duplicates the string, allocates memory and stores the string in it
	for(int i=0 ; i<SIZE ; i++)
	{
		parsedArray[i] = strsep(&tempInput,delimeter);  //parse a string into tokens
		if(parsedArray[i] == NULL)
			break;
		if(strlen(parsedArray[i]) == 0)
			i--;
	}
	free(tempInput);
}

void executeCommand(char **parsedArray)
{
	// This function will fork a new process to execute a command
	if(strcmp(parsedArray[0],"cd") == 0)
	{
		if(parsedArray[1] == NULL)
		{
			printf("Shell: Incorrect command\n");
			return;
		}
		chdir(parsedArray[1]);     //changes the current working directory of the calling process to the directory specified in the path
	}
	else
        {
            // create a new child process for execution of given command
            	pid_t pid = fork();
            	if(pid < 0)
            	{
            		exit(0);
            	}
		if (pid == 0) {
			// This is the child process.
			execvp(parsedArray[0], parsedArray);
			printf("Shell: Incorrect command\n");
			exit(1);
		} else {
			// This is the parent process.
			waitpid(pid, NULL, 0);
		}
        }
}

void executeSequentialCommands(char **parsedArray)
{
	// This function will run multiple commands in parallel
	for(int i=0 ; parsedArray[i] != NULL ; i++)
	{
		char *parsed[SIZE];
		parseInput(parsedArray[i],parsed," ");

		executeCommand(parsed);
	}
}

void executeParallelCommands(char **parsedArray)
{
	// This function will run multiple commands in parallel
	//here wo do not wait for the child process to complete its execution
	int i=0;
	pid_t child_pids[SIZE];
	for(i=0 ; parsedArray[i] != NULL ; i++)
	{
		char *parsed[SIZE];
		parseInput(parsedArray[i],parsed," ");

		if(strcmp(parsed[0],"cd") == 0)
		{
			if(parsed[1] == NULL)
			{
				printf("Shell: Incorrect command\n");
				return;
			}
			chdir(parsed[1]);
		}
		else
		{
		    // create a new child process for execution of given command
		    	pid_t pid = fork();
		    	if(pid < 0)
		    	{
		    		exit(0);
		    	}
			else if (pid == 0) {
				execvp(parsed[0], parsed);
				printf("Shell: Incorrect command\n");
				exit(1);
			}
			else
				child_pids[i] = pid;
		}
	}
	// Wait for all child processes to finish

	for(int j=0 ; j<i ; j++)
	{
		waitpid(child_pids[j], NULL, 0);
	}

}


void executeCommandRedirection(char **parsedArray)
{
	// This function will run a single command with output redirected to an output file specificed by user
	char *command[SIZE],*output_file[SIZE]; //array of strings
	parseInput(parsedArray[0],command," ");
	parseInput(parsedArray[1],output_file," ");
	pid_t pid = fork();

	if (pid < 0)
	{
		exit(0);
	}
	else if (pid == 0)
	{

		close(STDOUT_FILENO);				        //closing standard ouptu file
		open(output_file[0], O_CREAT | O_WRONLY | O_APPEND);	//opening a file in which the output should be redirected


		execvp(command[0], command);
		printf("Shell: Incorrect command\n");
		exit(0);
	}
	else
	{       // parent process waits until the child process gets terminated
		waitpid(pid, NULL, 0);
	}
}

int main()
{
	// Initial declarations
	char cwd[SIZE];
	size_t chars, size = 0;
	//four array of strings
	//1 - string passed by space , 2 - string passed by ## , 3 - string passed by && , 4 - string passed by > arrow
	char* parsedInput[SIZE],*parsedParallel[SIZE],*parsedSequential[SIZE],*parsedRedirection[SIZE];
	//handling the Ctrl+C and Ctrl+Z signals ignoring them so that it should not exit our shell
	signal(SIGINT, SIG_IGN);  // Ignore SIGINT signal
  	signal(SIGTSTP, SIG_IGN); // Ignore SIGTSTP signal

	while(1)	// This loop will keep your shell running until user exits.
	{
		// Print the prompt in format - currentWorkingDirectory$
		getcwd(cwd,sizeof(cwd));
		printf("%s$",cwd);
		// accept input with 'getline()'
		char* input = (char*)malloc(sizeof(char)*size);
		getline(&input,&size,stdin);

		int n = strlen(input);
		input[n-1] = '\0';
		// Parse input with 'strsep()' for different symbols (&&, ##, >) and for spaces.
		parseInput(input,parsedInput," ");
		parseInput(input,parsedParallel,"&&");
		parseInput(input,parsedSequential,"##");
		parseInput(input,parsedRedirection,">");

		if(strcmp(parsedInput[0],"exit") == 0)	// When user uses exit command.
		{
			printf("Exiting shell...\n");
			exit(0);
		}
		if(strcmp(parsedInput[0]," ") == 0)	// When user gives empty command
		{
			continue;
		}

		if(parsedParallel[1]!=NULL)          //means it has more than one element i.e && is present
			executeParallelCommands(parsedParallel);		// This function is invoked when user wants to run multiple commands in parallel (commands separated by &&)
		else if(parsedSequential[1]!=NULL)   //means it has more than one element i.e ## is present
			executeSequentialCommands(parsedSequential);	// This function is invoked when user wants to run multiple commands sequentially (commands separated by ##)
		else if(parsedRedirection[1]!=NULL)  //means it has more than one element i.e > is present
			executeCommandRedirection(parsedRedirection);	// This function is invoked when user wants redirect output of a single command to and output file specificed by user
		else
			executeCommand(parsedInput);		// This function is invoked when user wants to run a single commands

	}

	return 0;
}
