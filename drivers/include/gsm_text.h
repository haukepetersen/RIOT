
/* TODO */



typedef struct {
    int (*send_text)(void *dev, const char *number, const char *text);
} tm_driver_t;
