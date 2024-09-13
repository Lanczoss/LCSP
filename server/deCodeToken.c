#include "header.h"

int deCodeToken(char *buf){

    int uid = 0;

    // 初始化解码参数结构体
    struct l8w8jwt_decoding_params params;
    l8w8jwt_decoding_params_init(&params);  

    // 设置JWT使用的算法，这里使用HS512
    params.alg = L8W8JWT_ALG_HS512;

    char * str = buf;

    // 填入token字符串和长度
    params.jwt = (char*)str;
    params.jwt_length = strlen(str);

    printf("加密密文:%s\n",str);
    // 设置密钥
    char *key="Baidu_Cloud string token key";
    params.verification_key = (unsigned char*)key;
    params.verification_key_length = strlen(key);

    // 定义: 接收结果的指针和变量
    struct l8w8jwt_claim *claims = NULL;
    size_t claim_count = 0;
    enum l8w8jwt_validation_result validation_result;

    // 解码: 获得token中存储的信息
    int decode_result = l8w8jwt_decode(&params, &validation_result, &claims, &claim_count);

    // 判断是否是正确的token
    if (decode_result == L8W8JWT_SUCCESS && validation_result == L8W8JWT_VALID) {
        printf("Token validation successful!\n");

        // 打印token中解码出来的荷载信息
        for (size_t i = 0; i < claim_count; i++) {
            printf("Claim [%zu]: %s = %s\n", i, claims[i].key, claims[i].value);
            if(strcmp(claims[i].key,"aud") == 0){
                uid = atoi(claims[i].value);
            }
        }

    } else {
        printf("Token validation failed!\n");
    }

    l8w8jwt_free_claims(claims, claim_count);
    
    printf("deCode_uid:%d\n",uid);
    return uid;
}
