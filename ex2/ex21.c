// Lior Agron 208250225 
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>

#define CASE_DIFF (('a') - ('A'))
#define ABS(x) ((x) < 0 ? (-x) : x)
#define SAME_CON(stat1, stat2, sameFlag) (((stat1 != 0) || (stat2 != 0)) && sameFlag)
#define SIM_CON(stat1, stat2, simFlag) (((stat1 != 0) && (stat2 != 0)) && simFlag)
#define ERR_CON(stat1, stat2) ((stat1 == -1) || (stat2 == -1))


int isSimilar(char a, char b) {
    if (ABS(a - b) == CASE_DIFF) return 1;
    return 0;
}

int callError(int a, int b) {
        perror("Error");
        close(a);
        close(b);
        exit(-1);
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        perror("Not enough arguments");
        exit(-1);
    }
    int simFlag = 1;
    int sameFlag = 1;

    int f1 = open(argv[1], O_RDONLY);
    int f2 = open(argv[2], O_RDONLY);

    if (f1 == -1 || f2 == -1) {
        callError(f1, f2);
    }

    char p1;
    char p2;

    int stat1 = read(f1, &p1, 1);
    int stat2 = read(f2, &p2, 1);

    while (SAME_CON(stat1, stat2, sameFlag)) {

        if (ERR_CON(stat1, stat2)) {
            callError(f1, f2);
        }

        if (p1 != p2) {
            sameFlag = 0;
            break;
        }
    
        stat1 = read(f1, &p1, 1);
        stat2 = read(f2, &p2, 1);
    }


// change it to be a && con so that if one stat is 0, we'll check if the other has non-space chars left
    while (SIM_CON(stat1, stat2, simFlag)) {

        if (ERR_CON(stat1, stat2)) {
            callError(f1, f2);
        }

        while ((isspace(p1) != 0) && stat1 != 0) {
            stat1 = read(f1, &p1, 1);
        }
        
        while ((isspace(p2) != 0) && stat2 != 0) {
            stat2 = read(f2, &p2, 1);
        }

        if (p1 != p2) {
            simFlag = (ABS((p1 - p2)) == CASE_DIFF);
            if (simFlag == 0) {
                break;
            }
        }

        stat1 = read(f1, &p1, 1);
        stat2 = read(f2, &p2, 1);
    }

    if ((stat1 == 0) && (stat2 != 0)) {
        while ((isspace(p2) != 0) && stat2 != 0) {
            stat2 = read(f2, &p2, 1);
        }
        if ((isspace(p2) == 0)) {
            simFlag = 0;
        }
    }

    if ((stat1 != 0) && (stat2 == 0)) {
        while ((isspace(p1) != 0) && stat1 != 0) {
            stat1 = read(f1, &p1, 1);
        }
        if ((isspace(p1) == 0)) {
            simFlag = 0;
        }
    }

    close(f1);
    close(f2);

    if (sameFlag == 1) return 1;
    if (simFlag == 1) return 3;
    return 2;
}