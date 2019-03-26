


#include

#include "nimble_scanlist.h"
#include "nimble_scanner.h"

static void _cmd_info(void)
{
    puts("INFO");
}

static void _cmd_adv(const char *name)
{
    if (name) {
        printf("ADVERTISING %s\n", name);
    }
    else {
        puts("ADV default name");
    }
}

static void _cmd_scan(unsigned duration)
{
    printf("SCANNING for %uus\n", duration);
}

static void _cmd_connect(unsigned pos)
{
    printf("CONNECT to %u\n", pos);
}

static void _cmd_close(unsigned chan)
{
    printf("CLOSE %u\n", chan);
}

static int _ishelp(char *argv)
{
    return memcmp(argv, "help", 4) == 0;
}

void app_ble_init(void)
{
    /* setup the scanning environment */
    nimble_scanlist_init();
    nimble_scanner_init(NULL, nimble_scanlist_update());
}

int app_ble_cmd(int argc, char **argv)
{
    if ((argc == 1) || _ishelp(argv[1])) {
        printf("usage: %s [help|info|adv|scan|connect|close]\n", argv[0]);
        return 0;
    }
    if ((memcmp(argv[1], "info", 4) == 0)) {
        _cmd_info();
    }
    if ((memcmp(argv[1], "adv", 3) == 0)) {
        char *name = NULL;
        if (argc > 2) {
            if (_ishelp(argv[2])) {
                printf("usage: %s adv [help|name]", argv[0]);
                return 0;
            }
            name = argv[2];
        }
        _cmd_adv(name);
    }
    if ((memcmp(argv[1], "scan", 4) == 0)) {
        uint32_t duration = SCAN_DUR_DEFAULT;
        if (argc > 2) {
            if (_ishelp(argv[2])) {
                printf("usage: %s scan [help|duration]", argv[0]);
                return 0;
            }
            duration = (uint32_t)atoi(argv[2]);
        }
        _cmd_scan(duration);
    }
    if ((memcmp(argv[1], "connect", 7) == 0)) {
        if ((argc < 3) || _ishelp(argv[2])) {
            printf("usage: %s connect [help|scanlist entry #]", argv[0]);
        }
        unsigned pos = (unsigned)atoi(argv[2]);
        _cmd_connect(pos);
    }
    if ((memcmp(argv[1], "close", 5) == 0)) {
        if ((argc < 3) || _ishelp(argv[2])) {
            printf("usage: %s scan [help|duration]", argv[0]);
            return 0;
        }
        unsigned chan = (unsigned)atoi(argv[2]);
        _cmd_close(pos);
    }
}
