#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <unistd.h>
#include <ftw.h>
#include <sys/stat.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/signal.h>
#include <time.h>

const char *homedir = "/home/wakib"; //home directory
int searchResult = 0;

//these variables are all for finding files by name
char *filename; //file name for the w24fn filename function
char filefound[255];    //if file is found this will be sent back to the client

//these variablees are all for finding files by size
int fSizeL = 0; //file size lower limit
int fSizeU = 0; //file size upper limit
char filePathByS[200000];  //contains all the file paths
int fdsCounter = 0;

//these variablees are all for finding files by date
char *userGivenDate1;   //date given by client for w24fdb
char *userGivenDate2;   //date given by client for w24fda
char filePathByD[200000];   //if file is found this will be sent back to the client
time_t epochTimeB=0;
int fdbCounter = 0;
char filePathByA[100000];   //if file is found this will be sent back to the client
time_t epochTimeA=0;
int fdaCounter = 0;

char* extensionToCheck;
int taskStatus=0;
char systemCommand[200000];
int fdtCounter = 0;

//this function stores the user's inputs in an argument array
void getArgs(char *inp, char *args[]){

    char *token; //variable to store argument
    int count = 0; //index of argArray
    char *inpCpy = strdup(inp);
    // printf("\ninpCpy: %s\n", inpCpy);

    token = strtok(inpCpy, " ");  //tokenizing using space
    while(token != NULL){
        // printf("\nToken at getArgs is: %s\n", token);
        args[count] = token;
        // printf("\nargArray[%d]: %s", count, args[count]);

        token = strtok(NULL, " ");
        count++;
    }
    
}

//a very basic function to return whatever is after the last slash in the file path [reused from Assignment 1]
char *getFileName(const char *p) {
    char *slash = strrchr(p, '/'); //looks for last occurence of "/" in the file path
    if (slash){
        return strdup(slash + 1); //moving the cursor one position forward so that the "/" doesn't get returned
    } 
    else{
        printf("\nInvalid File Path.\n");
        return strdup(p);
    }
}

//function for creating tar file for w24fz
int createTarSize(char *fSPaths){

    char *tarCreate = (char*)malloc(strlen("tar --transform='flags=r;s|.*/||S' -czf ") + strlen("./arch-size.tar.gz ") + strlen(fSPaths) + 1);

    strcpy(tarCreate, "tar --transform='flags=r;s|.*/||S' -czf ");
    strcat(tarCreate, "./arch-size.tar.gz ");
    strcat(tarCreate, fSPaths);

    int st = system(tarCreate);

    free(tarCreate);

    return 0;
}

//function for creating tar file for w24fdb, similar to the one above
int createTarBeforeDate(char *fSPaths){

    char *tarCreate = (char*)malloc(strlen("tar --transform='flags=r;s|.*/||S' -czf ") + strlen("./arch-db.tar.gz ") + strlen(fSPaths) + 1);

    strcpy(tarCreate, "tar --transform='flags=r;s|.*/||S' -czf ");
    strcat(tarCreate, "./arch-db.tar.gz ");
    strcat(tarCreate, fSPaths);

    int st = system(tarCreate);

    free(tarCreate);

    return 0;
}

//function for creating tar file for w24fda, similar to the ones above
int createTarAfterDate(char *fSPaths){

    char *tarCreate = (char*)malloc(strlen("tar --transform='flags=r;s|.*/||S' -czf ") + strlen("./arch-da.tar.gz ") + strlen(fSPaths) + 1);

    strcpy(tarCreate, "tar --transform='flags=r;s|.*/||S' -czf ");
    strcat(tarCreate, "./arch-da.tar.gz ");
    strcat(tarCreate, fSPaths);

    int st = system(tarCreate);

    free(tarCreate);

    return 0;
}

int createTarWithSameExt(char *fSPaths){

    char *tarCreate = (char*)malloc(strlen("tar --transform='flags=r;s|.*/||S' -czf ") + strlen("./arch-x.tar.gz ") + strlen(fSPaths) + 1);

    strcpy(tarCreate, "tar --transform='flags=r;s|.*/||S' -czf ");
    strcat(tarCreate, "./arch-x.tar.gz ");
    strcat(tarCreate, fSPaths);

    int st = system(tarCreate);

    free(tarCreate);

    return 0;
}

