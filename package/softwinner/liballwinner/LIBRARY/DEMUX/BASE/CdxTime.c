#include <CdxTypes.h>

#include <sys/time.h>

#include <CdxTime.h>

cdx_int64 CdxGetNowUs(cdx_void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);

    return (cdx_int64)tv.tv_usec + tv.tv_sec * 1000000ll;
}

