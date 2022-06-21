//Amiram Yassif 314985474
//region consts and libs
#include <stdio.h>
#include <unistd.h>
#include <malloc.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/wait.h>
#include <dirent.h>
#include <errno.h>
//#include <sys/types.h>

#define TIMEOUT_SEC             5
#define TIME_OUT_STATUS         14
#define MAX_INPUT_LEN           150

#define DIR_PATH                configData[0]
#define ARGS_PATH               configData[1]
#define CORRECT_OUTPUT_PATH     configData[2]

#define RESULT_PATH             "results.csv"
#define ERROR_PATH              "errors.txt"
#define PROG_RUN_OUTPUT_PATH    "output.txt"
#define COMP_FILE_PATH          "./comp.out"
#define COMPILED_FILE_PATH      "./tmp_compiled_file.out"
#define COMPILED_FILE_NO_REF    "tmp_compiled_file.out"

#define NO_C_FILE               ",0,NO_C_FILE\n"
#define COMPILATION_ERROR       ",10,COMPILATION_ERROR\n"
#define TIMEOUT                 ",20,TIMEOUT\n"
#define WRONG                   ",50,WRONG\n"
#define SIMILAR                 ",75,SIMILAR\n"
#define EXCELLENT               ",100,EXCELLENT\n"

#define ERROR                   -1
#define NO_C_FILE_GRADE         0
#define COMPILATION_ERROR_GRADE 10
#define TIMEOUT_GRADE           20
#define WRONG_GRADE             50
#define SIMILAR_GRADE           75
#define EXCELLENT_GRADE         100

//endregion
//region completed stuff
/**
 * Reads config file and separates to 3 strings.
 * @param argc num of args
 * @param argv args
 * @param mtr The strings to copy the file into
 * @return 0 in operation success, -1 ow.
 */
int readConfig(int argc,char *argv[], char mtr[3][MAX_INPUT_LEN]){
    //Checks for correct num of params.
    if (argc < 2) {
        write(STDERR_FILENO,"A single address param is required.\n",36);
        return ERROR;
    }
    //Open file
    char add[MAX_INPUT_LEN];
    strcpy(add, argv[1]);
    int conf_fd;
    conf_fd = open(add, O_RDONLY);
    if (conf_fd == ERROR) {
        write(2, "Not a valid directory\n",22);
        close(conf_fd);
        return ERROR;
    }
    //Read file.
    char c = '\0';
    long res = 1;
    //1st line.
    int firstlineiterator;
    for (firstlineiterator = 0;
         ((res == read(conf_fd, &c, 1)) != 0) && c != '\n';
        firstlineiterator++) {
        if (res < 0) {
            perror("Error in: read");
            close(conf_fd);
            return ERROR;
        }
        mtr[0][firstlineiterator] = c;
    }
    //2nd line
    int seclineiterator;
    for (seclineiterator = 0;
         ((res == read(conf_fd, &c, 1)) != 0) && c != '\n';
         seclineiterator++) {
        if (res < 0) {
            perror("Error in: read");
            close(conf_fd);
            return ERROR;
        }
        mtr[1][seclineiterator] = c;
    }
    //3rd line
    int trdlineiterator;
    for (trdlineiterator = 0;
         ((res == read(conf_fd, &c, 1)) != 0) && c != '\n';
         trdlineiterator++) {
        if (res < 0) {
            perror("Error in: read");
            close(conf_fd);
            return 1;
        }
        mtr[2][trdlineiterator] = c;
    }
    //in case one line is empty, program fails.
    if (!strcmp(mtr[0],"") || !strcmp(mtr[1],"") || !strcmp(mtr[2],""))
        return ERROR;
    close(conf_fd);
    return 0;
}

/**
 * Reads file's content
 * @param buffer writes content here
 * @param address file's address
 * @return 0 for success, 1 ow.
 */
int getFileContent (char buffer[MAX_INPUT_LEN], char *address) {
    //Open file
    char c = '\0';
    int fd = open(address, O_RDONLY);
    if (fd == -1) {
        perror("Error in: open");
        close(fd);
        return 1;
    }
    long res = 1;
    //Read char by char
    int i;
    for (i = 0; ((res = read(fd, &c, 1)) != 0) /* && c!='\r' && c == '\0' */; ++i) {
        if(res == -1) {
            // terminate failed connection.
            perror("Error in: read");
            close(fd);
            return 1;
        }
        buffer[i] = c;
    }
    // terminate successfully connection.
    close(fd);
    return 0;
}

