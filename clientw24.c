#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/signal.h>
#include <stdbool.h>

#define MAX_inp_LENGTH 1024 //this is the client's input length
char *date = "";    

int conCount = 0;   //this is the connection count

//function for getting the client's input from stdin, [reused from previous assignment]
char *getInput() {

    char *inp = (char *)malloc(MAX_inp_LENGTH * sizeof(char));
    if (inp == NULL) {
        perror("\nerror in malloc\n");
        exit(1);
    }
    if (fgets(inp, MAX_inp_LENGTH, stdin) == NULL) {
        free(inp);
        return NULL;
    }
    //checking if the user pressed enter, if so adding EOF after input and returning it
    char *ent = strchr(inp, '\n');
    if (ent != NULL) {
        *ent = '\0';
    }
    return inp;
}

//this function checks the number of arguments in a command [reused from previous assignment]
int argNum(char *inp){

    //making a copy of the original inp variable so that the original does not get changed by strtok
    char *inpCpy = strdup(inp);

    char *token; 
    int numOfArg = 0; 

    token = strtok(inpCpy, " ");  //tokenizing using space
    while(token != NULL){
        // fprintf(stdout, "\nToken at countArgs is: %s\n", token);
        token = strtok(NULL, " ");
        numOfArg++;
        
    }

    return numOfArg;
    
}

//this function stores the user's inputs in an argument array [reused from previous assignment]
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

//checking if the date format entered by the client is correct or not
bool isValidDate(const char *date) {
    //checking if the string has 10 characters (YYYY-MM-DD) and if the character at index 4 and 7 are '-'.
    if (strlen(date) != 10 || date[4] != '-' || date[7] != '-')
        return false;

    //parsing year, month, day
    int year, month, day;
    if (sscanf(date, "%d-%d-%d", &year, &month, &day) != 3)
        return false;

    //checkin if date is within valid ranges
    if (year < 0 || month < 1 || month > 12 || day < 1 || day > 31)
        return false;

    //for months with less than 31 days and February for leap years
    if ((month == 4 || month == 6 || month == 9 || month == 11) && day > 30)
        return false;
    if (month == 2) {
        if (day > 29) //for Feb
            return false;
        if (day == 29 && (year % 4 != 0 || (year % 100 == 0 && year % 400 != 0)))
            return false; //Feb in leap year
    }

    return true;
}

//checking the syntax further
int checkSyn(char *clInp, char *option){

    //further checking syntax of "w24fn"
    if (strcmp(option, "w24fn") == 0)
    {
        if (argNum(clInp) < 2)
        {
            fprintf(stdout, "Invalid Input. w24fn should be followed by 1 file name.\n");
            return 1;
        }
        else{
            //checking if the user has included the file extension alongside the file name or not
            char *argArray[2];
            getArgs(clInp, argArray);
            // fprintf(stdout, "argArray[1] is: %s\n", argArray[1]);
            char *filename = argArray[1];
            char ch = '.';
            if (strchr(filename, ch) == NULL)
            {
                fprintf(stdout, "Invalid Input. Provide proper file extension alongside the filename = [file name].[file extension]\n");
                return 1;
            }
            else{
                return 0;
            }
            
        }
        
    }
    //further checking syntax of "w24fz"
    else if (strcmp(option, "w24fz") == 0)
    {
        if (argNum(clInp) < 3)
        {
            fprintf(stdout, "Invalid Input. w24fz should be followed by 2 file sizes.\n");
            return 1;
        }
        else{
            //checking if the user has inputed numbers as file sizes or not
            char *argArray[3];
            getArgs(clInp, argArray);
            if (isdigit(*argArray[1]) && isdigit(*argArray[2]))
            {
                return 0;
            }
            else{
                fprintf(stdout, "Invalid Input. File sizes should be numbers\n");
                return 1;
            }
        }
    }
    //further checking syntax of "w24fdb"
    else if(strcmp(option,"w24fdb")==0){
        if (argNum(clInp) < 2)
        {
            fprintf(stdout, "Invalid Input. w24fdb should be followed by a date\n");
            return 1;
        }
        else{
            //checking if the user has included the file extension alongside the file name or not
            char *argArray[2];
            getArgs(clInp, argArray);
            // fprintf(stdout, "argArray[1] is: %s\n", argArray[1]);
            // char *date = argArray[1];
            // printf("%s",date);
            if (isValidDate(argArray[1])){
                // printf("Date is valid.\n");
                date=argArray[1];
                return 0;
            } else{
                // printf("Date is invalid.\n");
                return 1;
            }
        }
    }
    //further checking syntax of "w24fda"
    else if(strcmp(option,"w24fda")==0){
        if (argNum(clInp) < 2)
        {
            fprintf(stdout, "Invalid Input. w24fdb should be followed by a date\n");
            return 1;
        }
        else{
            //checking if the user has included the file extension alongside the file name or not
            char *argArray[2];
            getArgs(clInp, argArray);
            // fprintf(stdout, "argArray[1] is: %s\n", argArray[1]);
            // char *date = argArray[1];
            // printf("%s",date);
            if (isValidDate(argArray[1])){
                // printf("Date is valid.\n");
                date=argArray[1];
                return 0;
            } else{
                // printf("Date is invalid.\n");
                return 1;
            }
        }
    }
    else if(strcmp(option,"w24ft")==0){
        if (argNum(clInp) <2 || argNum(clInp)>4)
        {
            fprintf(stdout, "Invalid Input. Too more or less inputs.\n");
            return 1;
        }
        else{
           return 0;
        }
    }
    
    return 0;
}

