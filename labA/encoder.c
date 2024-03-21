#include <stdio.h>
#include <string.h>

int main(int argc, char **argv) {

FILE * input = stdin;
FILE * output = stdout;

char currChar; 
int action=0; //addition or subtraction: 1=add , -1= sub
int debug = 0; //if =0 debugging mode is off, else if =1 debugging mode is off,
char* pointer; //pointer to the "+e12345" string 

for(int i = 1; i< argc ; i++){
    if(strcmp(argv[i],"+D") == 0)
        debug = 1;
    else if(strcmp(argv[i],"-D") == 0)
        debug = 0;
    
    if(strncmp(argv[i],"-e",2) == 0){ // encode by sub
        action = -1 ; 
        pointer = argv[i]+2; //pointes to the first number 
    }
   
    else if(strncmp(argv[i],"+e",2) == 0){ // encode by adding
        action = 1;
        pointer = argv[i]+2; //takes the first nubmer 
    }

    else if(strncmp(argv[i],"-i",2) == 0){
        input = fopen(argv[i]+2, "r");
        if(input == NULL){
            fprintf(stderr, "file open fail\n"); //check if this is correct
            exit(0);
        }    
    }

    else if(strncmp(argv[i],"-o",2) == 0){
        output = fopen(argv[i]+2, "w");
        if(output == NULL){
            fprintf(stderr, "file open fail\n");
            exit(0);
        }  
    }
    
     ///////////CODE FROM TASK 1 - DEBUG///////////
    if(debug == 1 && strcmp(argv[i],"+D") != 0 && strcmp(argv[i],"-D") != 0){
        fputs(argv[i], stderr); //puts the string stored in argv[1] to the stderr
        fflush(stderr);
        fprintf(stderr, "\n");  
    }
    ///////////////////////////////////////////////////
}

    int index = 0;
    while ((currChar= fgetc(input)) != EOF && (currChar != '\0') && action != 0){ //thakes the next char from input string
        char update = action*((pointer[index])-'0');
        //the encription code is copied from ps5 presentation in course CSI
        if(currChar>='0' && currChar <='9'){
            currChar= currChar-'0';
            currChar= (currChar+update)%10 + '0';
        }
        else if(currChar>='a' && currChar <='z')
        {
            currChar= currChar-'a';
            currChar= (currChar+update)%26 + 'a';
        }
         else if(currChar>='A' && currChar <='Z')
        {
            currChar= currChar-'A';
            currChar= (currChar+update)%26 + 'A';
        }
        
        fputc(currChar, output);   
        index++;
        if(pointer[index] == '\0'){ //if we reached to the end of the string
            index = 0;  //reset the pointer
        }  
    }
    
return 0;
}