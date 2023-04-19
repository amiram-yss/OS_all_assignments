// Amiram Yassif 314985474
#define     MAX_CMD_SIZE    1024
#define     MAX_LINE_SIZE   100
#define     OVERRIDE_VAR    1
#define     EXIT            "exit"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

// Log dataset + filled places.
char *log_cmd[MAX_LINE_SIZE];
int log_ptr = 0;

void creatEnvironmentalVariables(int argc, char *pString[]);

void startRoutine();

void splitInput(char *cmd, char *pString[100]);

void executeCmd(char *args[100], char raw_cmd[100]);

void clearCmd(char *params[100]);

void logExecution(char cmd[100], pid_t pid);

void printCmdLog();

int main(int argc, char *argv[]) {
    creatEnvironmentalVariables(argc, argv);
    startRoutine();
}

/**
 * Starts CLI routine
 */
void startRoutine() {
    char cmd[MAX_LINE_SIZE];
    char *params[100];
    clearCmd(log_cmd);
    while (strcmp(cmd, EXIT)) {
        clearCmd(params);
        fflush(stdout);
        printf("$ ");
        fflush(stdout);
        fgets(cmd, MAX_LINE_SIZE, stdin);
        cmd[strlen(cmd) - 1] = '\0';
        char raw_cmd[MAX_LINE_SIZE];
        strcpy(raw_cmd, cmd);
        splitInput(cmd, params);
        executeCmd(params, raw_cmd);
    }
}

/**
 * Executes command according to argv.
 * @param argv From user input.
 * @param raw_cmd The line exactly the way it was submitted by usr.
 */
inline void executeCmd(char *argv[100], char raw_cmd[100]) {
    // cd runs on same process.
    if (!strcmp(argv[0],"cd")) {
        logExecution(raw_cmd, getpid());
        if (0 != chdir(argv[1])) {
            perror("cd failed");
//            exit();
        }
    }
    // history runs on same process.
    else if(!strcmp(argv[0], "history")){
        logExecution(raw_cmd, getpid());
        printCmdLog();
    }
    // everything else, excluding exit, runs on different process.
    else if(strcmp(argv[0], "exit")) {
        pid_t child_pid = fork();
        logExecution(raw_cmd, child_pid);
        if (child_pid == 0) {
            if (0 != execvp(argv[0], argv)) {
                char err[MAX_LINE_SIZE];
                strcpy(err, argv[0]);
                strcat(err, " failed");
                perror(err);
                exit(0);
            }
        }
        wait(&child_pid);
    }
}

/**
 * Prints the log saved as global variable.
 */
void printCmdLog() {
    int i;
    for(i=0; i<log_ptr; i++)
        printf("%s\n", log_cmd[i]);
}

/**
 * Logs a single execution, or execution attempt, with
 * it's pid, and full line.
 * @param cmd   The whole command.
 * @param pid   Process PID
 */
void logExecution(char cmd[100], pid_t pid) {
    char pidstr[6];
    sprintf(pidstr, "%d", pid);
    strcpy(log_cmd[log_ptr], pidstr);
    strcat(log_cmd[log_ptr], " ");
    strcat(log_cmd[log_ptr],  cmd);
    ++log_ptr;
}

/**
 * Split user command line to prog name + arr of params
 * @param cmd Line from input. Updated to prog name.
 * @param pString Updated to params list.
 */
inline void splitInput(char *cmd, char *params[100]) {
    int i;
    char *tok = strtok(cmd, " ");
    // Create params array.
    for (i = 0; tok != NULL; ++i) {
        params[i] = malloc(MAX_LINE_SIZE * sizeof (char));
        strcpy(params[i], tok);
        tok = strtok(NULL, " ");
    }
    //for (; i < MAX_LINE_SIZE ; i++) {
        params[i] = NULL;
    //}
}

/**
 * Sets clear string array.
 * @param params
 */
inline void clearCmd(char *params[100]) {
    int i;
    for (i = 0; i < MAX_LINE_SIZE; ++i) {
        params[i] = malloc(MAX_LINE_SIZE);
    }
}

/**
 * Set params as local environmental vars.
 * @param argc  Num of args.
 * @param pString   Addresses to add.
 */
void creatEnvironmentalVariables(int argc, char *pString[]) {
    /*printf("%s\n",getenv("PATH"));
    char strToUpload[MAX_CMD_SIZE];
    for (int i = 1; i<argc ; ++i) {
        //printf("%s\n", pString[i]);
        strcat(strToUpload,":");
        strcat(strToUpload, pString[i]);
    }
    char newPath[MAX_CMD_SIZE];
    strcpy(newPath, getenv("PATH"));
    printf("%s\n", strToUpload);
    strcat(newPath, strToUpload);

    setenv("PATH", newPath, OVERRIDE_VAR);
    printf("%s\n", getenv("PATH"));*/

    //SWITCHED ALGORITHMS:
    //Algo above gave me random stuff like _ between inited
    //path, or early_init, or � or other wierd crap.
    //This algorithm solved the problem.
    char *path = getenv("PATH");
    int i;
    for (i = 1; i < argc; ++i) {
        strcat(path,":");
        strcat(path,pString[i]);
        setenv("PATH", path, OVERRIDE_VAR);
    }
    //printf("%s\n", getenv("PATH"));
}
