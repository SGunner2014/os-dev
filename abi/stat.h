#ifndef INCLUDE_ABI_STAT_H
#define INCLUDE_ABI_STAT_H

#include "../types.h"   /* uint16_t, uint32_t, int32_t — adjust path to placement */

/*
 * The kernel <-> newlib stat ABI.
 *
 * This is the single source of truth for the byte layout of `struct stat`
 * as it crosses the syscall boundary. The kernel is built -nostdinc and must
 * NOT include newlib's <sys/stat.h>; instead both sides agree on the layout
 * defined here.
 *
 * Layout is byte-for-byte identical to newlib's default 32-bit `struct stat`
 * (i686-base toolchain), verified with offsetof/sizeof:
 *
 *     off  size  field           off  size  field
 *      0     2   st_dev           20   12   st_atim
 *      2     2   st_ino           32   12   st_mtim
 *      4     4   st_mode          44   12   st_ctim
 *      8     2   st_nlink         56    4   st_blksize
 *     10     2   st_uid           60    4   st_blocks
 *     12     2   st_gid           64    8   st_spare4[2]
 *     14     2   st_rdev
 *     16     4   st_size          sizeof == 72
 *
 * Note: newlib's time fields are `struct timespec { time_t tv_sec; long tv_nsec; }`
 * where time_t is 8 bytes (4-aligned on i686) => 12 bytes each. We have no
 * 64-bit type in the kernel, so each timespec is represented as a raw 12-byte
 * slot (three 32-bit words). The kernel just zeroes them for now.
 */
struct abi_stat {
    uint16_t st_dev;        /*  0 */
    uint16_t st_ino;        /*  2 */
    uint32_t st_mode;       /*  4 */
    uint16_t st_nlink;      /*  8 */
    uint16_t st_uid;        /* 10 */
    uint16_t st_gid;        /* 12 */
    uint16_t st_rdev;       /* 14 */
    int32_t  st_size;       /* 16 */
    int32_t  st_atim[3];    /* 20 : { tv_sec (8), tv_nsec (4) } */
    int32_t  st_mtim[3];    /* 32 */
    int32_t  st_ctim[3];    /* 44 */
    int32_t  st_blksize;    /* 56 */
    int32_t  st_blocks;     /* 60 */
    int32_t  st_spare4[2];  /* 64 */
};

/* Fail the build immediately if this ever drifts from newlib's ABI. */
_Static_assert(sizeof(struct abi_stat) == 72,
               "abi_stat must match newlib struct stat (72 bytes)");
_Static_assert(__builtin_offsetof(struct abi_stat, st_mode) == 4,   "abi_stat drift");
_Static_assert(__builtin_offsetof(struct abi_stat, st_size) == 16,  "abi_stat drift");
_Static_assert(__builtin_offsetof(struct abi_stat, st_blksize) == 56, "abi_stat drift");

/* newlib S_IF* file-type bits (octal), for st_mode. */
#define ABI_S_IFMT   0170000  /* type mask            */
#define ABI_S_IFCHR  0020000  /* character special    */
#define ABI_S_IFDIR  0040000  /* directory            */
#define ABI_S_IFREG  0100000  /* regular file         */

#endif /* INCLUDE_ABI_STAT_H */
