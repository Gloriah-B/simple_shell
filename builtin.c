#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdbool.h>

#define MAX_COMMAND_LENGTH 100

void display_prompt(void);
void execute_command(char *command);

/**
 * is_interactive - Check if the shell is running in interactive mode.
 *
 * Return:
 * true if running interactively, false otherwise.
 */
bool is_interactive(void)
{
	return (isatty(STDIN_FILENO));
}

/**
 * main - Entry point of the shell.
 *
 * Description:
 * This function serves as the entry point for the simple UNIX command.
 * Return: EXIT_SUCCESS on success, EXIT_FAILURE on failure.
 */

int main(void)
{
	bool interactive = is_interactive();
	bool should_print_prompt = true;

	while (1)
	{
		char command[MAX_COMMAND_LENGTH];
		size_t length;

		if (interactive && should_print_prompt)
		{
			display_prompt();
		}

		if (fgets(command, sizeof(command), stdin) == NULL)
		{
			break;
		}

		length = strlen(command);

		if (length > 0 && command[length - 1] == '\n')
		{
			command[length - 1] = '\0';
		}

		if (strcmp(command, "exit") == 0)
		{
			break;
		}

		if (strcmp(command, "env") == 0)
		{
			char *env_var = getenv("PATH");

			if (env_var != NULL)
			{
				if (interactive)
				{
					write(STDOUT_FILENO, env_var, strlen(env_var));
					write(STDOUT_FILENO, "\n", 1);
				}
			}
			should_print_prompt = true;
			continue;
		}
		if (interactive || command[0] != '\0')
		{
			execute_command(command);

			if (interactive)
			{
				write(STDOUT_FILENO, "($)\n", 5);
				should_print_prompt = true;
			}
			else
			{
				should_print_prompt = false;
			}
		}
	}

	return (EXIT_SUCCESS);
}

/**
 * display_prompt - Display the shell prompt
 */
void display_prompt(void)
{
	write(STDOUT_FILENO, "Gloriah_shell$ ", 15);
	fflush(stdout);
}

/**
 * execute_command - Execute the specified command
 * @command: The command to be executed
 *
 * Description:
 * This function forks a child process to execute a command.
 * It tokenizes the command and searches for the executable
 * in the current directory and directories specified in the PATH.
 * If the executable is found, it is executed using execve.
 * If the executable is not found, an error message is printed.
 */
void execute_command(char *command)
{
	pid_t pid = fork();

	if (pid == -1)
	{
		perror("fork");
		exit(EXIT_FAILURE);
	}

	if (pid == 0)
	{
		/* Child process code */

		char *token;
		char *args[MAX_COMMAND_LENGTH];
		int i;
		char *path_copy;
		char *dir;
		char *path;

		/* Tokenize the command */
		token = strtok(command, " ");
		i = 0;

		while (token != NULL && i < MAX_COMMAND_LENGTH - 1)
		{
			args[i++] = token;
			token = strtok(NULL, " ");
		}

		args[i] = NULL;

		/* Check if the command is "exit" */
		if (strcmp(args[0], "exit") == 0)
		{
			write(STDOUT_FILENO, " Exit \n", 6);
			exit(EXIT_SUCCESS);
		}

		/* Check if the executable is found in the current directory */
		if (access(args[0], X_OK) == 0)
		{
			execve(args[0], args, NULL);
			/* If execve fails */
			perror("execve");
			exit(EXIT_FAILURE);
		}

		path = getenv("PATH");

		/* Check if PATH environment variable is set */
		if (path == NULL)
		{
			write(STDERR_FILENO, "PATH environment variable not set,\n", 36);
			exit(EXIT_FAILURE);
		}

		/* Create a copy of PATH for tokenization */
		path_copy = strdup(path);
		dir = strtok(path_copy, ":");

		/* Iterate through each directory in PATH */
		while (dir != NULL)
		{
			char executable_path[MAX_COMMAND_LENGTH];

			/* Construct the full path of the executable */
			snprintf(executable_path, sizeof(executable_path), "%s/%s", dir, args[0]);

			/* Check if the executable is accessible */
			if (access(executable_path, X_OK) == 0)
			{
				execve(executable_path, args, NULL);
				/* If execve fails */
				perror("execve");
				exit(EXIT_FAILURE);
			}

			dir = strtok(NULL, ":");
		}

		/* Cleanup and handle command not found */
		free(path_copy);
		write(STDERR_FILENO, "Command not found: ", 20);
		write(STDERR_FILENO, args[0], strlen(args[0]));
		write(STDERR_FILENO, "\n", 1);
		exit(EXIT_FAILURE);
	}
	else
	{
		/* Parent process code */

		int status;

		/* Wait for the child process to complete */
		if (waitpid(pid, &status, 0) == -1)
		{
			perror("waitpid");
			exit(EXIT_FAILURE);
		}

		/* Optionally, you can check the exit status of the child process */
		if (WIFEXITED(status))
		{
			/* int exit_code = WEXITSTATUS(status);  // Not used in this version */
		}
	}
}

