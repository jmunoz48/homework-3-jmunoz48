#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h> // the man page for the exec functions says it needs this library
#include <sys/types.h> // apparently i also need these 3 things for open()
#include <sys/stat.h>
#include <fcntl.h>


void sigint_handler(int sig){
  char msg[] = "\ncaught sigint\n";
  write(1, msg, sizeof(msg));
  char msg2[] = "CS361 >";
  write(1, msg2, sizeof(msg2));
}


void sigstp_handler(int sig){
  char msg[] = "\ncaught sigstp\n";
  write(1, msg, sizeof(msg));
  char msg2[] = "CS361 >";
  write(1, msg2, sizeof(msg2));

}


// executes 1 command
void execArgs(char** parsed) 
{
    /*int i=0;
    while(parsed[i] != NULL){
	printf("parsed[%d]: %s\n", i, parsed[i]);
	i++;
    }*/

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
        return; 
    } 
} 


// executes 2 commands (outputs the pid and exit status only after both children are dead)
void exec2Args(char** parsed, char** parsed2) 
{ 
    // NOTE: this function deletes parsed[0] if parsed has more than 1 element and i have absolutely no idea why
    /*int i=0;
    while(parsed[i] != NULL){
	printf("parsed[%d]: %s\n", i, parsed[i]);
	i++;
    }

    i=0;
    while(parsed2[i] != NULL){
	printf("parsed2[%d]: %s\n", i, parsed2[i]);
	i++;
    }*/

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


// executes 1 command and redirects the output of the child to wherever
void execArgsRedirect(char** parsed, char* filename) 
{
    /*int i=0;
    while(parsed[i] != NULL){
	printf("parsed[%d]: %s\n", i, parsed[i]);
	i++;
    }*/

    // Forking a child 
    int pid = fork();  
  
    if (pid == -1) { 
        printf("Couldn't fork child\n"); 
        return; 
    } 
    // have the child execute the command
    else if (pid == 0) { 
	char line2[500];
	int outputFile = open(filename, O_RDWR|O_CREAT);
	dup2(outputFile, 1);
	fgets(line2, 500, stdout);
        write(outputFile, line2, 500);

        if (execv(parsed[0], parsed) < 0)
            printf("Couldn't execute command\n");  
        exit(0); 
    } 
    // have the parent wait for the child to die
    else { 
        int status;
        wait(&status);
	printf("pid:%d status:%d\n", pid, WEXITSTATUS(status));
        return; 
    } 
}


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
    //printf("word: %s\n", word);
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
    //char* argsarray[100]; // i spent HOURS trying to get the stupid type for this stupid array right

    //print prompt
    printf("CS361 >");

    //read line from terminal
    fgets(line, 500, stdin);
    
    //check if the user inputted a semicolon and if they did, try running those 2 commands
    char* semicolonPtr = strchr(line, ';');
    if(semicolonPtr != NULL){
	int position = line - semicolonPtr;
	position = position * -1;
	
	//put everything before the semicolon into an array
	char beforeSemicolon[position];
	memcpy(beforeSemicolon, &line, position);
	beforeSemicolon[position] = '\0';

	//parse that array into an argument list
	char** beforeSemicolonPARSED = parseArgs(beforeSemicolon);
	//printf("Executing the first command: \n");	
	//execArgs(beforeSemicolonPARSED);
	//printf("\nExecuting both commands: \n");

	//put everything after the semicolon into an array as well
        char afterSemicolon[strlen(line) - position];
	memcpy(afterSemicolon, &line[position+1], strlen(line));
	afterSemicolon[strlen(line) - position] = '\0';

	//parse that array into an argument list too
	char** afterSemicolonPARSED = parseArgs(afterSemicolon);

	//execute the two commands
	exec2Args(beforeSemicolonPARSED, afterSemicolonPARSED);
	free(beforeSemicolonPARSED);
	free(afterSemicolonPARSED);
    }    
    
    //check if the user inputted a '>' and if they did, redirect output to the given file
    char* rightBracketPtr = strchr(line, '>');
    if(rightBracketPtr != NULL){
	int position = line - rightBracketPtr;
	position = position * -1;
	
	//put every char before the '>' into an array
	char beforeBracket[position];
	memcpy(beforeBracket, &line, position);
	beforeBracket[position] = '\0';

	//then parse that array into an argument list
	char** beforeBracketPARSED = parseArgs(beforeBracket);

	//get the filename
        char afterBracket[strlen(line) - position];
	memcpy(afterBracket, &line[position+1], strlen(line));
	afterBracket[strlen(line) - position] = '\0';

	//run the command and redirect the output to the given file
	execArgsRedirect(beforeBracketPARSED, afterBracket);
	free(beforeBracketPARSED);
    }
    
    //check if the user inputted a '<' and if they did, use the given file as input
    char* leftBracketPtr = strchr(line, '<');
    if(leftBracketPtr != NULL){
	int position = line - leftBracketPtr;
	position = position * -1;
	
	//put every char before the '<' into an array
	char beforeBracket[position];
	memcpy(beforeBracket, &line, position);

	//get the filename
	char afterBracket[strlen(line) - position];
	memcpy(afterBracket, &line[position+1], strlen(line));
	afterBracket[strlen(line) - position] = '\0';

	printf("filename: %s\n", afterBracket);

	//transfer everything in the file into an array
	char stuffFromFile[100];
	char c;
	int i = 0;
	FILE* file;
	file = fopen(afterBracket, "r");
	if (file) {
	    printf("You finally got to this point\n\n");
	    while ((c = getc(file)) != EOF){
        	stuffFromFile[i] = c;
		i++;
	    }
	    fclose(file);
	}
	stuffFromFile[i] = '\0';

	int x = 0;
	printf("The stuff from file is: ");
	while(stuffFromFile[x] != '\0'){
		putchar(stuffFromFile[x]);
		x++;
	}	
	printf("\n");

	//concatenate the pre-'<' stuff with the file stuff
	strcat(stuffFromFile, beforeBracket);

	//parse the whole command
	char** wholeThingPARSED = parseArgs(beforeBracket);

	x = 0;
	printf("The concatenated argument is: ");
	while(beforeBracket[x] != '\0'){
		putchar(beforeBracket[x]);
		x++;
	}	
	printf("\n");

	//execute the command
	execArgs(wholeThingPARSED);
	free(wholeThingPARSED);
    }
    
    //if the user did none of those things, just run the inputted command normally
    if(semicolonPtr == NULL && rightBracketPtr == NULL && leftBracketPtr == NULL){
	char** argsarray = parseArgs(line);
	execArgs(argsarray);  
	free(argsarray);
    }
  }

  return 0;
}
