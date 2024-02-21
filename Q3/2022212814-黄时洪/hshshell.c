#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/signal.h>
#include <sys/types.h>
#include <errno.h>
#include <pwd.h>

#define BUFF_SIZE 256
#define TRUE 1
#define FALSE 0

const char *COMMAND_EXIT = "exit";
const char *COMMAND_HELP = "help";
const char *COMMAND_CD = "cd";
const char *COMMAND_IN = "<";
const char *COMMAND_OUT = ">";
const char *COMMAND_PIPE = "|";
const char *COMMAND_CATCHAD = ">>";
const char *COMMAND_ERROUT = "2>";
const char *COMMAND_BOTHERRST = "&>";
const char *COMMAND_BOTHERRSTAD = "&>>";
const char *COMMAND_ERROUTAD = "2>>";

//内置的状态码
enum
{
    RESULT_NORMAL,            //正常状态
    ERROR_FORK,               //fork错误
    ERROR_COMMAND,            //命令不存在错误
    ERROR_WRONG_PARAMETER,    //错误的参数
    ERROR_MISS_PARAMETER,     //缺少参数
    ERROR_TOO_MANY_PARAMETER, //参数过多
    ERROR_CD,                 //改变目录错误
    ERROR_SYSTEM,             //系统错误
    ERROR_EXIT,               //退出错误

    //重定向的错误信息
    ERROR_MANY_IN,        //输入重定向符号过多
    ERROR_MANY_OUT,       //输出重定向符号过多
    ERROR_FILE_NOT_EXIST, //输入重定向文件不存在

    //管道的错误信息
    ERROR_PIPE,               //管道错误
    ERROR_PIPE_MISS_PARAMETER //管道参数缺失
};

char username[BUFF_SIZE];
char hostname[BUFF_SIZE];
char curPath[BUFF_SIZE];
char commands[BUFF_SIZE][BUFF_SIZE];

int isCommandExist(const char *command);
void getUsername();
void getHostname();
int getCurWorkDir();
int splitCommands(char command[BUFF_SIZE]);
int callExit();
int callCommand(int commandNum);
int callCommandWithPipe(int left, int right);
int callCommandWithRedi(int left, int right);
int callCd(int commandNum);

int main()
{
    //获取当前工作目录、用户名、主机名
    int result = getCurWorkDir();
    if (ERROR_SYSTEM == result)
    {
        fprintf(stderr, "\e[31;1mError: 获取当前工作目录失败.\n\e[0m");
        exit(ERROR_SYSTEM);
    }
    getUsername();
    getHostname();

    //启动hshshell
    char argv[BUFF_SIZE];
    while (TRUE)
    {
        printf("\e[35;1mhshshell~%s@%s:%s\e[0m$ ", username, hostname, curPath);
        //获取用户输入的命令
        fgets(argv, BUFF_SIZE, stdin);
        int len = strlen(argv);
        if (len != BUFF_SIZE)
        {
            argv[len - 1] = '\0';
        }

        int commandNum = splitCommands(argv);

        if (commandNum != 0)
        { //用户有输入指令
            if (strcmp(commands[0], COMMAND_EXIT) == 0)
            { //exit命令
                result = callExit();
                if (ERROR_EXIT == result)
                {
                    exit(-1);
                }
            }
            else if (strcmp(commands[0], COMMAND_CD) == 0)
            { //cd命令
                result = callCd(commandNum);
                switch (result)
                {
                case ERROR_MISS_PARAMETER:
                    fprintf(stderr, "\e[31;1mError: 使用命令时缺少参数 \"%s\".\n\e[0m", COMMAND_CD);
                    break;
                case ERROR_WRONG_PARAMETER:
                    fprintf(stderr, "\e[31;1mError: 该目录不存在 \"%s\".\n\e[0m", commands[1]);
                    break;
                case ERROR_TOO_MANY_PARAMETER:
                    fprintf(stderr, "\e[31;1mError: 使用命令时参数过多 \"%s\".\n\e[0m", COMMAND_CD);
                    break;
                case RESULT_NORMAL: //cd命令正常执行，更新当前工作l目录
                    result = getCurWorkDir();
                    if (ERROR_SYSTEM == result)
                    {
                        fprintf(stderr, "\e[31;1mError: 获取当前工作目录失败.\n\e[0m");
                        exit(ERROR_SYSTEM);
                    }
                    else
                    {
                        break;
                    }
                }
            }
            else if (strcmp(commands[0], COMMAND_HELP) == 0)
            { //help命令
                fprintf(stderr, "\e[ 这是一个由hsh开发的简易的shell，cd、help、exit命令由自己实现，其他命令均直接调用\n支持2>、>>、&>、&>>、2>>、pipe\n若有疑问，请将问题发至邮箱1952023661@qq.com\n\e[0m");
            }
            else
            { //其它命令
                result = callCommand(commandNum);
                switch (result)
                {
                case ERROR_FORK:
                    fprintf(stderr, "\e[31;1mError: Fork错误.\n\e[0m");
                    exit(ERROR_FORK);
                case ERROR_COMMAND:
                    fprintf(stderr, "\e[31;1mError: hshshell不支持该命令.\n\e[0m");
                    break;
                case ERROR_MANY_IN:
                    fprintf(stderr, "\e[31;1mError: 重定向符号过多 \"%s\".\n\e[0m", COMMAND_IN);
                    break;
                case ERROR_MANY_OUT:
                    fprintf(stderr, "\e[31;1mError: 重定向符号过多 \"%s\".\n\e[0m", COMMAND_OUT);
                    break;
                case ERROR_FILE_NOT_EXIST:
                    fprintf(stderr, "\e[31;1mError: 输入重定向文件不存在.\n\e[0m");
                    break;
                case ERROR_MISS_PARAMETER:
                    fprintf(stderr, "\e[31;1mError: 缺少重定向参数.\n\e[0m");
                    break;
                case ERROR_PIPE:
                    fprintf(stderr, "\e[31;1mError: 打开管道错误.\n\e[0m");
                    break;
                case ERROR_PIPE_MISS_PARAMETER:
                    fprintf(stderr, "\e[31;1mError: 缺少管道参数.\n\e[0m");
                    break;
                }
            }
        }
    }
}

