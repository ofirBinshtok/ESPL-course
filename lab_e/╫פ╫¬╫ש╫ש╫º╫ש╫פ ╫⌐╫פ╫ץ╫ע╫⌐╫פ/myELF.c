#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <elf.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

typedef struct
{
    void *map1; // memory location of the mapped file
    void *map2;
    int fd1; // for mapping
    int fd2;
    char *fileName1;
    char *fileName2;
    struct stat file_info1; // gives information about the file
    struct stat file_info2;
} handle_file;

struct fun_desc
{
    char *name;
    void (*fun)(handle_file *currFd);
};

int numOpenFiles = 0; // 0, 1 or 2
int debug_mode = 0;   // debug mode is off
Elf32_Ehdr *header;
// Elf32_Shdr* sh_header;

char *sh_type(int sh_type)
{
    if (sh_type == SHT_NULL)
        return "NULL";
    if (sh_type == SHT_PROGBITS)
        return "PROGBITS";
    if (sh_type == SHT_SYMTAB)
        return "SYMTAB";
    if (sh_type == SHT_STRTAB)
        return "STRTAB";
    if (sh_type == SHT_RELA)
        return "RELA";
    if (sh_type == SHT_HASH)
        return "HASH";
    if (sh_type == SHT_DYNAMIC)
        return "DYNAMIC";
    if (sh_type == SHT_NOTE)
        return "NOTE";
    if (sh_type == SHT_NOBITS)
        return "NOBITS";
    if (sh_type == SHT_REL)
        return "REL";
    if (sh_type == SHT_SHLIB)
        return "SHLIB";
    if (sh_type == SHT_DYNSYM)
        return "DYNSYM";
    else
        return "UNKNOWN";
}

char *sh_name(int s_index)
{
    if (s_index == SHN_UNDEF)
        return "UNDEF";
    if (s_index == SHN_LORESERVE)
        return "LORESERVE";
    if (s_index == SHN_LOPROC)
        return "LOPROC";
    if (s_index == SHN_BEFORE)
        return "BEFORE";
    if (s_index == SHN_AFTER)
        return "AFTER";
    if (s_index == SHN_HIPROC)
        return "HIPROC";
    if (s_index == SHN_LOOS)
        return "LOOS";
    if (s_index == SHN_HIOS)
        return "HIOS";
    if (s_index == SHN_ABS)
        return "ABS";
    if (s_index == SHN_COMMON)
        return "COMMON";
    if (s_index == SHN_XINDEX)
        return "XINDEX";
    if (s_index == SHN_HIRESERVE)
        return "HIRESERVE";
    else
        return "UNKNOWN";
}

void toggle_Debug_Mode(handle_file *currFd)
{
    if (debug_mode == 0)
    { // debug mode is off
        printf("debug flag is now on\n");
        debug_mode = 1;
    }
    else
    { // debug mode is on
        printf("debug flag is now off\n");
        debug_mode = 0;
    }
}

