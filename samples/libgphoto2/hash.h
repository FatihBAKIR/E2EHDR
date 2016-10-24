//
// Created by Mehmet Fatih BAKIR on 24/10/2016.
//

#ifndef CAMERA_HASH_H
#define CAMERA_HASH_H
// Generate CRC lookup table
template <unsigned c, int k = 8>
struct f : f<((c & 1) ? 0xedb88320 : 0) ^ (c >> 1), k - 1> {};
template <unsigned c> struct f<c, 0>{enum {value = c};};

#define HASH_A(x) HASH_B(x) HASH_B(x + 128)
#define HASH_B(x) HASH_C(x) HASH_C(x +  64)
#define HASH_C(x) HASH_D(x) HASH_D(x +  32)
#define HASH_D(x) HASH_E(x) HASH_E(x +  16)
#define HASH_E(x) HASH_F(x) HASH_F(x +   8)
#define HASH_F(x) HASH_G(x) HASH_G(x +   4)
#define HASH_G(x) HASH_H(x) HASH_H(x +   2)
#define HASH_H(x) HASH_I(x) HASH_I(x +   1)
#define HASH_I(x) f<x>::value ,

constexpr unsigned crc_table[] = { HASH_A(0) };

// Constexpr implementation and helpers
constexpr uint32_t crc32_impl(const char* p, size_t len, uint32_t crc) {
    return len ?
            crc32_impl(p+1,len-1,(crc>>8)^crc_table[(crc&0xFF)^*p])
            : crc;
}

constexpr uint32_t crc32(const char* data, size_t length) {
    return ~crc32_impl(data, length, ~0);
}

constexpr size_t strlen_c(const char* str) {
    return *str ? 1+strlen_c(str+1) : 0;
}

constexpr int WSID(const char* str) {
    return crc32(str, strlen_c(str));
}

#endif //CAMERA_HASH_H