int isCommandExist(const char *command)
{ //判断指令是否存在
    if (command == NULL || strlen(command) == 0)
        return FALSE;

    int result = TRUE;

    int fds[2];
    if (pipe(fds) == -1)
    {
        result = FALSE;
    }
    else
    {
        //暂存输入输出重定向标志
        int inFd = dup(STDIN_FILENO);
        int outFd = dup(STDOUT_FILENO);

        pid_t pid = vfork();
        if (pid == -1)
        {
            result = FALSE;
        }
        else if (pid == 0)
        {
            //将结果输出重定向到文件标识符
            close(fds[0]);
            dup2(fds[1], STDOUT_FILENO);
            close(fds[1]);

            char tmp[BUFF_SIZE];
            sprintf(tmp, "command -v %s", command);
            system(tmp);
            exit(1);
        }
        else
        {
            waitpid(pid, NULL, 0);
            //输入重定向
            close(fds[1]);
            dup2(fds[0], STDIN_FILENO);
            close(fds[0]);

            if (getchar() == EOF)
            { //没有数据，意味着命令不存在
                result = FALSE;
            }

            //恢复输入、输出重定向
            dup2(inFd, STDIN_FILENO);
            dup2(outFd, STDOUT_FILENO);
        }
    }

    return result;
}

void getUsername()
{ //获取当前登录的用户名
    struct passwd *pwd = getpwuid(getuid());
    strcpy(username, pwd->pw_name);
}

void getHostname()
{ //获取主机名
    gethostname(hostname, BUFF_SIZE);
}

int getCurWorkDir()
{ //获取当前的工作目录
    char *result = getcwd(curPath, BUFF_SIZE);
    if (result == NULL)
        return ERROR_SYSTEM;
    else
        return RESULT_NORMAL;
}

int splitCommands(char command[BUFF_SIZE])
{ //以空格分割命令， 返回分割得到的字符串个数
    int num = 0;
    int i, j;
    int len = strlen(command);

    for (i = 0, j = 0; i < len; ++i)
    {
        if (command[i] != ' ')
        {
            commands[num][j++] = command[i];
        }
        else
        {
            if (j != 0)
            {
                commands[num][j] = '\0';
                ++num;
                j = 0;
            }
        }
    }
    if (j != 0)
    {
        commands[num][j] = '\0';
        ++num;
    }

    return num;
}

int callExit()
{ //发送terminal信号退出进程
    pid_t pid = getpid();
    if (kill(pid, SIGTERM) == -1)
        return ERROR_EXIT;
    else
        return RESULT_NORMAL;
}

