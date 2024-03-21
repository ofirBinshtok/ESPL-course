#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* suspectedFile = NULL; //holds the file recieved from the commannd line

typedef struct virus {
    unsigned short SigSize;
    char virusName[16];
    unsigned char* sig;
} virus; 

typedef struct link link;
struct link {
    link *nextVirus;
    virus *vir;
};

typedef struct fun_desc {
char *name;
link* (*fun)(link*);
} fun_desc;

virus* readVirus(FILE* file){
    virus* virus = malloc(sizeof(struct virus));
    int fileSize = fread(virus, 1, 18, file);
    if(fileSize != 18){ //first 18 bytes are the signatures length and name
        fprintf(stdout, "failed reading the first 18 bytes");
        exit(0);
    }
    else{
        virus->sig = malloc(virus->SigSize);
        fread(virus->sig, 1, virus->SigSize, file);
    }
    return virus;
}

void printHex(unsigned char* buffer, int length){
    int index = 0; 
    for(int i = 0; i < length; i++){
        if(index == 20){ //every 20 characters, create a new line
            printf("\n"); 
            index = 0;
        }
        printf("%02X ", buffer[i]);
        index++;
    }
    printf("\n\n"); 
}

void printVirus(virus* virus, FILE* output){
    fprintf(output, "Virus name: %s\n", virus->virusName);
    fprintf(output, "Virus size: %d\n", virus->SigSize);
    fprintf(output, "signature:\n");
    printHex(virus->sig, virus->SigSize); 
    printf("\n"); //each item is followed by a new line - for list_print function
}

/* Print the data of every link in list to the given stream. 
Each item followed by a newline character. */
void list_print(link *virus_list, FILE* output){
    link* curr = virus_list;
    while (curr != NULL){
        printVirus(curr->vir, output);
        curr = curr->nextVirus ;
    }
}

/* Add a new link with the given data to the list (at the end CAN ALSO AT BEGINNING), 
and return a pointer to the list (i.e., the first link in the list). 
If the list is null - create a new entry and return a pointer to the entry. */
link* list_append(link* virus_list, virus* data){
    if(virus_list == NULL){ //if the list is empty, create a list with a virus
        virus_list = malloc(sizeof(struct link));
        virus_list->vir = data;
        virus_list->nextVirus = NULL;
        return virus_list;
    }
    else{ //add the new link to the end of the linked list
        link* newLink = malloc(sizeof(struct link)); //create a new link
        link* endLink = virus_list;
        newLink->vir = data;
        while(endLink->nextVirus != NULL){
            endLink = endLink->nextVirus;
        }
        endLink->nextVirus = newLink;
        newLink->nextVirus = NULL;
        return virus_list;
    }
}
/* Free the memory allocated by the list. */
link* list_free(link *virus_list){
    link* temp = virus_list;
    while(virus_list != NULL){
        free(virus_list->vir->sig);
        free(virus_list->vir);
        temp = virus_list->nextVirus;
        free(virus_list);
        virus_list = temp;
    }
    return virus_list;
}

/*an auxiliary function that recives a file and for each virus creates a link and appends it to the list*/
link* list_maker(FILE* input){
    link* newLink = NULL; 
    int counter = 4; //we've already read 4 bytes (signature length)
    int size;
    fseek(input, 0, SEEK_END); 
    size = ftell(input);
    rewind(input);
    char magicNum[4]; //for little endian coding- VISL
    fread(magicNum, 1, 4, input);
  
    if(strncmp(magicNum, "VISL" ,4) != 0 ){
        printf("ERROR: wrong magic number");
        exit(1);
    }
   
    while(counter < size){
        virus* virus = readVirus(input);
        newLink = list_append(newLink, virus);   
        counter = counter + 18 + virus->SigSize;
    }
    fclose(input);
    return newLink;
}

