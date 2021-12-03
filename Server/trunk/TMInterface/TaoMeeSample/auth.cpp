/**
 * @file auth.cpp
 * @brief 验证方法集合
 * @author baron baron@taomee.com
 * @version 1.0
 * @date 2010-10-19
 */

#include "auth.h"

#include <stdio.h>
#include <string.h>
#include <openssl/md5.h>
#include <openssl/des.h>

void md5_16to32(const unsigned char *p_md5_buffer_16,
                unsigned char *p_md5_buffer_32,
                int is_upper)
{
    const char *lower_set = "0123456789abcdef";
    const char *upper_set = "0123456789ABCDEF";

    if (is_upper)
    {
        for (int i = 0; i < 16; ++ i)
        {
            p_md5_buffer_32[2 * i]
                = upper_set[static_cast<int>(p_md5_buffer_16[i] >> 4)];
            p_md5_buffer_32[2 * i + 1]
                = upper_set[static_cast<int>(p_md5_buffer_16[i] & 0x0F)];
        }
    }
    else
    {
        for (int i = 0; i < 16; ++ i)
        {
            p_md5_buffer_32[2 * i]
                = lower_set[static_cast<int>(p_md5_buffer_16[i] >> 4)];
            p_md5_buffer_32[2 * i + 1]
                = lower_set[static_cast<int>(p_md5_buffer_16[i] & 0x0F)];
        }
    }

    return;
}

int gen_chnlhash32(int chnl_id,
                   const char *p_chnl_key,
                   const char *p_data,
                   int data_length,
                   chnlhash32_t *p_chnlhash32)
{
    if (!p_chnl_key || !p_data || !p_chnlhash32 || data_length < 0)
    {
        return -1;
    }

    char hash_buffer[MAX_HASH_BUFFER_LENGTH];
    unsigned char md5_buffer_16[16];
    unsigned char md5_buffer_32[32];
    int length = sprintf(hash_buffer,
                         "channelId=%d&securityCode=%s&data=",
                         chnl_id,
                         p_chnl_key);

    if (length <= 0)
    {
        return -1;
    }

    // 用户hash的缓冲区不大于MAX_HASH_BUFFER_LENGTH
    int length_empty = static_cast<int>(sizeof(hash_buffer)) - length;
    int length_copy = data_length <= length_empty ? data_length : length_empty;

    memcpy(hash_buffer + length, p_data, length_copy);
    length += length_copy;

    MD5(reinterpret_cast<unsigned char *>(hash_buffer), length, md5_buffer_16);
    md5_16to32(md5_buffer_16, md5_buffer_32);
    char x[33] = {'\0'};
    memcpy(x, md5_buffer_32, 32);

    p_chnlhash32->chnl_id = chnl_id;
    memcpy(p_chnlhash32->vfy_code, md5_buffer_32, 32);
    return 0;
}

int verify_chnlhash32(const chnlhash32_t *p_chnlhash32,
                      const char *p_key,
                      const char *p_data,
                      int data_length)
{
    if (!p_chnlhash32 || !p_key || !p_data || data_length < 0)
    {
        return -1;
    }

    char hash_buffer[MAX_HASH_BUFFER_LENGTH];
    unsigned char md5_buffer_16[16];
    unsigned char md5_buffer_32[32];

    int length = sprintf(hash_buffer,
                         "channelId=%d&securityCode=%s&data=",
                         p_chnlhash32->chnl_id,
                         p_key);

    if (length <= 0)
    {
        return -1;
    }

    int length_empty = static_cast<int>(sizeof(hash_buffer)) - length;
    int length_copy = data_length <= length_empty ? data_length : length_empty;

    // 用户hash的缓冲区不大于MAX_HASH_BUFFER_LENGTH
    memcpy(hash_buffer + length, p_data, length_copy);
    length += length_copy;

    MD5(reinterpret_cast<unsigned char *>(hash_buffer), length, md5_buffer_16);
    md5_16to32(md5_buffer_16, md5_buffer_32);

    if (memcmp(p_chnlhash32->vfy_code, md5_buffer_32, 32))
    {
        return -1;
    }

    return 0;
}

int encrypt_des(void *key, void *plaintext, void *ciphertext, int n_8bytes)
{
    if (!key || !plaintext || !ciphertext || n_8bytes <= 0)
    {
        return -1;
    }

    DES_key_schedule schedule;
    DES_set_key(reinterpret_cast<unsigned char (*)[8]>(key), &schedule);

    for (int i = 0; i < n_8bytes; ++ i)
    {
        DES_ecb_encrypt(reinterpret_cast<unsigned char (*)[8]>(plaintext) + i,
                        reinterpret_cast<unsigned char (*)[8]>(ciphertext) + i,
                        &schedule,
                        1);
    }

    return 0;
}

int decrypt_des(void *key, void *ciphertext, void *plaintext, int n_8bytes)
{
    if (!key || !plaintext || !ciphertext || n_8bytes <= 0)
    {
        return -1;
    }

    DES_key_schedule schedule;
    DES_set_key(reinterpret_cast<unsigned char (*)[8]>(key), &schedule);

    for (int i = 0; i < n_8bytes; ++ i)
    {
        DES_ecb_encrypt(reinterpret_cast<unsigned char (*)[8]>(ciphertext) + i,
                        reinterpret_cast<unsigned char (*)[8]>(plaintext) + i,
                        &schedule,
                        0);
    }

    return 0;
}