void examine_ELF_File(handle_file *currFd)
{
    char buffer[BUFSIZ];
    int protectionFlag = PROT_READ;
    int mappingFlag = MAP_SHARED;
    int fd = -1;
    char fileName[256];
    printf("Please enter file name:\n");
    fscanf(stdin, "%s", buffer);
    strcpy(fileName, buffer);

    if (numOpenFiles == 2)
        printf("There are already 2 opened ELF files\n");
    else
    {
        int file = open(buffer, O_RDONLY);
        struct stat file_info;
        void *map;

        if (file < 0)
        {
            perror("failed opening the file\n");
            exit(1);
        }

        if (fstat(file, &file_info) < 0)
        { // fstat retrieve information about the file
            perror("fstat filed\n");
            close(file);
            exit(1);
        }
        map = mmap(0, file_info.st_size, protectionFlag, mappingFlag, file, 0);
        if (map == MAP_FAILED)
        {
            printf("Mapping failed\n");
            close(file);
            fd = -1;
            munmap(map, file_info.st_size);
            exit(1);
        }
        header = (Elf32_Ehdr *)map;

        if (strncmp((char *)header->e_ident, (char *)ELFMAG, 4) != 0)
        {
            printf("The magic number isn't consistent with an ELF file\n");
            close(file);
            fd = -1;
            munmap(0, file_info.st_size);
            exit(1);
        }
        else
            fd = file;

        if (numOpenFiles == 1)
        {
            currFd->fd2 = fd;
            currFd->fileName2 = malloc(strlen(fileName) + 1); // Allocate memory for the file name
            strcpy(currFd->fileName2, fileName);              // Copy the file name
            currFd->file_info2 = file_info;
            currFd->map2 = map;
        }
        else
        { // numOpenFiles is 0
            currFd->fd1 = fd;
            currFd->fileName1 = malloc(strlen(fileName) + 1); // Allocate memory for the file name
            strcpy(currFd->fileName1, fileName);              // Copy the file name
            currFd->file_info1 = file_info;
            currFd->map1 = map;
        }

        numOpenFiles++;

        printf("Bytes 1,2,3 of the magic number: %x %x %x \n", header->e_ident[EI_MAG1], header->e_ident[EI_MAG2], header->e_ident[EI_MAG3]);
        if (header->e_ident[EI_DATA] == ELFDATA2LSB)
            printf("The data encoding scheme of the object file is: 2's complement, little endian\n");
        else
            printf("The data encoding scheme of the object file is: 2's complement, big endian\n");
        printf("Entry point is: %x \n", header->e_entry);
        printf("The file offset in which the section header table resides: %d \n", header->e_shoff);
        printf("The number of section headers entries: %d \n", header->e_shnum);
        printf("The size of each section header entry: %d \n", header->e_shentsize);
        printf("The file offset in which the program header table resides: %d \n", header->e_phoff);
        printf("The number of program headers entries: %d \n", header->e_phnum);
        printf("The size of each program header entry: %d \n", header->e_phentsize);
    }
}

void print_Section_For_File(void *map, char *fileName)
{
    Elf32_Ehdr *elf_header = (Elf32_Ehdr *)map;
    Elf32_Shdr *str_table = map + elf_header->e_shoff + (elf_header->e_shstrndx * elf_header->e_shentsize); // find str_table location in memory
    printf("File %s \n", fileName);
    if (debug_mode == 0)
    {
        printf("[%5s] %-16s %-16s %-16s %-16s %-16s\n",
               "index", "section_name", "section_address", "section_offset", "section_size", "section_type");
        for (int i = 0; i < elf_header->e_shnum; i++)
        {
            Elf32_Shdr *curr_sectionHeader_entry = map + elf_header->e_shoff + ((elf_header->e_shentsize) * i);
            char *curr_sectionHeader_name = map + str_table->sh_offset + curr_sectionHeader_entry->sh_name;
            printf("[%2d] %2s %-16s %08x %7s %06x %9s %06x %10s %-20s\n", i, "", curr_sectionHeader_name, curr_sectionHeader_entry->sh_addr, "",
                   curr_sectionHeader_entry->sh_offset, "", curr_sectionHeader_entry->sh_size, "", sh_type(curr_sectionHeader_entry->sh_type));
        }
    }
    else
    { // debug mode is on
        printf("[%5s] %-16s %-16s %-16s %-16s %-16s %-16s %-16s\n",
               "index", "section_name", "section_address", "section_offset", "section_size", "section_type", "shstrndx", "section name offset");
        for (int i = 0; i < elf_header->e_shnum; i++)
        {
            Elf32_Shdr *curr_sectionHeader_entry = map + elf_header->e_shoff + ((elf_header->e_shentsize) * i);
            char *curr_sectionHeader_name = map + str_table->sh_offset + curr_sectionHeader_entry->sh_name;
            printf("[%2d] %2s %-16s %08x %7s %06x %9s %06x %10s %-15s %d %13s %d\n", i, "", curr_sectionHeader_name, curr_sectionHeader_entry->sh_addr, "",
                   curr_sectionHeader_entry->sh_offset, "", curr_sectionHeader_entry->sh_size, "", sh_type(curr_sectionHeader_entry->sh_type),
                   elf_header->e_shstrndx, "", (elf_header->e_shoff + (elf_header->e_shentsize * i)));
        }
    }
}

void print_Section_Names(handle_file *currFd)
{
    if (numOpenFiles == 0)
    {
        printf("No mapped file");
        return;
    }
    else
    {
        if (numOpenFiles == 1)
            print_Section_For_File(currFd->map1, currFd->fileName1);
        else
        { // num of open files is 2
            print_Section_For_File(currFd->map1, currFd->fileName1);
            print_Section_For_File(currFd->map2, currFd->fileName2);
        }
    }
}