/**
 * Returns if string hs .c suffix
 * @param str file name
 * @return has .c suffix
 */
bool isCFileName(char *str) {
    char dot = str[strlen(str)-2];
    char c = str[strlen(str)-1];
    return ((dot == '.') && (c == 'c'));
}

/**
 * Compiles a file into a paramless, a.out executable.
 * @param path path to .c file to compile.
 * @return 0 for success, any other int for failure.
 */
int compileFile(char path[MAX_INPUT_LEN]) {
    int status = 0;
    pid_t pid = fork();
    // Compile c file
    if (!pid) {
        alarm(5);
        char *params[] = {"gcc", path, "-o", COMPILED_FILE_NO_REF, NULL};
        execvp(params[0],params);
    }
    waitpid(pid, &status, 0);
    return status;
}

/**
 * Executes file in path
 * @param path file's path
 * @return program's return status (0 for success).
 */
int executeFile(char path[MAX_INPUT_LEN]) {
    int status = 0;
    pid_t pid = fork();
    if (!pid) {
        alarm(TIMEOUT_SEC);
        char *params[] = {COMPILED_FILE_PATH, NULL};
        execvp(params[0],params);
    }
    waitpid(pid,&status,0);
    // delete compiled file
    return status;
}
/*
 * Overrides fd with another fd
 * @param original_fd the fd to override
 * @param new_fd new fd to put instead
 * @return copy of old fd
 */
int redirectFdToNew(int original_fd, int new_fd) {
    int original_cpy = dup(original_fd);
    if (original_cpy == -1) {
        perror("Error in: dup");
        return -1;
    }
    //int fd = open(input_address, O_RDONLY, 0777);
    if(dup2(new_fd, original_fd) == -1) {
        perror("Error in: dup2");
        dup2(original_fd, new_fd);
        return -1;
    }
    return original_cpy;
}

/**
 * Redirect INPUT fd from STDIN to FILE.
 * @param input_address FILE address
 * @return STDIN's fd in case of success, -1 ow.
 */
int redirectInputFdToFile(char *input_address) {
    int fd = open(input_address, O_RDONLY, 0777);
    if(fd == -1) {
        perror("Error in: open");
        return -1;
    }
    return redirectFdToNew(STDIN_FILENO, fd);
}

/**
 * Redirect OUTPUT fd from STDOUT to FILE.
 * @param input_address FILE address
 * @return STDOUT's fd in case of success, -1 ow.
 */
int redirectOutputFdToFile(char *output_address) {
    int fd = open(output_address,  O_APPEND | O_WRONLY | O_TRUNC | O_CREAT, 0666);
    if(fd == -1) {
        perror("Error in: open");
        close(fd);
        return -1;
    }
    return redirectFdToNew(STDOUT_FILENO, fd);
}

int redirectErrorFdToFile(char *error_address) {
    int fd = open(error_address,  O_APPEND | O_WRONLY | O_CREAT, 0666);
    if(fd == -1) {
        perror("Error in: open");
        close(fd);
        return -1;
    }
    return redirectFdToNew(STDERR_FILENO, fd);
}

/**
 * Redirect INPUT fd from FILE to STDIN.
 * @param stdin_fd STDIN fd
 * @return true for success.
 */
bool redirectInputFdToSTD(int std_backup, int std_fileno) {
    int res = dup2(std_backup, std_fileno);
    if (res == -1) {
        perror("Error in: dup2");
        return false;
    }
    if(close(std_backup)) {
        perror("Error in: close");
        return false;
    }
    return true;
}


/**
 * compares output of specific run to the correct output.
 * @param correct_output_file_path path to correct output file.
 * @return 1 -> different. 2 -> identical. 3 -> similar.
 */
int checkResult(char *correct_output_file_path) {
    char *comp_params[] = {
            COMP_FILE_PATH,
            PROG_RUN_OUTPUT_PATH,
            correct_output_file_path,
            NULL
    };
    int status = 0;
    pid_t comp_proc = fork();
    if(comp_proc == ERROR) {
        perror("Error in: fork");
        close(comp_proc);
        return ERROR;
    }
    if (comp_proc == 0) {
        execvp(comp_params[0], comp_params);
    }
    waitpid(comp_proc, &status, 0);
    return WEXITSTATUS(status);
}

