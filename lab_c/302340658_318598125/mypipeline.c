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

    int child1 = fork();
    if(child1 == 0){ //child1 process 
        fclose(stdout);
        dup(pipeFd[1]);  //duplicating the write end
        close(pipeFd[1]);
        char* arg_list1[] = {"ls","-l",NULL} ;
        execvp(arg_list1[0],arg_list1);
    }

    else{
        close(pipeFd[1]);
        int child2 = fork();
        if(child2 == 0){
            fclose(stdin);
            dup(pipeFd[0]); //duplicating the read end
            close(pipeFd[0]);
            char* arg_list2[] = {"tail","-n","2",NULL} ;
            execvp(arg_list2[0],arg_list2);
        }
        else{
            close(pipeFd[0]);
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
        }      
    }
    return 0;
}