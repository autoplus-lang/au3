#ifndef _AU3_COMMON_H
#define _AU3_COMMON_H
#pragma once

#ifdef _MSC_VER
#pragma warning(disable: 4996)
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define AU3_COMBINE(l, r)   (uint8_t)(((char)(l) & 0xF) | ((char)(r) & 0xF) << 4)
#define AUP_COMBINE_L(c)    (char)(((c) & 0xF))
#define AUP_COMBINE_R(c)    (char)((((c) >> 4) & 0xF))

#define AU3_MAX_CONSTS      256
#define AU3_MAX_LOCALS      256
#define AU3_MAX_ARGS        32
#define AU3_MAX_FRAMES      64
#define AU3_MAX_STACK       (AU3_MAX_FRAMES * AU3_MAX_LOCALS)

#endif