//function for sending tar file to the client
int sendTar(int cfd, char *filep){

    //opening the tar file for reading
    FILE *tarFile = fopen(filep, "rb");
    if (tarFile == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    //getting the size of the tar file so that it can be sent to the client
    fseek(tarFile, 0, SEEK_END);
    long tarFileSize = ftell(tarFile);
    rewind(tarFile);

    //sending the tar file size to the client before sending the tar file
    if (send(cfd, &tarFileSize, sizeof(tarFileSize), 0) < 0) {
        perror("Error sending file size");
        exit(1);
    }

    //read from the tar file byte by byte and sending  it byte by byte
    char buffer[1024];
    size_t bytesRead;
    while ((bytesRead = fread(buffer, 1, 1024, tarFile)) > 0) {
        //using the send() function to send the tar file to the client
        if (send(cfd, buffer, bytesRead, 0) < 0) {
            perror("Error sending");
            exit(1);
        }
    }

    fclose(tarFile);
    return 0;
}


//function which handles the "dirlist -a" client request
char* listDirectoriesAlphabetically() {
    char* directories[128];
    int arrayIndex = 0;
    //define and storing the command
    char command[100];
    sprintf(command, "find %s/* -maxdepth 0 -type d -not -path '/.'", homedir);

    //this is the comparator function for qsort
    int compareStrings(const void *a, const void *b) {
        return strcmp(*(const char **) a, *(const char **) b);
    }

    //executeing the command and opening a file stream to read its output
    FILE *strm = popen(command, "r");

    if (!strm) {
        perror("popen");
        exit(EXIT_FAILURE);
    }

    //reading and processing the output line by line
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), strm) != NULL) {
        //finding the last occurrence of the character "/"
        char *last_slash = strrchr(buffer, '/');
        if (last_slash) {
            //if the new line character is present, removing the newline character
            char *newline = strchr(last_slash, '\n');
            if (newline)
                *newline = '\0';

            directories[arrayIndex] = malloc(strlen(last_slash) + 1);
            if (directories[arrayIndex] == NULL) {
                perror("malloc");
                exit(EXIT_FAILURE);
            }
            //copying the directory name
            strcpy(directories[arrayIndex], last_slash + 1);
            arrayIndex++;
        }
    }

    //closing the file stream
    pclose(strm);

    //sorting the directory names alphabetically using qsort
    qsort(directories, arrayIndex, sizeof(char *), compareStrings);

    
    size_t result_len = 1; // for the null terminator
    for(int i = 0; i < arrayIndex; i++) {
        result_len += strlen(directories[i]) + 1; // extra 1 for newline character
    }
    char* result = malloc(result_len);
    if (result == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    //concatenating the directory names into the result string
    result[0] = '\0'; 
    for(int i = 0; i < arrayIndex; i++) {
        strcat(result, directories[i]);
        strcat(result, "\n"); 
        free(directories[i]); 
    }
    return result;
}


//function which handles the "dirlist -t" client request
char* listDirectoriesByTime(){
    #define MAX_DIRECTORIES 128

    //struct object to hold directory information
    struct DirectoryInfo {
        char name[256];
        time_t accessTime;
    };

    ////this is the comparator function for qsort
    int compareDirectories(const void *a, const void *b) {
        const struct DirectoryInfo *dir_info_a = (const struct DirectoryInfo *)a;
        const struct DirectoryInfo *dir_info_b = (const struct DirectoryInfo *)b;
        return dir_info_a->accessTime - dir_info_b->accessTime;
    }
    
    char command[100];

    sprintf(command, "find %s/* -maxdepth 0 -type d -not -path '*/.*' -exec stat -c '%%n %%X' {} +", homedir);

    
    FILE *strm = popen(command, "r");
    if (!strm) {
        perror("popen");
        exit(EXIT_FAILURE);
    }

    //declaring an array of type struct to store directory information
    struct DirectoryInfo directories[MAX_DIRECTORIES];
    int num_directories = 0;

    
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), strm) != NULL && num_directories < MAX_DIRECTORIES) {
        
        char *last_slash = strrchr(buffer, '/');
        if (last_slash) {
            
            char *folder_name = last_slash + 1;
            
            
            char *newline = strchr(folder_name, '\n');
            if (newline) {
                *newline = '\0';
            }

            char folder_name_raw[256];
            time_t accessTime;
            if (sscanf(last_slash+1, "%255s %ld", folder_name_raw, &accessTime) == 2) {
                // printf("%s----\n", folder_name_raw);
                // printf("%ld-\n", accessTime);
            }

            
    
            strcpy(directories[num_directories].name, folder_name_raw);
            directories[num_directories].accessTime = accessTime;
            num_directories++;
        }
    }

    
    pclose(strm);

    //sorting the directory information array based on access time
    qsort(directories, num_directories, sizeof(struct DirectoryInfo), compareDirectories);

   
    size_t result_len = 1; 
    for(int i = 0; i < num_directories; i++) {
        result_len += strlen(directories[i].name) + 1; 
    }
    char* result = malloc(result_len);
    if (result == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    
    result[0] = '\0'; 
    for(int i = 0; i < num_directories; i++) {
        strcat(result, directories[i].name);
        strcat(result, "\n"); 
    }

    return result;
}

