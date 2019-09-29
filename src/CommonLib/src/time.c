#include "common_lib.h"
#include "time.h"

STATUS
TimeGetStringFormattedBuffer(
    IN                          DATETIME        DateTime,
    OUT_WRITES_Z(BufferSize)    char*           Buffer,
    IN                          DWORD           BufferSize
    )
{
    if (NULL == Buffer)
    {
        return STATUS_INVALID_PARAMETER2;
    }

    return snprintf(Buffer, BufferSize,
                    "%02d/%02d/%04d %02d:%02d:%02d",
                    DateTime.Date.Day,
                    DateTime.Date.Month,
                    DateTime.Date.Year,
                    DateTime.Time.Hour,
                    DateTime.Time.Minute,
                    DateTime.Time.Second
                    );
}