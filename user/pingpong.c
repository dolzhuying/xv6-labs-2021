#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc,char*argv[]){
    if(argc!=1){
        fprintf(2,"usage:pingpong\n");
        exit(1);
    }
    int p1[2],p2[2];//管道文件描述符,分别用于父传子和子传父
    //创建管道并处理失败情况
    if(pipe(p1)<0){
        fprintf(2,"pipe create failed\n");
        exit(1);
    }
    if(pipe(p2)<0){
        fprintf(2,"pipe create failed\n");
        exit(1);
    }
    char buf[10];//从管道读取出数据的缓冲区
    int pid=fork();
    if(pid==0){//子进程
        read(p1[0],buf,10);//先从父进程读取数据，在没有读取到父进程写入的数据之前会处于阻塞状态
        //子进程p1管道用于读取数据，不需要写入，读取完成后子进程不再使用该管道，关闭管道读写端口释放资源，防止资源泄露和不必要的阻塞
        //具体来讲，当子进程完成数据读取后，关闭管道的写端口可以通知父进程没有更多的数据会被写入
        //关闭写端口后，父进程在尝试读取管道数据时，如果管道已经空并且写端被关闭，读取操作会返回0，表示对端已关闭。这是一个明确的信号，指示父进程不再需要继续读取
        close(p1[0]);
        close(p1[1]);
        printf("%d:received ping\n",getpid());
        write(p2[1],"pong",4);
        close(p2[0]);
        close(p2[1]);
    }else if(pid>0){//父进程
        write(p1[1],"ping",4);
        close(p1[0]);
        close(p1[1]);
        read(p2[0],buf,10);
        close(p2[0]);
        close(p2[1]);
        printf("%d:received pong\n",getpid());
    }
    exit(0);


}