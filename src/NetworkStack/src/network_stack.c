#include "network_stack_base.h"
#include "network_stack.h"
#include "network_internal.h"
#include "network_operations.h"

NETWORK_STACK_DATA m_netStackData;

const MAC_ADDRESS MAC_BROADCAST = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

_No_competing_thread_
__forceinline
static
DEVICE_ID
_NetworkStackGetNextDeviceId(
    void
    )
{
    static DEVICE_ID __nextDeviceId = 0;

    return __nextDeviceId++;
}

_No_competing_thread_
void
NetworkStackPreinit(
    void
    )
{
    memzero(&m_netStackData, sizeof(NETWORK_STACK_DATA));

    InitializeListHead(&m_netStackData.NetworkDeviceList);
    RwSpinlockInit(&m_netStackData.DeviceLock);

    m_netStackData.NetworkingEnabled = TRUE;
}

_No_competing_thread_
STATUS
NetworkStackInit(
    IN  BOOLEAN     EnableNetworking
    )
{
    STATUS status;
    PDEVICE_OBJECT* pNetworkDevices;
    DWORD numberOfDevices;
    DWORD i;
    PNETWORK_DEVICE pNetDevice;

    LOG_FUNC_START;

    status = STATUS_SUCCESS;
    pNetDevice = NULL;
    pNetworkDevices = NULL;
    numberOfDevices = 0;

    status = IoGetDevicesByType(DeviceTypePhysicalNetcard, 
                                &pNetworkDevices, 
                                &numberOfDevices
                                );
    if (!SUCCEEDED(status))
    {
        LOG_FUNC_ERROR("IoGetDeviceByType", status);
        return status;
    }

    __try
    {
        for (i = 0; i < numberOfDevices; ++i)
        {
            if (NULL == pNetDevice)
            {
                pNetDevice = ExAllocatePoolWithTag(PoolAllocateZeroMemory, sizeof(NETWORK_DEVICE), HEAP_NET_TAG, 0);
                if (NULL == pNetDevice)
                {
                    status = STATUS_HEAP_INSUFFICIENT_RESOURCES;
                    LOG_FUNC_ERROR_ALLOC("ExAllocatePoolWithTag", sizeof(NETWORK_DEVICE));
                    __leave;
                }
            }

            ASSERT(NULL != pNetDevice);
            NetworkDevicePreinit(pNetDevice, pNetworkDevices[i], _NetworkStackGetNextDeviceId());

            status = NetworkDeviceInitialize(pNetDevice);
            if (!SUCCEEDED(status))
            {
                LOG_FUNC_ERROR("NetworkDeviceInitialize", status);
                __leave;
            }

            InsertTailList(&m_netStackData.NetworkDeviceList, &pNetDevice->NextDevice);
            m_netStackData.NumberOfDevices++;
            pNetDevice = NULL;
        }

        status = NetworkStackSetState(EnableNetworking);
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("NetworkStackSetState", status);
            __leave;
        }
    }
    __finally
    {
        if (NULL != pNetworkDevices)
        {
            IoFreeTemporaryData(pNetworkDevices);
            pNetworkDevices = NULL;
        }

        LOG_FUNC_END;
    }

    return status;
}

_No_competing_thread_
STATUS
NetworkStackSetState(
    IN  BOOLEAN     EnableNetworking
    )
{
    STATUS status;
    PLIST_ENTRY pCurEntry;
    NETWORK_DEVICE_STATUS devStatus;

    if (!(EnableNetworking ^ m_netStackData.NetworkingEnabled))
    {
        // no change in state
        return STATUS_ALREADY_INITIALIZED_HINT;
    }

    status = STATUS_SUCCESS;
    pCurEntry = NULL;
    memzero(&devStatus, sizeof(NETWORK_DEVICE_STATUS));

    devStatus.RxEnabled = EnableNetworking;
    devStatus.TxEnabled = EnableNetworking;

    LOG_FUNC_START;

    LOG("Will change networking state %u -> %u\n", 
        m_netStackData.NetworkingEnabled, EnableNetworking );

    for (pCurEntry = m_netStackData.NetworkDeviceList.Flink;
    pCurEntry != &m_netStackData.NetworkDeviceList;
        pCurEntry = pCurEntry->Flink
        )
    {
        PNETWORK_DEVICE pNetDevice = CONTAINING_RECORD(pCurEntry, NETWORK_DEVICE, NextDevice );

        status = NetOpSetDeviceStatus(pNetDevice->PhysicalDevice,
                                      &devStatus);
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("NetOpSetDeviceStatus", status );
            break;
        }
    }


    m_netStackData.NetworkingEnabled = EnableNetworking;

    LOG_FUNC_END;

    return status;
}