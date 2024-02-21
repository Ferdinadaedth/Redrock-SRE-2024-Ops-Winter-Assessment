// 引入标准输入输出库
#include <stdio.h>
// 引入标准库头文件。
#include <stdlib.h>
// 引入字符串操作库，提供字符串处理的函数
#include <string.h>
// 引入POSIX操作系统API，提供UNIX标准的函数定义
#include <unistd.h>
// 引入错误号定义库，提供通过错误号访问错误描述的方法
#include <errno.h>
// 引入信号处理库，提供信号处理的函数和宏定义。
#include <signal.h>
// 引入基本系统数据类型库，提供一些数据类型的定义
#include <sys/types.h>
// 引入等待子进程状态改变信息库，提供等待子进程相关的函数
#include <sys/wait.h>
// 引入文件状态库，提供获取文件状态的函数
#include <sys/stat.h>
// 引入文件控制库，提供对文件描述符的操作函数。
#include <fcntl.h>
// 引入GNU Readline库，提供读取输入行的函数
#include <readline/readline.h>
// 引入GNU Readline历史操作库，提供对交互输入历史的操作函数
#include <readline/history.h>

// 定义初始缓冲区大小为1024字节
#define INITIAL_BUFSIZE 1024


// 声明内建命令的处理函数
int exec_cd(char** char_list);
int exec_pwd(char** char_list);
int exec_help(char** char_list);
int exec_exit(char** char_list);
int exec_echo(char** char_list);
int exec_mkdir(char** char_list);

// 定义内建命令的字符串数组
static char *builtin_commands[] = {
    "cd",
    "pwd",
    "help",
    "exit",
    "echo",
    "mkdir"
};


/**
 * 内建命令数量计算函数
 *
 * @return 返回内建命令数组的长度
 */
size_t num_of_cmds(){
    // 使用数组的总大小除以单个元素的大小得到数组长度
    return sizeof(builtin_commands)/sizeof(char*);
}


/**
 * 错误信息打印函数
 *
 * @param function_name 出错的函数名
 * @param error_message 错误信息描述
 */
void print_error(const char* function_name, const char* error_message) {
    // 向stderr打印错误信息，包括触发错误的函数名和错误描述
    fprintf(stderr, "Error in %s: %s\n", function_name, error_message);
}


// 信号处理函数
void handle_signal(int signo) {
    // 输出新行以避开当前行，重新提示输入
    printf("\nCyShell-> ");
    // 确保提示符立即显示
    fflush(stdout); 
}


// 设置信号处理函数
void setup_signal_handlers() {
    struct sigaction sa;

    // 设置处理SIGINT和SIGTSTP的函数为handle_signal
    sa.sa_handler = &handle_signal;
    // 初始化信号屏蔽集为空
    sigemptyset(&sa.sa_mask);
    // 重新调用被信号中断的系统调用
    sa.sa_flags = SA_RESTART;

    // 为SIGINT设置信号处理函数
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("Error setting signal handler for SIGINT");
        exit(EXIT_FAILURE);
    }

    // 为SIGTSTP设置信号处理函数
    if (sigaction(SIGTSTP, &sa, NULL) == -1) {
        perror("Error setting signal handler for SIGTSTP");
        exit(EXIT_FAILURE);
    }
}


// 内建命令处理函数指针数组
static int (*funcs[])(char**) = {
    &exec_cd,
    &exec_pwd,
    &exec_help,
    &exec_exit,
    &exec_echo,
    &exec_mkdir
};


/**
 * 缓冲区按需重分配函数
 * 
 * @param buffer 指向当前缓冲区的指针
 * @param bufsize 指向存储当前缓冲区大小的变量的指针
 * @param current_size 实际使用的缓冲区大小
 * @return 返回新分配的缓冲区的指针，或者如果不需要重新分配则返回原始指针
 */
