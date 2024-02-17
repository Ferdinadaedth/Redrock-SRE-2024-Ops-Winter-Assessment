//cd命令和pwd命令需要改变/输出的是父进程的目录，所以不用mysh来实现，需要在mysh外定义
#include"head.h"
//判断cd命令输入的参数个数是否合法
bool argumentNum(char *cmd, int num, char *tips){
    int arg = 0;
    //统计空格数，cd命令空格数应该是1
    for(int i = 0; i < strlen(cmd); i++){
        if(cmd[i] == ' ') arg++;
    }
    if(arg != num){
        fprintf(stderr, "Usage: %s\n", tips);//stderr是标准错误流
        return false;
    }
    return true;
}
//pwd函数
void mypwd(){
    char dirbuf[100];
    getcwd(dirbuf, 100);
    printf("%s\n", dirbuf);
    return ;
}
//cd函数
void mycd(char *cmd){
    char *pathname = strstr(cmd, " ") + 1;//找到字符串cmd中第一个空格字符的位置，并将该位置的下一个字符的指针赋值给pathname
    //printf("pathname:%s\n", pathname);
    //判断是否有此目录（能否打开）
    if((opendir(pathname)) == NULL){
        perror("opendir");//当系统调用失败时，perror函数可以打印出相应的错误信息
        return ;
    }
    //改变工作目录
    chdir(pathname);
    return ;

}
//子进程调用sh来执行其他函数ls ps等
void mysh(char *cmd){
    pid_t pid;//在父进程中，fork返回新创建子进程的进程ID；在子进程中，fork返回0；如果出现错误，fork返回一个负值；
    if((pid = fork()) < 0){
        perror("fork");
        return ;
    }
    if(pid == 0){
        if(execl("/bin/sh", "sh", "-c", cmd, NULL) < 0){
            perror("execl");
            return ;
        }
    }else{
        wait(NULL);//等待子进程结束，这个wait还蛮重要的，避免了“僵尸进程”和“孤儿进程”
        return ;
    }
}

int main(int argc, char **argv){
    char cmd[100];
    while(1){
	printf("$ ");
        scanf("%[^\n]s", cmd);//使用正则表达式直到读取到回车才停下，若使用%s遇到空格就会停止读取
        getchar();
        char *cmdp = NULL;
        if((cmdp = strstr(cmd, "pwd")) != NULL){
            //printf("pwd\n");
            mypwd();
        }else if((cmdp = strstr(cmd, "cd")) != NULL){
            //printf("cd\n");
            if(argumentNum(cmd, 1, "cd dirname")){
                mycd(cmd);
            }else continue;
        }else{
            mysh(cmd);
        }
    }
    return 0;
}