int callCommand(int commandNum)
{ //给用户使用的函数，用以执行用户输入的命令
    pid_t pid = fork();
    if (pid == -1)
    {
        return ERROR_FORK;
    }
    else if (pid == 0)
    {
        //获取标准输入、输出的文件标识符
        int inFds = dup(STDIN_FILENO);
        int outFds = dup(STDOUT_FILENO);

        int result = callCommandWithPipe(0, commandNum);

        //还原标准输入、输出重定向
        dup2(inFds, STDIN_FILENO);
        dup2(outFds, STDOUT_FILENO);
        exit(result);
    }
    else
    {
        int status;
        waitpid(pid, &status, 0);
        return WEXITSTATUS(status);
    }
}

int callCommandWithPipe(int left, int right)
{ //所要执行的指令区间[left, right)，可能含有管道
    if (left >= right)
        return RESULT_NORMAL;
    //判断是否有管道命令
    int pipeIdx = -1;
    for (int i = left; i < right; ++i)
    {
        if (strcmp(commands[i], COMMAND_PIPE) == 0)
        {
            pipeIdx = i;
            break;
        }
    }
    if (pipeIdx == -1)
    { //不含有管道命令
        return callCommandWithRedi(left, right);
    }
    else if (pipeIdx + 1 == right)
    { //管道命令'|'后续没有指令，参数缺失
        return ERROR_PIPE_MISS_PARAMETER;
    }

    //执行命令
    int fds[2];
    if (pipe(fds) == -1)
    {
        return ERROR_PIPE;
    }
    int result = RESULT_NORMAL;
    pid_t pid = vfork();
    if (pid == -1)
    {
        result = ERROR_FORK;
    }
    else if (pid == 0)
    { //子进程执行单个命令
        close(fds[0]);
        dup2(fds[1], STDOUT_FILENO); // 将标准输出重定向到fds[1]
        close(fds[1]);

        result = callCommandWithRedi(left, pipeIdx);
        exit(result);
    }
    else
    { //父进程递归执行后续命令
        int status;
        waitpid(pid, &status, 0);
        int exitCode = WEXITSTATUS(status);

        if (exitCode != RESULT_NORMAL)
        { //子进程的指令没有正常退出，打印错误信息
            char info[4096] = {0};
            char line[BUFF_SIZE];
            close(fds[1]);
            dup2(fds[0], STDIN_FILENO); // 将标准输入重定向到fds[0]
            close(fds[0]);
            while (fgets(line, BUFF_SIZE, stdin) != NULL)
            { //读取子进程的错误信息
                strcat(info, line);
            }
            printf("%s", info); //打印错误信息

            result = exitCode;
        }
        else if (pipeIdx + 1 < right)
        {
            close(fds[1]);
            dup2(fds[0], STDIN_FILENO); //将标准输入重定向到fds[0]
            close(fds[0]);
            result = callCommandWithPipe(pipeIdx + 1, right); //递归执行后续指令
        }
    }

    return result;
}

