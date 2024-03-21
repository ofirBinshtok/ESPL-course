#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <linux/limits.h>
#include <unistd.h>
#include <wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <signal.h>
#include "LineParser.h"

#define TERMINATED -1
#define RUNNING 1
#define SUSPENDED 0

typedef struct process{
    cmdLine *cmd;         /* the parsed command line*/
    pid_t pid;            /* the process id that is running the command*/
    int status;           /* status of the process: RUNNING/SUSPENDED/TERMINATED */
    struct process *next; /* next process in chain */
} process;

int debugMod = 0;                    // 0 means debugmod is off / 1 means on
process *global_process_list = NULL; // global process list
#define HISTLEN 20
char historyQueue[HISTLEN][2048];
int historyIndex = 0; //a pointer to the next free spot in history
int oldest = 0;
int newest = -1;


void addProcess(process **process_list, cmdLine *cmd, pid_t pid){
    process *newProcess = malloc(sizeof(struct process)); // create a process
    newProcess->cmd = cmd;
    newProcess->pid = pid;
    newProcess->status = RUNNING;
    newProcess->next = NULL;
    if(cmd->blocking || cmd->next){
        newProcess->status = TERMINATED;
    }
    if (*process_list == NULL){
        *process_list = newProcess;
    }
    else{
        process *currProcess = *process_list;
        while (currProcess->next != NULL){
            currProcess = currProcess->next;
        }
        currProcess->next = newProcess;
    }
}

void updateProcessStatus(process *process_list, int pid, int status){
    int changed = 0;
    while (process_list != NULL && !changed){
        if (process_list->pid == pid){
            process_list->status = status;
            changed = 1;
        }
        else
            process_list = process_list->next;
    }
}

void updateProcessList(process **process_list){
    if (*process_list != NULL){
        process *currProcess = *process_list;
        while (currProcess != NULL){
            int status;
            if (waitpid(currProcess->pid, &status, WNOHANG | WUNTRACED | WCONTINUED) > 0){
                if(WIFEXITED(status) || WIFSIGNALED(status))
                    updateProcessStatus(currProcess, currProcess->pid, TERMINATED);
                if(WIFSTOPPED(status))
                    updateProcessStatus(currProcess, currProcess->pid, SUSPENDED);
                if(WIFCONTINUED(status))
                    updateProcessStatus(currProcess, currProcess->pid, RUNNING);
            }
            currProcess = currProcess->next;
        }
    }
}

void deleteProcess(process **process_list){
    process *prevProcess = NULL;
    process *currProcess = *process_list;

    while (currProcess != NULL){
        if (currProcess->status != TERMINATED){
            prevProcess = currProcess;
            currProcess = currProcess->next;
        }
        else{ // process to delete
            if (prevProcess == NULL){ // we want to delete the head of the list
                if (currProcess->next != NULL){
                    *process_list = currProcess->next;
                    freeCmdLines(currProcess->cmd);
                    free(currProcess);
                    currProcess = *process_list;
                }
                else{
                    freeCmdLines(currProcess->cmd);
                    free(currProcess);
                    *process_list = NULL;
                    currProcess = NULL;
                }
            }
            else{
                if (currProcess->next != NULL){
                    prevProcess->next = currProcess->next;
                    freeCmdLines(currProcess->cmd);
                    free(currProcess);
                    currProcess = prevProcess->next;
                }
                else{
                    freeCmdLines(currProcess->cmd);
                    free(currProcess);
                    prevProcess->next = NULL;
                    currProcess = NULL;
                }
            }
        }
    }
}

void printProcessList(process **process_list){
    updateProcessList(process_list);
    printf("index \t\t process id \t\t process status \t\t command \t\t\n");
    int index = 0;
    process *currProcess = *process_list;
    while (currProcess != NULL){
        char* status;
        if(currProcess->status == TERMINATED) 
            status = "TERMINATED";
        else if(currProcess->status == SUSPENDED) 
            status = "SUSPENDED";
        else 
            status = "RUNNING";
        printf("%d \t\t %d \t\t %s \t\t\t %s \t\t\t\n", index, currProcess->pid, status, currProcess->cmd->arguments[0]);
        currProcess = currProcess->next;
        index++;
    }
    deleteProcess(process_list);
}

