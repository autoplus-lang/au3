#pragma once

#include "common.h"

typedef struct _obj obj_t;
typedef struct _str str_t;
typedef struct _fun fun_t;
typedef struct _upv upv_t;
typedef struct _map map_t;

typedef enum {
    VT_NULL_,
    VT_BOOL_,
    VT_NUM,
    VT_OBJ,
    VT_CFN,
    VT_PTR_
#define VT_NULL VT_NULL_
#define VT_BOOL VT_BOOL_
#define VT_PTR  VT_PTR_
} vtype_t;

typedef enum {
    OT_STR,
    OT_FUN,
    OT_UPV,
    OT_MAP,
} otype_t;

enum {
    VT_NULL_NULL    = CMB_BYTES(VT_NULL, VT_NULL),
    VT_NULL_BOOL    = CMB_BYTES(VT_NULL, VT_BOOL),
    VT_NULL_NUM     = CMB_BYTES(VT_NULL, VT_NUM),
    VT_NULL_OBJ     = CMB_BYTES(VT_NULL, VT_OBJ),

    VT_BOOL_NIL     = CMB_BYTES(VT_BOOL, VT_NULL),
    VT_BOOL_BOOL    = CMB_BYTES(VT_BOOL, VT_BOOL),
    VT_BOOL_NUM     = CMB_BYTES(VT_BOOL, VT_NUM),
    VT_BOOL_OBJ     = CMB_BYTES(VT_BOOL, VT_OBJ),

    VT_NUM_NIL      = CMB_BYTES(VT_NUM, VT_NULL),
    VT_NUM_BOOL     = CMB_BYTES(VT_NUM, VT_BOOL),
    VT_NUM_NUM      = CMB_BYTES(VT_NUM, VT_NUM),
    VT_NUM_OBJ      = CMB_BYTES(VT_NUM, VT_OBJ),

    VT_OBJ_NIL      = CMB_BYTES(VT_OBJ, VT_NULL),
    VT_OBJ_BOOL     = CMB_BYTES(VT_OBJ, VT_BOOL),
    VT_OBJ_NUM      = CMB_BYTES(VT_OBJ, VT_NUM),
    VT_OBJ_OBJ      = CMB_BYTES(VT_OBJ, VT_OBJ),

    VT_CFN_CFN      = CMB_BYTES(VT_CFN, VT_CFN),
    VT_PTR_PTR      = CMB_BYTES(VT_PTR, VT_PTR)
};

struct _val {
    vtype_t type;
    union {
        bool Bool : 1;
        double Num;
        cfn_t CFn;
        obj_t *Obj;
        void *Ptr;
        uint64_t Raw;
    };
};

typedef struct {
    int count;
    int capacity;
    val_t *values;
} arr_t;

static const val_t VAL_NULL = { .type = VT_NULL, .Raw = 0 };
static const val_t VAL_TRUE = { .type = VT_BOOL, .Bool = true };
static const val_t VAL_FALSE = { .type = VT_BOOL, .Bool = false };
static const val_t VAL_NULLPTR = { .type = VT_PTR, .Ptr = NULL };

#define VAL_BOOL(b)     ((val_t){ .type = VT_BOOL, .Bool = (b) })
#define VAL_NUM(n)      ((val_t){ .type = VT_NUM, .Num = (n) })
#define VAL_OBJ(o)      ((val_t){ .type = VT_OBJ, .Obj = (obj_t *)(o) })
#define VAL_CFN(c)      ((val_t){ .type = VT_CFN, .CFn = (c) })
#define VAL_PTR(p)      ((val_t){ .type = VT_PTR, .Ptr = (void *)(p) })

#define AS_BOOL(v)      ((v).Bool)
#define AS_NUM(v)       ((v).Num)
#define AS_OBJ(v)       ((v).Obj)
#define AS_CFN(v)       ((v).CFn)
#define AS_PTR(v)       ((v).Ptr)

#define IS_NULL(v)      (AS_TYPE(v) == VT_NULL)
#define IS_BOOL(v)      (AS_TYPE(v) == VT_BOOL)
#define IS_NUM(v)       (AS_TYPE(v) == VT_NUM)
#define IS_OBJ(v)       (AS_TYPE(v) == VT_OBJ)
#define IS_CFN(v)       (AS_TYPE(v) == VT_CFN)
#define IS_PTR(v)       (AS_TYPE(v) == VT_PTR)

#define AS_INT(v)       ((int)AS_NUM(v))
#define AS_INT64(v)     ((int64_t)AS_NUM(v))
#define AS_RAW(v)       ((v).Raw)
#define AS_TYPE(v)      ((v).type)

#define IS_FALSEY(v)    (!(bool)AS_RAW(v))

void val_print(val_t value);
bool val_equal(val_t a, val_t b);

void arr_init(arr_t *array);
void arr_free(arr_t *array);
int arr_add(arr_t *array, val_t value, bool allowdup);
