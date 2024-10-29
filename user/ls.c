#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

char*
fmtname(char *path)
{
  static char buf[DIRSIZ+1];
  char *p;

  for(p=path+strlen(path); p >= path && *p != '/'; p--)
    ;
  p++;//e.g. ./a/b/ccc 指向ccc的第一个c

  if(strlen(p) >= DIRSIZ)
    return p;
  memmove(buf, p, strlen(p));//将ccc的第一个c移动到buf开头
  memset(buf+strlen(p), ' ', DIRSIZ-strlen(p));//ccc后面的buf剩余部分以空格填充
  return buf;
}

void
ls(char *path)
{
  char buf[512], *p;
  int fd;
  struct dirent de;
  /* fs.h中的定义
  #define DIRSIZ 14  文件长度最大值

  struct dirent {
    ushort inum;  存储文件的inode号：文件系统内部用来标识该文件的唯一编号
    char name[DIRSIZ]; 文件名称
  };

  */
  struct stat st;
  /* ulib.c中的定义
  int
  stat(const char *n, struct stat *st)
  {
    int fd;
    int r;

    fd = open(n, O_RDONLY); 只读模式打开
    if(fd < 0)
      return -1;
    r = fstat(fd, st); 存储文件信息
    close(fd);
    return r;  fstat成功返回0 失败-1
  }
  */

  if((fd = open(path, 0)) < 0){//使用 open 系统调用来打开一个目录时，返回的文件描述符 fd 指向的是该目录的数据结构
    fprintf(2, "ls: cannot open %s\n", path);
    return;
  }

  if(fstat(fd, &st) < 0){
    /*
    system call 用于存储打开的文件的信息存储到st
    int fstat(int fd, struct stat*);
    */
    fprintf(2, "ls: cannot stat %s\n", path);
    close(fd);
    return;
  }

  switch(st.type){
  case T_FILE:
    printf("%s %d %d %l\n", fmtname(path), st.type, st.ino, st.size);//文件则输出
    break;

  case T_DIR:
    if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){//检查构建完整路径后的长度是否超过buf的大小限制
      printf("ls: path too long\n");
      break;
    }
    strcpy(buf, path);
    p = buf+strlen(buf);
    *p++ = '/';//假设目录是aaa =>./aaa/ 指向末尾/

    //对于每个有效的目录条目（即inum不为0），构建完整路径并使用stat获取文件状态信息
    //从目录中读取一个目录项，并将其存储在 de 结构体中。
    while(read(fd, &de, sizeof(de)) == sizeof(de)){
      if(de.inum == 0)
        continue;
      memmove(p, de.name, DIRSIZ);//拼接文件名到路径
      p[DIRSIZ] = 0;
      if(stat(buf, &st) < 0){//buf指向文件完整路径开头
        printf("ls: cannot stat %s\n", buf);
        continue;
      }
      printf("%s %d %d %d\n", fmtname(buf), st.type, st.ino, st.size);
    }
    break;
  }
  close(fd);
}

int
main(int argc, char *argv[])
{
  int i;

  if(argc < 2){
    ls(".");
    exit(0);
  }
  for(i=1; i<argc; i++)
    ls(argv[i]);
  exit(0);
}
