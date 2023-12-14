#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <pwd.h>
#include <ctype.h>
#include <sys/wait.h>
#include <sys/stat.h>

//+
// File:	shell.c
//
// Pupose:	This program implements a simple shell program. It does not start
//		processes at this point in time. However, it will change directory
//		and list the contents of the current directory.
//
//		The commands are:
//		   cd name -> change to directory name, print an error if the directory doesn't exist.
//		              If there is no parameter, then change to the home directory.
//		   ls -> list the entries in the current directory.
//			      If no arguments, then ignores entries starting with .
//			      If -a then all entries
//		   pwd -> print the current directory.
//		   exit -> exit the shell (default exit value 0)
//				any argument must be numeric and is the exit value
//
//		if the command is not recognized an error is printed.
//-

#define CMD_BUFFSIZE 1024
#define MAXARGS 10

int splitCommandLine(char * commandBuffer, char* args[], int maxargs);
int doInternalCommand(char * args[], int nargs);
int doProgram(char * args[], int nargs);

//+
// Function:	main
//
// Purpose:	The main function. Contains the read
//		eval print loop for the shell.
//
// Parameters:	(none)
//
// Returns:	integer (exit status of shell)
//-
int main() {

    char commandBuffer[CMD_BUFFSIZE];
    // note the plus one, allows for an extra null
    char *args[MAXARGS+1];

    // print prompt.. fflush is needed because
    // stdout is line buffered, and won't
    // write to terminal until newline
    printf("%%> ");
    fflush(stdout);

    while(fgets(commandBuffer,CMD_BUFFSIZE,stdin) != NULL){
        //printf("%s",commandBuffer);

        // remove newline at end of buffer
        int cmdLen = strlen(commandBuffer);
        if (commandBuffer[cmdLen-1] == '\n'){
            commandBuffer[cmdLen-1] = '\0';
            cmdLen--;
                //printf("<%s>\n",commandBuffer);
        }

        // split command line into words.(Step 2)
        int nargs = splitCommandLine(commandBuffer, args, MAXARGS); // calls splitCommandLine

        // add a null to end of array (Step 2)
        args[nargs] = NULL;

        // debugging
        //printf("%d\n", nargs);
       // int i;
       // for (i = 0; i < nargs; i++){
       // printf("%d: %s\n",i,args[i]);
       // }
        // element just past nargs
        //printf("%d: %x\n",i, args[i]);

        // check if 1 or more args (Step 3)
        if (nargs > 0){
            // if one or more args, call doInternalCommand  (Step 3)
            int internalCommand = doInternalCommand(args, nargs);
        
            // if doInternalCommand returns 0, call doProgram  (Step 4)
            if(!internalCommand){
                int program = doProgram(args, nargs);

                // if doProgram returns 0, print error message (Step 3 & 4)
                // that the command was not found.
                if(!program){
                    fprintf(stderr, "Error: Command Not Found!\n");
                }
            }
        }

        // print prompt
        printf("%%> ");
        fflush(stdout);
    }

    return 0;
}

////////////////////////////// String Handling (Step 1) ///////////////////////////////////

//+
// Function:	skipChar
//
// Purpose:	This function skips over a given char in a string
//		For security, will not skip null chars.
//
// Parameters:
//    charPtr	Pointer to string
//    skip	character to skip
//
// Returns:	Pointer to first character after skipped chars
//		ID function if the string doesn't start with skip,
//		or skip is the null character
//-
char * skipChar(char * charPtr, char skip){
    // if skip is null char
    if (skip == '\0'){
        return charPtr; // return end of string
    }
    // else iterate through the string until a character different from 'skip' is found.
    while(*charPtr == skip){
        charPtr+=1; // Move the pointer to the next character.
    }
    // Return the pointer to the first character that is not 'skip'.
    return charPtr;
}

