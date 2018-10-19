#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h> // the man page for the exec functions says it needs this library
#include <sys/types.h> // apparently i also need these 3 things for open()
#include <sys/stat.h>
#include <fcntl.h>


//signal handler for CTRL+C
//makes it very annoying to terminate the program if it gets stuck
void sigint_handler(int sig){
  char msg[] = "\ncaught sigint\n";
  write(1, msg, sizeof(msg));
  char msg2[] = "CS361 >";
  write(1, msg2, sizeof(msg2));
}


//signal handler for CTRL+Z
void sigstp_handler(int sig){
  char msg[] = "\ncaught sigstp\n";
  write(1, msg, sizeof(msg));
  char msg2[] = "CS361 >";
  write(1, msg2, sizeof(msg2));

}


// executes 1 command
void execArgs(char** parsed) 
{
    // Forking a child 
    int pid = fork();  
  
    if (pid == -1) { 
        printf("Couldn't fork child\n"); 
        return; 
    } 
    // have the child execute the command
    else if (pid == 0) { 
        if (execv(parsed[0], parsed) < 0)
            printf("Couldn't execute command\n");  
        exit(0); 
    } 
    // have the parent wait for the child to die
    else { 
        int status;
        wait(&status);
	printf("pid:%d status:%d\n", pid, WEXITSTATUS(status));
    } 
} 


// executes 2 commands and outputs the pid and exit status only after both children are dead
void exec2Args(char** parsed, char** parsed2) 
{ 
    // Forking a child 
    int pid = fork();
    int status;  
  
    if (pid == -1) { 
        printf("Couldn't fork child\n"); 
        return; 
    } 
    // have the child execute the command
    else if (pid == 0) { 
        if (execv(parsed[0], parsed) < 0)
            printf("Couldn't execute command\n");  
        exit(0); 
    } 
    // have the parent wait for the child to die
    else { 
        wait(&status);
    } 

    // Forking another child 
    int pid2 = fork();  
    int status2;

    if (pid2 == -1) { 
        printf("Couldn't fork child\n"); 
        return; 
    } 
    // have the child execute the command
    else if (pid2 == 0) { 
        if (execv(parsed2[0], parsed2) < 0)
            printf("Couldn't execute command\n");  
        exit(0); 
    } 
    // have the parent wait for the child to die
    else { 
        wait(&status2); 
    }
    
    printf("pid:%d status:%d\n", pid, WEXITSTATUS(status));
    printf("pid:%d status:%d\n", pid2, WEXITSTATUS(status2));
}


// executes 1 command and redirects the output to the given file
void execArgsRedirect(char** parsed, char* filename) 
{
    // Forking a child 
    int pid = fork();  
  
    if (pid == -1) { 
        printf("Couldn't fork child\n"); 
        return; 
    } 
    // have the child execute the command
    else if (pid == 0) { 
	// redirect the child's output from stdout to the given file
	int outputFile = open(filename, O_RDWR|O_CREAT, 00700);
	dup2(outputFile, 1);
	
        if (execv(parsed[0], parsed) < 0)
            printf("Couldn't execute command\n");  
        exit(0); 
    } 
    // have the parent wait for the child to die
    else { 
        int status;
        wait(&status);
	printf("pid:%d status:%d\n", pid, WEXITSTATUS(status));
    } 
}


// takes an array and turns it into a null terminated argument list
char** parseArgs(char* line){
  //break the line up into words
  char *word = strtok(line, " \n");

  //terminate the program if the user inputted "exit"
  if(strcmp(word, "exit") == 0)
    exit(0);

  char** parsedArgs = malloc(100 * sizeof(char*));

  //put all the words on the line into the argsarray
  int i = 0;
  while (word) {
    //copy a word to the arg array
    parsedArgs[i] = word;
    //get next word
    word = strtok(NULL, " \n");
    i = i + 1;
  }
    
  parsedArgs[i] = NULL;
  i++;
    
  return parsedArgs;
}


