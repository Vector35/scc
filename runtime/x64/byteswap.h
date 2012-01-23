#ifndef __LIBC__BYTESWAP_H__
#define __LIBC__BYTESWAP_H__

#define htons(x) __byteswap((uint16_t)(x))
#define htonl(x) __byteswap((uint32_t)(x))
#define ntohs(x) __byteswap((uint16_t)(x))
#define ntohl(x) __byteswap((uint32_t)(x))

#endif