//+
// Funtion:	splitCommandLine
//
// Purpose:	Take arguments commandBuffer, args array and length of args array (maxargs)
// and seperates the words in commandBuffer (user input) and stores pointers at the begining of each word
// returns the number of words (number of pointers in args array)
//
// Parameters:
// @param commandBuffer The input command line string to be split.
// @param args An array to store the individual arguments.
// @param maxargs The maximum number of arguments that can be stored in 'args'.
//
// Returns:	Number of arguments (< maxargs).
//
//-
int splitCommandLine(char * commandBuffer, char* args[], int maxargs){
    int count = 0;

    while (count < maxargs - 1) {
        // Use the skipChar function to skip spaces
        commandBuffer = skipChar(commandBuffer, ' ');

        if (*commandBuffer == '\0') {
            break; // No more arguments
        }

        args[count++] = commandBuffer; // pointer to thee first char in word

        // Find the next space or the end of the string
        commandBuffer = strchr(commandBuffer, ' ');

        if (commandBuffer == NULL) {
            break; // No more arguments
        }

        *commandBuffer++ = '\0'; // Null-terminate the argument
    }

    // if number of words larger than maxargs
    if (count >= maxargs) {
        fprintf(stderr, "Too many arguments!\n");
        return 0;
    }

    return count;
}

////////////////////////////// External Program  (Note this is step 4, complete doeInternalCommand first!!) ///////////////////////////////////

// list of directorys to check for command.
// terminated by null value
char * path[] = {
    ".",
    "/usr/bin",
    NULL
};

//+
// Funtion:	doProgram
//
// Purpose:	 function to search through system to find requested file
// returns 1 if the program is found and executed, and 0 if it cannot be found or if there are memory allocation issues.
//
// Parameters:
//	 @param args An array of program arguments, with the program name as args[0].
//   @param nargs The number of arguments in the 'args' array.
// Returns	int
//		1 = found and executed the file
//		0 = could not find and execute the file
//-
int doProgram(char * args[], int nargs){
    char *cmd_path = NULL; //path string

    // Iterate through the directories in the 'path' variable
    int i=0;
    while(path[i] != NULL) {
        // Allocate memory for the full path to the executable
        cmd_path = (char *)malloc(strlen(path[i]) + strlen(args[0]) + 2); 

        if (cmd_path == NULL) { //if the command path is false
            fprintf(stderr, "Memory allocation failed");
            return 0; // Return 0 on memory allocation failure
        }

        // Construct the full path to the executable
        sprintf(cmd_path, "%s/%s", path[i], args[0]);

        // int stat(const char *path, struct stat *buf); // stat definition
        struct stat st;
        //call to the stat struct passing cmd_path as the path
        int fileRetrieve = stat(cmd_path, &st); // returns 0 if file exits
        
        // Check if the file exists and is regulare using S_ISREG(mode) and S_IXUSR if executable
        if (!fileRetrieve && S_ISREG(st.st_mode) && (st.st_mode & S_IXUSR) != 0) {
            break; // if the file is regular and executable break loop
        }

        free(cmd_path); // Free memory for the current cmd_path
        cmd_path = NULL;
        i++;
    }

    // if the cmd_path is NULL return 0
    if (cmd_path == NULL){
        return 0;
    }

    //check if process is child, If it's the child process (return value of fork is 0)
    if(fork() == 0){
        // use execv to replace the current process with program specified by cmd_path
        execv(cmd_path, args);
    } else { // otherwise parent process
        // use wait(NULL) to wait for the child process to complete executing
        wait(NULL);
    }

    // for either process free cmd_path 
    free(cmd_path);
    // return 1 to indicate program ran sucessfully
    return 1; 
}

////////////////////////////// Internal Command Handling (Step 3) ///////////////////////////////////

// define command handling function pointer type
typedef void(*commandFunc)(char * args[], int nargs);

// associate a command name with a command handling function
struct cmdStruct{
   char 	* cmdName;
   commandFunc 	cmdFunc;
};

// prototypes for command handling functions exit, pwd, ls and cd 
void exitFunc(char * args[], int nargs);
void pwdFunc(char * args[], int nargs);
void lsFunc(char * args[], int nargs);
void cdFunc(char * args[], int nargs);


// list commands and functions
// must be terminated by {NULL, NULL} 
// in a real shell, this would be a hashtable.
struct cmdStruct commands[] = {
    // list functions exit, pwd, ls and cd 
    {"exit", exitFunc},
    {"pwd", pwdFunc},
    {"ls", lsFunc},
    {"cd", cdFunc},
    {NULL, NULL}		// terminator
};


