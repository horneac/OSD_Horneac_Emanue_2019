#include "common_lib.h"
#include "intutils.h"

QWORD
CalculatePercentage(
    IN      QWORD       WholeValue,
    IN      WORD        HundredsOfPercentage
    )
{
    ASSERT_INFO(HundredsOfPercentage <= 10000, 
                "This function should only be used for sub-unitary ratios!");
    ASSERT_INFO((MAX_QWORD / HundredsOfPercentage) >= WholeValue,
                "The multiplication between WholeValue and HundredsOfPercentage would overflow!");

    return (WholeValue * HundredsOfPercentage ) / 10000;
}