//the callback function for nftw, using it for searching file by name [reused from Assignment 1]
int fileNameSearch(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf){

    searchResult = 0;

    //seperating the file name from the path and storing it in a variable
    char *justFileName = getFileName(fpath);

    //in the following conditional statement I'm checking if the current path which is being traversed is a path to a file. For this I'm checking if the typeflag is equal to FTW_F which according to the documentation of nftw, indicates that it is a file.
    //I'm also comparing if the justFileName and filename match
    if (typeflag == FTW_F && strcmp(justFileName, filename) == 0){

        //storing file name
        strcpy(filefound, "File name: ");
        strcat(filefound, justFileName);
        strcat(filefound, "\n");

        //storing file size
        struct stat fileStat;
        char fileSize[20];
        char fileDate[30];
        char filePerm[15];

        //storing the file size(in bytes), fize creation date and file permission respectively
        if (stat(fpath, &fileStat) == 0) {
            sprintf(fileSize, "%lld", (long long)fileStat.st_size);
            strftime(fileDate, sizeof(fileDate), "%Y-%m-%d %H:%M:%S", localtime(&fileStat.st_ctime));
            snprintf(filePerm, sizeof(filePerm), "%o", fileStat.st_mode & 0777);
        } 
        else {
            searchResult = 0;
            return 1;
        }

        //storing file size, date and permissions
        strcat(filefound, "File size: ");
        strcat(filefound, fileSize);
        strcat(filefound, " bytes");
        strcat(filefound, "\n");
        strcat(filefound, "File creation date: ");
        strcat(filefound, fileDate);
        strcat(filefound, "\n");
        strcat(filefound, "File permissions: ");
        strcat(filefound, filePerm);

        free(justFileName);
        searchResult = 1;
        return 1;
    }

    free(justFileName);

    return 0;
}

//the callback function for nftw, using it for searching file by size [reused from Assignment 1]
int fileSizeSearch(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf){

    if (strstr(fpath, "/workspace/") == NULL && strstr(fpath, "/.") == NULL) // Ignore folders named "workspace"
    {
        if (typeflag == FTW_F){
            off_t fsize = sb->st_size;
            
            if (fsize >= fSizeL && fsize <= fSizeU)
            {
                // char *temp = (char*)realloc(filePathByS, (strlen(fpath) + strlen(" ") + 1));
                // filePathByS = temp;
                // printf("%s",fpath);
                strcat(filePathByS, fpath);
                strcat(filePathByS, " ");
                

                fdsCounter++;

                if (fdsCounter >= 100) {
                    // Return a non-zero value to stop traversal
                    return 1;
                }
            }
            searchResult=1;
        }
    }

    return 0;
}

//finding files with same extension
int getFilesWithSameExt(const char *filepath) {
    size_t fpath_len = strlen(filepath); //getting the length of the filepath
    size_t ext_len = strlen(extensionToCheck); //getting the length of the extension

    //if the length of the filepath is less than the extension then it will return 0 which means there is a mismatch.
    if (fpath_len < ext_len) {
        return 0;
    }
    return strcmp(filepath + fpath_len - ext_len, extensionToCheck); //otherwise it will seperate the portion of the extension and then it will compare with the extension. if the extension matches then it will return a 0 otherwise, it will return 1.
}