char *realloc_buffer_if_needed(char *buffer, size_t *bufsize, size_t current_size) {
    // 检查当前使用的大小是否达到或超过了缓冲区的大小
    if (current_size >= *bufsize) {
        // 计算新的缓冲区大小，设为当前大小的两倍
        size_t new_size = *bufsize * 2;

        // 尝试重新分配更大的缓冲区
        char *new_buffer = realloc(buffer, new_size);

        // 检查重新分配是否成功
        if (!new_buffer) {
            // 如果失败则向标准错误输出错误信息
            fprintf(stderr, "allocation error\n");
            // 返回NULL表示内存分配失败
            return NULL;
        }

        // 如果重新分配成功则更新缓冲区大小的记录
        *bufsize = new_size;
        // 将指向新缓冲区的指针赋给buffer
        buffer = new_buffer;
    }

    // 返回新缓冲区的指针，若没有重新分配则返回原始指针
    return buffer;
}


/**
 * cd命令实现函数
 * 
 * @param char_list 通过空格分割后的用户输入字符串数组，其中char_list[0]是"cd"，char_list[1]是目标目录
 * @return 总是返回1，表示shell应该继续运行
 */
int exec_cd(char** char_list){
    // 检查'cd'命令后是否附带有目标目录参数
    if(char_list[1] == NULL){
        // 如果没有提供参数则打印错误信息
        print_error("cd", "expected argument to \"cd\"");
    } else {
        // 尝试改变到指定的目录
        if(chdir(char_list[1]) != 0){
            // 如果目录改变失败则输出错误信息
            perror("cyshell");
        }
    }

    return 1;
}


//pwd命令实现函数
int exec_pwd(char** char_list){
    // 初始缓冲区大小
    size_t bufsize = INITIAL_BUFSIZE;
    // 分配初始缓冲区
    char *buffer = malloc(bufsize);
    // 检查内存分配是否成功
    if (!buffer) {
        // 打印错误消息并退出
        perror("allocation error in exec_pwd");
        exit(EXIT_FAILURE);
    }

    // 尝试获取当前工作目录
    while (getcwd(buffer, bufsize) == NULL) {
        // 如果失败并且错误是 ERANGE，说明缓冲区大小不足
        if (errno == ERANGE) {
            // 尝试重新分配更大的缓冲区
            char *new_buffer = realloc_buffer_if_needed(buffer, &bufsize, strlen(buffer));
            // 检查重新分配是否成功
            if (!new_buffer) {
                // 如果失败，则释放原来的缓冲区并退出
                free(buffer);
                exit(EXIT_FAILURE);
            }
            // 使用新的缓冲区指针
            buffer = new_buffer;
        } else {
            // 如果获取当前工作目录失败并且错误不是 ERANGE，打印错误消息并退出
            perror("getcwd failed");
            free(buffer);
            exit(EXIT_FAILURE);
        }
    }
    // 打印当前工作目录
    printf("%s\n", buffer);
    // 释放缓冲区内存
    free(buffer);

    return 1;
}


// help命令实现函数
int exec_help(char** char_list){
    int i;
    printf("---------------------[CyShell Help]---------------------\n");
    printf("\n");
    printf("Type program names and arguments, and hit enter.\n");
    printf("Support for simple pipes and redirects.\n");
    printf("\n");
    printf("----------The following commands are built in:----------\n");
    // 逐条打印内建命令数组中的命令
    for (i = 0; i < num_of_cmds(); i++){
        printf("%s\n", builtin_commands[i]);
    }
    printf("--------------------------------------------------------\n");

    return 1;  
}


// exit命令实现函数
int exec_exit(char** char_list){
    return 0;
}


/**
 * echo 命令实现函数
 *
 * @param char_list 包含命令和参数的字符串数组
 * @return 总是返回 1 以继续执行下一个命令
 */
