#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MAX_COMMAND_LENGTH 1024
#define MAX_ARGS 64

void execute_command(char* command) {
    // 解析命令和参数
    char* args[MAX_ARGS];
    int num_args = 0;
    char* token = strtok(command, " ");
    while (token != NULL) {
        args[num_args] = token;
        num_args++;
        token = strtok(NULL, " ");
    }
    args[num_args] = NULL;

    // 创建子进程来执行命令
    pid_t pid = fork();
    if (pid < 0) {
        fprintf(stderr, "Failed to fork()\n");
        exit(1);
    } else if (pid == 0) {
        // 子进程
        execvp(args[0], args);
        fprintf(stderr, "Failed to execute command: %s\n", command);
        exit(1);
    } else {
        // 父进程
        wait(NULL);
    }
}

int main() {
    while (1) {
        // 读取并解析命令
        printf("$ ");
        char command[MAX_COMMAND_LENGTH];
        fgets(command, MAX_COMMAND_LENGTH, stdin);
        command[strcspn(command, "\n")] = '\0';

        // 检查是否需要输入/输出重定向
        char* input_file = NULL;
        char* output_file = NULL;
        char* append_file = NULL;
        int background = 0;

        char* token = strtok(command, " ");
        while (token != NULL) {
            if (strcmp(token, "<") == 0) {
                token = strtok(NULL, " ");
                if (token != NULL) {
                    input_file = token;
                }
            } else if (strcmp(token, ">") == 0) {
                token = strtok(NULL, " ");
                if (token != NULL) {
                    output_file = token;
                }
            } else if (strcmp(token, ">>") == 0) {
                token = strtok(NULL, " ");
                if (token != NULL) {
                    append_file = token;
                }
            } else if (strcmp(token, "&") == 0) {
                background = 1;
            }

            token = strtok(NULL, " ");
        }

        // 创建子进程来执行命令
        pid_t pid = fork();
        if (pid < 0) {
            fprintf(stderr, "Failed to fork()\n");
            exit(1);
        } else if (pid == 0) {
            // 子进程

            // 检查输入重定向
            if (input_file != NULL) {
                int fd = open(input_file, O_RDONLY);
                if (fd < 0) {
                    fprintf(stderr, "Failed to open file: %s\n", input_file);
                    exit(1);
                }
                dup2(fd, STDIN_FILENO);
                close(fd);
            }

            // 检查输出重定向
            if (output_file != NULL) {
                int fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd < 0) {
                    fprintf(stderr, "Failed to open file: %s\n", output_file);
                    exit(1);
                }
                dup2(fd, STDOUT_FILENO);
                close(fd);
            }

            // 检查追加重定向
            if (append_file != NULL) {
                int fd = open(append_file, O_WRONLY | O_CREAT | O_APPEND, 0644);
                if (fd < 0) {
                    fprintf(stderr, "Failed to open file: %s\n", append_file);
                    exit(1);
                }
                dup2(fd, STDOUT_FILENO);
                close(fd);
            }

            // 执行命令
            execute_command(command);

            exit(0);
        } else {
            // 父进程

            // 检查是否在后台运行
            if (!background) {
                wait(NULL);
            }
        }
    }

    return 0;
}
