#include <stdio.h>
#include <stdbool.h>  
#include <string.h>
#include <ctype.h>

#define FILE_PATH "users.dat"


typedef enum errCodes{
    SUCCESS,
    FILE_OPEN_ERR,
    USER_NOT_FOUND,
    USER_FOUND,
} errCodes;


typedef struct User{
    char login[7];
    int pin;
    int cmds_num;
} User;

errCodes base_save_user(User *u) {
    FILE *file = fopen(FILE_PATH, "ab"); // "append binary"
    if (!file) {
        return FILE_OPEN_ERR;
    }
    fwrite(u, sizeof(User), 1, file);
    fclose(file);
    return SUCCESS;
}

errCodes base_auth_user(User *u) {
    FILE *file = fopen(FILE_PATH, "rb");
    if (!file) {
        return FILE_OPEN_ERR;
    }

    User temp;
    while (fread(&temp, sizeof(User), 1, file)) {
        if (strcmp(temp.login, u->login) == 0 && temp.pin == u->pin) {
            fclose(file);
            return USER_FOUND;
        }
    }

    fclose(file);
    return USER_NOT_FOUND;
}

bool validate_login(char* s){
    const int len = strlen(s);
    if (len > 6) return false;
    for (int i = 0; i < len; ++i){
        if (!isalnum(s[i])) return false;
    }
    return true;
}

bool validate_pin(int n){
    if (n >= 0 && n <= 1000000) return true;
    return false;
}


errCodes login_user(User *u) {
    while (true){
        char temp[8];
        printf("Enter login: ");
        if(scanf("%7s", temp) == 1 && validate_login(temp)){
            strcpy(u->login, temp);
            while (getchar() != '\n');
            break;
        }
        printf("Invalid login. Try again\n");
        while (getchar() != '\n');
    }
    while (true){
        int temp;
        printf("Enter pin: ");
        if (scanf("%d", &temp) == 1 && validate_pin(temp)){
            u->pin = temp;
            while (getchar() != '\n');
            break;
        }
        printf("Invalid pin. Try again\n");
        while (getchar() != '\n');
    }   

    errCodes result = base_auth_user(u);
    switch (result){
    case USER_FOUND:
        printf("Welcome, %s!\n", u->login);
        break;
    case USER_NOT_FOUND:
        printf("Invalid login or PIN.\n");
        break;
    case FILE_OPEN_ERR:
        printf("Error opening user file!\n");
        break;
    default:
        break;
    }
    return result;
}

errCodes register_user(User *u) {
    printf("\nNew user registration\n");
    printf("Create a login (login can be no more than 6 characters long and consist of characters of the Latin alphabet and numbers)\n");
    while (true){
        char temp[8];
        printf("Enter your login: ");
        if(scanf("%7s", temp) == 1 && validate_login(temp)){
            strcpy(u->login, temp);
            while (getchar() != '\n');
            break;
        }
        printf("Invalid login. Try again\n");
        while (getchar() != '\n');
    }
    printf("\nCreate a PIN (pin must be an integer in the range from 0 to 1000000)\n");
    while (true){
        int temp;
        printf("Enter your pin: ");
        if (scanf("%d", &temp) == 1 && validate_pin(temp)){
            u->pin = temp;
            while (getchar() != '\n');
            break;
        }
        printf("Invalid pin. Try again\n");
        while (getchar() != '\n');
    }
    u->cmds_num = -1;
    
    errCodes result = base_save_user(u);
    switch (result){
    case SUCCESS:
        printf("User registered successfully!\n");
        break;
    case FILE_OPEN_ERR:
        printf("Error saving user to file!\n");
        break;
    default:
        break;
    }

    return result;
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

        User u;
        errCodes result;
        switch (command){
        case 1:
            result = login_user(&u);
            if (result == USER_FOUND){
                printf("session..."); // session()
            }
            break;
        case 2:
            result = register_user(&u);
            if (result == SUCCESS){
                printf("session..."); // session()
            }
            break;
        default:
            printf("unknown command\n");
            while (getchar() != '\n');
            break;
        }
    }
    

    return 0;
}