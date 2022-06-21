//Amiram Yassif 314985474
// region irrelevant
#include <stdio.h>
#include <unistd.h>
#include <malloc.h>
//#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdbool.h>

#define TRUE        1
#define FALSE       0

#define ERROR       -1
#define IDENT       1
#define DIFFERENT   2
#define SIMILAR     3

#define CAP_LOW_DIFF 32

bool isWhiteSpace(char c1);

bool sameLetter(char c1, char c2);

/**
 * Closes both files' fd
 * @param pid1
 * @param pid2
 */
void endConnection(pid_t pid1, pid_t pid2) {
    close(pid1);
    close(pid2);
}
// endregion
int main(int argc, char *argv[]) {
    //region setup
    //Open files
    pid_t pid1, pid2;
    if (argc != 3) {
        perror("2 params are required.");
        return ERROR;
    }
    char c1, c2, tmp ;
    pid1 = open(argv[1], O_RDONLY);
    ssize_t res1, res2;
    if ( pid1 < 0 ) {
        perror("Error in: open");
        close(pid1);
        return -1;
    }
    pid2 = open(argv[2], O_RDONLY);
    if ( pid2 < 0 ) {
        perror("Error in: open");
        endConnection(pid1, pid2);
        return -1;
    }
    //endregion
    // Routine.
    while (TRUE) {
        // region 1
        //reads a letter from each file
        res1 = read(pid1, &c1, 1);
        if(res1 < 0) {
            perror("Error in: read");
            endConnection(pid1,pid2);
            return ERROR;
        }
        res2 = read(pid2, &c2, 1);
        if(res2 < 0) {
            perror("Error in: read");
            endConnection(pid1, pid2);
            return ERROR;
        }

        //If reached EOF on both files, and all letters are the same, return IDENTICAL (1)

        if(res1 == 0 && res2 == 0) {
            endConnection(pid1, pid2);
            return IDENT;
        }
        //endregion
        //region 2
        //If one file reached the end and the other still scanning:
        if(res1 == 0 || res2 == 0) {
            // If hasn't reached EOF in file 1: OR DIFFERENT OR SIMILAR
            if (res1 != 0) { //Changed from == to !=
                do {
                    res1 = read(pid1, &c1, 1);
                    if (res1 < 0) {
                        perror("Error in: read");
                        endConnection(pid1, pid2);
                        return ERROR;
                    }
                    //If reached a visible letter, different
                    if (!isWhiteSpace(c1)) {
                        endConnection(pid1, pid2);
                        return DIFFERENT;
                    }
                } while (isWhiteSpace(c1) && res1 != 0);
            } else {
                do {
                    //Symmetrically, on file 2:
                    res2 = read(pid2, &c2, 1);
                    if (res2 < 0) {
                        perror("Error in: read");
                        endConnection(pid1, pid2);
                        return ERROR;
                    }
                    //If reached a visible letter, different
                    if (!isWhiteSpace(c2)) {
                        endConnection(pid1, pid2);
                        return DIFFERENT;
                    }
                    //printf("%c %c || %d %d\n", c1,c2,c1,c2);
                } while (isWhiteSpace(c2) && res2 != 0);
            }
            // Both files only differ in num of spaces, therefore: similar.
            endConnection(pid1, pid2);
            return SIMILAR;
        }
        //endregion
        //region 3
        // If entered this section, files are not ident. Maybe similar.
        if(c1 != c2) {
            //read from each file
            while (isWhiteSpace(c1) && res1 != 0) {
                res1 = read(pid1, &c1, 1);
            }
            while (isWhiteSpace(c2) && res2 != 0) {
                res2 = read(pid2, &c2, 1);
            }
            while (TRUE) {
                if(res1 == 0 && res2 == 0) {
                    endConnection(pid1, pid2);
                    return SIMILAR;
                }
                if(res1 == 0 || res2 == 0) {
                    endConnection(pid1, pid2);
                    return DIFFERENT;
                }
                if (!sameLetter(c1,c2)) {
                    endConnection(pid1, pid2);
                    return DIFFERENT;
                }
                //read chars until we reach visible char
                do {
                    tmp = c1;
                    res1 = read(pid1, &c1, 1);
                    if (res1 < 0) {
                        perror("Error in: read");
                        endConnection(pid1, pid2);
                        return -1;
                    }
                } while (isWhiteSpace(c1) && res1 != 0);
                if (res1 == 0)
                    c1 = tmp;
                //read chars until we reach visible char
                do {
                    tmp = c2;
                    res2 = read(pid2, &c2, 1);
                    if (res2 < 0) {
                        perror("Error in: read");
                        endConnection(pid1,pid2);
                        return -1;
                    }
                } while ( isWhiteSpace (c2) && res2 != 0);
                if (res2 == 0)
                    c2 = tmp;
            }
        }
        //endregion
    }
}

/**
 * Is capital letter?
 * @param c char
 * @return if 'A'<=c<='Z'
 */
bool isCapLetter(char c) {
    if (c >= 65 && c <= 90)
        return 1;
    return 0;
}

/**
 * Is low letter?
 * @param c char
 * @return if 'a'<=c<='z'
 */
bool isLowLetter(char c) {
    if (c >= 97 && c <= 122)
        return 1;
    return 0;
}

/**
 * are c1 and c2 same letter s.a: a,A, or both white.
 * @param c1
 * @param c2
 * @return if are both same letter or both white.
 */
bool sameLetter(char c1, char c2) {
    if (c1==c2)
        return 1;
    if (isLowLetter(c1) && (c1 == c2 + CAP_LOW_DIFF))
        return 1;
    if (isLowLetter(c2) && (c2 == c1 + CAP_LOW_DIFF))
        return 1;
    if (isWhiteSpace(c1) && isWhiteSpace(c2))
        return 1;
    return 0;
}
/**
 * is c an invisible char?
 * @param c1 char
 * @return true if invisible.
 */
bool isWhiteSpace(char c1) {
    if (' ' == c1)
        return 1;
    if ('\n' == c1)
        return 1;
    if ('\t' == c1)
        return 1;
    if ('\r' == c1)
        return 1;
    if ('\v' == c1)
        return 1;
    if ('\f' == c1)
        return 1;
    return 0;
}
