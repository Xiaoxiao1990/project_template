#include <stdio.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>

#include "log.h"
#include "times.h"
#include "utils.h"

#define FILE_NAME_LEN                       256
#define FILE_PATH_LEN                       256

static bool log_is_init = false;
static char log_file_name[FILE_NAME_LEN];

static void create_log_dir(void) {
    char trace_dir[FILE_PATH_LEN];

    memset(trace_dir, 0, sizeof(trace_dir));
    sprintf(trace_dir, "%s/%s/", LOG_PATH, LOG_DIR);
    if (!is_dir_exist(trace_dir)) {
        mk_dir(trace_dir);
    }
    printf("Root path of vsim logs:%s\n", trace_dir);
}

void log_trim(void) {
    DIR *dir;
    struct dirent *ptr;
    struct stat buf;
    int result;
    struct timeval tv;
    struct tm w;
    time_t timeBegin, timeEnd;
    char base[1000];
    char basePath[FILE_NAME_LEN];

    memset(basePath, 0, sizeof(basePath));
    snprintf(basePath, FILE_NAME_LEN, "%s/%s", LOG_PATH, LOG_DIR);

    gettimeofday(&tv, NULL);
    timeBegin = tv.tv_sec;
    timeEnd = timeBegin - 15 * 24 * 3600;//retain logs for 15 days.
    LOGI("current_log_file_name:%s", log_file_name);
    LOGI("Only retain logs for 15 days, cut range:timeBegin=%ld, timeEnd=%ld", timeBegin, timeEnd);
    if ((dir = opendir(basePath)) == NULL) {
        LOGI("Open dir error...");
    } else {
        while ((ptr = readdir(dir)) != NULL) {
            if (strcmp(ptr->d_name, ".") == 0 ||
                strcmp(ptr->d_name, "..") == 0)    ///current dir OR parent dir
                continue;
            else if (ptr->d_type == 8) {//file
                memset(base, 0, sizeof(base));
                sprintf(base, "%s/%s", basePath, ptr->d_name);
                result = stat(base, &buf);
                if (result != 0) {
                    LOGI("list file info error.");//No such file or directory
                } else {
                    if (buf.st_ctime < timeEnd) {
                        remove(base);
                        LOGI("%s is deleted.", ptr->d_name);
                        gmtime_r(&buf.st_mtime, &w);
                        LOGI("Last Modify Time: %ld,%04d-%02d-%02d-%02d-%02d-%02d",
                             buf.st_ctime, w.tm_year + 1900, w.tm_mon + 1, w.tm_mday,
                             w.tm_hour, w.tm_min, w.tm_sec);
                    }
                }
            } else if (ptr->d_type == 10)    //link file
                LOGI("10,d_name:%s/%s", basePath, ptr->d_name);
            else if (ptr->d_type == 4)    //dir
                LOGI("4,d_name:%s/%s", basePath, ptr->d_name);
        }
        closedir(dir);
    }
}

void log_init(void)
{
    create_log_dir();   //create directory  /home/root/linux_cpe/logs
    memset(log_file_name, 0, sizeof(log_file_name));

    snprintf(log_file_name, FILE_NAME_LEN, "%s/%s/%s.txt", LOG_PATH, LOG_DIR,"cpe_logs");
    printf("Log File Name: %s\n", log_file_name);
    log_is_init = true;

    if (!is_file_exist("/etc/localtime"))
        set_timezone(8);
}

int log_raw(const char *fmt, ...)
{
    va_list list;
    char buffer[LOGS_BUFFER_LENGTH];

    va_start(list, fmt);
    vsnprintf(buffer, LOGS_BUFFER_LENGTH, fmt, list);
    va_end(list);

    printf("%s\n", buffer);
#ifdef _LOG_TO_FILE_
    FILE *log_file;

    log_file = fopen(log_file_name, "a+");

    if(log_file == NULL)return -1;

    fprintf(log_file, "%s\n", buffer);
    fclose(log_file);
#endif
    return 0;
}

int logs(const char *file, const char *func, uint32_t line, const char *LOG_COLOR, const char *fmt, ...)
{
    va_list list;
    struct date_time_str time;
    char buffer[LOGS_BUFFER_LENGTH] = {0};
    char header[100] = {0};

    if (!log_is_init)
        log_init();

    get_datetime_string(&time);
    snprintf(header, 100, "%s  line:%-4d", file, line);

    va_start(list, fmt);
    vsnprintf(buffer, LOGS_BUFFER_LENGTH, fmt, list);
    va_end(list);

    printf("[%s]%s %s  %s"LOG_RESET_COLOR"\n", time.string, LOG_COLOR, header, buffer);
#ifdef _LOG_TO_FILE_
    FILE *log_file;

    log_file = fopen(log_file_name, "a+");

    if(log_file == NULL)return -1;

//    fprintf(log_file, "[%s]%s %s  %s"LOG_RESET_COLOR"\n", time.string, LOG_COLOR, header, buffer);
    fprintf(log_file, "[%s] %s %s\n", time.string, header, buffer);
    fclose(log_file);
#endif
    return 0;
}

int log_hex(uint8_t *arr, size_t arr_len, const char *fmt, ...)
{
    va_list list;
//    struct date_time_str time;
    size_t i = 0, j = 0;
    char hex[PRINT_BUFFER_LENGTH] = {0};
    char notice[PRINT_BUFFER_LENGTH] = {0};

//    get_datetime_string(&time);

    for(j = 0;j < PRINT_BUFFER_LENGTH - 3;){
        if(i >= arr_len)break;
        snprintf(&hex[j], PRINT_BUFFER_LENGTH,"%02X ", arr[i++]);
        j += 3;
    }
    hex[j] = 0;

    va_start(list, fmt);
    vsnprintf(notice, LOGS_BUFFER_LENGTH, fmt, list);
    va_end(list);

//    printf("[%s]%s %s  %s"LOG_RESET_COLOR"\n", time.string, LOG_COLOR, header, buffer);
//#ifdef _LOG_TO_FILE_
//    FILE *log_file;
//
//    log_file = fopen(log_file_name, "a+");
//
//    if(log_file == NULL)return -1;
//
////    fprintf(log_file, "[%s] %s %s\n", time.string, header, buffer);
//    fprintf(log_file, "[%s]%s %s  %s"LOG_RESET_COLOR"\n", time.string, LOG_COLOR, header, buffer);
//    fclose(log_file);
//#endif

    LOGI("%s%s", notice, hex);

    return 0;
}
