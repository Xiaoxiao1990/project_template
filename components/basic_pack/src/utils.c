//#include <azure_c_shared_utility/refcount.h>

/**
 * Copyright (c) 2018 TGT, Inc. All Rights Reserved.
 * Date   : 8/30/18 5:54 PM
 * Author : Zhou Linlin
 * e-mail : 461146760@qq.com
 * */

#include "utils.h"
#include "types.h"
#include "log.h"

#include <stdarg.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <openssl/md5.h>
#include <stdio.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <ctype.h>

char big_endian_test(void)
{
	/*定义一个2个字节长度的数据，并赋值为1,则n的16进制表示为0x0001
	如果系统以“大端”存放数据，也即是以MSB方式存放，那么低字节存放的必定是0x00，高字节存放的必定是0x01
	如果系统以“小端”存放数据，也即是以LSB方式存放，那么低字节存放的必定是0x01，高字节存放的必定是0x00
	所谓MSB，就是将最重要的位存入低位，而LSB则是将最不重要的位存入低位
	我们可以通过检测低位的数值就可以知道系统的字节序
	*/
	const s16 n = 1;
	if (*(char *)&n) {
		return LittleEndian;
	}
	return BigEndian;
}

int str2hex(const char *str, unsigned char *hex, int hex_len)
{
	int i = 0;
	unsigned char ch;

	if (str == NULL || hex == NULL || hex_len <= 0)
		return -1;

	while (*str) {
		if (*str >= '0' && *str <= '9') {
			ch = *str - '0';
		} else if (*str >= 'a' && *str <= 'f') {
			ch = *str - 'a' + 10;
		} else if (*str >= 'A' && *str <= 'F') {
			ch = *str - 'A' + 10;
		} else {
			return -1;
		}

		if ((i % 2) == 0)
			hex[i / 2] = (unsigned char)((ch << 4) & 0xF0);
		else {
			hex[i / 2] |= (ch & 0x0F);
			if (i / 2 >= hex_len)
				return hex_len;
		}
		i++;
		str++;
	}

	return i >> 1;
}

int hex2str(const unsigned char *hex, const int hex_len, char *str, int str_len)
{
	char ch[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
	int i = 0, j = 0;

	if ((hex == NULL) || (str == NULL) || hex_len <= 0 || str_len <= 0)
		return -1;

	while ((i + 2) < str_len) {
		str[i++] = ch[((hex[j] >> 4) & 0x0F)];
		str[i++] = ch[(hex[j++] & 0x0F)];
		str[i] = 0;
		if (j >= hex_len)
			break;
	}

	return i;
}

/* naive function to check whether char *s is an ip address */
int is_ip_address(const char *s)
{
	u32 n1, n2, n3, n4;

	if (sscanf(s, "%u.%u.%u.%u", &n1, &n2, &n3, &n4) != 4)
		return 0;

	if ((n1 <= 255) && (n2 <= 255) && (n3 <= 255) && (n4 <= 255))
		return 1;

	return 0;
}

#define BUF_SIZE 1024

void get_pid_by_name(pid_t *pid, const char *task_name)
{
    DIR *dir;
    struct dirent *ptr;
    FILE *fp;
    char filepath[1024];
    char cur_task_name[256];
    char buf[BUF_SIZE];

    dir = opendir("/proc");
    if (NULL != dir) {
        while ((ptr = readdir(dir)) != NULL) { //Cycle read every file/folder under /proc
            // Skip if it reads "." or "..", and skip if it reads not the folder name
            if ((strcmp(ptr->d_name, ".") == 0) || (strcmp(ptr->d_name, "..") == 0))
                continue;

            if (DT_DIR != ptr->d_type)
                continue;

            sprintf(filepath, "/proc/%s/status", ptr->d_name);//Generate the path of the file to be read
            fp = fopen(filepath, "r");
            if (NULL != fp) {
                if(fgets(buf, BUF_SIZE - 1, fp) == NULL) {
                    fclose(fp);
                    continue;
                }

                sscanf(buf, "%*s %s", cur_task_name);

                //If the file content meets the requirements, print the name of the path (that is, the PID of the process)
                if (0 == strcmp(task_name, cur_task_name)){
                    sscanf(ptr->d_name, "%d", pid);
                    fclose(fp);
                    closedir(dir);
                    return;
                }

                fclose(fp);
            }
        }
        closedir(dir);
    }
}

bool is_process_running(const char *process)
{
	pid_t pid = -1;

	get_pid_by_name(&pid, process);

    return (pid != -1);
}

void get_name_by_pid(pid_t pid, char *task_name)
{
    char proc_pid_path[BUF_SIZE];
    char buf[BUF_SIZE];

    sprintf(proc_pid_path, "/proc/%d/status", pid);
    FILE* fp = fopen(proc_pid_path, "r");
    if(NULL != fp){
        if(fgets(buf, BUF_SIZE-1, fp)== NULL){
            fclose(fp);
        }
        fclose(fp);
        sscanf(buf, "%*s %s", task_name);
    }
}

int my_kill(const char *process)
{
	char cmd[MAX_COMMAND_STRING_LENGTH];
	FILE *pF = NULL;
	pid_t pid;

	get_pid_by_name(&pid, process);

	if (pid < 0)
		return -1;

	snprintf(cmd, MAX_COMMAND_STRING_LENGTH, "kill -9 %d", pid);
	LOGI("{Kill process[%s]}[%s]", process, cmd);
	if ((pF = popen(cmd, "r")) == NULL) {
		return -1;
	}
	pclose(pF);

	return 0;
}

int my_pkill(const char *process)
{
	pid_t pid;

	do {
		get_pid_by_name(&pid, process);
		if (pid <= 0)
			break;

		LOGI("Kill pid:%d", pid);
		my_kill(process);
		sleep(1);
	} while (1);

	return 0;
}

static char file_md5_checksum[MD5_DIGEST_LENGTH * 2 + 1];
char *get_md5_checksum_from_buffer(const unsigned char *data, size_t len)
{
	unsigned char md[MD5_DIGEST_LENGTH] = {0};

	MD5(data, len, md);
	memset(file_md5_checksum, 0, sizeof(file_md5_checksum));
	for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
		sprintf(file_md5_checksum + i * 2, "%02x", md[i]);
	}
	file_md5_checksum[MD5_DIGEST_LENGTH * 2] = '\0';
	return file_md5_checksum;
}

