#include <stdio.h>
#include <string.h> 
#include <stdbool.h>  
#include <ctype.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

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
    INVALID_COPY_N_ERR,
    FORK_ERR,
    SUBSTR_FOUND,
    SUBSTR_NOT_FOUND,
} errCodes;

typedef enum flagOptions{
    XOR_N, 
    MASK,
    COPY_N,
    FIND,
    UNKNOWN,
}flagOptions;

/**
 * @brief Reads an entire text file into a dynamically allocated string.
 *
 * @param s Pointer to a string where the file content will be stored.  
 *          The caller is responsible for freeing the allocated memory using free().  
 * @param src Pointer to the file (must be opened for reading).
 * @return The number of bytes read (excluding the null terminator),  
 *         0 if the file is empty, or -1 in case of an error (e.g., memory allocation failure).
 *
 * @note This function uses dynamic memory allocation,  
 *       so it is important to free `*s` after use.
 */
ssize_t read_file_to_str(char ** s, FILE* src) {
    if (!src) return -1;
    ssize_t capacity = 1024;
    ssize_t len = 0;
    char *buffer = (char*)malloc(sizeof(char) * capacity);
    
    if (!buffer) return -1; 

    int ch;
    while ((ch = fgetc(src)) != EOF) {
        if (len + 1 >= capacity) { 
            capacity *= 2;
            char *new_buffer = (char*)realloc(buffer, sizeof(char) * capacity);
            if (!new_buffer) {
                free(buffer);
                return -1;
            }
            buffer = new_buffer;
        }
        buffer[len++] = (char)ch;
    }

    if (len == 0 && ch == EOF) { 
        free(buffer);
        return 0;
    }

    buffer[len] = '\0';
    *s = buffer;

    return len;
}