int main(){	
  signal(SIGINT, sigint_handler);
  signal(SIGTSTP, sigstp_handler);
	  
  while(1){
    //create some space for our strings
    char line[500];

    //print prompt
    printf("CS361 >");

    //read line from terminal
    fgets(line, 500, stdin);
    
    //check if the user inputted a semicolon and if they did, try running those 2 commands
    char* semicolonPtr = strchr(line, ';');
    if(semicolonPtr != NULL){
	//get the index of the ';' character in the line
	int position = line - semicolonPtr;
	position = position * -1;
	
	//NOTE: You M U S T copy onto the afterSemicolon array before you copy onto the beforeSemicolon array
	//I have absolutely no idea why but if you don't copy in this order,
	//beforeSemicolon[0] gets erased and you can't do anything about it

	//put everything after the semicolon into an array
        char afterSemicolon[strlen(line) - position];
	strncpy(afterSemicolon, &line[position+1], strlen(line));
	afterSemicolon[strlen(line) - position] = '\0';

	//put everything before the semicolon into an array
	char beforeSemicolon[position];
	strncpy(beforeSemicolon, line, position);
	
	//parse both arrays into argument lists
	char** afterSemicolonPARSED = parseArgs(afterSemicolon);
	char** beforeSemicolonPARSED = parseArgs(beforeSemicolon);	

	//execute the two commands
	exec2Args(beforeSemicolonPARSED, afterSemicolonPARSED);

	free(beforeSemicolonPARSED);
	free(afterSemicolonPARSED);
    }    
    
    //check if the user inputted a '>' and if they did, redirect output to the given file
    char* rightBracketPtr = strchr(line, '>');
    if(rightBracketPtr != NULL){
	//get the index of the '>' character in the line
	int position = line - rightBracketPtr;
	position = position * -1;
	
	//put every char before the '>' into an array
	char beforeBracket[position];
	memcpy(beforeBracket, line, position);

	//then parse that array into an argument list
	char** beforeBracketPARSED = parseArgs(beforeBracket);

	//get the filename
	char* filename = strtok(&line[position+1], " \n");

	//run the command and redirect the output to the given file
	execArgsRedirect(beforeBracketPARSED, filename);
	free(beforeBracketPARSED);
    }
    
    //check if the user inputted a '<' and if they did, use the given file as input
    //i honestly don't know what this argument is even supposed to do
    char* leftBracketPtr = strchr(line, '<');
    if(leftBracketPtr != NULL){
	//get the index of the '<' character in the line
	int position = line - leftBracketPtr;
	position = position * -1;

	line[position] = ' ';	
	char** parsedArgs = parseArgs(line);
	execArgs(parsedArgs);

	//put every char before the '<' into an array
	/*char beforeBracket[position];
	memcpy(beforeBracket, &line, position);
	printf("stuff before the bracket: %s#\n", beforeBracket);

	//get the filename
	char* filename = strtok(&line[position+1], " \n");
	printf("filename: %s\n", filename);

	//transfer everything in the file into an array
	char stuffFromFile[50];
	int fd1 = open(filename, O_RDONLY);
	read(fd1, stuffFromFile, 49);
	stuffFromFile[49] = '\0';
	printf("Stuff from file: %s\n", stuffFromFile);

	//concatenate the pre-'<' stuff with the file stuff
	//strcat(stuffFromFile, beforeBracket);
	/*strcat(beforeBracket, filename);

	printf("concatenated argument: %s#\n", beforeBracket);

	//parse the whole command
	char** wholeThingPARSED = parseArgs(beforeBracket);

	int x = 0;
	printf("The concatenated argument is: ");
	while(beforeBracket[x] != '\0'){
		putchar(beforeBracket[x]);
		x++;
	}	
	printf("\n");*/

	//execute the command
	//execArgs(wholeThingPARSED);
	//free(wholeThingPARSED);
	//memset(stuffFromFile, 0, 50);
    }
    
    //if the user did not use ';' '>' or '<', then just run the inputted command normally
    if(semicolonPtr == NULL && rightBracketPtr == NULL && leftBracketPtr == NULL){
	char** argsarray = parseArgs(line);
	execArgs(argsarray);  
	free(argsarray);
    }
  }

  return 0;
}
