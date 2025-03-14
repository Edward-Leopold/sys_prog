#include <stdio.h>
#include <string.h> 
#include <stdbool.h>  
#include <ctype.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>
#include <stdint.h>

typedef enum errCodes{
    SUCCESS,
    TOO_FEW_PARAMS_ERR,
    NO_FLAG_ERR,
    FILE_OPEN_ERR,
    FILE_READ_ERR,
    MEM_ALLOC_ERR,
    UNKNOWN_FLAG_ERR,
    INVALID_HEX_MASK_ERR,
    TOO_BIG_HEX_MASK_ERR,
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

size_t xor_per_file(FILE *file, int n) {
    // Узнаем размер файла в байтах
	fseek(file, 0, SEEK_END);
	size_t size_file = ftell(file);
	rewind(file);

	size_t total_bits = size_file * 8;
	const size_t block_bits = 1 << n;
	const size_t block_bytes = block_bits / 8;
	const size_t count_blocks = (total_bits + block_bits - 1) / block_bits;

	size_t val = 0;
	size_t tmp_val;
	unsigned char buf[block_bytes];
	for (size_t i = 0; i < count_blocks; ++i) {
		tmp_val = 0;

		// Читаем block_bytes байт
		size_t real_count_bytes = fread(buf, 1, block_bytes, file);

		if (real_count_bytes < block_bytes) {
			memset(buf + real_count_bytes, 0, block_bytes - real_count_bytes);
		}

		int bit;
		for (int current_bit = 0; current_bit < block_bits; ++current_bit) {
			size_t byte_idx = current_bit / 8;
			size_t bit_idx = 7 - (current_bit % 8);
			bit = (buf[byte_idx] >> bit_idx) & 1;
			tmp_val = (tmp_val << 1) | bit;
		}
		// Сложение по модулю 2
		val ^= tmp_val;
	}

	return val;
}

void print_xor_results(const int argc, char **argv, size_t *results) {
    for (int i = 1; i < argc - 1; ++i) {
        printf("File <%s>: %zu\n", argv[i], results[i - 1]);
    }
}

errCodes xorN(const int argc, char ** argv, int n){
    size_t *results = (size_t *)calloc(argc - 2, sizeof(size_t));
    if (results == NULL){
        return MEM_ALLOC_ERR;
    }

    FILE *file;
    for (int i = 0; i < argc - 2; ++i){
        file = fopen(argv[i + 1], "rb");

        if (file == NULL){
            free(results);
            return FILE_OPEN_ERR;
        }

        results[i] = xor_per_file(file, n);

        fclose(file);
    }

    print_xor_results(argc, argv, results);
    free(results);
    return SUCCESS;
}


int charToInt(char c){
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'Z') return c - 'A' + 10;
	if (c >= 'a' && c <= 'z') return c - 'a' + 10;
    return -1;
}

long long gorner(const char *number, int base) {
    long long res = 0;
    int digit;
    for (int i = 0; number[i]; ++i){
        digit = charToInt(number[i]);
        if (digit < 0 || digit >= base){
            return -1;
        }
        res = res * base + digit;
    }
    
    return res;
}

size_t hex_to_dec(const char * hex_num){
    size_t res = gorner(hex_num, 16);
    return res;
}

void print_mask_results(const int argc, char **argv, size_t *results) {
    for (int i = 1; i < argc - 2; ++i) {
        printf("File <%s>: %zu\n", argv[i], results[i - 1]);
    }
}

errCodes mask(const int argc, char ** argv){
    const size_t mask_decimal = hex_to_dec(argv[argc - 1]);
    if (mask_decimal > UINT32_MAX) {
        return TOO_BIG_HEX_MASK_ERR;
    }
    printf("hex mask: %zu\n", mask_decimal);

    size_t * cnt_results = (size_t *)calloc(argc - 3, sizeof(size_t));
    if (cnt_results == NULL){
        return MEM_ALLOC_ERR;
    }

    FILE *file;
    for (int i = 1; i < argc - 2; ++i){
        file = fopen(argv[i], "rb");
        if (file == NULL){
            free(cnt_results);
            return FILE_OPEN_ERR;
        }
        printf("file %s\n", argv[i]);

        uint32_t num;
        while (fread(&num, sizeof(uint32_t), 1, file) == 1){
            printf("got num: %d\n", num);
            if (num == mask_decimal) {
                cnt_results[i - 1]++;
                
            }
        }

        fclose(file);
    }
    
    print_mask_results(argc, argv, cnt_results);
    free(cnt_results);
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
        // file does not exist or it cannot be openeds
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
        printf("Processing mask flag...\n");
        result = mask(argc, argv);
        if (result == MEM_ALLOC_ERR){
            printf("mem alloc err\n");
        } else if (result == FILE_OPEN_ERR){
            printf("passed file cannot be opened\n");
        } else if (result == TOO_BIG_HEX_MASK_ERR){
            printf("passed hex mask is too big\n");
        }
        break;
    case XOR_N:
        printf("Processing xor flag...\n");
        result = xorN(argc, argv, n);
        if (result == MEM_ALLOC_ERR){
            printf("mem alloc err\n");
        } else if (result == FILE_OPEN_ERR){
            printf("passed file cannot be opened\n");
        }
        break;
    case COPY_N:
        printf("Processing copyN flag...\n");
        break;
    case FIND:
        printf("Processing find flag...\n");
        break;
    default:
        break;
    }

    
    return 0;
}