void freeProcessList(process *process_list){
    process *tempProcess = process_list;
    while (process_list != NULL){
        freeCmdLines(tempProcess->cmd);
        tempProcess = process_list->next;
        free(process_list);
        process_list = tempProcess;
    }
}

void addCmdToHistory(char* unParsed){
    newest = (newest + 1 ) % HISTLEN;
    strcpy(historyQueue[newest], unParsed);
    if(historyIndex == HISTLEN){
        oldest = (oldest + 1 ) % HISTLEN;
    } else {
        historyIndex++;
    }
}

void printHistory(){
    for(int i = 0 ; i < historyIndex ; i++){
        printf("%d) %s", i + 1,historyQueue[(oldest + i) % HISTLEN]);
    }
}
    
void myPipe(cmdLine *pCmdLine){
    int state; // will keep the child's state - once the child process terminates the state will change

    int pipeFd[2]; /*file descriptor parameter for the pipe
                    pipeFd[0] - read end of pipe, pipeFd[1]-write end of pipe    */

    if (pipe(pipeFd) == -1){
        perror("failed to pipe\n");
        exit(1);
    }
    if (pCmdLine->outputRedirect != NULL){
        fprintf(stderr, "ERROR: trying to redirect the output of left hand side process\n");
        return;
    }

    if (pCmdLine->next->inputRedirect != NULL){
        fprintf(stderr, "ERROR: trying to redirect the input of right hand side process\n");
        return;
    }
    int child1 = fork();
    if (child1 == 0){ // child1 process
        close(STDOUT_FILENO);
        dup(pipeFd[1]);   // duplicating the write end
        close(pipeFd[1]); // closing the write end of th pipe

        if (pCmdLine->inputRedirect != NULL){
            char const *inputFile = pCmdLine->inputRedirect;
            int openNum = open(inputFile, O_RDONLY);
            if (openNum == -1){
                perror("failed to open redirection file");
                exit(1);
            }
            int error = dup2(openNum, 0);
            if (error == -1){
                perror("failed to dup");
                exit(1);
            }
            close(openNum);
        }
        execvp(pCmdLine->arguments[0], pCmdLine->arguments);
    }

    else{
        close(pipeFd[1]);
        int child2 = fork();
        if (child2 == 0){
            close(STDIN_FILENO);
            dup(pipeFd[0]); // duplicating the read end
            close(pipeFd[0]);
            if (pCmdLine->next->outputRedirect != NULL){
                char const *outputFile = pCmdLine->next->outputRedirect;
                int outputNum = open(outputFile, O_WRONLY | O_CREAT, 0666);
                if (outputNum == -1){
                    perror("failed to open redirection file");
                    exit(1);
                }
                int error = dup2(outputNum, 1);
                if (error == -1){
                    perror("failed to dup");
                    exit(1);
                }
                close(outputNum);
            }
            execvp(pCmdLine->next->arguments[0], pCmdLine->next->arguments);
        }
        else{
            addProcess(&global_process_list, pCmdLine, child1);
            close(pipeFd[0]);
            int error1 = waitpid(child1, &state, 0);
            if (error1 == -1){
                perror("ERROR\n");
                exit(1);
            }
            int error2 = waitpid(child2, &state, 0);
            if (error2 == -1){
                perror("ERROR\n");
                exit(1);
            }
        }
    }
}