int callCommandWithRedi(int left, int right)
{ //所要执行的指令区间[left, right)，不含管道，可能含有重定向
    if (!isCommandExist(commands[left]))
    { //指令不存在
        return ERROR_COMMAND;
    }

    //判断是否有重定向
    int inNum = 0, outNum = 0;
    char *inFile = NULL, *outFile = NULL;
    int endIdx = right; //指令在重定向前的终止下标
    //支持追加重定向
    char *appendOutFile = NULL;
    int appendOutNum = 0;
    //支持标准错误输出重定向
    char *errOutFile = NULL;
    int errOutNum = 0;
    //支持同时重定向标准输出和标准错误输出
    char *bothOutFile = NULL;
    int bothOutNum = 0;
    //支持同时追加重定向标准输出和标准错误输出
    char *bothAppendOutFile = NULL;
    int bothAppendOutNum = 0;
    //支持追加重定向标准错误输出
    char *appendErrOutFile = NULL;
    int appendErrOutNum = 0;

    for (int i = left; i < right; ++i)
    {
        if (strcmp(commands[i], COMMAND_CATCHAD) == 0) 
        { //追加输出重定向
            ++appendOutNum;
            if (i + 1 < right)
                appendOutFile = commands[i + 1];
            else
                return ERROR_MISS_PARAMETER; //重定向符号后缺少文件名

            if (endIdx == right) endIdx = i;
        }
        else if (strcmp(commands[i], COMMAND_ERROUT) == 0) 
        { //标准错误输出重定向
            ++errOutNum;
            if (i + 1 < right)
                errOutFile = commands[i + 1];
            else
                return ERROR_MISS_PARAMETER;

            if (endIdx == right) endIdx = i;
        }
        else if (strcmp(commands[i], COMMAND_BOTHERRST) == 0) 
        { //同时重定向标准输出和标准错误输出
            ++bothOutNum;
            if (i + 1 < right)
                bothOutFile = commands[i + 1];
            else
                return ERROR_MISS_PARAMETER;

            if (endIdx == right) endIdx = i;
        }
        else if (strcmp(commands[i], COMMAND_BOTHERRSTAD) == 0) 
        { //同时追加重定向标准输出和标准错误输出
            ++bothAppendOutNum;
            if (i + 1 < right)
                bothAppendOutFile = commands[i + 1];
            else
                return ERROR_MISS_PARAMETER;

            if (endIdx == right) endIdx = i;
        }
        else if (strcmp(commands[i], COMMAND_ERROUTAD) == 0) 
        { //追加重定向标准错误输出
        ++appendErrOutNum;
        if (i + 1 < right)
            appendErrOutFile = commands[i + 1];
        else
            return ERROR_MISS_PARAMETER;

        if (endIdx == right) endIdx = i;
        }
        else if (strcmp(commands[i], COMMAND_IN) == 0)
        { //输入重定向
            ++inNum;
            if (i + 1 < right)
                inFile = commands[i + 1];
            else
                return ERROR_MISS_PARAMETER;

            if (endIdx == right)
                endIdx = i;
        }
        else if (strcmp(commands[i], COMMAND_OUT) == 0)
        { //输出重定向
            ++outNum;
            if (i + 1 < right)
                outFile = commands[i + 1];
            else
                return ERROR_MISS_PARAMETER;

            if (endIdx == right)
                endIdx = i;
        }
    }
    //处理重定向
    if (inNum == 1)
    {
        FILE *fp = fopen(inFile, "r");
        if (fp == NULL) //输入重定向文件不存在
            return ERROR_FILE_NOT_EXIST;

        fclose(fp);
    }

    if (inNum > 1)
    { //输入重定向符超过一个
        return ERROR_MANY_IN;
    }
    else if (outNum > 1 || appendOutNum > 1 || errOutNum > 1 || bothOutNum > 1 || bothAppendOutNum > 1 || appendErrOutNum > 1)
    { //输出重定向符超过一个
        return ERROR_MANY_OUT;
    }
    int result = RESULT_NORMAL;
    pid_t pid = vfork();
    if (pid == -1)
    {
        result = ERROR_FORK;
    }
    else if (pid == 0)
    {
        //输入输出重定向
        if (inNum == 1)
            freopen(inFile, "r", stdin);
        if (outNum == 1 || bothOutNum == 1)
            freopen(bothOutNum == 1 ? bothOutFile : outFile, "w", stdout);
        if (appendOutNum == 1 || bothAppendOutNum == 1)
            freopen(bothAppendOutNum == 1 ? bothAppendOutFile : appendOutFile, "a", stdout); 
        if (errOutNum == 1 || bothOutNum == 1)
            freopen(bothOutNum == 1 ? bothOutFile : errOutFile, "w", stderr);
        if (bothAppendOutNum == 1 || appendErrOutNum == 1)
            freopen(appendErrOutNum == 1 ? appendErrOutFile : bothAppendOutFile, "a", stderr);

        //执行命令
        char *comm[BUFF_SIZE];
        for (int i = left; i < endIdx; ++i)
            comm[i] = commands[i];
        comm[endIdx] = NULL;
        execvp(comm[left], comm + left);
        exit(errno); //执行出错，返回errno
    }
    else
    {
        int status;
        waitpid(pid, &status, 0);
        int err = WEXITSTATUS(status); // 读取子进程的返回码

        if (err)
        { //返回码不为0，意味着子进程执行出错，用红色字体打印出错信息
            printf("\e[31;1mError: %s\n\e[0m", strerror(err));
        }
    }

    return result;
}

int callCd(int commandNum)
{ //执行cd命令
    int result = RESULT_NORMAL;

    if (commandNum < 2)
    {
        result = ERROR_MISS_PARAMETER;
    }
    else if (commandNum > 2)
    {
        result = ERROR_TOO_MANY_PARAMETER;
    }
    else
    {
        int ret = chdir(commands[1]);
        if (ret)
            result = ERROR_WRONG_PARAMETER;
    }

    return result;
}