int exec_echo(char** char_list){
    // i 用于在循环中迭代参数
    int i;
    // 检查是否提供了至少一个参数（char_list[1] 是第一个参数）
    if (char_list[1] == NULL){
        // 如果没有提供参数，打印提示信息
        printf("Enter correct output.\n");
    } else {
        // 如果提供了参数，则遍历所有参数
        for (i = 1; char_list[i] != NULL; i++)
        {
            // 打印每个参数，参数之间用空格隔开
            printf("%s ", char_list[i]);
        }
        // 在所有参数打印完毕后打印一个换行符
        printf("\n");
    }

    return 1;
}


/**
 * mkdir命令实现函数
 * 
 * @param char_list 包含命令和参数的字符串数组
 * @return 总是返回1，表示shell应该继续运行
 */
int exec_mkdir(char** char_list){
    // 检查是否提供了目录名
    if(char_list[1] == NULL){
        // 如果没有提供目录名，输出错误信息
        print_error("mkdir", "expected argument to \"mkdir\"");
    } else {
        // 使用提供的目录名创建目录，设置权限为0775（即用户和组有读写执行权限，其他用户有读和执行权限）
        if (mkdir(char_list[1], 0775) != 0){
            // 如果mkdir调用失败，通过perror输出错误信息
            perror("cyshell");
        }
    }

    return 1;
}


/**
 * 外部命令执行函数
 *
 * @param char_list 用户输入的命令分割后的字符串数组
 * @return 返回值为1表示命令执行后继续保持shell运行状态
 */