void execute(cmdLine *pCmdLine){
    int error = 0;
    int shouldWeFree = 1;
    if (strcmp(pCmdLine->arguments[0], "quit") == 0){
        freeProcessList(global_process_list);
        freeCmdLines(pCmdLine);
        exit(0);
    }

    else if (strcmp(pCmdLine->arguments[0], "cd") == 0){
        int result = chdir(pCmdLine->arguments[1]); // the path to change to is in argv[1]
        if (result == -1)
            fprintf(stderr, "cd operation failed\n");
    }

    else if (strcmp(pCmdLine->arguments[0], "procs") == 0){
        printProcessList(&global_process_list);
    }

    else if (strcmp(pCmdLine->arguments[0], "suspend") == 0){
        int suspended = kill(atoi(pCmdLine->arguments[1]), SIGTSTP);
        if (suspended == 0){
            printf("running process was suspended\n");
        }
    }

    else if (strcmp(pCmdLine->arguments[0], "wake") == 0){
        int wait = kill(atoi(pCmdLine->arguments[1]), SIGCONT);
        if (wait == 0){
            printf("sleeping process was woken up\n");
        }
    }

    else if (strcmp(pCmdLine->arguments[0], "kill") == 0){
        int killed = kill(atoi(pCmdLine->arguments[1]), SIGINT);
        if (killed == 0){
            printf("running/sleeping process was interrupted\n");
        }
    }

    else{
        if (pCmdLine->next != NULL){ // means that there was a pipe in the command line
            myPipe(pCmdLine);
            shouldWeFree = 0;
        } else {
            int PID = fork();
            if (!PID){ // =0 means its the child process
                if (pCmdLine->inputRedirect != NULL){
                    char const *inputFile = pCmdLine->inputRedirect;
                    int openNum = open(inputFile, O_RDONLY);
                    if (openNum == -1){
                        perror("failed to open redirection file");
                        exit(1);
                    }
                    int error = dup2(openNum, 0);
                    if (error == -1){
                        perror("failed to dup");
                        exit(1);
                    }
                    close(openNum);
                }
                if (pCmdLine->outputRedirect != NULL){
                    char const *outputFile = pCmdLine->outputRedirect;
                    int outputNum = open(outputFile, O_WRONLY | O_CREAT, 0666);
                    if (outputNum == -1){
                        perror("failed to open redirection file");
                        exit(1);
                    }
                    int error = dup2(outputNum, 1);
                    if (error == -1){
                        perror("failed to dup");
                        exit(1);
                    }
                    close(outputNum);
                }

                error = execvp(pCmdLine->arguments[0], pCmdLine->arguments); // the name of the fie is in arguments[0]
                if (error == -1){
                    perror("execv failed");
                    _exit(1);
                }
            }
            // it is the parent process
            else{
                addProcess(&global_process_list, pCmdLine, PID);
            }

            if (debugMod == 1){
                fprintf(stderr, "PID: %d\n", PID);
                fprintf(stderr, "Executing command: %s\n", pCmdLine->arguments[0]);
            }

            if (pCmdLine->blocking == 1){
                waitpid(PID, NULL, 0); // NULL & 0 - because we aren't intrested in saving the information
            }
        }
        shouldWeFree = 0;
    }
    if(shouldWeFree == 1){
        freeCmdLines(pCmdLine);
    }
}

int main(int argc, char **argv){
    for (int i = 1; i < argc; i++){
        if (strcmp(argv[i], "-d") == 0)
            debugMod = 1;
    }

    while (1){
        int history = 0;
        char pathBuffer[PATH_MAX];
        char inputBuffer[2048];
        char *path = getcwd(pathBuffer, PATH_MAX); // copies the path of the current working directory to the buffer
        printf("%s> ", path);
        //fgets(inputBuffer, 2048, stdin); // reads a line from the user
        if(fgets(inputBuffer, 2048, stdin) == NULL)
            break;
        if(strcmp(inputBuffer, "\n") == 0)
            continue;
        cmdLine *parsedInput = parseCmdLines(inputBuffer);
        char* unParsedCmd = inputBuffer;
        if (strcmp(parsedInput->arguments[0], "history") == 0){
            history = 1;
            addCmdToHistory(unParsedCmd);
            printHistory();
        }
        else if(strcmp(parsedInput->arguments[0], "!!") == 0){
            if(newest == -1){
                fprintf(stderr, "History is empty\n!");
            } else {
                freeCmdLines(parsedInput);
                if(strncmp(historyQueue[newest], "history", 7) == 0){
                    history = 1;
                    printHistory();
                }     
                else
                    parsedInput = parseCmdLines(historyQueue[newest]);   
            }
        }
        else if(strncmp(parsedInput->arguments[0], "!", 1) == 0){
            int n = atoi(parsedInput->arguments[0] + 1);
            if(newest == -1 || n == 0 || n > historyIndex){
                fprintf(stderr, "Invalid input\n!");
            } else {
                freeCmdLines(parsedInput);
                if(strncmp(historyQueue[(oldest + n - 1) % HISTLEN], "history", 7) == 0){
                    history = 1;
                    printHistory();
                }     
                else
                    parsedInput = parseCmdLines(historyQueue[(oldest + n - 1) % HISTLEN]);
            }
        }
        else{
            addCmdToHistory(unParsedCmd);
        }
        if(history == 0)    
            execute(parsedInput);
    }
    freeProcessList(global_process_list);
    return 0;
}
