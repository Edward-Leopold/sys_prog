#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <time.h>

void get_permissions(const mode_t mode, char *str) {
    str[0] = S_ISDIR(mode)  ? 'd' :
             S_ISLNK(mode)  ? 'l' :
             S_ISCHR(mode)  ? 'c' :
             S_ISBLK(mode)  ? 'b' :
             S_ISFIFO(mode) ? 'p' :
             S_ISSOCK(mode) ? 's' : '-';


    str[1] = (mode & S_IRUSR) ? 'r' : '-';
    str[2] = (mode & S_IWUSR) ? 'w' : '-';
    str[3] = (mode & S_IXUSR) ? 'x' : '-';
    str[4] = (mode & S_IRGRP) ? 'r' : '-';
    str[5] = (mode & S_IWGRP) ? 'w' : '-';
    str[6] = (mode & S_IXGRP) ? 'x' : '-';
    str[7] = (mode & S_IROTH) ? 'r' : '-';
    str[8] = (mode & S_IWOTH) ? 'w' : '-';
    str[9] = (mode & S_IXOTH) ? 'x' : '-';
    str[10] = '\0';
}

void list_directory(const char *dirname) {
    DIR *dir = opendir(dirname);
    if (!dir) {
        printf("Error while opening directory %s\n", dirname);
        return;
    }

    printf("Directory: %s\n", dirname);

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        size_t path_len = strlen(dirname) + strlen(entry->d_name) + 2;
        char *path = (char *)malloc(path_len);
        if (!path) {
            printf( "Error memory allocation\n");
            continue;
        }
        snprintf(path, path_len, "%s/%s", dirname, entry->d_name);

        struct stat st;
        if (lstat(path, &st) == -1) {
            printf("Error file access %s\n", path);
            free(path);
            continue;
        }

        char perms[11];
        get_permissions(st.st_mode, perms);

        printf("%s   size %ld byte \t inode: %lu\t name %s\n",
               perms,
               st.st_size,
               st.st_ino,
               entry->d_name);

        free(path);
    }

    closedir(dir);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Error: no directory to list \n");
        return 1;
    }

    for (int i = 1; i < argc; ++i) {
        list_directory(argv[i]);
    }

    return 0;
}
