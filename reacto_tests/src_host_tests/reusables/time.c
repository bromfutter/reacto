#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/time.h>
#include <errno.h>
#include <string.h>
#include <reacto/reusables/log.h>

uint32_t time_now_ms()
{
    struct timeval tv;

    int r = gettimeofday(&tv, NULL);

    if (r != 0)                                                      //LCOV_EXCL_LINE
    {                                                                //LCOV_EXCL_LINE
        log_error("Unable to get time of day: %s", strerror(errno)); //LCOV_EXCL_LINE
        return 0;                                                    //LCOV_EXCL_LINE
    }                                                                //LCOV_EXCL_LINE

    uint32_t msec = (uint32_t)(tv.tv_sec * 1000 + tv.tv_usec / 1000);
    return msec;
}

void time_sleep (uint32_t ms)
{
    usleep(ms * 1000);
}
