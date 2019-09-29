#include "ut_base.h"
#include "ut_cl_bitmap.h"
#include "bitmap.h"

BOOLEAN
TcBitmapRun(
    void
    )
{
    BITMAP bmp;

    printf("Will test BITMAP functions\n");

    BitmapPreinit(&bmp, 10 );

    return TRUE;
}