#include <stdlib.h>
#include <stdio.h>
#include <string.h>
 
char* map(char *array, int array_length, char (*f) (char)){
  char* mapped_array = (char*)(malloc(array_length*sizeof(char)));
  /* TODO: Complete during task 2.a */
  for(int i= 0; i< array_length; i++){
    mapped_array[i] = (*f)(array[i]);
  }
  return mapped_array;
}

/* Ignores c, reads and returns a character from stdin using fgetc. */
char my_get(char c){
  char input = fgetc(stdin);
  return input;
}

/* If c is a number between 0x20 and 0x7E, cprt prints the character of ASCII value c followed by a new line. 
Otherwise, cprt prints the dot ('.') character. After printing, cprt returns the value of c unchanged. */
char cprt(char c){
  if(0x20<= c && c<=0x7E)
    printf("%c\n", c); //%d  prints the ASCII value of c
  else
    printf("%c\n", '.');
  return c;
}

/* Gets a char c and returns its encrypted form by adding 1 to its value. If c is not between 0x20 and 0x7E it is returned unchanged */
char encrypt(char c){
  if(0x20<= c && c<=0x7E)
    return c+1;
  else
    return c;
}
 
/* Gets a char c and returns its decrypted form by reducing 1 from its value. If c is not between 0x20 and 0x7E it is returned unchanged */
char decrypt(char c){
  if(0x20<= c && c<=0x7E)
    return c-1;
  else
    return c;
}

/* xprt prints the value of c in a hexadecimal representation followed by a new line, and returns c unchanged. */  
char xprt(char c){
    if(0x20<= c && c<=0x7E)
        printf("%x\n",c);
    else
        printf("%c\n", '.');
  return c; 
}

struct fun_desc {
    char *name;
    char (*fun)(char);
};


int main(int argc, char **argv){
    char input[20];
   
    char* carray = malloc(5);
    carray[0] = '\0';
    struct fun_desc menu[] = { { "Get string", my_get }, { "Print string", cprt },{ "Encrypt", encrypt }, { "Decrypt", decrypt }, { "Print Hex", xprt },  { NULL, NULL } }; 
    int bound;
    int choise; 
    
    do{
      printf("Select operation from the following menu:\n", stdout);
      int i = 0;
      while(menu[i].name != NULL){
        printf("%d) %s\n", i, menu[i].name);
        i++;
      } 

      scanf("%d", &choise);
      if(0<= choise && choise <=bound)
        printf("Within bound\n");
      else{
        printf("Not within bound\n");
        free(carray);
        exit(0);
      }

    carray = map(carray, 5, menu[choise].fun);
    printf("DONE. \n");
    printf("\n");
    
    }
    
    while((fgets(input, 20, stdin)) != NULL );
    free(carray);
    exit(0);
    

}