void print_Symbols_For_File(void *map, char *fileName)
{
    Elf32_Ehdr *elf_header = (Elf32_Ehdr *)map;
    Elf32_Shdr *symtab = NULL;
    Elf32_Shdr *dynsym = NULL;
    Elf32_Shdr *str_table_temp = NULL;
    Elf32_Shdr *str_table;
    Elf32_Shdr *section_entry;
    Elf32_Sym *curr_entry;

    str_table = map + elf_header->e_shoff + (elf_header->e_shstrndx * elf_header->e_shentsize);

    for (int i = 0; i < elf_header->e_shnum && (symtab == NULL || dynsym == NULL || str_table_temp == NULL); i++)
    {
        curr_entry = map + elf_header->e_shoff + ((elf_header->e_shentsize) * i);
        char *curr_name = map + str_table->sh_offset + curr_entry->st_name;
        if (strcmp(curr_name, ".symtab") == 0)
        {
            symtab = (Elf32_Shdr *)curr_entry;
        }
        else if (strcmp(curr_name, ".dynsym") == 0)
        {
            dynsym = (Elf32_Shdr *)curr_entry;
        }
        else if (strcmp(curr_name, ".strtab") == 0)
        {
            str_table_temp = (Elf32_Shdr *)curr_entry;
        }
    }

    if (dynsym != NULL)
    {
        int num_of_entries = dynsym->sh_size / sizeof(Elf32_Sym);
        str_table = str_table_temp;
        printf("File %s \n", fileName);
        printf("Symbol table '.dynsym' \n");
        printf("[%5s] %-16s %-16s %-16s %-16s\n",
               "index", "value", "section index", "section name", "symbol name");
        for (int i = 0; i < num_of_entries; i++)
        {
            curr_entry = map + dynsym->sh_offset + (sizeof(Elf32_Sym) * i);
            char *section_name = sh_name(curr_entry->st_shndx);
            if (section_name == "UNKNOWN")
            {
                section_entry = map + elf_header->e_shoff + (curr_entry->st_shndx * elf_header->e_shentsize);
                section_name = map + str_table->sh_offset + section_entry->sh_name;
            }
            char *symbol_name = map + str_table->sh_offset + curr_entry->st_name;

            printf("[%d] %3s %-16x %-16d %-16s %-16s\n", i, "", curr_entry->st_value, curr_entry->st_shndx,
                   section_name, symbol_name);
        }

        if (debug_mode == 1)
        {
            printf("\nsize of symbol table: 0000%0x \n", dynsym->sh_size);
            printf("num of symbols: %d \n\n", num_of_entries);
        }
    }

    if (symtab != NULL)
    {
        int num_of_entries = symtab->sh_size / sizeof(Elf32_Sym);
        str_table = str_table_temp;
        if (dynsym == NULL)
            printf("\nFile %s \n", fileName);
        printf("Symbol table '.symtab' \n");
        printf("[%5s] %-16s %-16s %-24s %-16s\n",
               "index", "value", "section index", "section name", "symbol name");
        for (int i = 0; i < num_of_entries; i++)
        {
            curr_entry = map + symtab->sh_offset + (sizeof(Elf32_Sym) * i);
            char *section_name = sh_name(curr_entry->st_shndx);
            if (section_name == "UNKNOWN")
            {
                section_entry = map + elf_header->e_shoff + (curr_entry->st_shndx * elf_header->e_shentsize);
                section_name = map + str_table->sh_offset + section_entry->sh_name;
            }
            char *symbol_name = map + str_table->sh_offset + curr_entry->st_name;
            printf("[%2d] %3s %-16x %-16d %-24s %-16s\n", i, "", curr_entry->st_value, curr_entry->st_shndx,
                   section_name, symbol_name);
        }

        if (debug_mode == 1)
        {
            printf("\nsize of symbol table: 000%0x \n", symtab->sh_size);
            printf("num of symbols: %d \n", num_of_entries);
        }
    }
}

void print_Symbols(handle_file *currFd)
{
    if (numOpenFiles == 0)
    {
        printf("No mapped file");
        return;
    }
    else
    {
        if (numOpenFiles == 1)
            print_Symbols_For_File(currFd->map1, currFd->fileName1);
        else
        { // num of open files is 2
            print_Symbols_For_File(currFd->map1, currFd->fileName1);
            print_Symbols_For_File(currFd->map2, currFd->fileName2);
        }
    }
}