int findFilesByExtension(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf){
    if (strstr(fpath, "/workspace/") == NULL && strstr(fpath, "/.") == NULL) // Ignore folders named "workspace"
    {
        if (typeflag == FTW_F) { // Check if the traversed entry is a regular file
            if (getFilesWithSameExt(fpath) == 0) { // Check if the file has the same extension
                strcat(systemCommand, fpath);
                strcat(systemCommand, " ");
            }
        }
    }
    if(getFilesWithSameExt(systemCommand) == 0) { //then this function will all the file with the user specified extension has been picked or not in the systemCommand.
        taskStatus = 1; //if the function returns 0 then it indicates success.
    }
    return 0; //otherwise it will return zero and keep traversing
}

//function to search files before date
int filesByBeforeDateSearch(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf){
    // 
    if (strstr(fpath, "/workspace/") == NULL && strstr(fpath, "/.") == NULL) // Ignore folders named "workspace"
    {
        if (typeflag == FTW_F) {
        time_t accessTime = sb->st_atime; // access time of the file
        struct tm tm;

        if (strptime(userGivenDate1, "%Y-%m-%d", &tm) != NULL) {
            //setting the time given by the client to 00:00:00
            tm.tm_hour = 0;
            tm.tm_min = 0;
            tm.tm_sec = 0;
            epochTimeB = mktime(&tm);
            // fprintf(stdout, "epochtime: %d\n", epochTimeB);
        } else {
            printf("Error: Unable to parse the time string.\n");
        }
        //checking the time given by the client with the access time of the files being traversed
        if (accessTime <= epochTimeB) {
            //if condition is true saving the file paths
            strcat(filePathByD, fpath);
            strcat(filePathByD, " ");
            searchResult = 1;

            fdbCounter++;

            
            if (fdbCounter >= 100) {
                // Return a non-zero value to stop traversal
                return 1;
            }
        }
        }
    }

    return 0;
}

//function to search files before date, almost exactly similar to the previous function just a minor change in checking logic
int filesByAfterDateSearch(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf){

    // char *justFileName = getFileName(fpath);

    // if (justFileName[0] == '.') {
    //     return 0; // Skip hidden files
    // }

    if (strstr(fpath, "/workspace/") == NULL && strstr(fpath, "/.") == NULL){
    if (typeflag == FTW_F) {

        time_t accessTime = sb->st_atime; // access time of the file
        struct tm tm;

        if (strptime(userGivenDate2, "%Y-%m-%d", &tm) != NULL) {
            //setting the time given by the client to 00:00:00
            tm.tm_hour = 0;
            tm.tm_min = 0;
            tm.tm_sec = 0;
            epochTimeA = mktime(&tm);
            // fprintf(stdout, "epochtime: %d\n", epochTimeA);
        } else {
            printf("Error: Unable to parse the time string.\n");
        }
        //checking the time given by the client with the access time of the files being traversed
        if (accessTime >= epochTimeA) {
            //if condition is true saving the file paths
            strcat(filePathByA, fpath);
            strcat(filePathByA, " ");
            searchResult = 1;

            fdaCounter++;

            // If 15 files have been stored, stop traversal
            if (fdaCounter >= 100) {
                // Return a non-zero value to stop traversal
                return 1;
            }
        }
    }
    }

    return 0;
}

