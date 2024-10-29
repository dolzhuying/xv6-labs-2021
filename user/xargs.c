#include "kernel/param.h"
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

// 创建子进程执行命令
void execargs(char *argv[], char *args[MAXARG]) {
    int pid = fork();
    if (pid < 0) {
        fprintf(2, "fork error\n");
        exit(-1);
    } else if (pid == 0) {
        exec(argv[1], args);
        exit(1);
    } else {
        wait(0);
    }
}

//在这里，argv为xargs开始的参数
int main(int argc, char *argv[]) {
     if (argc < 2) {
        fprintf(2, "usage: xargs (command args)\n");
        exit(-1);
    }
    if (argc + 1 > MAXARG) {
        fprintf(2, "too many args\n");
        exit(-1);
    }
    char *args[MAXARG];//存储参数
    char buf[1024];
    for(int i=1;i<argc;i++){
        args[i-1]=argv[i]; //最后一位 args[argc-1] 留给从标准输入读取的参数
    }
    while(1){
        int arg_size=0;
        while(1){//循环逐个字符读取来自管道的标准输入的参数，以换行符为分界，逐个字符读取可保证一定能读取完，也可以避免buf溢出的问题
            int n=read(0,&buf[arg_size],1);
            if(buf[arg_size]=='\n'||n==0)break;
            arg_size++;
        }
        if(arg_size==0)break;//全部读取完
        buf[arg_size]=0;//尾部\0
        args[argc-1]=buf;////最后一位 args[argc-1] 留给从标准输入读取的参数
        execargs(argv,args);//创建子进程并传递参数执行指令
    }
    exit(0);
}