/**
 * Returns grade for a .c file.
 * @param path path to .c file
 * @return grade:
 *                  compilation error   -> 10
 *                  timeout error       -> 20
 *                  different results   -> 50
 *                  similar results     -> 75
 *                  excellent results   -> 100
 */
int getExistingFileGrade(char path[150], char *correct_output_file_path) {
    // Compilation error
    if(compileFile(path) != 0)
        return COMPILATION_ERROR_GRADE;
    // In case of timeout error
    if(executeFile(path) == TIME_OUT_STATUS)
        return TIMEOUT_GRADE;
    // Get execution result.
    int similarity = checkResult(correct_output_file_path);
    switch (similarity) {
        case 1:
            return EXCELLENT_GRADE;
            break;
        case 2:
            return WRONG_GRADE;
            break;
        case 3:
            return SIMILAR_GRADE;
            break;
        default:
            return ERROR;
    }
}

/**
 * Handles a single file
 * @param path path to file
 * @param prog_params_path path to input stream file
 * @return File's grade, or -1 if error occurs.
 */
int singleFileHandler(char path[150], char *prog_params_path, char *correct_output_file_path) {
    //Redirect streams
    int stdin_fd_bkp = redirectInputFdToFile(prog_params_path);
    if (stdin_fd_bkp == -1) {
        redirectInputFdToSTD(stdin_fd_bkp, STDIN_FILENO);
        return ERROR;
    }
    int stdout_fd_bkp = redirectOutputFdToFile(PROG_RUN_OUTPUT_PATH);
    if (stdout_fd_bkp == -1) {
        redirectInputFdToSTD(stdin_fd_bkp, STDIN_FILENO);
        redirectInputFdToSTD(stdout_fd_bkp, STDOUT_FILENO);
        return ERROR;
    }
    int stderr_fd_bkp = redirectErrorFdToFile(ERROR_PATH);
    if (stderr_fd_bkp == -1) {
        redirectInputFdToSTD(stdin_fd_bkp, STDIN_FILENO);
        redirectInputFdToSTD(stdout_fd_bkp, STDOUT_FILENO);
        redirectInputFdToSTD(stderr_fd_bkp, STDERR_FILENO);
        return ERROR;
    }
    // get grade
    int grade = getExistingFileGrade(path, correct_output_file_path);
    // Return stream to terminal
    redirectInputFdToSTD(stdin_fd_bkp, STDIN_FILENO);
    redirectInputFdToSTD(stdout_fd_bkp, STDOUT_FILENO);
    redirectInputFdToSTD(stderr_fd_bkp, STDERR_FILENO);
    //delete output.
    //printf("%s's grade: %d\n", path, grade);
    return grade;
}
//endregion
//region irrelevant

/**
 * Runs routine for a single dir
 * @param dent dirent to dir
 * @param path full path to dir
 * @return grade for folder
 */
int singleDirOperation(struct dirent *dent, char *path, char *prog_params_path, char *correct_output_file_path) {
    int grade = NO_C_FILE_GRADE;
    //char fileName[MAX_INPUT_LEN];
    DIR *dir = opendir(path);
    char cFileAdd[MAX_INPUT_LEN];
    struct dirent *scanner = (struct dirent *) opendir(path);
    while ((scanner = readdir(dir))!=NULL) {
        if(scanner == NULL) {
            perror("Error in: readdir");
            closedir(dir);
            return 1;
        }
        if (scanner->d_type!=DT_REG) continue;
        if(isCFileName(scanner->d_name)) {
            memset(cFileAdd, '\0', MAX_INPUT_LEN);
            strcat(cFileAdd, path);
            strcat(cFileAdd, "/");
            strcat(cFileAdd, scanner->d_name);
            grade = singleFileHandler(cFileAdd, prog_params_path, correct_output_file_path);
        }
    }
    return grade;
}

/**
 * Scans through each sub-dir in dir and runs the procedure there
 * @param args path to config
 * @param dir_path path to dir
 * @param correctOutputPath path of correct output file.
 * @return true for successful operation.
 */
 //endregion

