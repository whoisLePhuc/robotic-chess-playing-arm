#include "common/chess_types.h"
#include "utils/string_utils.h"

char* trim_string(char *str) {
    if (!str) return str;
    
    char *end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    end[1] = '\0';
    
    while (isspace((unsigned char)*str)) str++;
    return str;
}

void clear_input_buffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}