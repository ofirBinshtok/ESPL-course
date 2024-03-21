#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <linux/limits.h>
#include <unistd.h>
#include "LineParser.h"
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <signal.h>

int debugMod = 0; //0 means debugmod is off / 1 means on

void execute(cmdLine *pCmdLine){
    int error = 0 ;
    if(strcmp(pCmdLine->arguments[0],"quit")==0){
        exit(0);
    }

    if(strcmp(pCmdLine->arguments[0],"cd")==0){
        int result = chdir(pCmdLine->arguments[1]);  //the path to change to is in argv[1]
        if(result == -1)
            fprintf(stderr, "cd operation failed\n");
    }
    if(strcmp(pCmdLine->arguments[0],"suspend")==0){
            int suspended = kill(atoi(pCmdLine->arguments[1]), SIGTSTP);
            if(suspended == 0){
                printf("running process was suspended\n");
            }
    }

    if(strcmp(pCmdLine->arguments[0],"wake")==0){
        int wait = kill(atoi(pCmdLine->arguments[1]), SIGCONT);
        if(wait == 0){
            printf("sleeping process was woken up\n");
        }
    }

    if(strcmp(pCmdLine->arguments[0],"kill")==0){
        int killed = kill(atoi(pCmdLine->arguments[1]), SIGINT);
        if(killed == 0){
            printf("running/sleeping process was interrupted\n");
        }
    }

    else{
        int PID = fork();
        
        if(!PID){ // =0 means its the child process 
            if(pCmdLine->inputRedirect != NULL){
                char const* inputFile = pCmdLine->inputRedirect;
                int openNum = open(inputFile,O_RDONLY);
                if(openNum == -1){
                    perror("failed to open redirection file");
                    exit(1);
                }
                int error = dup2(openNum,0);
                if(error == -1){
                    perror("failed to dup");
                    exit(1);
                }
                close(openNum);
            }
           if( pCmdLine->outputRedirect != NULL){
                char const* outputFile = pCmdLine->outputRedirect;
                int outputNum = open(outputFile,O_WRONLY | O_CREAT ,0666);
                if(outputNum == -1){
                    perror("failed to open redirection file");
                    exit(1);
                }
                int error = dup2(outputNum,1);
                if(error == -1){
                    perror("failed to dup");
                    exit(1);
                }
                close(outputNum);
            }
            if(strcmp(pCmdLine->arguments[0],"cd")!=0){
                error = execvp(pCmdLine->arguments[0], pCmdLine->arguments); //the name of the fie is in arguments[0]
                if(error == -1){
                    perror("execv failed");
                    _exit(1);
                }
            }
            
        } 
        //it is the parent process
        if(debugMod ==1){
            fprintf(stderr, "PID: %d\n", PID);
            fprintf(stderr, "Executing command: %s\n", pCmdLine->arguments[0]);
        }
        if(pCmdLine->blocking == 1){
            waitpid(PID,NULL, 0); //NULL & 0 - because we aren't intrested in saving the information
        }
    }
}

int main(int argc, char **argv){
    for(int i=1; i < argc ; i++){
        if(strcmp(argv[i],"-d")==0)
            debugMod =1;
    }
    
    while(1){
        char pathBuffer[PATH_MAX]; 
        char inputBuffer[2048]; 
        char* path = getcwd(pathBuffer,PATH_MAX);  //copies the path of the current working directory to the buffer
        printf("curr path: %s\n", path);
        fgets(inputBuffer,2048,stdin);            //reads a line from the user 
        cmdLine* parsedInput = parseCmdLines(inputBuffer); 
        execute(parsedInput);
        freeCmdLines(parsedInput);
    }
    return 0;
}
