#include <elf.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>

extern int startup(int argc, char **argv, void (*start));

Elf32_Ehdr* header;
Elf32_Phdr* program_header;

int foreach_phdr(void *map_start, void (*func)(Elf32_Phdr *,int), int arg){
    header = (Elf32_Ehdr*)map_start;
    program_header = (Elf32_Phdr*)(map_start + header->e_phoff);
    for(int i = 0; i < header->e_phnum; i++){
        func(&program_header[i], arg);
    }
    return 0;
}

char* p_type(int p_type){
    if(p_type == PT_NULL)
        return "NULL";
    if(p_type == PT_LOAD)
        return "LOAD";
    if(p_type == PT_DYNAMIC)
        return "DYNAMIC";
    if(p_type == PT_INTERP)
        return "INTERP";
    if(p_type == PT_NOTE)
        return "NOTE)";
    if(p_type == PT_SHLIB)
        return "SHLIB";
    if(p_type == PT_PHDR)
        return "PHDR";
    if(p_type ==  PT_TLS)
        return "TLS";
    if(p_type == PT_NUM)
        return "NUM";
    if(p_type == PT_LOOS)
        return "LOOS";
    if(p_type == PT_GNU_EH_FRAME)
        return "GNU_EH_FRAME";
    if(p_type == PT_GNU_STACK)
        return "GNU_STACK";
    if(p_type == PT_GNU_RELRO)
        return "GNU_RELRO";
    if(p_type == PT_LOSUNW)
        return "LOSUNW";
    if(p_type == PT_SUNWBSS)
        return "SUNWBSS";
    if(p_type == PT_HISUNW)
        return "HISUNW";
    if(p_type == PT_HIOS)
        return "HIOS";
    if(p_type == PT_LOPROC)
        return "LOPROC";
    if(p_type == PT_HIPROC)
        return "HIPROC";
    else
        return "UNKNOWN";
}

void readElf(Elf32_Phdr* program_header, int index){
    char* flag = malloc(4);
    int protection_flag = 0; 
    int mapping_flag = 0;
    mapping_flag += (int)MAP_PRIVATE;
    mapping_flag += (int)MAP_FIXED;

    if(program_header->p_flags == PF_R){
        flag = "R";
        protection_flag = PROT_READ;
    }

    else if(program_header->p_flags == PF_W){
        flag = "W";
        protection_flag = PROT_WRITE;
    }

    else if(program_header->p_flags == PF_X){
        flag = "E";
        protection_flag = PROT_EXEC;
    }

    else if(program_header->p_flags == (PF_R | PF_W)){
        flag = "R W";
        protection_flag = PROT_READ | PROT_WRITE;
    }

    else if(program_header->p_flags == (PF_R | PF_X)){
        flag = "R E";
        protection_flag = PROT_READ | PROT_EXEC;
    }
    
    else if(program_header->p_flags == (PF_W | PF_X)){
        flag = "W E";
        protection_flag = PROT_WRITE | PROT_EXEC;
    }

    else if(program_header->p_flags == (PF_R | PF_W | PF_X)){
        flag = "R W E";
        protection_flag = PROT_READ | PROT_WRITE | PROT_EXEC;
    }

    else{
        flag = NULL;
        protection_flag = PROT_NONE;
    }

    printf("%s\t 0x%06x \t 0x%06x \t 0x%06x \t 0x%02x \t\t 0x%02x \t\t %s\t  0x%02x\t %x \t %x \t\n",
        p_type(program_header->p_type), program_header->p_offset, program_header->p_vaddr, program_header->p_paddr, program_header->p_filesz, program_header->p_memsz,
        flag, program_header->p_align, protection_flag, mapping_flag);
}

void load_phdr(Elf32_Phdr *phdr, int fd){
    if(phdr->p_type == PT_LOAD){
        int mapping_flag = 0;
        mapping_flag += (int)MAP_PRIVATE;
        mapping_flag += (int)MAP_FIXED;
        int vaddr = phdr->p_vaddr&0xfffff000;
        int offset = phdr->p_offset&0xfffff000;
        int padding = phdr->p_vaddr & 0xfff;
        void* map = mmap((void*)vaddr, phdr->p_memsz+padding, phdr->p_flags, mapping_flag, fd, offset); 
    }  
}

int main(int argc, char **argv){
    struct stat file_info; //gives information about the file

    if(argc == 1){
        printf("please provide file name");
        exit(1);
    }

    int file = open(argv[1], O_RDONLY);

    if(file < 0){
    perror("failed opening the file");
    exit(1);
    }

    if(fstat(file, &file_info) < 0){ //fstat retrieve information about the file
        perror("fstat filed");
        close(file);
        exit(1);
    }

    void* map_start = mmap(NULL, file_info.st_size, PROT_READ, MAP_PRIVATE, file, 0);
    printf("Type \t Offset \t VirtAddr \t PhysAddr \t FileSiz \t MemSiz \t Flg \t Align \t protFlag \t mapFlag\n");
    foreach_phdr(map_start, readElf, 0);
    foreach_phdr(map_start, load_phdr, file) ;
    startup(argc-1, argv+1, (void *)(header->e_entry));
    munmap(map_start, file_info.st_size); //removes mapping

    close(file);
    return 0;
}