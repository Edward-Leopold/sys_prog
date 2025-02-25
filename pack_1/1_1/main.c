#include <stdio.h>
#include <stdbool.h>  
#include <string.h>

#define FILE_PATH "users.dat"


typedef enum errCodes{
    SUCCESS,
    FILE_OPEN_ERR,
    USER_NOT_FOUND,
    USER_FOUND,
} errCodes;


struct user{
    char login[7];
    int pin;
};

errCodes save_user(struct user *u) {
    FILE *file = fopen(FILE_PATH, "ab"); // "append binary"
    if (!file) {
        return FILE_OPEN_ERR;
    }
    fwrite(u, sizeof(struct user), 1, file);
    fclose(file);
    return SUCCESS;
}

errCodes check_user(struct user *u) {
    FILE *file = fopen(FILE_PATH, "rb");
    if (!file) {
        return FILE_OPEN_ERR;
    }

    struct user temp;
    while (fread(&temp, sizeof(struct user), 1, file)) {
        if (strcmp(temp.login, u->login) == 0 && temp.pin == u->pin) {
            fclose(file);
            return USER_FOUND;
        }
    }

    fclose(file);
    return USER_NOT_FOUND;
}


int main(){

    while (true){
        printf("\nAuthorise or register in the system:\n");
        printf("1: Authorisation\n");
        printf("2: Registration\n");
        printf("Enter command number: ");

        int command;
        if (scanf("%d", &command) != 1) {
            printf("Invalid input. Please enter a number.\n");
            while (getchar() != '\n');
            continue;
        }

        struct user u;
        errCodes result;
        switch (command){
        case 1:
            printf("Enter login: ");
            scanf("%6s", u.login);
            printf("Enter PIN: ");
            scanf("%d", &u.pin);

            result = check_user(&u);
            if (result == USER_FOUND) {
                printf("Welcome, %s!\n", u.login);
            } else if (result == USER_NOT_FOUND) {
                printf("Invalid login or PIN.\n");
            } else {
                printf("Error opening user file!\n");
            }
            break;
        case 2:
            printf("Enter new login: ");
            scanf("%6s", u.login);
            printf("Enter new PIN: ");
            scanf("%d", &u.pin);

            result = save_user(&u);
            if (result == SUCCESS) {
                printf("User registered successfully!\n");
            } else {
                printf("Error saving user to file!\n");
            }
            break;
        default:
            printf("unknown command\n");
            while (getchar() != '\n');
            continue;
            break;
        }
    }
    

    return 0;
}