#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>

#define MAXPARAMETERS 20
#define MAXSTRINGDIMENSION 40

void choppy(char*);
void cancel_spaces_tabs(char*);
void execute_command(char*, char**, char**);

int main(int argc, char* argv[]){

	//files_path useful for searching command executables
	char* shell_path[MAXPARAMETERS];

	char* args[MAXPARAMETERS]; //array of  MAX PARAMETERS strings

	for(int i = 0; i < MAXPARAMETERS; i++){
		shell_path[i] = (char*)malloc(sizeof(char) * MAXSTRINGDIMENSION);	
		if(shell_path[i] == NULL){
			fprintf(stderr,"A malloc error is occured\n");
			exit(1);
		}		
	}
	strcpy(shell_path[0], "/bin/");
		

	//interactive mode
	if(argc == 1){
		while(1){
			printf("$> ");


			//string that rapresent the line with the command and all of the args
			char* line = NULL;
			//useful for getline method
			size_t len = 0;
			//number of character read on a line + \0"
			ssize_t nread; 

			//read all of the line and print the number of characters read + the \0 "delimitator character"
			nread = getline(&line, &len,stdin);
			if(nread == -1){
				fprintf(stderr,"an error is occured in getLine method\n");
			}
			execute_command(line,shell_path,args);
		}
	//batch mode	
	}else if(argc == 2){
		//commands from file

		char* line = NULL;
		size_t len = 0;
		FILE* file_pointer;
		file_pointer = fopen(argv[1],"r");
		while(getline(&line, &len,file_pointer) != -1){
			execute_command(line,shell_path,args);
		}
		fclose(file_pointer);
		
	//error too many arguments	
	}else if(argc > 2){
		fprintf(stderr,"Error, too many arguments\n");
		exit(1);
	}


	//freeing memory
	for(int i = 0; i < MAXPARAMETERS; i++){
		free(shell_path[i]);
		shell_path[i] = NULL;		
	}
	return 0;
}



void choppy(char *line ){
    int lastLetter = 0;
		for(int i = 0; i < strlen(line) - 1; i++){
			if(line[i] != '\n' && line[i] != ' ' && line[i] != '\0' && line[i] != '\t'){
				lastLetter = i;
			}
		}
	if(lastLetter == 0){ // means that the line has no commands
		line[lastLetter] = '\0';
	}else{
		line[lastLetter + 1] = '\0';
	}
	
}
void cancel_spaces_tabs(char *s){ 
    int i = 0, j = 0;
    while (s[i] == ' ' || s[i] == '\t') { 
        i++; 
    } 
    while (s[i] != '\0') { 
        s[j] = s[i]; 
		j++;
		i++;
    } 
    s[j] = '\0'; 
}


void execute_command(char* line, char* shell_path[MAXPARAMETERS], char* args[MAXPARAMETERS]){
	char error_message[30] = "An error has occurred\n";
	//allocate memory for the command and parameters
	for(int i = 0; i < MAXPARAMETERS; i++){
		args[i] = (char*)malloc(sizeof(char) * MAXSTRINGDIMENSION);
		if(args[i] == NULL){
			write(STDERR_FILENO, error_message, strlen(error_message));
			exit(1);
		}
	}


	int index = 0;
	//cancel the spaces or '\t' at the beginning of the line
	cancel_spaces_tabs(line);
	//we need to put a method to cancel spaces and tabs at the end of the line, TODO
	choppy(line);
	strcpy(args[0],strsep(&line," "));
	while(args[index] != NULL){
		index++;
		if(line == NULL){ //there is nothing to parse
			//freeing the memory
			for(int i = index; i < MAXPARAMETERS; i++){
				free(args[i]);
				args[i] = NULL;
			}
			break;
		}else{
			//cancel the spaces or '\t' at the beginning of the line for each parsed "word"
			cancel_spaces_tabs(line);
			strcpy(args[index],strsep(&line," "));
		}	
	}

	if(strcmp(args[0],"\0") == 0) return; //case where there is no command


	//control if there is > (a redirection)
	int output_redirection_type0 = 0;
	int output_redirection_type1 = 0;
	char* file = (char*) malloc(sizeof(char) * MAXSTRINGDIMENSION);
	index = 0;
	while(args[index] != NULL){
		if(strcmp(args[index], ">") == 0){ //are equal
			if(args[index + 1] != NULL ){
				if(args[index + 2] != NULL) break; // syntatical error
				strcpy(file,args[index + 1]);
				free(args[index]);
				free(args[index + 1]);
				args[index] = NULL;
				args[index + 1] = NULL;
			}
			output_redirection_type0 = 1;
			break;
		}else if(strcmp(args[index], ">>") == 0){
			if(args[index + 1] != NULL ){
				if(args[index + 2] != NULL) break; // syntatical error
				strcpy(file,args[index + 1]);
				free(args[index]);
				free(args[index + 1]);
				args[index] = NULL;
				args[index + 1] = NULL;
			}
			output_redirection_type1 = 1;
			break;
		}
		index++;
	}
			

	//the first 3 are built-in commands
	if(strcmp(args[0], "cd") == 0){
			if(args[1] == NULL || (args[1] != NULL && args[2] != NULL)){ //control of the input
				write(STDERR_FILENO, error_message, strlen(error_message));
			}else if(chdir(args[1]) != 0){
				write(STDERR_FILENO, error_message, strlen(error_message));
			}	

	}else if(strcmp(args[0], "path") == 0){

		int index = 1;
		while(args[index] != NULL){
			char* temp = (char*)malloc(sizeof(char) * MAXSTRINGDIMENSION);
			strcpy(temp,args[index]);
			free(shell_path[index]);
			shell_path[index] = strcat(temp, "/");
			temp = NULL;
			index++;
		}
		for(int i = index; i < MAXPARAMETERS; i++){
			shell_path[i] = NULL;
		}

	}else if(strcmp(args[0], "exit") == 0){
		//control if exit has the right parameters
		if(args[1] != NULL){
			write(STDERR_FILENO, error_message, strlen(error_message));
		}else{
			exit(0);
		}
	}else{
		pid_t numFork = fork();
		if(numFork == 0){
			//we are in the child process
			char* commandPath = (char*)malloc(sizeof(char) * MAXSTRINGDIMENSION);
			int index = 0;
			int found = 0;
			while(shell_path[index] != NULL){
				strcpy(commandPath, shell_path[index]);
				strcat(commandPath, args[0]);
				if(access(commandPath, X_OK) == 0){
					strcpy(args[0], commandPath);
					found = 1;
					break;
				}
				index++;
			}
			if(found == 0){
				free(commandPath);
				printf("%s: command not found\n", args[0]);
				exit(0);
			}
			free(commandPath);

			if(output_redirection_type0 == 1){ //if there was a >
				close(1); //1 is the file descriptor associated with stdout
				open(file, O_RDWR | O_CREAT | O_TRUNC, S_IRWXU);
			}else if(output_redirection_type1 == 1){ //if there was a >>
				close(1);
				open(file, O_RDWR | O_CREAT | O_APPEND, S_IRWXU);
			}
			free(file);

			if(execv(args[0], args) == -1){
				write(STDERR_FILENO, error_message, strlen(error_message));
			}
			exit(0);
		}else if(numFork == -1){
			write(STDERR_FILENO, error_message, strlen(error_message));
		}
		wait(NULL);
	}		
			
			
			
	for(int i = 0; i < MAXPARAMETERS; i++){
		free(args[i]);
		args[i] = NULL;
	}


}