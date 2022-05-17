// Lior Agron 208250225 
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/wait.h>
#include <sys/stat.h>

#define MAX_CHARS 150
#define CLOSE_ERR "close() failed"
#define OPEN_ERR "open() failed"


void handleErr(char* err) {
    // write(fd, err, strlen(err));
    perror(err);
}

void endScript(int fd1, int fd2, int stat) {
    close(fd1);
    close(fd2);
    exit(stat);
}

int isC(char* file) {
    int fLen = strlen(file);
    if (fLen < 3) {
        return 0;
    }
    return strncmp(file + fLen - 2, ".c", 2) == 0;
}

void gradeStudent(int fd, char* name, char* path, char* expectedPath, int grade) {
    char line[150];
    if(grade == 0) {
        strcpy(line, name);
        strcat(line, ",0,NO_C_FILE\n");
        return;
    }
    if(grade == 10) {
        strcpy(line, name);
        strcat(line, ",10,COMPILATION_ERROR\n");
    }
    else {
        char outputPath[150];
        strcpy(outputPath, path);
        strcat(outputPath, "/");
        strcat(outputPath, "output.txt");
        
        int pid = fork();
        if (pid == -1) {
            handleErr("fork() failed");
        } else if (pid == 0) {
            // printf("%s\n", newdirPath);
            char* args[] = {"./comp.out", expectedPath, outputPath, NULL};
            if (execvp("./comp.out", args) == -1) {
                handleErr("exec failed");
            }
        }
        else {
            int status;
            wait(&status);
            if (WIFEXITED(status)) {
                if(WEXITSTATUS(status) == 3) {
                    strcpy(line, name);
                    strcat(line, ",50,WRONG\n");
                }
                if(WEXITSTATUS(status) == 2) {
                    strcpy(line, name);
                    strcat(line, ",75,SIMILAR\n");
                }
                if(WEXITSTATUS(status) == 1) {
                    strcpy(line, name);
                    strcat(line, ",100,EXCELLENT\n");
                }
                write(fd, line, strlen(line));
            }
        }
    }
}

void compileC(char* name, char* path) {
    char newdirPath[150];
    // strcpy(newdirPath, "./");
    strcpy(newdirPath, path);
    strcat(newdirPath, "/");
    strcat(newdirPath, name);

    long pid = fork();
        if (pid == -1) {
            handleErr("fork() failed");
        } else if (pid == 0) {
            // printf("%s\n", newdirPath);
            char* args[] = {"gcc", newdirPath, NULL};
            if (execvp("gcc", args) == -1) {
                handleErr("exec failed");
                // printf("%s from %s got: 10\n", name, newdirPath);
                // gradeStudent(-1);
            }
        }
        else {
            wait(NULL);
        }
}

void excecFile(int fd, char* name, char* path, char* inputPath) {
    char exePath[150];
    // strcpy(exePath, path);
    strcpy(exePath, "./");
    strcat(exePath, "a.out");

    char outputPath[150];
    strcpy(outputPath, path);
    strcat(outputPath, "/");
    strcat(outputPath, "output.txt");
    int inputFD = open(inputPath, O_RDONLY);
    int outputFD = open(outputPath, O_CREAT | O_RDWR, 0666);
    if ((inputFD < 0) || (outputFD < 0)) {
        handleErr(OPEN_ERR);
    }
    long pid = fork();
    if (pid == -1) {
        handleErr("fork() failed");
    } else if (pid == 0) {
        dup2(outputFD, STDOUT_FILENO);
        dup2(inputFD, STDIN_FILENO);
        if (access(exePath, F_OK) == 0) {

            char* args[] = {exePath, NULL};
            if ((execvp(exePath, args) == -1)) {
                handleErr("exec failed");
                exit(-1);
            }
        } else {
            write(outputFD, "-1", 2);
            gradeStudent(fd, name, path, "", 10);
            exit(-1);
        }
    }
    else {
        wait(NULL);
        if ((close(inputFD) < 0) || (close(outputFD) < 0)) {
            handleErr(CLOSE_ERR);
        }

    }
}

int main(int argc, char** argv) {

    if(argc < 2) {
        handleErr("not enough arguments");
        exit(-1);
    }

    if (access("./a.out", F_OK) == 0) {
        remove("./a.out");
    }

    int fdErr = open("errors.txt", O_CREAT | O_WRONLY | O_TRUNC, 0666);
    int fdRes = open("results.csv", O_CREAT | O_WRONLY | O_TRUNC, 0666);
    
    if ((fdErr == -1 || (fdRes == -1))) {
        handleErr("coulnd't open");
        exit(-1);
    }

    dup2(fdErr, STDERR_FILENO);

    char dirPath[MAX_CHARS];
    char inputPath[MAX_CHARS];
    char expectedOutputPath[MAX_CHARS];

    char buff[(MAX_CHARS * 3)];


    int config;
    if ((config = open(argv[1], O_RDONLY)) < 0) {
        handleErr("open failed");
    } 

    int i = 0;
    while (read(config, (buff + i), 1) != 0) {
        i++;
    }

    close(config);

    strcpy(dirPath, strtok(buff, "\n"));
    strcpy(inputPath, strtok(NULL, "\n"));
    strcpy(expectedOutputPath, strtok(NULL, "\n"));

    DIR* dir = opendir(dirPath);

    if (dir == NULL) {
        handleErr("opendir() failed\n");
    }

    struct dirent* entry;
    struct dirent* subEntry;

    while ((entry = readdir(dir)) != NULL) {
        char* dirName = entry->d_name;
        if ((strcmp(dirName, ".")) != 0 && (strcmp(dirName, "..")) != 0){
            char newdirPath[MAX_CHARS];
            strcpy(newdirPath, dirPath);
            strcat(newdirPath, "/");
            strcat(newdirPath, dirName);
            DIR* newDir = opendir(newdirPath);
            if (newDir == NULL) {
                handleErr("opendir() failed\n");
            }
            printf("Directory: %s\nFiles:", dirName);
            int cFileFlag = 0;
            while ((subEntry = readdir(newDir)) != NULL) {
                char* subName = subEntry->d_name;
                if (((strcmp(subName, ".")) != 0 && (strcmp(subName, "..")) != 0) && isC(subName) != 0){
                    cFileFlag = 1;
                    printf("\tname: %s\n", subName);
                    compileC(subName, newdirPath);
                    excecFile(fdRes, dirName, newdirPath, inputPath);
                    gradeStudent(fdRes, dirName, newdirPath, expectedOutputPath, 999);

                }
            }
            if (cFileFlag == 0) {
                char outputPath[150];
                strcpy(outputPath, newdirPath);
                strcat(outputPath, "/");
                strcat(outputPath, "output.txt");
                int inputFD = open(inputPath, O_RDONLY);
                int outputFD = open(outputPath, O_CREAT | O_RDWR, 0666);
                if ((inputFD < 0) || (outputFD < 0)) {
                    handleErr(OPEN_ERR);
                }
                write(outputFD, "0", 1);
                if ((close(outputFD) < 0)) {
                    handleErr(CLOSE_ERR);
                }
                gradeStudent(fdRes, dirName, newdirPath, expectedOutputPath, 0);
            }
            closedir(newDir);
        }
    }


    if (closedir(dir) == -1) {
        handleErr("closedir() failed\n");
    }
    close(fdErr);
}