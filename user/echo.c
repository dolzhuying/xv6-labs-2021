#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

//将命令行参数打印到标准输出
int
main(int argc, char *argv[])//argc：参数数量， argv[0] 是程序名称，argv[1] 及以后的元素是传递给程序的参数。
{
  int i;

  for(i = 1; i < argc; i++){//对每一个参数处理，
    write(1, argv[i], strlen(argv[i]));//将参数写入1（标准输出文件描述符）
    if(i + 1 < argc){//对每个参数间加上空格
      write(1, " ", 1);
    } else {
      write(1, "\n", 1);//末尾换行
    }
  }
  exit(0);
}