Elf32_Shdr *find_symbol_table(void *map)
{
    Elf32_Ehdr *elf_header = (Elf32_Ehdr *)map;
    Elf32_Shdr *symtab = NULL;
    Elf32_Shdr *str_table;
    Elf32_Sym *curr_entry;

    str_table = map + elf_header->e_shoff + (elf_header->e_shstrndx * elf_header->e_shentsize);

    for (int i = 0; i < elf_header->e_shnum && (symtab == NULL); i++)
    {
        curr_entry = map + elf_header->e_shoff + ((elf_header->e_shentsize) * i);
        char *curr_name = map + str_table->sh_offset + curr_entry->st_name;
        if (strcmp(curr_name, ".symtab") == 0)
        {
            return (Elf32_Shdr *)curr_entry;
        }
    }
    return NULL;
}

int check_num_symbol_tables(void* map){
    Elf32_Ehdr* elf_header = (Elf32_Ehdr*)map;
    Elf32_Shdr* symtab = NULL;
    Elf32_Shdr* dynsym = NULL;
    Elf32_Shdr* str_table;
    Elf32_Sym* curr_entry;

    str_table = map + elf_header->e_shoff + (elf_header->e_shstrndx * elf_header->e_shentsize);
    
    for(int i = 0; i < elf_header->e_shnum && (symtab == NULL || dynsym == NULL); i++){
        curr_entry = map + elf_header->e_shoff + ((elf_header->e_shentsize) * i);
        char* curr_name = map + str_table->sh_offset + curr_entry->st_name;
        if(strcmp(curr_name, ".symtab") == 0) {
            symtab = (Elf32_Shdr*)curr_entry;
        }
        else if(strcmp(curr_name, ".dynsym") == 0){
            dynsym = (Elf32_Shdr*)curr_entry;
        }
    }
    if(symtab != NULL && dynsym != NULL)
        return 0;
     
    return 1;
}

void check_Files_For_Merge(handle_file* currFd){
    if(numOpenFiles !=2){
         printf("2 ELF files required for merging\n");
         return;
    }
    void* map1 = currFd->map1;
    void* map2 = currFd->map2;

    int check_file1 = check_num_symbol_tables(map1);
    int check_file2 = check_num_symbol_tables(map2);

    if(check_file1 == 0 || check_file2 == 0){  //means there is more than one sym table 
        printf("feature not supported\n");
        return;
    }
    
    Elf32_Shdr* symtab_file1 = find_symbol_table(map1);
    Elf32_Shdr* symtab_file2 = find_symbol_table(map2);
    Elf32_Ehdr* elf_header_file1 = (Elf32_Ehdr*)(map1);
    Elf32_Ehdr* elf_header_file2 = (Elf32_Ehdr*)(map2);
    int num_of_entries_file1 = symtab_file1->sh_size / sizeof(Elf32_Sym);
    int num_of_entries_file2 = symtab_file2->sh_size / sizeof(Elf32_Sym);
    Elf32_Shdr* str_table1 = map1 + elf_header_file1->e_shoff + (symtab_file1->sh_link * elf_header_file1->e_shentsize);
    Elf32_Shdr* str_table2 = map2 + elf_header_file2->e_shoff + (symtab_file2->sh_link * elf_header_file2->e_shentsize);
    
    for(int i = 1; i < num_of_entries_file1; i++){
        Elf32_Sym* curr_entry1 = map1 + symtab_file1->sh_offset + (sizeof(Elf32_Sym) * i);
        char* section_name1 = sh_name(curr_entry1->st_shndx);
        char* symbol_name1 = map1 + str_table1->sh_offset + curr_entry1->st_name;
        if(curr_entry1->st_shndx == SHN_UNDEF){
            int found = 0;
            for(int j = 1; j < num_of_entries_file2 && (found == 0); j++){
                Elf32_Sym* curr_entry2 = map2 + symtab_file2->sh_offset + (sizeof(Elf32_Sym) * j);
                char* section_name2 = sh_name(curr_entry2->st_shndx);
                char* symbol_name2 = map2 + str_table2->sh_offset + curr_entry2->st_name;
                if(strcmp(symbol_name1, symbol_name2) == 0){
                    found = 1;
                    if(curr_entry2->st_shndx == SHN_UNDEF)
                        printf("symbol sym undefined\n");
                }
            }
            if(found == 0)
                printf("symbol sym undefined\n");
        }
        else{
            int found = 0;
            for(int j = 1; j < num_of_entries_file2 && (found == 0); j++){
                Elf32_Sym* curr_entry2 = map2 + symtab_file2->sh_offset + (sizeof(Elf32_Sym) * j);
                char* section_name2 = sh_name(curr_entry2->st_shndx);
                char* symbol_name2 = map2 + str_table2->sh_offset + curr_entry2->st_name;

                if((strcmp(symbol_name1, "") == 0) && (strcmp(symbol_name2, "") == 0))
                    continue;

                if(strcmp(symbol_name1, symbol_name2) == 0){
                    found = 1;
                    if(curr_entry2->st_shndx != SHN_UNDEF)
                        printf("symbol sym multiply defined\n");
                }
            }
        }          
    }
}

