#include "ut_base.h"
#include <vector>
#include <string>
#include <functional>
#include "cl_interface.h"
#include "cl_memory.h"
#include "ut_cl_string.h"
#include "ut_cl_stack_dynamic.h"
#include "ut_cl_hash_table.h"

typedef struct _CL_UNIT_TEST
{
    const std::string               TestName;
    const std::function<STATUS()>   TestFunction;
} CL_UNIT_TEST;

STATUS TstStrings(void)
{
    ASSERT(strcmp("Ion", "Ion") == cl_strcmp("Ion", "Ion"));

    return CL_STATUS_SUCCESS;
}

const CL_UNIT_TEST CL_TESTS[] =
{
    {"Strings", UtClStrings},
    {"Memory", TstStrings},
    {"DynamicStack", UtClStackDynamic},
    {"HashTable", UtClHashTable},
};

static constexpr auto NO_OF_CL_TESTS = ARRAYSIZE(CL_TESTS);
static constexpr auto DEFAULT_TIMES_TO_RUN = 1;

int main(
    int argc,
    char* argv[]
)
{
    DWORD result;
    STATUS status;
    COMMON_LIB_INIT init;

    result = 0;
    cl_memzero(&init, sizeof(COMMON_LIB_INIT));

    init.MonitorSupport = FALSE;
    init.AssertFunction = UtCommonLibAssert;
    init.Size = sizeof(COMMON_LIB_INIT);

    status = CommonLibInit(&init);
    if (!SUCCEEDED(status))
    {
        LOG_ERROR("CommonLibInit failed with status: 0x%X\n", status );
        return status;
    }

    if (argc < 2)
    {
        LOG_ERROR("Usage %s $TEST_CASE [$NO_OF_TIMES]\n", argv[0]);
        return 1;
    }

    auto timesToRun = (argc >= 3) ? std::strtol(argv[2], nullptr, 10) : DEFAULT_TIMES_TO_RUN;
    auto tstToRun = argv[1];

    LOG("Will run testcase [%s] %d times!\n",
        tstToRun, timesToRun);

    for (const auto& tst : CL_TESTS)
    {
        if (tst.TestName == tstToRun)
        {
            for (auto i = 0; i < timesToRun; ++i)
            {
                status = tst.TestFunction();
                if (!SUCCEEDED(status))
                {
                    LOG_ERROR("Testcase [%s] failed on iteration %d/%d\n",
                        tstToRun, i + 1, timesToRun);
                }
                else
                {
                    LOG("Iteration %d/%d of testcase [%s] succeeded!\n",
                        i + 1, timesToRun, tstToRun);
                }
            }
        }
    }

    return result;
}