// Simple grep.  Only supports ^ . * $ operators.

#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

char buf[1024];//存储读取的输入数据
int match(char*, char*);//匹配正则表达式

void
grep(char *pattern, int fd)//接受一个模式和文件描述符，读取输入并查找匹配的行。
{
  int n, m;//n:读取的字符数量，m：已读取的字符数量
  char *p, *q;//截取字符串，遍历缓冲区

  m = 0;
  while((n = read(fd, buf+m, sizeof(buf)-m-1)) > 0){//循环读取数据到buf
    m += n;//已经读取的字符
    buf[m] = '\0';//当前缓冲区已经读取数据末尾添加\0，方便处理字符串
    p = buf;
    while((q = strchr(p, '\n')) != 0){
      *q = 0;//查找每一行结束符\n并替换为\0，方便处理字符串
      if(match(pattern, p)){
        *q = '\n';
        write(1, p, q+1 - p);//调用 match 检查当前行是否匹配模式，如果匹配，则恢复换行符并输出。
      }
      p = q+1;//p指向q下一位，开始检查新一段的字符串
    }
    if(m > 0){
      m -= p - buf;//如果缓冲区中仍有数据，更新缓冲区,去除已处理部分，
      memmove(buf, p, m);//将剩余的部分移动到缓冲区的开头，以便下一次读取时能正确处理，等于p重新指向buf[0]
    }
  }
}

int
main(int argc, char *argv[])
{
  int fd, i;
  char *pattern;

  if(argc <= 1){
    fprintf(2, "usage: grep pattern [file ...]\n");
    exit(1);
  }
  pattern = argv[1];

  if(argc <= 2){
    grep(pattern, 0);//如果没有文件名，从标准输入读取并调用 grep。
    exit(0);
  }

  for(i = 2; i < argc; i++){
    if((fd = open(argv[i], 0)) < 0){
      printf("grep: cannot open %s\n", argv[i]);
      exit(1);
    }
    grep(pattern, fd);
    close(fd);
  }
  exit(0);
}

// Regexp matcher from Kernighan & Pike,
// The Practice of Programming, Chapter 9.

int matchhere(char*, char*);
int matchstar(int, char*, char*);

int
match(char *re, char *text)
{
  if(re[0] == '^')
    return matchhere(re+1, text);
  do{  // must look at empty string
    if(matchhere(re, text))
      return 1;
  }while(*text++ != '\0');
  return 0;
}

// matchhere: search for re at beginning of text
int matchhere(char *re, char *text)
{
  if(re[0] == '\0')
    return 1;
  if(re[1] == '*')
    return matchstar(re[0], re+2, text);
  if(re[0] == '$' && re[1] == '\0')
    return *text == '\0';
  if(*text!='\0' && (re[0]=='.' || re[0]==*text))
    return matchhere(re+1, text+1);
  return 0;
}

// matchstar: search for c*re at beginning of text
int matchstar(int c, char *re, char *text)
{
  do{  // a * matches zero or more instances
    if(matchhere(re, text))
      return 1;
  }while(*text!='\0' && (*text++==c || c=='.'));
  return 0;
}

