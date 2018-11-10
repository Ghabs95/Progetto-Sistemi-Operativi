#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>


#define MAX 100
#define SEQUENTIAL "-s"
#define PARALLEL "-p"

int strInput(char str[]);
void executeProcess(char *command, int fileNum);
char *myStrDup(char *str);
void getCommand(char* command, char* commandExec[MAX]);
void generateProcess(char *command, int fileNum);

int main(int argc, char **argv) {
	int fileNum = 0;
	char command[MAX];
	int status;

	/* rimuove tutti i file creati precedentemente con prefisso "out.", per non appesantire la directory */
	system("rm -f out.*");

	if (argc > 1 && !strcmp(argv[1], SEQUENTIAL)) {
		printf("Command Executor: SEQUENTIAL, insert an empty string for exit\n");
		while (strInput(command) > 0) {
			fileNum++;
			generateProcess(command, fileNum);
			wait(&status);
		}
	} else if (argc > 1 && !strcmp(argv[1], PARALLEL)) {
		printf("Command Executor: PARALLEL, insert an empty string for exit\n");
		int i = 0, process = 0;
		char buffer[MAX][MAX];
		while (strInput(command) > 0) {
			memcpy(&buffer[i++], &command[0], sizeof(buffer[i]));
		}
		for (process = 0; process < i; process++) {
			generateProcess(buffer[process], ++fileNum);
		}
		wait(&status);
	} else {
		perror("call the program with -s or -p arguments");
	}

	printf("\nExit...\n");
	usleep(10000);
	exit(0);
}

void generateProcess(char *command, int fileNum) {
	pid_t pid = fork();
	if (pid < 0) {
		fprintf(stderr, "Fork Failed");
		exit(-1);
	}
	if (pid == 0) { // child
		printf("Process %d, generate...\n", getpid());
		executeProcess(command, fileNum);
	}
}

int strInput(char str[]) {
	printf("Insert command/s:\n");
	fgets(str, MAX, stdin);

	str[strlen(str) - 1] = '\0';
	return strlen(str);
}

void executeProcess(char *command, int fileNum) {
	char *commandExec[MAX];
	getCommand(command, commandExec);

	char catedString[10];
	sprintf(catedString, "out.%d", fileNum);
	int fd = open(catedString, O_CREAT | O_RDWR | O_TRUNC, S_IRUSR | S_IWUSR);
	if (fd == -1) {
		perror("open error");
	}

	dup2(fd, STDOUT_FILENO);   	// make stdout go to file
	dup2(fd, STDERR_FILENO);	// make stderr go to file
	close(fd);
	if (execvp(commandExec[0], commandExec) < 0) {
		system(commandExec[0]);
		perror("Error exec");
		exit(-1);
	}
}

void getCommand(char* command, char* commandExec[MAX]) {
	int i = 0;
	char* str = strtok(command, " ");
	while (str != NULL) {
		commandExec[i++] = myStrDup(str);
		str = strtok(NULL, " ");
	}
	commandExec[i] = NULL;
}

char *myStrDup(char *str) {
	char *other = malloc(strlen(str) + 1);
	if (other != NULL) {
		strcpy(other, str);
	}
	return other;
}
