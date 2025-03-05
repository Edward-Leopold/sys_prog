#include <stdio.h>
#include <string.h> 
#include <stdbool.h>  
#include <ctype.h>

typedef enum errCodes{
    SUCCESS,
    TOO_FEW_PARAMS_ERR,
    NO_FLAG_ERR,
    FILE_OPEN_ERR,
    MEM_ALLOC_ERR,

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
        char * p = str[4];
        for (; p <= str[strlen(str) - 1]; p++){
            if(!isdigit(*p)) is_valid_flag = false;
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

    // if (argv[argc - 1] == strstr(argv[argc - 1], "xor") && strlen(argv[argc - 1]) == 4){
    //     char *s = argv[argc - 1];
    //     for (int i = 0; i < strlen(valid_n_values_xor); ++i){
    //         if (s[3] == valid_n_values_xor[i]){
    //             is_valid_flag = true;
    //             *n = (int)(s[3] - '0');
    //             *flag = XOR_N;
    //         }
    //     }
    // } else if (argv[argc - 1] == strstr(argv[argc - 1], "copy") && argv[argc - 1][4]){
    //     char *s = argv[argc - 1];
    //     is_valid_flag = true;
    //     for (char * p = s[4]; p <= s[strlen(s) - 1]; s++){
    //         if(!isdigit(*p)) is_valid_flag = false;
    //     }
    //     if (is_valid_flag) *flag = COPY_N;
    // }

    for (int i = 1; i < argc; ++i){

    }

    return SUCCESS;
}


int main(int argc, char ** argv){


    return 0;
}