flagOptions try_get_flag(const char * str, int * n){
    const char valid_n_values_xor[] = "23456";

    if (str == strstr(str, "xor") && strlen(str) == 4){
        for (size_t i = 0; i < strlen(valid_n_values_xor); ++i){
            if (str[3] == valid_n_values_xor[i]){
                *n = (int)(str[3] - '0');
                return XOR_N;
            }
        }
    } else if(str == strstr(str, "copy") && strlen(str) >= 5){
        const char *p = str + 4;
        if (*p != '\0' && strspn(p, "0123456789") == strlen(p)) { 
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

    for (size_t i = 0; i < strlen(src); ++i){
        bool is_valid_symbol = false;
        for (size_t j = 0; j < strlen(hex); ++j){
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
    
    if(*flag == COPY_N){
        if (*n <= 0) return INVALID_COPY_N_ERR;
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
		for (size_t current_bit = 0; current_bit < block_bits; ++current_bit) {
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

        uint32_t num;
        while (fread(&num, sizeof(uint32_t), 1, file) == 1){
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

char* generate_filename(const char* filename, const unsigned int n){
    unsigned int n_len = 0;
    for (unsigned int i = n; i != 0; i /= 10) ++n_len;

    char * res = (char *)calloc(strlen(filename) + 1 + n_len + 1, sizeof(char));
    if (!res) return NULL;

    size_t len = strlen(filename);
    int k = -1;
	for (size_t i = len; i >= 0; --i) {
		if (filename[i] == '.') {
			k = (int)i;
            break;
		}
	}

    if (k == -1) {
        sprintf(res, "%s_%d", filename, n);
    } else {
        char name_part[k + 1];
        char ext_part[len - k + 1]; 
        strncpy(name_part, filename, k);
        name_part[k] = '\0';
        strcpy(ext_part, filename + k);
        sprintf(res, "%s_%d%s", name_part, n, ext_part);
    }

    return res;
}

errCodes copyN(const int argc, char ** argv, int n){
    for (int i = 1; i < argc - 1; ++i) {

        for (int j = 1; j <= n; ++j) {
            pid_t pid = fork();
            if (pid < 0) {
                return FORK_ERR; 
            }
            if (pid == 0) {
                FILE *src = fopen(argv[i], "rb");
                if (!src) {
                    exit(FILE_OPEN_ERR);
                }

                char *new_filename = generate_filename(argv[i], j);
                if (!new_filename) {
                    fclose(src);
                    exit(MEM_ALLOC_ERR);
                }

                FILE *dest = fopen(new_filename, "wb");
                if (!dest) {
                    free(new_filename);
                    fclose(src);
                    exit(FILE_OPEN_ERR);
                }

                char buffer[4096];
                size_t bytes;
                rewind(src);
                while ((bytes = fread(buffer, 1, sizeof(buffer), src)) > 0) {
                    fwrite(buffer, 1, bytes, dest);
                }

                fclose(dest);
                free(new_filename);
                fclose(src);
                exit(SUCCESS);
            }
        }

        
        for (int j = 1; j <= n; ++j) {
            int status;
            wait(&status);
            if (status != SUCCESS){
                return status;
            }
        }
    }

    return SUCCESS;
}

errCodes findStr(const int argc, char ** argv, int ** files_indecies){
    *files_indecies = (int *)calloc(argc - 3, sizeof(int));
    if (!(*files_indecies)){
        return MEM_ALLOC_ERR;
    }

    int len = 0;
    bool is_str_found = false;

    for (int i = 1; i < argc - 2; ++i) {
        pid_t pid = fork();
        if (pid < 0) {
            return FORK_ERR; 
        }
        if (pid == 0) {
            FILE *src = fopen(argv[i], "rb");
            if (!src) {
                exit(FILE_OPEN_ERR);
            }

            errCodes res;
            char * line = NULL;
            if(read_file_to_str(&line, src) != -1){
                const char* pos = strstr(line, argv[argc - 1]);
                if (pos){
                    res = SUBSTR_FOUND;
                } else{
                    res = SUBSTR_NOT_FOUND;
                }
            } else{
                res = MEM_ALLOC_ERR;
            }
            
            fclose(src);
            exit(res);
        }
    }

    for (int j = 1; j < argc - 2; ++j) {
        int status;
        wait(&status);
        if (WIFEXITED(status)) {
            int exit_code = WEXITSTATUS(status);
            if (exit_code == MEM_ALLOC_ERR || exit_code == FILE_OPEN_ERR) {
                return exit_code;
            } else if (exit_code == SUBSTR_FOUND) {
                is_str_found = true;
                (*files_indecies)[len++] = j;
            }
        }
    }

    (*files_indecies)[len] = -1;
    if (!is_str_found) return SUBSTR_NOT_FOUND;
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
    case INVALID_COPY_N_ERR:
        printf("Ivalid number passed for copy flag\n");
        break;
    case FILE_OPEN_ERR:
        // file does not exist or it cannot be opened
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

        printf("copyN will create %d copies.\n", n);

        result = copyN(argc, argv, n);
        if (result == MEM_ALLOC_ERR){
            printf("mem alloc err\n");
        } else if (result == FILE_OPEN_ERR){
            printf("passed file cannot be opened\n");
        } else if (result == FORK_ERR){
            printf("fork error occcured during execution of the program\n");
        }
        break;
    case FIND:
        printf("Processing find flag...\n");

        int * files_indecies = NULL;
        result = findStr(argc, argv, &files_indecies);
        if (result == MEM_ALLOC_ERR){
            printf("mem alloc err\n");
        } else if (result == FILE_OPEN_ERR){
            printf("passed file cannot be opened\n");
        } else if (result == FORK_ERR){
            printf("fork error occcured during execution of the program\n");
        } else if (result == SUBSTR_NOT_FOUND){
            printf("string was not found in any of the files\n");
        } else if(result == SUCCESS && files_indecies){
            for (int i = 0; files_indecies[i] != -1; ++i){
                printf("String was found at: %s\n", argv[files_indecies[i]]);
            }
            free(files_indecies);
        }
        break;
    default:
        break;
    }

    
    return 0;
}