char *get_md5_checksum_from_file(const char *file_name)
{
	FILE* fp = NULL;
	unsigned char md5_hex[MD5_DIGEST_LENGTH];
	char *line = NULL;
	size_t len = 0;
	ssize_t nread;
	MD5_CTX ctx;

	fp = fopen(file_name, "rb");
	if (NULL == fp) {
		return NULL;
	}

	MD5_Init(&ctx);

	while ((nread = getline(&line, &len, fp)) != -1) {
		MD5_Update(&ctx, line, nread);
	}
	free(line);
	MD5_Final(md5_hex, &ctx);
	fclose(fp);

	memset(file_md5_checksum, 0, sizeof(file_md5_checksum));

	for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
		sprintf(file_md5_checksum + i * 2, "%02x", md5_hex[i]);
	}
	file_md5_checksum[MD5_DIGEST_LENGTH * 2] = '\0';

	return file_md5_checksum;
}

int my_system(const char *fmt, ...) /* with appropriate signal handling */
{
	va_list list;
	char cmdstring[MAX_COMMAND_STRING_LENGTH] = {0};
	pid_t pid;
	int status;
	struct sigaction ignore, saveintr, savequit;
	sigset_t chldmask, savemask;

	va_start(list, fmt);
	vsnprintf(cmdstring, MAX_COMMAND_STRING_LENGTH, fmt, list);
	va_end(list);

//	if (cmdstring == NULL)
//		return (1); /* always a command processor with UNIX */

	ignore.sa_handler = SIG_IGN; /* ignore SIGINT and SIGQUIT */
	sigemptyset(&ignore.sa_mask);
	ignore.sa_flags = 0;
	if (sigaction(SIGINT, &ignore, &saveintr) < 0)
		return (-1);
	if (sigaction(SIGQUIT, &ignore, &savequit) < 0)
		return (-1);
	sigemptyset(&chldmask); /* now block SIGCHLD */
	sigaddset(&chldmask, SIGCHLD);
	if (sigprocmask(SIG_BLOCK, &chldmask, &savemask) < 0)
		return (-1);

	if ((pid = fork()) < 0) {
		status = -1; /* probably out of processes */
	} else if (pid == 0) { /* child */
		/* restore previous signal actions & reset signal mask */
		sigaction(SIGINT, &saveintr, NULL);
		sigaction(SIGQUIT, &savequit, NULL);
		sigprocmask(SIG_SETMASK, &savemask, NULL);
		execl("/bin/sh", "sh", "-c", cmdstring, (char *)0);
		_exit(127); /* exec error */
	} else { /* parent */
		while (waitpid(pid, &status, 0) < 0)
			if (errno != EINTR) {
				status = -1; /* error other than EINTR from waitpid() */
				break;
			}
	}

	/* restore previous signal actions & reset signal mask */
	if (sigaction(SIGINT, &saveintr, NULL) < 0)
		return (-1);
	if (sigaction(SIGQUIT, &savequit, NULL) < 0)
		return (-1);
	if (sigprocmask(SIG_SETMASK, &savemask, NULL) < 0)
		return (-1);

	return (status);
}

int is_file_exist(const char *file_path)
{
    if(file_path == NULL)return 0;

    if(access(file_path, F_OK) == 0)return 1;

    return 0;
}

int is_dir_exist(const char *dir_path)
{
    DIR *dir;
    if(dir_path == NULL)return 0;

    if((dir = opendir(dir_path)) == NULL)return 0;
    else closedir(dir);

    return 1;
}

#define MODE                                (S_IRWXU | S_IRWXG | S_IRWXO)
int mk_dir(const char *dir)
{
    DIR *logdir = NULL;

    if((logdir = opendir(dir)) == NULL){
        if(0 != (mkdir(dir, MODE))){
            return -1;
        }
    } else closedir(logdir);

    return 0;
}

