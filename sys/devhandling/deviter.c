

extern uint32_t _sdevlist;
extern uint32_t _edevlist;

rdev_t *rdev_by_type(rdev_type_t type, uint32_t mask)
{
    for (rdev_t *dev = (rdev_t *)&_sdevlist;
         (dev < &_edevlist) && (dev != NULL);
         ++dev) {
        if ((dev->type.u32 & mask) == type.u32) {
            return dev;
        }
    }
    return NULL;
}
