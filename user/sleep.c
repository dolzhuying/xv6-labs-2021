#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

//sleep本身有系统调用的接口，在这里进行二次封装成一个可由用户在终端执行的用户程序
int main(int argc,char*argv[]){
    if(argc!=2){//判断参数数量
        fprintf(2,"usage:sleep time");
        exit(1);
    }
    int n=atoi(argv[1]);//转成整数，获取休眠时间
    sleep(n);//sleep系统调用
    exit(0);
}