int mk_path(const char *path)
{
    char command[PATH_MAX];

    if(path == NULL)return -1;

    if(snprintf(command, PATH_MAX, "mkdir -p %s", path) == 0)return -1;

    return system(command);
}

int __execute_command(const char *file, const char *func, uint32_t line, char *resp, size_t resp_len, const char *fmt, ...)
{
    va_list list;
    FILE *pF = NULL;
    int res;
    static char cmd[MAX_COMMAND_STRING_LENGTH] = {0};

    if (resp_len < 0 || resp == NULL) {
        LOGE("No space for command response!");
        return -1;
    }

    memset(cmd, 0, MAX_COMMAND_STRING_LENGTH);

    va_start(list, fmt);
    vsnprintf(cmd, MAX_COMMAND_STRING_LENGTH, fmt, list);
    va_end(list);

//    LOGI("command: %s", cmd);
    logs(file, "", line, LOG_COLOR_I, "%s call execute_command: %s", func, cmd);

    if ((pF = popen(cmd, "r")) == NULL) {
        return -1;
    }

    if (0 < fread(resp, resp_len, resp_len, pF)) {
        LOGE("Get response error!");
        res = -1;
    } else
        res = 0;

    pclose(pF);

    return res;
}

int hex_char_to_int(char hex)
{
    int outHex;

    if (isdigit(hex)) {
        outHex = hex - '0';
    } else if (isupper(hex)) {
        outHex = hex - 'A' + 10;
    } else {
        outHex = hex - 'a' + 10;
    }

    return outHex;
}

int __execute_shell_command(const char *file, const char *func, uint32_t line, const char *fmt, ...)
{
    va_list list;
    char cmd[PATH_MAX] = {0};

    va_start(list, fmt);
    vsnprintf(cmd, LOGS_BUFFER_LENGTH, fmt, list);
    va_end(list);

    logs(file, "", line, LOG_COLOR_I, "%s call execute_shell_command: %s", func, cmd);

    return system(cmd);
}

int copy(const char *src, const char *dest)
{
    return execute_shell_command("cp %s %s", src, dest);
}

int mv(const char *src, const char *dest)
{
    return execute_shell_command("mv %s %s", src, dest);
}

size_t file_size(const char *file)
{
    struct stat buf;

    if (stat(file, &buf) < 0)
        return 0;
    return buf.st_size;
}

size_t get_directory_size(const char *path)
{
    struct dirent *dir = NULL;
    DIR *pdir = NULL;
    char child[512] = {0};
    size_t t_size = 0, c_size;
    if (path == NULL)
        return 0;

    pdir = opendir(path);
    if (pdir == NULL)
        return 0;

    while (NULL != (dir = readdir(pdir))) {
        if (dir->d_type & DT_DIR) {
            if ((0 == strcmp(dir->d_name, ".")) || (0 == strcmp(dir->d_name, ".."))) {
                continue;
            }
            printf("%s\n", dir->d_name);
            memset(child, 0, sizeof(child));
            snprintf(child, sizeof(child), "%s/%s", path, dir->d_name);
            t_size += get_directory_size(child);
        } else {
            snprintf(child, sizeof(child), "%s/%s", path, dir->d_name);
            c_size = file_size(child);
            t_size += c_size;
            printf("%s\t%zu\n", dir->d_name, c_size);
        }
    }

    closedir(pdir);
    return t_size;
}

int string_set(const char **target, const char *value)
{
    if (target == NULL)
        return -1;

    free((void *)*target);
    *target = strdup(value);

    return 0;
}

void string_free(const char *string)
{
    if (string)
        free((void *)string);
}

/*****************************************************************************
 函 数 名  : strtrim
 功能描述  : 删除行首、行尾空白符号
 输入参数  : char *is
 输出参数  : 无
 返 回 值  : char *
*****************************************************************************/
char *strtrim(char *s)
{
    char *p = s;
    char *q = s;

    //去掉行首的空格
    while (*p == ' ' || *p == '\t')
        ++p;
    //赋值
    while ((p != NULL)&&(q != NULL)&&(*p != '\0')&&(*p != '\0')) {
        *q++ = *p++;
    };

    //删除'\0'字符 注意是 -2 上面q++是先操作再自加
    q -= 2;
    //去掉行末的空格
    while (*q == ' ' || *q == '\t')
        --q;
    //给字符串添加字符结束标志
    *(q+1) ='\0';
    //这里的return s要注意看好
    //因为p q经过一系列操作后，已经不是原来的位置，越界了 ，s还在原来位置，所以return s才是正确的。
    return s;
}

/*****************************************************************************
 函 数 名  : strtrimc
 功能描述  : 删除字符串空白符,包括行首和行尾
 输入参数  : char * s
 输出参数  : 无
 返 回 值  : char *
*****************************************************************************/
char *strtrimc(char *s)
{
    char * p1 = s;
    char * p2 = s;

    while(*p1 != '\0') {
        while(*p1 == ' ' || *p1 == '\t') {
            p1 ++;
        }
        * p2 ++ = *p1 ++;
    }

    *p2 = '\0';

    return (s);
}