//function to handle client requests
int crequest(int cfd){
    char msg[255];  //client's message
    // default server message to the client
    char* serverMsg = "Connected to the server...";
    char* invalid = "Try again";    //message to client in case of invalid input
    write(cfd, serverMsg, strlen(serverMsg) + 1);
    
    for (;;) {

        filefound[0] = '\0';
        
        fprintf(stdout, "Waiting for client request...\n");

        if (!read(cfd, msg, 255)) {
            //if client has quit, closing the socket and waiting for a new client
            close(cfd);
            fprintf(stdout, "Client has quit. Waiting for another client...\n");
            exit(0);
        }
        //checking if the client has quit or not
        else if (strcmp(msg, "quitc") == 0) {
            //if client has quit, closing the socket and waiting for a new client
            close(cfd);
            fprintf(stdout, "Client has quit. Waiting for another client...\n");
            exit(0);
        }

        //incase client gives an invalid input
        else if (strstr(msg, "invalid") != 0)
        {
            write(cfd, invalid, strlen(invalid) + 1);
        }
        
        //functionalities begin here
        else if(strcmp(msg, "dirlist -a") == 0){
            fprintf(stdout, "Handling the case of: %s\n", msg);
            char* singleDirectory=listDirectoriesAlphabetically();
            write(cfd, singleDirectory, strlen(singleDirectory) + 1);
        }
        else if (strcmp(msg, "dirlist -t") == 0)
        {
            fprintf(stdout, "Handling the case of: %s\n", msg);
            char* singleDirectory=listDirectoriesByTime();
            write(cfd, singleDirectory, strlen(singleDirectory) + 1);
        }
        //file search by name
        else if (strstr(msg, "w24fn") != 0)
        {
            fprintf(stdout, "Handling the case of: %s\n", msg);
            char *argArray[2];
            getArgs(msg, argArray);

            filename = argArray[1];

            nftw(homedir, fileNameSearch, 50, FTW_PHYS);

            if (searchResult == 1)
            {
                write(cfd, filefound, strlen(filefound) + 1);
            }
            else{
                char *notFound = "File not found";
                write(cfd, notFound, strlen(notFound) + 1);
            }
        }

        //file search by size and send tar.gz file containing all files that fall within the given size range
        else if (strstr(msg, "w24fz") != 0)
        {
            fprintf(stdout, "Handling the case of: %s\n", msg);
            char *argArray[3];
            // filePathByS = (char*)malloc(1);
            // strcpy(filePathByS, "");
            getArgs(msg, argArray);

            fSizeL = atoi(argArray[1]);
            fSizeU = atoi(argArray[2]);

            // printf("%d\n",fSizeL);
            // printf("%d\n",fSizeU);

            nftw(homedir, fileSizeSearch, 50, FTW_PHYS);

            if (searchResult == 1)
            {
                // printf("%s",filePathByS);
                if (createTarSize(filePathByS) == 0)
                {
                    char *tarSPath = "./arch-size.tar.gz";  //tar file path for this specific functionality

                    if (sendTar(cfd, tarSPath) == 0)
                    {
                        //removing the tar file from server side after it has been sent to the client
                        remove(tarSPath);
                        char *tarSent = "Tar file received from server\n";
                        write(cfd, tarSent, strlen(tarSent) + 1);
                    }
                }
            }
            else{
                char *notFound = "Files not found";
                write(cfd, notFound, strlen(notFound) + 1);
                // free(filePathByS);
            }
        }
        //file search before date and send tar to client
        else if (strstr(msg, "w24fdb") != 0)
        {
            searchResult = 0;
            fprintf(stdout, "Handling the case of: %s\n", msg);

            char *argArray[2];
            getArgs(msg, argArray);
            
            userGivenDate1 = argArray[1];

            // printf("date: %s\n", userGivenDate1);

            nftw(homedir,filesByBeforeDateSearch, 30, FTW_PHYS);


            if (searchResult == 1)
            {
                // fprintf(stdout,"%d",searchResult);
                // fprintf(stdout, "%s\n",filePathByD);
                if (createTarBeforeDate(filePathByD) == 0)
                {
                    char *tarSPath = "./arch-db.tar.gz";  //tar file path for this specific functionality

                    if (sendTar(cfd, tarSPath) == 0)
                    {
                        // printf("In sendtar");
                        userGivenDate1="";
                        //removing the tar file from server side after it has been sent to the client
                        remove(tarSPath);
                        char *tarSent = "Tar file received from server\n";
                        write(cfd, tarSent, strlen(tarSent) + 1);
                    }
                }
            }
            else{
                // fprintf(stdout, "%s\n",filePathByD);
                // fprintf(stdout,"%d",searchResult);
                char *notFound = "Files not found";
                write(cfd, notFound, strlen(notFound) + 1);
                // free(filePathByS);
            }
        }
        //file search after date and send tar to client
        else if (strstr(msg, "w24fda") != 0)
        {
            searchResult = 0;
            fprintf(stdout, "Handling the case of: %s\n", msg);

            char *argArray[2];
            getArgs(msg, argArray);
            
            userGivenDate2 = argArray[1];

            // printf("date: %s\n", userGivenDate1);

            nftw(homedir,filesByAfterDateSearch, 30, FTW_PHYS);


            if (searchResult == 1)
            {
                // fprintf(stdout,"%d",searchResult);
                // fprintf(stdout, "%s\n",filePathByD);
                if (createTarAfterDate(filePathByA) == 0)
                {
                    char *tarSPath = "./arch-da.tar.gz";  //tar file path for this specific functionality

                    if (sendTar(cfd, tarSPath) == 0)
                    {
                        // printf("In sendtar");
                        userGivenDate1="";
                        //removing the tar file from server side after it has been sent to the client
                        remove(tarSPath);
                        char *tarSent = "Tar file received from server\n";
                        write(cfd, tarSent, strlen(tarSent) + 1);
                    }
                }
            }
            else{
                // fprintf(stdout, "%s\n",filePathByD);
                // fprintf(stdout,"%d",searchResult);
                char *notFound = "Files not found";
                write(cfd, notFound, strlen(notFound) + 1);
                // free(filePathByS);
            }
        }
        else if(strstr(msg,"w24ft")!=0){
            char *extensionArray[5]; // Array to store extensions
            getArgs(msg, extensionArray); // Tokenize clientInp and store tokens in extensionArray
        
            // Print each token in extensionArray
            // printf("Extensions:\n");
            for (int i = 1; extensionArray[i] != NULL; i++) {
                printf("%s\n", extensionArray[i]);
                extensionToCheck=extensionArray[i];
                nftw(homedir,findFilesByExtension,50,FTW_PHYS);
                extensionToCheck="";
            }

            if(taskStatus==1){
                // printf("%s",systemCommand);
                // system(systemCommand);
                // printf("Tar file has been created successfully! \n");
                // free(systemCommand);
                // exit(0);
                if (createTarWithSameExt(systemCommand) == 0)
                {
                    // printf("%s",systemCommand);
                    char *tarSPath = "./arch-x.tar.gz";  //tar file path for this specific functionality

                    if (sendTar(cfd, tarSPath) == 0)
                    {
                        // printf("In sendtar");
                        // userGivendate="";
                        //removing the tar file from server side after it has been sent to the client
                        remove(tarSPath);
                        char *tarSent = "Tar file received from server\n";
                        write(cfd, tarSent, strlen(tarSent) + 1);
                    }
                }
            }
            else{
                // fprintf(stdout, "%s\n",filePathByD);
                // fprintf(stdout,"%d",searchResult);
                char *notFound = "Files not found";
                write(cfd, notFound, strlen(notFound) + 1);
                // free(filePathByS);
            }
        }
        else{
            //displaying the message sent by the client
            fprintf(stdout, "Client's message: %s\n", msg);
        } 
        
    }
}