int process(char** char_list){
    // 创建进程标识符和等待的进程标识符
    pid_t pid = fork(), wpid;
    // 进程退出状态
    int status;

    // 判断fork()的结果
    if (pid == 0){
        // 子进程执行用户输入的命令
        if (execvp(char_list[0], char_list) == -1){
            // 如果execvp执行失败，输出错误信息
            perror("cyshell");
            // 并以失败状态码退出子进程
            exit(EXIT_FAILURE);
        }
    } else if (pid < 0){
        // 如果fork失败，输出错误信息
        perror("cyshell");
        // 以失败状态码退出主进程
        exit(EXIT_FAILURE);
    } else {
        // 父进程等待子进程结束
        do {
            // 使用waitpid等待子进程状态改变，即子进程结束运行
            wpid = waitpid(pid, &status, WUNTRACED);
        // 循环直到子进程退出或者被信号终止
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
}


/**
 * 用户输入读取函数
 *
 * 这里使用readline库函数读取输入，一旦用户输入了一行并按下回车键，这行输入将被返回
 * 如果这行输入非空，它将被添加到历史记录中，以便使用上下箭头导航到先前的输入
 *
 * @return 返回用户输入的行作为一个字符串。如果遇到文件结束符(EOF)，将返回NULL
 */
char *shell_readline(void) {
    // 显示提示符并读取用户输入的行
    char *line = readline("CyShell-> ");
    
    // 如果line不为NULL且第一个字符不是终止符，即输入不为空
    if (line && *line) {
        // 将输入的行添加到历史记录中
        add_history(line);
    }
    
    // 返回用户输入的行
    return line;
}


/**
 * 字符串分割函数，将一行字符串切割为多个命令或参数
 *
 * @param line 需要被切割的字符串
 * @return 返回一个字符串数组，其中包含了切割得到的所有命令和参数
 */
char **cut_line(char *line) {
    // 设置初始的缓冲区大小
    size_t bufsize = 64;
    // 位置变量，用于跟踪目前字符串数组中已经存放的字符串数量
    size_t position = 0;
    // 分配内存空间用于存放指向子字符串的指针
    char **tokens = malloc(bufsize * sizeof(char*));
    // 用于暂存每次调用strtok函数得到的子字符串
    char *token;
    // 定义分隔符为空格、制表符、回车和换行
    const char *delim = " \t\r\n";

    // 如果分配内存失败，则打印错误并退出程序
    if (!tokens) {
        fprintf(stderr, "allocation error\n");
        exit(EXIT_FAILURE);
    }

    // 使用strtok函数开始切割字符串
    token = strtok(line, delim);
    // 当token不为空时，继续切割
    while (token != NULL) {
        // 将得到的子字符串存储到tokens中
        tokens[position++] = token;
        // 如果tokens数组已满，则将其大小加倍，进行扩容
        if (position >= bufsize) {
            bufsize *= 2;
            // 重新分配内存空间
            char **tokens_tmp = realloc(tokens, bufsize * sizeof(char*));
            // 如果内存重新分配失败，释放原来的内存空间，打印错误并退出程序
            if (!tokens_tmp) {
                free(tokens);
                fprintf(stderr, "allocation error\n");
                exit(EXIT_FAILURE);
            }
            // 如果内存分配成功，则将新的内存地址赋值给tokens
            tokens = tokens_tmp;
        }
        // 继续使用strtok进行切割，NULL作为第一个参数表示继续切割上一次的字符串
        token = strtok(NULL, delim);
    }
    // 最后一个位置设置为NULL，标识字符串数组的结束
    tokens[position] = NULL;
    // 返回分割好的字符串数组
    return tokens;
}


/**
 * 字符串剪切函数，从给定的字符串中剪切出一个子串
 * 
 * @param left 子串开始的位置（包含该位置）
 * @param right 子串结束的位置（包含该位置）
 * @param line 源字符串
 * @return 返回分配好内存并包含子串的新字符串指针
 */
char *cut_str(int left, int right, char *line){
    // 设置缓冲区大小为512
    int bufsize = 512;
    
    // 分配内存以存储子串
    char *buffer = malloc(sizeof(char) * bufsize);
    
    // 初始化索引变量以在新缓冲区中存放字符
    int j = 0;
    
    // 检查内存分配是否成功
    if(!buffer){
        // 如果内存分配失败，打印错误信息并退出程序
        printf("allocation error\n");
        exit(1);
    }
    
    // 通过循环从left到right将字符从line复制到buffer
    for (size_t i = left; i <= right; i++){
        // 复制字符
        buffer[j] = line[i];
        // 移动到下一个索引位置
        j++;
    }
    
    // 在新字符串的末尾添加空字符以形成合法的C字符串
    buffer[j] = '\0';
    
    // 返回包含子串的新分配内存的指针
    return buffer;
}


/**
 * 命令执行函数，执行命令行参数数组中指定的命令
 * 
 * @param char_list 字符串数组，包含要执行的命令和它的参数
 * @return 返回值通常是1，表示继续执行；如果执行exit命令则可能返回0，表示退出程序
 */
int execute(char **char_list){
    int i;

    // 如果第一个参数（命令名）为空，则直接返回1，不执行任何操作
    if(char_list[0] == NULL){
        return 1;
    }

    // 遍历所有内建命令
    for (i = 0; i < num_of_cmds(); i++){
        // 使用strcmp比较输入的命令和内建命令列表中的命令
        if (strcmp(char_list[0], builtin_commands[i]) == 0){
            // 如果找到匹配的内建命令，执行对应的函数
            // funcs是指向函数的指针数组，其中每个函数都接受char_list作为参数
            return (*funcs[i])(char_list);
        }
    }

    // 如果输入的命令不是内建命令，调用process函数尝试执行外部命令
    return process(char_list);
}


/**
 * 管道操作函数
 * 
 * @param line 用户输入的命令行字符串
 * @return 执行管道命令后的返回值
 */
int commandwithpipe(char *line){
    // 用于记录找到的管道符号的索引位置
    int pipeIdx = -1;
    
    // 循环遍历输入的命令行字符串
    for (size_t i = 0; i < strlen(line); ++i) {
        // 检查是否存在管道符号且两边有空格
        if (line[i] == '|' && i > 0 && line[i-1] == ' ' && i < strlen(line)-1 && line[i+1] == ' ') {
            pipeIdx = i; // 存储管道符号的索引位置
            break; // 找到后跳出循环
        }
    }
    
    // 检查是否没有找到管道符号或管道符号后面没有命令
    if (pipeIdx == -1 || pipeIdx+2 >= strlen(line)) {
        printf("Error: Invalid pipe usage.\n"); // 打印错误信息
        return 1; // 返回错误状态码
    }

    // 创建文件描述符数组用于管道
    int fds[2];
    // 创建管道失败则报错并退出
    if (pipe(fds) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }
    // 默认返回结果为失败
    int result = 1;

    // 创建子进程
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork"); // fork失败
        exit(EXIT_FAILURE);
    } else if (pid == 0) { // 子进程
        close(fds[0]); // 关闭读端
        // 将标准输出重定向到管道的写端
        if (dup2(fds[1], STDOUT_FILENO) == -1) {
            perror("dup2");
            exit(EXIT_FAILURE);
        }
        close(fds[1]); // 关闭写端的额外文件描述符

        // 根据管道符号前的命令内容截取字符串
        char *new_str = cut_str(0, pipeIdx - 2, line); 
        char **simple_line = cut_line(new_str); // 解析命令参数
        execute(simple_line); // 执行管道左侧的命令
        // 释放资源
        free(new_str);
        free(simple_line);
        exit(EXIT_FAILURE); // 子进程执行完毕后退出
    } else { // 父进程
        waitpid(pid, NULL, 0); // 等待子进程结束

        close(fds[1]); // 关闭管道写端
        // 将标准输入重定向到管道的读端
        if (dup2(fds[0], STDIN_FILENO) == -1) {
            perror("dup2");
            exit(EXIT_FAILURE);
        }
        close(fds[0]); // 关闭读端的额外文件描述符

        // 根据管道符号后的命令内容截取字符串
        char *new_str = cut_str(pipeIdx + 2, strlen(line), line);
        char **simple_line = cut_line(new_str); // 解析命令参数
        result = execute(simple_line); // 执行管道右侧的命令
        // 释放资源
        free(new_str);
        free(simple_line);
    }

    // 返回执行结果
    return result;
}


/**
 * 输入输出重定向函数
 *
 * @param line 用户输入的命令行字符串
 * @return 执行命令后的返回值，成功为0，失败为1
 */
int commandWithRedi(char* line) {
    // 初始化指向重定向输入和输出文件以及命令部分的指针
    char* inFile = NULL;  // 输入文件名
    char* outFile = NULL; // 输出文件名
    char* command = NULL; // 命令部分
    int inFd, outFd;      // 文件描述符（inFd对应输入，outFd对应输出）
    int result = 1;       // 默认返回结果为失败

    size_t len = strlen(line); // 获取命令行的长度
    for (size_t i = 0; i < len; i++) {
        if (line[i] == '>') {
            // 如果找到输出重定向符号'>'
            line[i] = '\0';         // 截断字符串，分离命令和后续内容
            command = line;         // 指向命令的开始部分
            outFile = line + i + 1; // 指向输出重定向文件名的开始部分

            while(isspace(*outFile)) outFile++; // 跳过文件名前的空白字符
            if(*outFile == '\0') { // 如果输出文件名为空，则报错并返回
                fprintf(stderr, "Output redirection missing filename.\n");
                return 1;
            }
        } else if (line[i] == '<') {
            // 如果找到输入重定向符号'<'
            line[i] = '\0';        // 截断字符串，分离命令和后续内容
            command = line;        // 指向命令的开始部分
            inFile = line + i + 1; // 指向输入重定向文件名的开始部分

            while(isspace(*inFile)) inFile++; // 跳过文件名前的空白字符
            if(*inFile == '\0') { // 如果输入文件名为空，则报错并返回
                fprintf(stderr, "Input redirection missing filename.\n");
                return 1;
            }
        }
    }

    pid_t pid = fork(); // 创建子进程
    if (pid == -1) {
        perror("fork"); // fork失败
        return 1;
    } else if (pid == 0) {
        // 子进程中执行命令与重定向
        if(inFile) {
            // 如果存在输入重定向，打开对应的文件
            inFd = open(inFile, O_RDONLY); // 以只读方式打开输入文件
            if (inFd == -1) {
                perror("open"); // 打开文件失败
                exit(EXIT_FAILURE);
            }
            // 将标准输入重定向到输入文件
            if (dup2(inFd, STDIN_FILENO) == -1) {
                perror("dup2"); // dup2调用失败
                close(inFd); // 关闭文件描述符
                exit(EXIT_FAILURE);
            }
            close(inFd); // 关闭不需要的文件描述符
        }
        if(outFile) {
            // 如果存在输出重定向，创建或打开对应的文件
            outFd = open(outFile, O_WRONLY | O_CREAT | O_TRUNC, 0644); // 以写方式打开输出文件
            if (outFd == -1) {
                perror("open"); // 打开文件失败
                exit(EXIT_FAILURE);
            }
            // 将标准输出重定向到输出文件
            if (dup2(outFd, STDOUT_FILENO) == -1) {
                perror("dup2"); // dup2调用失败
                close(outFd); // 关闭文件描述符
                exit(EXIT_FAILURE);
            }
            close(outFd); // 关闭不需要的文件描述符
        }

        // 将命令字符串分割成命令和参数
        char** char_list = cut_line(command);
        // 使用execvp执行命令
        if (execvp(char_list[0], char_list) == -1) {
            perror("execvp"); // execvp调用失败
            exit(EXIT_FAILURE); // 子进程失败，退出
        }
        exit(EXIT_FAILURE); // execvp不应该返回，如果返回则是失败
    } else {
        wait(NULL); // 父进程等待子进程结束
    }

    return result; // 返回执行结果
}


/**
 * 单行命令执行函数
 * 
 * @param line 用户输入的一行命令字符串
 * @return 执行结果状态码
 */
int execute_line(char *line){
    // 检查命令行中是否含有管道符号
    if (strchr(line, '|')) {
        // 如果有管道符号，调用 commandwithpipe 函数处理管道命令
        return commandwithpipe(line);
    } else if (strchr(line, '>') || strchr(line, '<')) {
        // 否则检查是否有重定向符号 '>' 或 '<'
        // 如果有，调用 commandWithRedi 函数处理重定向命令
        return commandWithRedi(line);
    } else {
        // 如果命令行中既没有管道符号也没有重定向符号则将其切割成命令和参数列表
        char **simple_line = cut_line(line);
        // 调用 execute 函数执行命令
        int result = execute(simple_line);
        // 执行完毕后释放切割后的命令和参数列表所占用的内存
        free(simple_line);
        // 返回执行结果状态码
        return result;
    }
}


/**
 * Shell主循环，反复读取用户输入并执行
 *
 * @return 返回状态码
 */
int loop() {
    char *line; // 用于存储读取的行
    int state = 1; // 状态变量，用于控制循环，1表示继续循环，0表示退出循环

    // 使用do-while循环持续读取并执行命令
    do {
        // 调用shell_readline函数读取用户输入的一行命令
        line = shell_readline();
        
        // 处理读取失败（可能因为到达输入流的末尾或发生错误）
        if (!line) {
            // 如果读取到文件结束符（EOF），通常是用户按下了Ctrl+D
            if (feof(stdin)) {
                // 退出循环
                break;
            } else {
                // 如果不是文件结束符，则认为发生了读取错误
                perror("readline");
                // 输出错误信息并退出程序，返回失败状态码
                exit(EXIT_FAILURE);
            }
        }

        // 调用execute_line函数执行读取的命令行
        state = execute_line(line);

        // 执行完命令后释放存储命令行的内存
        free(line);
    // 如果execute_line返回1，则继续循环；如果返回0（如执行了退出命令），则退出循环
    } while (state);

    // 返回1表示shell主循环正常结束
    return 1;
}


// 主函数
int main() {
    // 设置信号处理函数
    setup_signal_handlers();
    // 进入主循环
    loop();
    // 退出时返回状态码
    return EXIT_SUCCESS;
}
