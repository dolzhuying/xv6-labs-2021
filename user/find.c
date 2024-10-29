#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

void find(char*path,char*filename);

int main(int argc,char*argv[]){
    if(argc!=3){
        fprintf(2,"usage:find root_dir target_fileanme");
        exit(1);
    }
    find(argv[1],argv[2]);//起始目录及目标文件名
    exit(0);

}

void find(char*path,char*target){
    char *p,buf[1024];
    int fd;
    struct dirent de;
    struct stat st;
    
    if((fd=open(path,0))<0){
      fprintf(2,"open failed\n");
      exit(1);
    }
    if(fstat(fd,&st)<0){//将当前path信息存储到st中
      fprintf(2,"fstat error\n");
      close(fd);
      exit(1);
    }
    if(strlen(path)+1+DIRSIZ+1>sizeof(buf)){
      fprintf(2,"path too long\n");
      close(fd);
      exit(1);
    }
    while(read(fd, &de, sizeof(de)) == sizeof(de)){//对于目录，读取目录下存储文件的数据结构
      if(de.inum==0||!strcmp(de.name,".")||!strcmp(de.name,".."))continue;//避免不存在或损坏文件，及. ..的循环递归
      strcpy(buf,path);
      p=buf+strlen(buf);
      *p++='/';//./aa/bb/vv/ 指向最后的/
      memmove(p,de.name,DIRSIZ);
      p[DIRSIZ]=0;
      
      if(stat(path,&st)<0)continue;
      switch(st.type){
        case T_FILE:
          if(!strcmp(de.name,target)){//符合条件则输出
            printf("%s\n",path);
          }
          break;
        case T_DIR:
          find(buf,target);//遇到目录则递归
          break;
      }
    }
    close(fd);
}




