#include <stdio.h>
#include <stdbool.h>  
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <stdlib.h>

#define FILE_PATH "users.dat"


typedef enum errCodes{
    SUCCESS,
    FILE_OPEN_ERR,
    USER_NOT_FOUND,
    USER_FOUND,
    MEM_ALLOC_ERR,
} errCodes;

typedef enum cmdsCodes{
    NORMAL,
    LOGOUT,
    ERR_OPEN_FILE,  
    DATE_FORMAT_ERR,
} cmdsCodes;


typedef struct User{
    char login[7];
    int pin;
    int cmds_num;
} User;

errCodes dynamic_fgets(char ** s) {
    size_t size = 16;  // Начальный размер буфера
    size_t len = 0;    // Текущая длина строки
    char *buffer = malloc(size);
    
    if (!buffer) return MEM_ALLOC_ERR;  // Проверяем выделение памяти

    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF) {
        if (len + 1 >= size) {  // Увеличиваем буфер при необходимости
            size *= 2;
            char *new_buffer = realloc(buffer, size);
            if (!new_buffer) {
                free(buffer);
                return MEM_ALLOC_ERR;
            }
            buffer = new_buffer;
        }
        buffer[len++] = (char)ch;
    }

    if (len == 0 && ch == EOF) {  // Если EOF в самом начале — вернуть NULL
        free(buffer);
        return MEM_ALLOC_ERR;
    }

    buffer[len] = '\0';  // Завершаем строку
    *s = buffer;

    return SUCCESS;
}

char* get_current_time_str() {
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    char time_str[9];
    snprintf(time_str, sizeof(time_str), "%02d:%02d:%02d", tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec);
    return time_str;
}

char* get_current_date_str(){
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    char date_str[11];
    snprintf(date_str, sizeof(date_str), "%02d:%02d:%04d", tm_info->tm_mday, tm_info->tm_mon + 1, tm_info->tm_year + 1900);
    return date_str;
}

int day_of_month(int month, int year){
    int days[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (year % 4 == 0){
        days[1] = 29;
        if (year % 100 == 0 && year % 400 != 0) days[1] = 28;
    } 
    return days[month - 1];
}

cmdsCodes parse_time(const char* datetime, struct tm* timestruct){
    int day, month, year, hours, minutes, seconds;
    if (sscanf(datetime, "%d:%d:%d-%d:%d:%d", &day, &month, &year, &hours, &minutes, &seconds) != 6) {
        return DATE_FORMAT_ERR;
    }

    if ((seconds >= 60 || seconds < 0) || (minutes >= 60 || minutes < 0) || (hours >= 24 || hours < 0)) return DATE_FORMAT_ERR;
    if ((year > 2050 || year < 1950) || (month < 1 || month > 12)) return DATE_FORMAT_ERR;
    if (day < 1 || day > day_of_month(month, year)) return DATE_FORMAT_ERR;

    timestruct->tm_mday = day;
    timestruct->tm_mon = month - 1;
    timestruct->tm_year = year - 1900;
    timestruct->tm_hour = hours;
    timestruct->tm_min = minutes;
    timestruct->tm_sec = seconds;
    timestruct->tm_isdst = -1;

    time_t timestamp = mktime(timestruct);
    if (timestamp == -1) {
        return DATE_FORMAT_ERR;
    }

    return SUCCESS;
}

cmdsCodes cmd_time() {
    printf("Current time: %s\n", get_current_time_str());
    return NORMAL;
}

cmdsCodes cmd_date() {
    printf("Current date: %s\n", get_current_date_str());
    return NORMAL;
}

cmdsCodes cmd_logout(char* args) {
    printf("Logging out...\n");
    return LOGOUT;
}

cmdsCodes cmd_howmuch() {
    char *input;
    
    if (dynamic_fgets(&input) != SUCCESS) {
        printf("Memory allocation error.\n");
        return DATE_FORMAT_ERR;
    }

    char time_str[21], flag[4], extra[2];  
    memset(time_str, 0, sizeof(time_str));
    memset(flag, 0, sizeof(flag));
    memset(extra, 0, sizeof(extra));
    int count = sscanf(input, "%20s %3s %1s", time_str, flag, extra);
    free(input);
    
    if(time_str[19]){
        printf("Invalid time format. Expected format: DD:MM:YYYY-HH:MM:SS\n");
        return DATE_FORMAT_ERR;
    }else if(flag[2]){
        printf("Invalid flag format\n");
        return DATE_FORMAT_ERR; // нужно придумать свой код ошибки, если неправильно написан флаг в к
    }

    if (count < 2) {  
        printf("Invalid format. Usage: Howmuch <time> <flag>\n");
        return DATE_FORMAT_ERR;
    }
    if (count > 2) {
        printf("time_str: %s, flag: %s, extra: %s\n", time_str, flag, extra);  
        printf("Too many arguments. Usage: Howmuch <time> <flag>\n");
        return DATE_FORMAT_ERR;
    }

    struct tm parsed_time;
    if (parse_time(time_str, &parsed_time) != NORMAL) {
        printf("Invalid time format. Expected format: DD:MM:YYYY-HH:MM:SS\n");
        return DATE_FORMAT_ERR;
    }

    time_t now = time(NULL);
    time_t past_time = mktime(&parsed_time);

    if (past_time == -1) {
        printf("Error converting time.\n");
        return DATE_FORMAT_ERR;
    }

    double diff_seconds = difftime(now, past_time);

    if (strcmp(flag, "-s") == 0) {
        printf("Elapsed time: %.0f seconds\n", diff_seconds);
    } else if (strcmp(flag, "-m") == 0) {
        printf("Elapsed time: %.0f minutes\n", diff_seconds / 60);
    } else if (strcmp(flag, "-h") == 0) {
        printf("Elapsed time: %.1f hours\n", diff_seconds / 3600);
    } else if (strcmp(flag, "-y") == 0) {
        printf("Elapsed time: %.2f years\n", diff_seconds / (3600 * 24 * 365.25));
    } else {
        printf("Invalid flag. Use -s, -m, -h, or -y.\n");
        return DATE_FORMAT_ERR;
    }

    return NORMAL;
}

cmdsCodes cmd_sanctions(char* args) {
    printf("Executing Sanctions with args: %s\n", args);
}


typedef struct Command{
    char* name;
    int params;
    void (*execute)();
} Command;

const Command commands[5] = {
    {
        .name = "Time",
        .params = 0,    
        .execute = cmd_time,
    },
    {
        .name = "Date",
        .params = 0,    
        .execute = cmd_date,
    },
    {
        .name = "Howmuch",
        .params = 2,    
        .execute = cmd_howmuch,
    },
    {
        .name = "Logout",
        .params = 0,    
        .execute = cmd_logout,
    },
    {
        .name = "Sanctions",
        .params = 2,    
        .execute = cmd_sanctions,
    },
};

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

void execute_cmd(char * cmd){
    int cmds = sizeof(commands) / sizeof(Command);
    for (int i = 0; i < cmds; ++i){
        if (strcmp(cmd, commands[i].name) == 0){
            commands[i].execute();
            while (getchar() != '\n');
            return;
        }
    }
    printf("No such command\n");
    while (getchar() != '\n');
    return;
}

void session(User *u){
    printf("\nSystem\n");

    char command[20];
    while (true){
        printf("Enter a command: ");
        scanf("%19s", command);
        execute_cmd(command);
    }
    
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
                // printf("session..."); // session()
                session(&u);
            }
            break;
        case 2:
            result = register_user(&u);
            if (result == SUCCESS){
                // printf("session..."); // session()
                session(&u);
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