bool printResultToFile(int grade, char name[MAX_INPUT_LEN]) {
    // open result file
    int fd = open(RESULT_PATH, O_WRONLY | O_APPEND | O_CREAT, 0666);
    if (fd == ERROR) {
        perror("Error in open");
        return false;
    }
    //build csv for grades
    char str[MAX_INPUT_LEN];
    strcpy(str, name);
    switch (grade) {
        case NO_C_FILE_GRADE:
            strcat(str, NO_C_FILE);
            break;
        case COMPILATION_ERROR_GRADE:
            strcat(str, COMPILATION_ERROR);
            break;
        case TIMEOUT_GRADE:
            strcat(str, TIMEOUT);
            break;
        case WRONG_GRADE:
            strcat(str, WRONG);
            break;
        case SIMILAR_GRADE:
            strcat(str, SIMILAR);
            break;
        case EXCELLENT_GRADE:
            strcat(str, EXCELLENT);
            break;
        default:
            strcpy(str, "");
    }
    // write the stream to file
    write(fd,str, strlen(str));
    // closes stream
    close(fd);
}
// region starting point
bool routine(char args[150],
             char *dir_path,
             char correctOutputPath[150],
             char *prog_params_path,
             char correct_output_file_path[MAX_INPUT_LEN]) {
    int grade;
    //open work lib
    DIR *dir = opendir(dir_path);
    if (dir == NULL) {
        write(2,"Not a valid directory",21);
        closedir(dir);
        return false;
    }
    char address[MAX_INPUT_LEN];
    struct dirent *dent = (struct dirent *) opendir(dir_path);
    if(dent == NULL) {
        write(2,"Not a valid directory",21);
        closedir(dir);
        return false;
    }
    // foreach subdir, and execute procedure.
    while ((dent = readdir(dir))!=NULL) {
        if(dent == NULL) {
            perror("Error in: opendir");
            closedir(dir);
            return 1;
        }
        if (dent->d_type != DT_DIR)
            continue;
        if (!strcmp(dent->d_name,".")|| !strcmp(dent->d_name,".."))
            continue;
        memset(address,'\0', MAX_INPUT_LEN);
        strcpy(address, dir_path);
        strcat(address, "/");
        strcat(address, dent->d_name);
        //puts(dent->d_name); = folder's name
        grade = singleDirOperation(dent, address, prog_params_path, correct_output_file_path);
        printResultToFile(grade, dent->d_name);
    }
    closedir(dir);
    //dup2(stdin_cpy, 0);
    return true;
}

/**
 * Check all addresses are correct
 * @param dir_path path to base dir
 * @param input_path path to simulated user input
 * @param correct_output_path path to expected output
 * @return true if all files exist. false ow.
 */
bool checkRequiredFiles(char *dir_path, char *input_path, char *correct_output_path) {
    if (access(dir_path, F_OK) != 0) {
        write(2,"Not a valid directory\n",22);
        return false;
    }
    DIR *dir = opendir(dir_path);
    //if dir exists
    if (dir)
        closedir(dir);
    else if (ENOENT == errno) {
        write(2,"Not a valid directory\n",22);
        return false;
    } else {
        write(2,"Couldn't open directory\n",22);
        return false;
    }
    if (access(input_path, F_OK) != 0) {
        write(2, "Input file not exist\n",21);
        return false;
    }
    if (access(correct_output_path, F_OK) != 0) {
        write(2, "Output file not exist\n",21);
        return false;
    }
    return true;
}

/**
 * closes all dfs, and removes junk temp files
*/
void prepareEndOfRun() {
    int max_fd = sysconf(_SC_OPEN_MAX);
    int iterator = 3;
    for (; iterator<=max_fd; iterator++)
        close(iterator);
    remove(COMPILED_FILE_NO_REF);
    remove(PROG_RUN_OUTPUT_PATH);
}

/**
 * starting point
 * @param argc arg counter
 * @param argv args
 * @return 0 for successful operation, 1 ow.
 */
int main(int argc, char* argv[]) {
    // Process conf file into arr.
    char configData[3][MAX_INPUT_LEN];
    if (readConfig(argc, argv, configData) == ERROR)
        return ERROR;
    if (!checkRequiredFiles(DIR_PATH, ARGS_PATH, CORRECT_OUTPUT_PATH)) {
        prepareEndOfRun();
        return ERROR;
    }
    //Get args from file
    char args[MAX_INPUT_LEN] = {""};
    getFileContent(args, ARGS_PATH);
    // Start routine
    if(!routine(args, DIR_PATH, CORRECT_OUTPUT_PATH, ARGS_PATH, CORRECT_OUTPUT_PATH)) {
        prepareEndOfRun();
        return ERROR;
    }
    prepareEndOfRun();
    return 0;
}
// endregion
