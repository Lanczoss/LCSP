#include "gongju.h"
#include <stdlib.h>
#include <string.h>

/**
  ******************************************************************************
  * 功能: 获取文件名
  * 参数: arg -> 输入字符串
  *        filename -> 输出文件名的缓冲区
  *        max_len -> 文件名缓冲区的最大长度
  * 返回值: 返回 0 表示成功，返回 -1 表示失败
  ******************************************************************************
**/
int extractFilename(const char *arg, char *filename, int max_len) {
    // 找到最后一个/
    const char *last_slash = strrchr(arg, '/');

    // 如果没有/，找到最后一个空格
    if (last_slash == NULL) {
        last_slash = strrchr(arg, ' ');
    }

    // 如果没有找到任何分隔符，则返回 -1
    if (last_slash == NULL) {
        return -1;
    }

    // 获取文件名
    strncpy(filename, last_slash + 1, max_len - 1);
    filename[max_len - 1] = '\0'; // 确保以 '\0' 结尾

    return 0; // 成功
}

/**
  ******************************************************************************
  * 功能: 提取第 num 个参数
  * 参数: str -> 输入字符串
  *        num -> 目标子字符串的索引，从1开始
  *        parameter -> 输出子字符串的缓冲区
  *        max_len -> 子字符串缓冲区的最大长度
  * 返回值: 返回 0 表示成功，返回 -1 表示失败
  ******************************************************************************
**/
int extractParameters(const char *str, int num, char *parameter, int max_len) {
    if (str == NULL || num < 1) {
        return -1;
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
            strncpy(parameter, token, max_len - 1);
            parameter[max_len - 1] = '\0'; // 确保以 '\0' 结尾
            return 0; // 成功
        }
        token = strtok(NULL, delimiters);
    }

    return -1; // 未找到第 num 个子字符串
}

/**
  ******************************************************************************
  * 功能: 提取第一个 / 后面的字符
  * 参数: arg -> 输入字符串
  *        result -> 输出 / 后的字符串的缓冲区
  *        max_len -> 输出缓冲区的最大长度
  * 返回值: 返回 0 表示成功，返回 -1 表示失败
  ******************************************************************************
**/
int extract_path_after_first_slash(const char *arg, char *result, int max_len) {
    const char* slash = strchr(arg, '/'); // 查找第一个 '/'

    if (slash == NULL) {
        // 如果没有找到 '/'，则返回 -1
        return -1;
    }

    // 计算需要复制的字符数（包括结尾的 '\0'）
    size_t length = strlen(slash + 1); // 从 '/' 之后开始到字符串末尾的长度

    // 检查输出缓冲区的长度
    if (length >= max_len) {
        return -1; // 输出缓冲区太小
    }

    // 复制字符串（从 '/' 之后开始）
    strcpy(result, slash + 1);

    return 0; // 成功
}

/**
 * 功能: 从给定的路径字符串中提取出斜杠（`/`）前的部分
 * 参数: path -> 输入的路径字符串
 *        base_path -> 输出路径部分的缓冲区
 *        max_len -> 输出缓冲区的最大长度
 * 返回值: 返回 0 表示成功，返回 -1 表示失败
 */
int extractBasePath(const char *path, char *base_path, int max_len) {
    if (path == NULL) {
        return -1;
    }

    // 查找第一个 '/' 的位置
    const char *first_slash = strchr(path, '/');

    if (first_slash == NULL) {
        // 如果没有找到 '/'，返回原始字符串的副本
        strncpy(base_path, path, max_len - 1);
        base_path[max_len - 1] = '\0'; // 确保以 '\0' 结尾
        return 0; // 成功
    }

    // 计算需要的长度，包括第一个 '/'
    size_t length = first_slash - path + 1; // +1 保留 '/'

    // 检查输出缓冲区的长度
    if (length >= max_len) {
        return -1; // 输出缓冲区太小
    }

    // 复制字符串
    strncpy(base_path, path, length);
    base_path[length] = '\0'; // 确保以 '\0' 结尾

    return 0; // 成功
}
