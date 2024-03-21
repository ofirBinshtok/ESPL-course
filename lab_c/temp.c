void printProcessList(process** process_list){
    printf("index \t\t process id \t\t process status \t\t command \t\t\n");
    if(*process_list != NULL){
        updateProcessList(process_list);
        int index = 0;
        process* prevProcess;
        process* currProcess = *process_list;
        while (currProcess != NULL){
           printf("%d \t\t %d \t\t\t %d \t\t\t\t %s \t\t\t\n", index, currProcess->pid, currProcess->status, currProcess->cmd->arguments[0]);
            while (currProcess != NULL && currProcess->status != -1){
                prevProcess = currProcess; 
                currProcess = currProcess->next;
            }
            
            if(currProcess !=NULL && currProcess->status == -1){ //means the process "freshly" terminated
                if(prevProcess == NULL){
                    *process_list = currProcess->next;
                    freeCmdLines(currProcess->cmd);
                    free(currProcess);
                    currProcess = *process_list;
                }
                else{
                    prevProcess->next = currProcess->next;
                    freeCmdLines(currProcess->cmd); 
                    free(currProcess);
                    currProcess = currProcess->next;
                }          
            } 
            index++;
        }
    }    
}