/**
 * @file auth.h
 * @brief 验证方法集合
 * @author baron baron@taomee.com
 * @version 1.0
 * @date 2010-10-19
 */

#ifndef __AUTH_H__
#define __AUTH_H__

#define MAX_HASH_BUFFER_LENGTH 8192

#include <stdint.h>

/**
 * @brief 渠道验证结构
 */
typedef struct {
    uint16_t chnl_id;
    uint8_t vfy_code[32];
}__attribute__((packed)) chnlhash32_t;

/**
 * @brief 16位md5码转32位md5码
 *
 * @param p_md5_buffer_16 指向16位md5码缓冲区
 * @param p_md5_buffer_32 指向32位md5码缓冲区
 * @param is_upper        是否大写(默认0,小写)
 */
void md5_16to32(const unsigned char *p_md5_buffer_16,
                unsigned char *p_md5_buffer_32,
                int is_upper = 0);

/**
 * @brief 生成渠道验证结构体
 *
 * @param chnl_id                   渠道ID
 * @param p_chnl_key                指向渠道验证密钥
 * @param p_data                    指向数据缓冲区
 * @param data_length               指向数据缓冲区
 * @param p_chnlhash32_vfy_header   数据缓冲区大小
 *
 * @return 0-成功 -1-失败
 */
int gen_chnlhash32(int chnl_id,
                   const char *p_chnl_key,
                   const char *p_data,
                   int data_length,
                   chnlhash32_t *p_chnlhash32);

/**
 * @brief 验证渠道结构体
 *
 * @param p_chnlhash32  指向渠道验证结构体
 * @param p_key         指向验证密钥
 * @param p_data        指向数据缓冲区
 * @param data_length   数据缓冲区大小
 *
 * @return 0-成功 -1-失败
 */
int verify_chnlhash32(const chnlhash32_t *p_chnlhash32,
                      const char *p_key,
                      const char *p_data,
                      int data_length);

/**
 * @brief 使用DES加密,明、密文缓冲区需要是8字节倍数，密钥大小8字节
 *
 * @param key           密钥
 * @param plaintext     明文
 * @param ciphertext    密文
 * @param n_8bytes      明文缓冲区对于8字节的倍数
 *
 * @return 0-成功 -1-失败
 */
int encrypt_des(void *key, void *plaintext, void* ciphertext, int n_8bytes);

/**
 * @brief 使用DES加密,明、密文缓冲区需要是8字节倍数，密钥大小8字节
 *
 * @param key           密钥
 * @param ciphertext    密文
 * @param plaintext     明文
 * @param n_8bytes      明文缓冲区对于8字节的倍数
 *
 * @return 0-成功 -1-失败
 */
int decrypt_des(void *key, void *ciphertext, void *plaintext, int n_8bytes);

#endif //!__AUTH_H__
