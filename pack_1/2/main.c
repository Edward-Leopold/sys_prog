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
    INVALID_HEX_MASK_ERR,
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

errCodes validate_hex_mask(const char * src){
    const char *hex = "0123456789ABCDEFabcdef";

    for (int i = 0; i < strlen(src); ++i){
        bool is_valid_symbol = false;
        for (int j = 0; j < strlen(hex); ++j){
            if (src[i] == hex[j]){
                is_valid_symbol = true;
                break;
            }
        }
        if (!is_valid_symbol){
            return INVALID_HEX_MASK_ERR;
        }
    }

    return SUCCESS;
}

errCodes parse_argv(const int argc, char ** argv, flagOptions * flag, int * n){
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

    const int last_ind = (*flag == XOR_N || *flag == COPY_N) ? argc - 1 : argc - 2;
    FILE *file;
    for (int i = 1; i < last_ind; ++i){
        file = fopen(argv[i], "r");
        if (!file) return FILE_OPEN_ERR;
        fclose(file);
    }

    if (*flag == MASK){
        errCodes result = validate_hex_mask(argv[argc - 1]); 
        if (result != SUCCESS){
            return result;
        }
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
    case INVALID_HEX_MASK_ERR:
        printf("Invalid hex mask for flag mask\n");
        break;
    case FILE_OPEN_ERR:
        printf("Unable to open one of the passed files\n");
        break;
    case SUCCESS:
        printf("success!\n");
        break;
    default:
        break;
    }

    if (result != SUCCESS) {
        return 1;
    }

    switch (flag){
    case MASK:
        printf("some process for mask flag...\n");
        break;
    case XOR_N:
        printf("some process for xorN flag...\n");
        break;
    case COPY_N:
        printf("some process for copyN flag...\n");
        break;
    case FIND:
        printf("some process for find flag...\n");
        break;
    default:
        break;
    }

    
    return 0;
}