link* load_sigs(link* link){
    char* fileName = NULL; 
    char buffer[BUFSIZ]; //BUFSIZ = max buffer size
    printf("Please enter file name\n");
    fgets(buffer, BUFSIZ, stdin); //reads the users input and stores it in to the buffer
    sscanf(buffer,"%ms", &fileName); //reads the file name from the buffer 
    //"%ms" - allocates memory in order to avoid buffer overflow issues wjile reading the input from the user
    FILE* signatureFile = fopen(fileName, "rb");
    free(fileName);
    if(signatureFile == NULL){
        fprintf(stderr, "failed reading signature file\n");
        exit(1);
    }
    return list_maker(signatureFile); //creates a linked list of viruses
}

link* print_sigs(link* list){
    FILE* output = stdout;
    list_print(list, output);
    return list;
}

void detect_virus(char* buffer, unsigned int size, link* virus_list){
    link* curr = virus_list;
    while (curr != NULL){
        virus* currVirus = curr->vir;
        for(int byteLocation = 0 ; byteLocation < size ; byteLocation++){
            if(memcmp(buffer+byteLocation, currVirus->sig , currVirus->SigSize) == 0){
                printf("Starting byte location in the suspected file: %X\n", byteLocation);
                printf("virus name: %s\n", currVirus->virusName);
                printf("size of the virus signature: %d\n", currVirus->SigSize);
                printf("\n");
            }
        }
        curr = curr->nextVirus;
    }
}

link* detect_vir(link* list){
    FILE* fileToDetect = fopen(suspectedFile, "rb");
    if(fileToDetect == NULL){
        fprintf(stderr, "failed opening file\n");
        return list;
    }
    char buffer[10000];
    int fileSize = fread(buffer, 1, 10000, fileToDetect);
    detect_virus(buffer, fileSize, list); //detect viruses
    fclose(fileToDetect);
    return list;
}

void neutralize_virus(char* fileName, int signatureOffset) {
    FILE* virusFile = fopen(fileName,"wb+"); //to be able to "edit" the file
    if(virusFile == NULL){
        fprintf(stderr, "failed opening file\n");
        exit(1);
    }
    fseek(virusFile, signatureOffset, SEEK_SET);
    char* change = malloc(1);
    strcpy(change, "0xc3");
    fwrite(change, 1, 1, virusFile); //replace the first byte of the virus code to c3
    free(change);
    fclose(virusFile);
}

link* fix_file(link* list){
    FILE* fileToFix = fopen(suspectedFile, "rb");
    if(fileToFix == NULL){
        fprintf(stderr, "failed opening file\n");
        return list;
    }
    char buffer[10000];
    int fileSize = fread(buffer, 1, 10000, fileToFix); //reading byte by byte from suspected file
    link* curr = list;
    while(curr != NULL){
        virus* currVirus = curr->vir;
        for(int offsetIndex = 0 ; offsetIndex < fileSize ; offsetIndex++){
            if(memcmp(buffer+offsetIndex, currVirus->sig , currVirus->SigSize) == 0){
                neutralize_virus(suspectedFile, offsetIndex); //neutralize the viruses
            }
        }
        curr = curr->nextVirus;
    }
    fclose(fileToFix);
    return list;
}

link* quit(link* list){
    if(list != NULL)
        list_free(list);
    exit(0);
    return list;
}

int main(int argc, char **argv){
    link* virList = NULL;
    struct fun_desc menu[] = {{"Load signature", load_sigs}, {"Print signature", print_sigs}, {"Detect viruses", detect_vir}, {"Fix file", fix_file},
                                {"Quit", quit}, {NULL, NULL}};

    if(argc > 1)
        suspectedFile = argv[1];

    while(1){
        int menuSize = sizeof(menu) / sizeof(struct fun_desc) - 1;
        int i = 0;
        fprintf(stdout, "Select operation from the following menu:\n");
        while(menu[i].name != NULL){
            printf("%d) %s\n", i+1, menu[i].name); 
            i++;
        }
        int option; //In order to save the option the user chose
        printf("option:");
        scanf("%d", &option);
        fgetc(stdin);

        if(feof(stdin)) //when user presses CTRL^D- exit
          break;

        if(option > 0 && option <= menuSize){
            printf("Within bounds\n");
            printf("\n");
        }
        else{
            printf("Not within bounds\n");
            exit(0);
        }
        virList = menu[option - 1].fun(virList);        
        printf("\n");
    }
}