int main(int argc, char *argv[]){
    int serverFd, portNum, clientFd; //server file descriptor, port number and client file descriptor respectively
    struct sockaddr_in sAdrStr; //server socket address structure object
    // socklen_t serSockLen; //server socket address structure length

    //socket creation, socket domain = AF_INET and socket type = SOCK_strm aka TCP
    if((serverFd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        //socket creation failed
        printf("Error while creating socket\n");
        exit(1);
    }

    //creating server socket address structure 
    sAdrStr.sin_family = AF_INET;
    sAdrStr.sin_addr.s_addr = htonl(INADDR_ANY);    //converting the 32 bit host byte order of IP address to network byte order
    sAdrStr.sin_port = htons(7999);     //converting the 16 bit host byte order of port number to network byte order

    //binding the socket to the server address structure
    bind(serverFd, (struct sockaddr*) &sAdrStr, sizeof(sAdrStr));

    //listening for clients
    listen(serverFd, 100);

    

    for (;;) {
        // Accepting incoming connections
        clientFd = accept(serverFd, (struct sockaddr*) NULL, NULL);
        printf("Client Found\n");

        //forking to handle the client client request in a separate child process
        int pid = fork();
        if (pid == 0){
            crequest(clientFd);
        }
        else if(pid < 0){
            printf("Fork Error\n");
        }
        else{
            //closing the accepted client's socket descriptor and going back to the loop to listen to more client requests
            close(clientFd);
            
        }
    }
}