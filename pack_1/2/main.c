#include <stdio.h>

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
}flagOptions;

errCodes parse_argv(int argc, char ** argv, flagOptions * flag){
    if (argc < 3) return TOO_FEW_PARAMS_ERR;
    
    const int valid_n_values_xor = {2, 3, 4, 5, 6};

    if (argv[argc - 1] == "xorN")

    return SUCCESS;
}


int main(int argc, char ** argv){


    return 0;
}