//+
// Function:	doInternalCommand
//
// Purpose:	Check the first string of args and call the associated command function
// if the string does not match a command return 0.
//
// Parameters:
//	 @param args[] An array of program arguments, with the program name as args[0].
//   @param nargs The number of arguments in the 'args' array.
//
// Returns	int
//		1 = args[0] is an internal command
//		0 = args[0] is not an internal command
//-
int doInternalCommand(char * args[], int nargs){
    int j = 0;
    
    while(commands[j].cmdName != NULL){ // loop through the cmdStruct commands untill the terminator

        if(strcmp(commands[j].cmdName, args[0]) == 0){ // check if command is equal to the string at args[0]
            // if true call the associated command function
            commands[j].cmdFunc(args, nargs);
            return 1;
        }

        j++; //increment index counter
    }
    return 0; // otherwise commad is not in commnads return 0
}


///////////////////////////////
// comand Handling Functions //
///////////////////////////////


//+
// Function: exitFunc
//
// Purpose: to run the exit(0) command when called and exit the program
//
// Parameters:
//	 @param args[] An array of program arguments, with the program name as args[0].
//   @param nargs The number of arguments in the 'args' array.
//
// Returns:	void
//-
void exitFunc(char* args[], int nargs){
    exit(0);
}


//+
// Function: pwdFunc
//
// Purpose: to print the working directory using the cwd function
//
// Parameters:
//	 @param args[] An array of program arguments, with the program name as args[0].
//   @param nargs The number of arguments in the 'args' array.
//
// Returns:	void
//-
void pwdFunc(char* args[], int nargs){
    char * cwd = getcwd(NULL,0);// call the getcwd functio to get the working dir
    printf("%s\n", cwd); // print the working directory
    free(cwd); //the getcwd allocates the string,release it using free 
}

//+
// Function: filterHiddenFiles
//
// Purpose: This function is used as a filter for directory entries to exclude hidden files and directories.
//
// Parameters:
//   const struct dirent *d - A pointer to a directory entry structure.
//
// Returns: 
//   int - Returns 1 if the directory entry does not represent a hidden file or directory (i.e., its name does not start with a dot '.'), 
//   and 0 otherwise.
//-
int filterHiddenFiles(const struct dirent *d){
    return d->d_name[0]!='.';
}

//+
// Function: lsFunc
//
// Purpose: to print the current directory contents 
//
// Parameters:
//	 @param args[] An array of program arguments, with the program name as args[0].
//   @param nargs The number of arguments in the 'args' array.
//
// Returns:	void
//-
void lsFunc(char* args[], int nargs){
    struct dirent ** namelist;
    int numEnts;
    // need to check for ls -a option
    if (nargs == 2){
        if (strcmp(args[1], "-a")==0){
            numEnts = scandir(".",&namelist,NULL,NULL);
        }else{
            fprintf(stderr,"Error: ls passed with argument besides -a\n");
            return;
        }
    }
    else {
        numEnts = scandir(".",&namelist,filterHiddenFiles,NULL);
    }

    for(int i =0; i<numEnts; i++){
        printf("%s ", namelist[i]->d_name);
    }
    printf("\n");
}


//+
// Function: cdFunc
//
// Purpose: to print the working directory using the cwd function
//
// Parameters:
//	 @param args[] An array of program arguments, with the program name as args[0].
//   @param nargs The number of arguments in the 'args' array.
//
// Returns:	void
//-
void cdFunc(char* args[], int nargs){
    struct passwd *pw = getpwuid(getuid()); // get a pointer to the password file entry of a user

    if (pw == NULL){ // check if the name of the home directory is null
        fprintf(stderr,"Cannot to get pointer to password entry\n");
        return;
    }

    if (pw->pw_dir == NULL){ // check if the name of the home directory is null
        fprintf(stderr,"Cannot to retrieve user home directory\n");
        return;
    }

    char * directoryName; // char pointer for directory name

    if (nargs == 1){ // one argument passed
        directoryName = pw->pw_dir;

    }
    else if (nargs == 2){ // 2 arguments passed
        directoryName = args[1];
    }
    else{
        fprintf(stderr,"Cannot cd to directory. Incorrect number of paramerters passed\n");
        return;
    }

    int changeDir = chdir(directoryName); // call the chdir function to change the directory 
    if (changeDir==-1){ // chdir returns 0 if sucessfull, print error message if returns 1
        fprintf(stderr,"Did not successfully change current directory\n");
        return;
    }
}

