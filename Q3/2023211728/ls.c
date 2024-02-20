#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>

int main(int argc, char *argv[]) {
    DIR *dir;
    struct dirent *entry;
    if (argc == 1) {
        dir = opendir(".");
    } else {
        dir = opendir(argv[1]);
    }
    if (dir == NULL) {
        perror("opendir");
        return EXIT_FAILURE;
    }
    while ((entry = readdir(dir)) != NULL) {
        printf("%s\t", entry->d_name);
    }
    printf("\n");
    closedir(dir);
    return EXIT_SUCCESS;
}
