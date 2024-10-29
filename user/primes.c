// const int N = 1e8 + 10;
// int st[N], primes[N];
// void Ey(int n)//埃氏筛
// {
// 	st[0], st[1] = 1;
// 	for (int i = 2; i <= n / i; i++)
// 	{
// 		if (!st[i])
// 		{
// 			for (int j = i * i; j <= n; j += i)
// 			{
// 				st[j] = 1;
// 			}
// 		}
// 	}
// }

#include"kernel/types.h"
#include"user/user.h"

void pass_primes(int*pf);

int main(int argc,int *argv[]){
    if(argc!=1){
        fprintf(2,"usage:prime\n");
        exit(1);
    }
    //pipe_latter，当前进程与下一个进程通信的管道
    int pl[2];
    pipe(pl);
    int pid=fork();
    if(pid>0){
        close(pl[0]);//关闭pipelatter读端
        for(int i=2;i<=35;i++){
            write(pl[1],&i,sizeof(i));//向第一个子进程传递所有数字
        }
        close(pl[1]);//关闭写端
        wait(0);//等待子进程结束
        exit(0);
    }
    else if(pid==0){
        pass_primes(pl);
        exit(0);
    }
    else{
        close(pl[0]);
        close(pl[1]);
        exit(1);
    }
}

void pass_primes(int*pf){//pipe_former，当前进程与上一个进程通信的管道
    close(pf[1]);//关闭pipeformer的写端
    int prime;
    int n=read(pf[0],&prime,sizeof(prime));//读取读取第一个数字，必定是素数
    if(n==0){//已经全部读取完毕，返回
        close(pf[0]);
        return;
    }
    printf("prime %d\n",prime);
    int pl[2];//当前进程的pipelatter
    pipe(pl);
    int pid=fork();
    int read_num,t;
    if(pid>0){
        close(pl[0]);//当前进程为父进程，关闭pipelatter的读端
        while(read(pf[0],&read_num,4)==4){//逐个读取父进程传递的数字，是素数则向下一个进程传递(32位int)
            if(read_num%prime){
                t=read_num;
                write(pl[1],&t,sizeof(t));
            }
        }
        close(pf[0]);//读取完毕，关闭pipeformer的读端
        close(pl[1]);//写入完毕，关闭pipelatter写端
        wait(0);
        exit(0);
    }
    else if(pid==0){
        pass_primes(pl);//递归。子进程递归为下一阶段的当前进程
        exit(0);
    }
    else{//创建进程失败
        close(pf[0]);
        close(pl[0]);
        close(pl[1]);
        exit(1);
    }
}
