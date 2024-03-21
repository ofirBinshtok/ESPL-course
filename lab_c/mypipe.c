#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char **argv){
    int pipeFd[2]; /*file descriptor parameter for the pipe
                    pipeFd[0] - read end of pipe, pipeFd[1]-write end of pipe    */ 
    if(pipe(pipeFd)== -1){
        perror("failed to pipe\n");
        exit(1);
    }
    int pid = fork();
    /*code taken from the pipe() manual*/
    if(pid == 0 ){ // child process 
        close(pipeFd[0]); //closing unused pipe end
        char* message = "hello";
        write(pipeFd[1], message, strlen(message));
        close(pipeFd[1]);
        _exit(0);
    } 
    else{ //parent process
        close(pipeFd[1]); //closing unused pipe end
        char toRead[BUFSIZ];
        read(pipeFd[0],&toRead,BUFSIZ);
        printf("message recived: %s\n", toRead);
        close(pipeFd[0]);
        exit(0);
    }

}