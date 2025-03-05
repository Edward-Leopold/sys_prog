#include <stdio.h>
#include <string.h> 
#include <stdbool.h>  
#include <ctype.h>
#include <stdlib.h>

typedef enum errCodes{
    SUCCESS,
    TOO_FEW_PARAMS_ERR,
    NO_FLAG_ERR,
    FILE_OPEN_ERR,
    MEM_ALLOC_ERR,
    UNKNOWN_FLAG_ERR,
} errCodes;

typedef enum flagOptions{
    XOR_N, 
    MASK,
    COPY_N,
    FIND,
    UNKNOWN,
}flagOptions;

flagOptions try_get_flag(const char * str, int * n){
    const char valid_n_values_xor[] = "23456";

    if (str == strstr(str, "xor") && strlen(str) == 4){
        for (int i = 0; i < strlen(valid_n_values_xor); ++i){
            if (str[3] == valid_n_values_xor[i]){
                *n = (int)(str[3] - '0');
                return XOR_N;
            }
        }
    } else if(str == strstr(str, "copy") && strlen(str) >= 5){
        bool is_valid_flag = true;
        const char *p = str + 4;  
        for (; *p != '\0'; p++){
            if (!isdigit(*p)) {
                is_valid_flag = false;
                break;
            }
        }
        if (is_valid_flag){
            *n = atoi(p); 
            return COPY_N;
        }
    } else if(str == strstr(str, "mask") && strlen(str) == 4){
        *n = 0;
        return MASK;
    } else if(str == strstr(str, "find") && strlen(str) == 4){
        *n = 0;
        return FIND;
    }

    return UNKNOWN;
}

errCodes parse_argv(int argc, char ** argv, flagOptions * flag, int * n){
    if (argc < 3) return TOO_FEW_PARAMS_ERR;
    
    bool is_valid_flag = false;
    const char valid_n_values_xor[] = "23456";

    if(try_get_flag(argv[argc - 1], n) == XOR_N){
        *flag = XOR_N;
    } else if(try_get_flag(argv[argc - 1], n) == COPY_N){
        *flag = COPY_N;
    } else if (try_get_flag(argv[argc - 2], n) == MASK){
        *flag = MASK;
    } else if (try_get_flag(argv[argc - 2], n) == FIND){
        *flag = FIND;
    } else{
        return UNKNOWN_FLAG_ERR;
    }
    
    
    return SUCCESS;
}


int main(int argc, char ** argv){
    int n = 0;
    flagOptions flag;
    errCodes result = parse_argv(argc, argv, &flag, &n);

    switch (result){
    case TOO_FEW_PARAMS_ERR:
        printf("Too few params. You have to pass filenames and a flag\n");
        break;
    case UNKNOWN_FLAG_ERR:
        printf("unknown flag\n");
        break;
    case SUCCESS:
        printf("success!\n");
        break;
    default:
        break;
    }

    return 0;
}