void merge_ELF_Files(handle_file *currFd){
    void *map1 = currFd->map1;
    void *map2 = currFd->map2;
    Elf32_Ehdr *elf_header1 = (Elf32_Ehdr *)map1;
    Elf32_Ehdr *elf_header2 = (Elf32_Ehdr *)map2;
    Elf32_Shdr *section_header1 = map1 + elf_header1->e_shoff;
    Elf32_Shdr *section_header2 = map2 + elf_header2->e_shoff;
    Elf32_Shdr* symtab_file1 = find_symbol_table(map1);
    Elf32_Shdr* symtab_file2 = find_symbol_table(map2);
    Elf32_Shdr* str_table1 = map1 + elf_header1->e_shoff + (symtab_file1->sh_link * elf_header1->e_shentsize);
    Elf32_Shdr* str_table2 = map2 + elf_header2->e_shoff + (symtab_file2->sh_link * elf_header2->e_shentsize);
    int num_of_entries_file1 = symtab_file1->sh_size / sizeof(Elf32_Sym);
    int num_of_entries_file2 = symtab_file2->sh_size / sizeof(Elf32_Sym);
   
    int out_file = open("out.ro", O_WRONLY | O_CREAT, 0666); // create a new ELF file
    if (out_file == -1) {
        printf("failed to create out file\n");
        return;
    }
    
    write(out_file, (char*)elf_header1, elf_header1->e_ehsize);    // copy initial version of elf1's header as the new ELF's header
   
    //declares an array of Elf32_Shdr. The size of the array is determined by header1->e_shnum, which represents the number of entries in the 
    //section header table of the first ELF file.                                                                             
    Elf32_Shdr section_header_arr[elf_header1->e_shnum]; 
    
    //copy the content of the section_header1 to the section_header_arr                                                                            
    memcpy((char*)section_header_arr, (char*)section_header1, elf_header1->e_shnum*sizeof(Elf32_Shdr));                                                                            

    for (int i = 0; i < elf_header1->e_shnum; i++){ // Loop over the entries of the new section header table
        section_header_arr[i].sh_offset = lseek(out_file, 0, SEEK_CUR);
        Elf32_Shdr *curr_section_header = map1 + elf_header1->e_shoff + ((elf_header1->e_shentsize) * i);
        char *curr_section_name = map1 + str_table1->sh_offset + curr_section_header->sh_name;
        if (strcmp(curr_section_name, ".text") == 0 || strcmp(curr_section_name, ".data") == 0 || strcmp(curr_section_name, ".rodata") == 0){
            // Copy the section contents from the first file
            write(out_file, (char *)elf_header1 + curr_section_header->sh_offset, curr_section_header->sh_size); 

            // Get the corresponding section header from the second file
            for(int j = 0; j< elf_header2->e_shnum; j++){
                Elf32_Shdr *curr_section_header2 = map2 + elf_header2->e_shoff + ((elf_header2->e_shentsize) * j);
                char *curr_section_name2 = map2 + str_table2->sh_offset + curr_section_header2->sh_name;
                if(strcmp(curr_section_name, curr_section_name2) == 0){
                    // Append the section contents from the second file
                    write(out_file, (char*)elf_header2 + section_header2->sh_offset, section_header2->sh_size); 
                    section_header_arr[i].sh_size = section_header_arr[i].sh_size + curr_section_header2->sh_size;
                }
            }
            // Set sh_offset after writing section contents
            curr_section_header->sh_offset = lseek(out_file, 0, SEEK_CUR); // Set sh_offset after writing section contents
        }
    

        else if(strcmp(curr_section_name, ".symtab") == 0){
            //declares an array of Elf32_Sym. The size of the array is num_of_entries_file1
            Elf32_Sym symtab[num_of_entries_file1];
            Elf32_Sym* curr_entry1 = map1 + symtab_file1->sh_offset + (sizeof(Elf32_Sym) * i);
            memcpy((char*)symtab, (char*)curr_entry1, section_header_arr[i].sh_size);
            
            for(int j=0 ; j< num_of_entries_file1; j++){
                 Elf32_Sym* curr_entry_j = map1 + symtab_file1->sh_offset + (sizeof(Elf32_Sym) * j);
                char* section_name1 = sh_name(curr_entry_j->st_shndx);
                char* symbol_name1 = map1 + str_table1->sh_offset + curr_entry_j->st_name;
                if(curr_entry_j->st_shndx == SHN_UNDEF){
                    for(int k = 0; k < num_of_entries_file2; k++){
                        Elf32_Sym* curr_entry2 = map2 + symtab_file2->sh_offset + (sizeof(Elf32_Sym) * k);
                        char* section_name2 = sh_name(curr_entry2->st_shndx);
                        char* symbol_name2 = map2 + str_table2->sh_offset + curr_entry2->st_name;
                        if(strcmp(symbol_name1, symbol_name2) == 0){
                            symtab[k].st_value = curr_entry2[k].st_value;
                            for (int x = 0; x < elf_header1->e_shnum; x++) {
                                Elf32_Shdr *curr_section_header_x = map1 + elf_header1->e_shoff + ((elf_header1->e_shentsize) * x);
                                char *curr_section_name_x = map1 + str_table1->sh_offset + curr_section_header_x->sh_name;
                                if (strcmp(curr_section_name_x,section_name2) == 0) {
                                    symtab[j].st_shndx = x;
                                }
                            }
                        }
                    }
                }
            }
            write(out_file, (char*)symtab, section_header1[i].sh_size);
        } 

        else {
            write(out_file, (char*)(map1 + section_header1[i].sh_offset), section_header1[i].sh_size);
        }
    }
    int sh_offset = lseek(out_file, 0, SEEK_CUR);
    write(out_file, (char*)section_header_arr, elf_header1->e_shnum * sizeof(Elf32_Shdr));
    lseek(out_file, 32, SEEK_SET);
    write(out_file, (char*)(&sh_offset), sizeof(int));
    close(out_file); // close the file after writing
}



