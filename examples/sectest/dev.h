#include <limits.h>
#include <errno.h>

#define DEV_DEF             __attribute__ ((section ("devparams")))

// #define DEV_NAME(x)         #ifdef DEVELHELP \ .base.name(x), \ #endif

#define DEV_DONT_LIST       UINT_MAX

enum {
    DEV_NET             = (1 << 24),
    DEV_SENSE           = (2 << 24),
    DEV_ACT             = (3 << 24),
    DEV_PERIPH          = (4 << 24),   /* ??? */
    DEV_MEM             = (5 << 24)
};

enum {
    DEV_NET_ETH         = DEV_NET | (1 << 16),
    /* ... */
    DEV_SENSE_TEMP      = DEV_SENSE | (1 << 16),
    DEV_SENSE_LIGHT     = DEV_SENSE | (2 << 16),
    /* ... */
    DEV_ACT_MOT         = DEV_ACT | (1 << 16)
};

/**
 * global enum containing all supported devices
 */
enum {
    DEV_NET_ETH_W5100       = DEV_NET_ETH | 1,
    DEV_NET_ETH_ENC28J60    = DEV_NET_ETH | 2,
    DEV_NET_ETH_ENCX24J600  = DEV_NET_ETH | 3,
    /* ... */
    DEV_SENSE_TEMP_ABCDEV   = DEV_SENSE_TEMP | 1,
    /* ... */
    DEV_ACT_MOT_GENERIC     = DEV_ACT_MOT | 1,
    /* ... */
};

typedef union {
        uint32_t full;
        struct {
            uint8_t cls;
            uint8_t fam;
            uint16_t type;
        } sub;
} rdev_type_t;

typedef struct dev_params dev_params_t;
typedef struct rdev rdev_t;

typedef enum {
    FOOBAR,
    BARMUH
} dev_opt_t;

typedef struct {
    rdev_type_t type;
    size_t psize;
    void *api;
    void (*setup)(void *mem, dev_params_t *params, unsigned pos);
    void (*init)(rdev_t *root);
    int (*set)(rdev_t *dev, dev_opt_t opt, void *val, size_t len);
    int (*get)(rdev_t *dev, dev_opt_t opt, void *res, size_t max_len);
} dev_core_t;


struct dev_params {
    dev_core_t core;
#ifdef DEVELHELP
    const char *name;
#endif
};

struct rdev {
    dev_params_t *p;
};

// extern rdev_t *dev_list;
// extern size_t dev_list_len;

// void dev_basesetup(void *mem, size_t dev_des_len, unsigned pos)
// {
//     if (!mem) {
//         /* only applicable during init phase: allocate device descriptor */
//         rdev_t *dev = (rdev_t *)kinit_malloc(dev_des_len);
//     }
//     /* add device to device list if applicable */
//     if (pos != DEV_DONT_LIST) {
//         dev_list[pos] = dev;
//     }
// }


int dev_notsup(rdev_t *dev, dev_opt_t opt, void *res, size_t len)
{
    (void)dev;
    (void)opt;
    (void)res;
    (void)len;
    return -ENOTSUP;
}