//function for receiving tar files
int receiveTar(int clFd, char *filep){
    FILE *tarFile = fopen(filep, "wb");
    if (tarFile == NULL) {
        perror("Error creating file");
        exit(EXIT_FAILURE);
    }


    // Receive the file size
    long tarFSize;
    if (recv(clFd, &tarFSize, sizeof(tarFSize), 0) < 0) {
        perror("Error receiving file size");
        exit(EXIT_FAILURE);
    }

    // Allocate buffer for receiving file content
    char *file_content = (char *)malloc(tarFSize);
    if (!file_content) {
        perror("Error allocating memory");
        exit(EXIT_FAILURE);
    }

    //receiving file content byte by byte
    long totalBytesRec = 0;
    int bytesRec;
    while (totalBytesRec < tarFSize) {
        bytesRec = recv(clFd, file_content + totalBytesRec, tarFSize - totalBytesRec, 0);
        if (bytesRec <= 0) {
            if (bytesRec == 0) {
                //this means the connection was closed by the server
                break;
            } else {
                perror("Error receiving file content");
                exit(EXIT_FAILURE);
            }
        }
        totalBytesRec += bytesRec;
    }

    //writing all of the received content to file
    fwrite(file_content, 1, tarFSize, tarFile);

    fclose(tarFile);

    free(file_content);

    return 0;
}

//handler for SIGINT, in case client closes with ctrl+c
void sigintHandler(int sig) {
    printf("\nReceived SIGINT signal. Exiting...\n");

    //reducing connection count before exiting
    conCount--;

    FILE *checkConCount;

    char *countFile = "./connect-count.txt";

    checkConCount = fopen(countFile, "w");

    if (checkConCount == NULL) {
        printf("Error opening file for writing!\n");
        exit(1);
    }

    fprintf(checkConCount, "%d", conCount);
    fclose(checkConCount);

    exit(0);
}

