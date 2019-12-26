#ifndef _AU3_COMMON_H
#define _AU3_COMMON_H
#pragma once

#ifdef _MSC_VER
#pragma warning(disable: 4996)
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define AU3_MAX_CONSTS  256
#define AU3_MAX_LOCALS  256
#define AU3_MAX_ARGS    32
#define AU3_MAX_FRAMES  64
#define AU3_MAX_STACK   (AU3_MAX_FRAMES * AU3_MAX_LOCALS)

#endif
