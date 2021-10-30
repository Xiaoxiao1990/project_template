#ifndef __LOG_H__
#define __LOG_H__

#include "types.h"
#include <string.h>
#include <stddef.h>

#define _LOG_TO_FILE_
#ifdef _LOG_TO_FILE_
#define LOG_PATH                 "/home/root/linux_cpe"
#define LOG_DIR                  "logs"
#endif

#define LOGS_BUFFER_LENGTH             4096
#define PRINT_BUFFER_LENGTH            4096

#define LOG_COLOR_BLACK   "30"
#define LOG_COLOR_RED     "31"
#define LOG_COLOR_YELLOW  "32"
#define LOG_COLOR_BROWN   "33"
#define LOG_COLOR_BLUE    "34"
#define LOG_COLOR_PURPLE  "35"
#define LOG_COLOR_CYAN    "36"
#define LOG_COLOR(COLOR)  "\033[0;" COLOR "m"
#define LOG_BOLD(COLOR)   "\033[1;" COLOR "m"
#define LOG_RESET_COLOR   "\033[0m"
#define LOG_COLOR_E       LOG_COLOR(LOG_COLOR_RED)
#define LOG_COLOR_W       LOG_COLOR(LOG_COLOR_BROWN)
#define LOG_COLOR_I       LOG_COLOR(LOG_COLOR_YELLOW)
#define LOG_COLOR_D
#define LOG_COLOR_V

#ifdef __cplusplus
extern "C"{
#endif

void log_init(void);
void log_trim(void);
int log_raw(const char *fmt, ...);
int logs(const char *file, const char *func, uint32_t line, const char *LOG_COLOR, const char *fmt, ...);
int log_hex(uint8_t *arr, size_t arr_len, const char *fmt, ...);

#define CONSTANT_STRING_DEFINE(s)  const char * const s##_CONST_STRING = #s
#define CONSTANT_STRING_DECLARE(s)  extern const char * const s##_CONST_STRING

#define LOGI(...)       logs(__FILENAME__, __func__, __LINE__, LOG_COLOR_I, ##__VA_ARGS__)
#define LOGE(...)       logs(__FILENAME__, __func__, __LINE__, LOG_COLOR_E, ##__VA_ARGS__)
#define LOGW(...)       logs(__FILENAME__, __func__, __LINE__, LOG_COLOR_W, ##__VA_ARGS__)
#define LOG_HEX(_arr, _len, ...)    log_hex(_arr, _len," %s\t%s()\tline:%d\t", __FILENAME__, __func__, __LINE__, ##__VA_ARGS__)
#define LOG_MARK        LOGI("")

#ifdef __cplusplus
}
#endif

#endif /* __LOG_H__ */
