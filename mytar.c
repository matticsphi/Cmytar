/*
 *  mytar.c
 *  
 *
 *  Created by Mattics on 5/15/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include "mytar.h"

#define LENGTH 2048

void otherFunctions(int argc, char *argv[]);
void checkOptions(int argc, char *argv[]);
int createTar(int options[], char *argv[]);
void printTable(int options[], char *currentDirectory, struct stat root);
DIR *openDirectory(char *currentDirectory);
struct stat lStatRoot(char *rootDirectory);
void printFilePath(struct stat root);
struct stat getCurrentDirectory();
struct stat getParentDirectory();
int scanFiles(DIR *directory, struct stat currentDirectory, int numDirectories);
void printPath(int numDirectories);

static char **pathname;

/*******************************************************/

int main(int argc, char *argv[])
{
    pathname = malloc(sizeof(*pathname) * (LENGTH + 1));
    otherFunctions(argc, argv);
    
    return 0;
}

/*******************************************************/

void otherFunctions(int argc, char *argv[])
{
    checkOptions(argc, argv);
}

/*******************************************************/

void checkOptions(int argc, char *argv[])
{
    int i = 0, options[5], newFile;
    
    if (argc < 3) {
        perror("Usage: mytar [ctxvS]f tarfile [ path [ ...  ]  ]\n");
        exit(-1);
    } else if (argc >= 3) {
        while (i <= (strlen(argv[1]) - 2)) {
            if (argv[1][i] == 'c') {
                options[0] = 1;
            } else if (argv[1][i] == 't') {
                options[1] = 1;
            } else if (argv[1][i] == 'x') {
                options[2] = 1;
            } else if (argv[1][i] == 'v') {
                options[3] = 1;
            } else if (argv[1][i] == 'S') {
                options[4] = 1;
            } else {
                perror("Usage: mytar [ctxvS]f tarfile [ path [ ...  ]  ]\n");
                exit(-2);
            }
            i++;
        }
    }
    
    
    
    if (options[0] == 1) {
        newFile = createTar(options, argv);
    } else if (options[1] == 1) {
        
        /* Sends in lstat of root as third parameter so we can print the path
         * to the root file
         */
        printTable(options, argv[3], lStatRoot(argv[3]));
    }
    
    
    
    
}
               
/*******************************************************/

int createTar(int options[], char *argv[])
{
    int fp;
    
    fp = open(argv[3], O_RDWR | O_CREAT | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO);
    
    return fp;
}

/*******************************************************/

void printTable(int options[], char *currentDirectory, struct stat root)
{   
    DIR *dir;
    struct dirent *file;
    struct stat file1;
    
    /* Opens current directory */
    dir = openDirectory(currentDirectory);
    
    while ((file = readdir(dir)) != NULL) {
        if (lstat(file->d_name, &file1)) {
            perror("cannot get current file");
            exit(-4);
        }
        
        if (file->d_name[0] == '.') {
            continue;
        } else if (S_ISDIR(file1.st_mode)) {
            
            /*printf("directory name: %s\n", file->d_name);*/
            printFilePath(root);
            
            dir = openDirectory(currentDirectory);
            
            printTable(options, file->d_name, root);
            
        } else {
            
            /*printf("file name: %s\n", file->d_name);*/
            printFilePath(root);
            
            dir = openDirectory(currentDirectory);
            
            printTable(options, file->d_name, root);
            
        }
    }
    closedir(dir);
    
    if (chdir("..")) {
        perror("cannot change directory");
        exit(-6);
    }
}

/*******************************************************/

/* Opens current directory */
DIR *openDirectory(char *currentDirectory)
{
    DIR *dir;
    
    if ((dir = opendir(currentDirectory)) == NULL) {
        perror("opening directory");
        exit(-3);
    }
    
    if (chdir(currentDirectory)) {
        perror("cannot change directory");
        exit(-5);
    }
    
    return dir;
}

/*******************************************************/

/* LSTATS root directory */
struct stat lStatRoot(char *rootDirectory)
{
    struct stat root;
    
    if (lstat(rootDirectory, &root)) {
        perror("mypwd\n");
        exit(0);
    }
    
    return root;
}

/*******************************************************/

void printFilePath(struct stat rootDirectory)
{
    int numDirectories = 0, i;
    DIR *directory;
    struct stat currentDirectory, parentDirectory;
    
    for (i = 0; i < LENGTH; i++) {
        pathname[i] = NULL;
    }
    
    while ((directory = opendir("..")) != NULL) {
        
        currentDirectory = getCurrentDirectory();
        
        /* Changes directory to parent */
        if (chdir("..")) {
            perror("mypwd\n");
            exit(0);
        }
        
        parentDirectory = getParentDirectory();
        
        numDirectories = scanFiles(directory, currentDirectory, numDirectories);
        
        /* Checks to see if parent directory is root */
        if ((rootDirectory.st_dev == parentDirectory.st_dev) &&
            (rootDirectory.st_ino == parentDirectory.st_ino)) {
            break;
        }
    }
    
    printPath(numDirectories);
}

/*******************************************************/

struct stat getCurrentDirectory()
{
    struct stat current;
        
    if (lstat(".", &current)) {
        perror("mypwd\n");
        exit(0);
    }
        
    return current;
}

/*******************************************************/

struct stat getParentDirectory()
{
    struct stat parent;
    
    if (lstat(".", &parent)) {
        perror("mypwd\n");
        exit(0);
    }
    
    return parent;
}

/*******************************************************/

int scanFiles(DIR *directory, struct stat currentDirectory, int numDirectories)
{
    struct dirent *file;
    struct stat direct;
    
    /* Scan in each file in directory to find child */
    while ((file = readdir(directory)) != NULL) {
        if (stat(file->d_name, &direct)) {
            perror("mypwd\n");
        }
        
        if (direct.st_ino == currentDirectory.st_ino &&
            direct.st_dev == currentDirectory.st_dev) {
            if ((strcmp(file->d_name, "..") != 0) &&
                strcmp(file->d_name, ".") != 0) {
                pathname[numDirectories] = file->d_name;
                printf("path: %s\n", pathname[numDirectories]);
                numDirectories++;
            }
        }
    }
    
    return numDirectories;
}

/*******************************************************/

void printPath(int numDirectories)
{
    /* Prints out the path */
    if (numDirectories == 0) {
        printf("/\n");
    } else if (numDirectories > LENGTH) {
        printf("Path too long.\n");
        exit(0);
    }else {
        for (numDirectories -= 1; numDirectories >= 0; numDirectories--) {
            printf("/%s", pathname[numDirectories]);
        }
        printf("\n");
    }
}

/*******************************************************/
/*******************************************************/
/*******************************************************/
/*******************************************************/
/*******************************************************/
/*******************************************************/
/*******************************************************/
/*******************************************************/
/*******************************************************/
/*******************************************************/
/*******************************************************/
/*******************************************************/
/*******************************************************/
/*******************************************************/
/*******************************************************/
/*******************************************************/
/*******************************************************/