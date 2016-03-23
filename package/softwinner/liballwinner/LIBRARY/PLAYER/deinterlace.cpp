#include <deinterlace.h>
#include <deinterlaceHw.h>

#ifdef USE_SW_DEINTERLACE
#include <sw-deinterlace/deinterlaceSw.h>
#endif

Deinterlace *DeinterlaceCreate()
{
    Deinterlace *di = NULL;
    
#if (CONFIG_CHIP == OPTION_CHIP_1639 || CONFIG_CHIP == OPTION_CHIP_1680 || CONFIG_CHIP == OPTION_CHIP_1689)

    di = new DeinterlaceHw();
    
#elif CONFIG_CHIP == OPTION_CHIP_1673

    /* PD should add sw interlace here... */
    #ifdef USE_SW_DEINTERLACE
    di = new DeinterlaceSw();
    #endif

#endif
    return di;
}