int main(int argc, char* argv[]){

    //creating a txt file which holds the count of how many connections are being made to the server
    FILE *checkConCount;

    char *countFile = "./connect-count.txt";

    //opening text file
    checkConCount = fopen(countFile, "r");

    //if the txt file does not exist, creating it
    if (checkConCount == NULL) {
        checkConCount = fopen(countFile, "w");
        if (checkConCount == NULL) {
            printf("Error creating file!\n");
            exit(1);
        }
        fprintf(checkConCount, "0\n"); // initialize the connection count to 0 after creating the txt file if the file doesn't exist
        fclose(checkConCount);

    } else {
        //reading the connection count from the file
        fscanf(checkConCount, "%d", &conCount);
        fclose(checkConCount);
    }
    

    int clFd, portNum; //server file descriptor for client side and port number respectively
    struct sockaddr_in sAdrStr; //server socket address structure object
    // socklen_t serSockLen; //server socket address structure length
    char *clientInp;
    char message[255];
    char *invInp = "Client inputted invalid input. Waiting for request from client again...";

    //server arguments should be 2: [./serverw24.c] [port number]
    if (argc != 2) {
        printf("Arguments should be in the form of: %s <IP address>\n", argv[0]);
        exit(0);
    }

    //socket creation, socket domain = AF_INET and socket type = SOCK_STREAM aka TCP
    if((clFd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        //socket creation failed
        printf("Error while creating socket\n");
        exit(1);
    }

    int mainSport = 7999;   //port number of main server
    int mirro1port = 7998;  //port number of mirror 1
    int mirro2port = 7997;  //port number of mirror 2

    //connections 1-3 are handled by main server, connections 4-6 by mirror 1, connections 7-9 by mirror 2
    if (conCount < 3)
    {
        portNum = mainSport;
    }
    else if(conCount < 6){
        portNum = mirro1port;
    }
    else if(conCount < 9){
        portNum = mirro2port;
    }
    //the rest are handled alternatively in the following sequence: main>mirror 1>mirror 2>main>mirror1>mirror2>..............
    else if(conCount % 3 == 0){
        portNum = mainSport;
    }
    else if(conCount % 3 == 1){
        portNum = mirro1port;
    }
    else{
        portNum = mirro2port;
    }
    

    //creating server socket address structure 
    sAdrStr.sin_family = AF_INET;
    sAdrStr.sin_port = htons(portNum); //converting the 16 bit host byte order of port number to network byte order

    //converting the user entered IP address from presentation to network format
    if (inet_pton(AF_INET, argv[1], &sAdrStr.sin_addr) < 0) {
        printf("Error in inet_pton().\n");
        exit(2);
    }

    //connecting to the server using connect()
    if (connect(clFd, (struct sockaddr *) &sAdrStr, sizeof(sAdrStr)) < 0) {
        printf("Error in connect().\n");
        exit(3);
    }

    conCount++;
    
    checkConCount = fopen(countFile, "w");
    fprintf(checkConCount, "%d", conCount);
    fclose(checkConCount);

    signal(SIGINT, sigintHandler);

    for(;;){
        //reading message from server
        if (read(clFd, message, 255) < 0) {
            printf("Error in read().\n");
            exit(3);
        }
        fprintf(stdout, "%s\n", message);

        //telling the client to enter a message or quit
        fprintf(stdout, "Enter quitc to exit or enter an appropriate command to send to the server\n");
        clientInp = getInput();

        //if the user enters 'quitc' the socket is closed, client exits and connection count is reduced
        if (strcmp(clientInp, "quitc") == 0) {
            write(clFd, clientInp, strlen(clientInp) + 1);
            close(clFd);

            conCount--;


            checkConCount = fopen(countFile, "w");

            if (checkConCount == NULL) {
                printf("Error opening file for writing!\n");
                exit(1);
            }

            
            fprintf(checkConCount, "%d", conCount);
            fclose(checkConCount);

            exit(0);
        }
        else{
            
            if (strcmp(clientInp, "dirlist -a") == 0) {
                printf("Showing the directory sorted alphabetically!\n");
                sleep(1.5);
                write(clFd, clientInp, strlen(clientInp) + 1);
            }
            else if (strcmp(clientInp, "dirlist -t") == 0) {
                printf("Showing the directory sorted by the creation time!\n");
                sleep(1.5);
                write(clFd, clientInp, strlen(clientInp) + 1);
            }
            else if (strstr(clientInp, "w24fn") != 0) {
                if (checkSyn(clientInp, "w24fn") == 0)
                {
                    // fprintf(stdout, "success!!!\n");
                    write(clFd, clientInp, strlen(clientInp) + 1);
                }
                else{
                    write(clFd, invInp, strlen(invInp) + 1);
                }
            }
            else if (strstr(clientInp, "w24fz") != 0) {
                if (checkSyn(clientInp, "w24fz") == 0)
                {
                    // fprintf(stdout, "success!!!\n");
                    write(clFd, clientInp, strlen(clientInp) + 1);
                    char *tarSPath = "./temp.tar.gz";
                    if (receiveTar(clFd, tarSPath) == 0)
                    {
                        char *tarRec = "Tar file received by client\n";
                        write(clFd, tarRec, strlen(tarRec) + 1);
                    }
                }
                else{
                    write(clFd, invInp, strlen(invInp) + 1);
                }
            }
            else if (strstr(clientInp, "w24fdb") != 0) {
                if (checkSyn(clientInp, "w24fdb") == 0)
                {
                    // fprintf(stdout, "success!!!\n");
                    // clientInp=date;
                    write(clFd, clientInp, strlen(clientInp) + 1);
                    // write(clFd, clientInp, strlen(clientInp) + 1);
                    char *tarSPath = "./temp.tar.gz";
                    if (receiveTar(clFd, tarSPath) == 0)
                    {
                        char *tarRec = "Tar file received by client\n";
                        write(clFd, tarRec, strlen(tarRec) + 1);
                    }
                }
                else{
                    write(clFd, invInp, strlen(invInp) + 1);
                }
            }
            else if (strstr(clientInp, "w24fda") != 0) {
                // write(clFd, clientInp, strlen(clientInp) + 1);
                if (checkSyn(clientInp, "w24fda") == 0)
                {
                    // fprintf(stdout, "success!!!\n");
                    // clientInp=date;
                    write(clFd, clientInp, strlen(clientInp) + 1);
                    // write(clFd, clientInp, strlen(clientInp) + 1);
                    char *tarSPath = "./temp.tar.gz";
                    if (receiveTar(clFd, tarSPath) == 0)
                    {
                        char *tarRec = "Tar file received by client\n";
                        write(clFd, tarRec, strlen(tarRec) + 1);
                    }
                }
                else{
                    write(clFd, invInp, strlen(invInp) + 1);
                }
            }
            else if (strstr(clientInp, "w24ft") != 0) {
                if (checkSyn(clientInp, "w24ft") == 0){
                write(clFd, clientInp, strlen(clientInp) + 1);
                char *tarSPath = "./temp.tar.gz";
                    if (receiveTar(clFd, tarSPath) == 0)
                    {
                        // printf("Received!\n");
                        char *tarRec = "Tar file received by client\n";
                        write(clFd, tarRec, strlen(tarRec) + 1);
                    }
                }
                else{
                    write(clFd, invInp, strlen(invInp) + 1);
                }
            }
            //incase of any invalid inputs
            else{
                write(clFd, invInp, strlen(invInp) + 1);
                fprintf(stdout, "Invalid Input\n");
            }
        }

        free(clientInp);
    }
}
