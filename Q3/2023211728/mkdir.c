#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <directory_name>\n", argv[0]);
        return 1;
    }

    const char *dir_path = argv[1];
    int result = mkdir(dir_path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    if (result == -1) {
        perror("Error creating directory");
        return 1;
    }

    printf("Directory created successfully: %s\n", dir_path);
    return 0;
}
