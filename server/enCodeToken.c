#include "header.h"
int enCodeToken(train_t t, char *buf){

    //生成存储jwt
    char *jwt = NULL;

    //记录jwt长度
    size_t jwt_len = 0;

    // 初始化编码参数结构体
    struct l8w8jwt_encoding_params params;
    l8w8jwt_encoding_params_init(&params);

    // 设置JWT使用的算法，这里使用HS512
    params.alg = L8W8JWT_ALG_HS512;

    // 设置JWT的主要荷载部分内容:
    // 主题
    params.sub = "Baidu_Cloud";  
    // 签发者
    params.iss = "WuKong";  
    // 接收方
    char uid_str[20] = { 0 };
    sprintf(uid_str,"%d",t.uid);
    params.aud = uid_str;  
    // token过期/有效时间(0x7fffffff:最大值-> 2038年1月19日)
    params.exp = 0x7fffffff;  
    // token签发时间
    params.iat = 0; 

    // 设置加密密钥
    params.secret_key = (unsigned char*)"snow string token key";
    params.secret_key_length = strlen((char *)params.secret_key);

    // 输出变量
    params.out = &jwt;
    params.out_length = &jwt_len;

    // 加密
    int ret = l8w8jwt_encode(&params);
    if(ret != L8W8JWT_SUCCESS){
        printf("JWT encoding failed with error code: %d\n", ret);
        return -1;  // 如果加密失败，返回NULL
    }
        
    strcpy(buf,jwt);

    // 释放token 字符串的内存
    l8w8jwt_free(jwt);

    return 0;
}

