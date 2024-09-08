#include "gongju.h"
#include <stdlib.h>
#include <string.h>

/**
  ******************************************************************************
  * 功能: 获取文件名
  * 参数: arg-> 输入字符串
  * 返回值: 返回提取的文件名，如果没有找到返回 NULL
  ******************************************************************************
**/
char* extractFilename(const char *arg) {
    // 找到最后一个/
    const char *last_slash = strrchr(arg, '/');

    // 如果没有/，找到最后一个空格
    if (last_slash == NULL) {
        last_slash = strrchr(arg, ' ');
    }

    // 如果没有找到任何分隔符，则返回 NULL
    if (last_slash == NULL) {
        return NULL;
    }

    // 获取文件名
    return strdup(last_slash + 1); // 使用 strdup 来分配内存并复制字符串
}

/**
  ******************************************************************************
  * 功能: 提取第 num 个参数
  * 参数: str -> 输入字符串，num -> 目标子字符串的索引，从1开始
  * 返回值: 返回提取的子字符串，如果没有找到返回 NULL
  ******************************************************************************
**/
char* extractParameters(const char *str, int num) {
    if (str == NULL || num < 1) {
        return NULL;
    }

    char temp[256] = {0};
    strcpy(temp, str);

    char *token;
    char *delimiters = " "; // 以空格作为第一个分隔符
    int count = 0;

    // 第一次分割，分割空格
    token = strtok(temp, delimiters);
    while (token != NULL) {
        count++;
        if (count == num) {
            // 找到目标子字符串
            return strdup(token); // 使用 strdup 来分配内存并复制字符串
        }
        token = strtok(NULL, delimiters);
    }

    return NULL; // 未找到第 num 个子字符串
}

/**
  ******************************************************************************
  * 功能: 提取第一个 / 后面的字符
  * 参数: arg -> 输入字符串
  * 返回值: 返回 / 后的字符串，如果没有找到返回 NULL
  ******************************************************************************
**/
char* extract_path_after_first_slash(const char *arg) {
    const char* slash = strchr(arg, '/'); // 查找第一个 '/'

    if (slash == NULL) {
        // 如果没有找到 '/'，则返回 NULL
        return NULL;
    }

    // 计算需要复制的字符数（包括结尾的 '\0'）
    size_t length = strlen(slash + 1); // 从 '/' 之后开始到字符串末尾的长度

    // 分配内存
    char* result = (char*)malloc(length + 1); // 加1是为了 '\0'

    if (result != NULL) {
        // 复制字符串（从 '/' 之后开始）
        strcpy(result, slash + 1);
    }

    return result;
}