void quit(handle_file *currFd){
    if (debug_mode == 1)
    { // debug mode is on
        printf("quitting\n");
    }

    if (currFd->fd1 != -1)
    {
        munmap(currFd->map1, currFd->file_info1.st_size);
        close(currFd->fd1);
        numOpenFiles--;
    }

    if (currFd->fd2 != -1)
    {
        munmap(currFd->map2, currFd->file_info2.st_size);
        close(currFd->fd2);
        numOpenFiles--;
    }
    exit(0);
}

int main(int argc, char **argv)
{
    handle_file *currFd = malloc(sizeof(handle_file));
    currFd->fd1 = -1;
    currFd->fd2 = -1;

    struct fun_desc menu[] = {{"Toggle Debug Mode", toggle_Debug_Mode},
                              {"Examine ELF File", examine_ELF_File},
                              {"Print Section Names", print_Section_Names},
                              {"Print Symbols", print_Symbols},
                              {"Check Files For Merge", check_Files_For_Merge},
                              {"Merge ELF Files", merge_ELF_Files},
                              {"Quit ", quit},
                              {NULL, NULL}};

    while (1)
    {
        int menuSize = sizeof(menu) / sizeof(struct fun_desc) - 1;
        int i = 0;

        fprintf(stdout, "Choose action:\n");
        while (menu[i].name != NULL)
        {
            printf("%d) %s\n", i, menu[i].name);
            i++;
        }

        int action = -1; // In order to save the option the user chose
        printf("action:\n");
        scanf("%d", &action);
        fgetc(stdin);

        if (feof(stdin)) // when user presses CTRL^D- exit
            break;

        if (action >= 0 && action < menuSize)
        {
            printf("Within bounds\n");
            printf("\n");
        }
        else
        {
            printf("Not within bounds\n");
            exit(0);
        }
        menu[action].fun(currFd);
        printf("\n");
    }
}