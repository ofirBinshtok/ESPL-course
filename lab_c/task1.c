#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char **argv){
    int state; //will keep the child's state - once the child process terminates the state will change

    int pipeFd[2]; /*file descriptor parameter for the pipe
                    pipeFd[0] - read end of pipe, pipeFd[1]-write end of pipe    */ 
    
    if(pipe(pipeFd)== -1){
        perror("failed to pipe\n");
        exit(1);
    }
    
    fprintf(stderr, "(parent_process>forking…)\n");
    int child1 = fork();
    fprintf(stderr,"(parent_process>created process with id: %d)\n", child1);
    
    if(child1 == 0){ //child1 process 
        fclose(stdout);
        fprintf(stderr,"(child1>redirecting stdout to the write end of the pipe…)\n");
        dup(pipeFd[1]);  //duplicating the write end
        fprintf(stderr,"(child1>going to execute cmd: ls)\n");
        close(pipeFd[1]);
        char* arg_list1[] = {"ls","-l",NULL} ;
        execvp(arg_list1[0],arg_list1);
    }

    else{
        fprintf(stderr,"(parent_process>closing the write end of the pipe…)\n");
        close(pipeFd[1]);
        fprintf(stderr, "(parent_process>forking…)\n");
        int child2 = fork();
        fprintf(stderr,"(parent_process>created process with id: %d)\n", child2);
        if(child2 == 0){
            fclose(stdin);
            fprintf(stderr, "(child2>redirecting stdin to the read end of the pipe…)\n");
            dup(pipeFd[0]); //duplicating the read end
            fprintf(stderr,"(child2>going to execute cmd: tail)\n");
            close(pipeFd[0]);
            char* arg_list2[] = {"tail","-n","2",NULL} ;
            execvp(arg_list2[0],arg_list2);
        }
        else{
            fprintf(stderr,"(parent_process>closing the read end of the pipe…)\n");
            close(pipeFd[0]);
            fprintf(stderr,"(parent_process>waiting for child processes to terminate…)\n");
            int error1 = waitpid(child1,&state, 0);
            if(error1 == -1){
                perror("ERROR\n");
                exit(1);
            }
            int error2 = waitpid(child2,&state, 0);
            if(error2 == -1){
                perror("ERROR\n");
                exit(1);
            }
            fprintf(stderr,"(parent_process>exiting…)\n");
        }      
    }
    return 0;
}