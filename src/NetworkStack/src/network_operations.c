#include "network_stack_base.h"
#include "network_internal.h"
#include "network_operations.h"

STATUS
NetOpGetPhysicalAddress(
    IN          PDEVICE_OBJECT          DeviceObject,
    OUT         PMAC_ADDRESS            Address
    )
{
    STATUS status;
    PIRP pIrp;
    NET_GET_SET_PHYSICAL_ADDRESS physAddr;

    LOG_FUNC_START;

    ASSERT( NULL != DeviceObject );
    ASSERT( NULL != Address );

    status = STATUS_SUCCESS;
    pIrp = NULL;
    memzero(&physAddr, sizeof(NET_GET_SET_PHYSICAL_ADDRESS));

    __try
    {
        pIrp = IoBuildDeviceIoControlRequest(IOCTL_NET_GET_PHYSICAL_ADDRESS,
                                             DeviceObject,
                                             NULL,
                                             0,
                                             &physAddr,
                                             sizeof(NET_GET_SET_PHYSICAL_ADDRESS)
        );
        ASSERT(NULL != pIrp);

        status = IoCallDriver(DeviceObject,
                              pIrp
        );
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("IoCallDriver", status);
            __leave;
        }

        status = pIrp->IoStatus.Status;
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("IoCallDriver", status);
            __leave;
        }

        memcpy(Address, &physAddr.Address, sizeof(MAC_ADDRESS));
    }
    __finally
    {
        if (NULL != pIrp)
        {
            IoFreeIrp(pIrp);
            pIrp = NULL;
        }

        LOG_FUNC_END;
    }

    return status;
}

PTR_SUCCESS
PNETWORK_DEVICE
NetOpGetDeviceById(
    IN          DEVICE_ID               Device
    )
{
    INTR_STATE intrState;
    PLIST_ENTRY pEntry;
    PNETWORK_DEVICE pResult;

    pEntry = NULL;
    pResult = NULL;

    RwSpinlockAcquireShared(&m_netStackData.DeviceLock, &intrState);

    for (pEntry = m_netStackData.NetworkDeviceList.Flink;
        pEntry != &m_netStackData.NetworkDeviceList;
        pEntry = pEntry->Flink)
    {
        PNETWORK_DEVICE pNetDevice = CONTAINING_RECORD(pEntry, NETWORK_DEVICE, NextDevice );

        if (pNetDevice->Info.DeviceId == Device)
        {
            pResult = pNetDevice;
            break;
        }
    }

    RwSpinlockReleaseShared(&m_netStackData.DeviceLock, intrState );

    return pResult;
}

STATUS
NetOpGetDeviceStatus(
    IN          PDEVICE_OBJECT          DeviceObject,
    OUT         PNETWORK_DEVICE_STATUS  DeviceStatus
    )
{
    STATUS status;
    PIRP pIrp;
    NET_GET_SET_DEVICE_STATUS devStatus;

    LOG_FUNC_START;

    ASSERT(NULL != DeviceObject);
    ASSERT(NULL != DeviceStatus);

    status = STATUS_SUCCESS;
    pIrp = NULL;
    memzero(&devStatus, sizeof(NET_GET_SET_DEVICE_STATUS));

    __try
    {
        pIrp = IoBuildDeviceIoControlRequest(IOCTL_NET_GET_DEVICE_STATUS,
                                             DeviceObject,
                                             NULL,
                                             0,
                                             &devStatus,
                                             sizeof(NET_GET_SET_DEVICE_STATUS)
        );
        ASSERT(NULL != pIrp);

        status = IoCallDriver(DeviceObject,
                              pIrp
        );
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("IoCallDriver", status);
            __leave;
        }

        status = pIrp->IoStatus.Status;
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("IoCallDriver", status);
            __leave;
        }

        memcpy(DeviceStatus, &devStatus.DeviceStatus, sizeof(NET_GET_SET_DEVICE_STATUS));
    }
    __finally
    {
        if (NULL != pIrp)
        {
            IoFreeIrp(pIrp);
            pIrp = NULL;
        }

        LOG_FUNC_END;
    }

    return status;
}

STATUS
NetOpSetDeviceStatus(
    IN          PDEVICE_OBJECT          DeviceObject,
    IN          PNETWORK_DEVICE_STATUS  DeviceStatus
    )
{
    STATUS status;
    PIRP pIrp;
    NET_GET_SET_DEVICE_STATUS devStatus;

    LOG_FUNC_START;

    ASSERT(NULL != DeviceObject);
    ASSERT(NULL != DeviceStatus);

    status = STATUS_SUCCESS;
    pIrp = NULL;
    memzero(&devStatus, sizeof(NET_GET_SET_DEVICE_STATUS));
    memcpy(&devStatus.DeviceStatus, DeviceStatus, sizeof(NETWORK_DEVICE_STATUS));

    __try
    {
        pIrp = IoBuildDeviceIoControlRequest(IOCTL_NET_SET_DEVICE_STATUS,
                                             DeviceObject,
                                             &devStatus,
                                             sizeof(NET_GET_SET_DEVICE_STATUS),
                                             NULL,
                                             0
        );
        ASSERT(NULL != pIrp);

        status = IoCallDriver(DeviceObject,
                              pIrp
        );
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("IoCallDriver", status);
            __leave;
        }

        status = pIrp->IoStatus.Status;
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("IoCallDriver", status);
            __leave;
        }
    }
    __finally
    {
        if (NULL != pIrp)
        {
            IoFreeIrp(pIrp);
            pIrp = NULL;
        }

        LOG_FUNC_END;
    }

    return status;
}

STATUS
NetOpGetLinkStatus(
    IN          PDEVICE_OBJECT          DeviceObject,
    OUT         BOOLEAN*                LinkStatus
    )
{
    STATUS status;
    PIRP pIrp;
    NET_GET_LINK_STATUS linkStatus;

    LOG_FUNC_START;

    ASSERT(NULL != DeviceObject);
    ASSERT(NULL != LinkStatus);

    status = STATUS_SUCCESS;
    pIrp = NULL;
    memzero(&linkStatus, sizeof(NET_GET_LINK_STATUS));

    __try
    {
        pIrp = IoBuildDeviceIoControlRequest(IOCTL_NET_GET_LINK_STATUS,
                                             DeviceObject,
                                             NULL,
                                             0,
                                             &linkStatus,
                                             sizeof(NET_GET_LINK_STATUS)
        );
        ASSERT(NULL != pIrp);

        status = IoCallDriver(DeviceObject,
                              pIrp
        );
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("IoCallDriver", status);
            __leave;
        }

        status = pIrp->IoStatus.Status;
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("IoCallDriver", status);
            __leave;
        }

        *LinkStatus = linkStatus.LinkUp;
    }
    __finally
    {
        if (NULL != pIrp)
        {
            IoFreeIrp(pIrp);
            pIrp = NULL;
        }

        LOG_FUNC_END;
    }

    return status;
}