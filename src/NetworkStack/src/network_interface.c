#include "network_stack_base.h"
#include "network.h"
#include "network_internal.h"
#include "network_operations.h"

STATUS
NetSendFrame(
    IN                      BOOLEAN         SendOnAllInterfaces,
    _When_(SendOnAllInterfaces, _Reserved_)
    _When_(!SendOnAllInterfaces, IN)
                            DEVICE_ID       DeviceId,
    IN_READS_BYTES(Size)    PVOID           Buffer,
    IN                      DWORD           Size,
    IN                      MAC_ADDRESS     DestinationAddress
    )
{
    STATUS status;
    PNETWORK_DEVICE pNetDevice;
    PIRP pIrp;
    PETHERNET_FRAME pFrame;

    if (NULL == Buffer)
    {
        return STATUS_INVALID_PARAMETER3;
    }

    if (0 == Size)
    {
        return STATUS_INVALID_PARAMETER4;
    }

    if (SendOnAllInterfaces)
    {
        LOG_ERROR("SendOnAllInterfaces not implemented!\n");
        return STATUS_NOT_IMPLEMENTED;
    }

    status = STATUS_NOT_IMPLEMENTED;
    pIrp = NULL;
    pNetDevice = NULL;
    pFrame = Buffer;

    pNetDevice = NetOpGetDeviceById(DeviceId);
    if (NULL == pNetDevice)
    {
        return STATUS_DEVICE_DOES_NOT_EXIST;
    }

    memcpy(&pFrame->Source, &pNetDevice->Info.PhysicalAddress, sizeof(MAC_ADDRESS));
    memcpy(&pFrame->Destination, (PMAC_ADDRESS) &DestinationAddress, sizeof(MAC_ADDRESS) );

    __try
    {
        pIrp = IoBuildDeviceIoControlRequest(IOCTL_NET_SEND_FRAME,
                                             pNetDevice->PhysicalDevice,
                                             Buffer,
                                             Size,
                                             NULL,
                                             0
        );
        ASSERT(NULL != pIrp);

        status = IoCallDriver(pNetDevice->PhysicalDevice,
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
    }

    return status;
}

STATUS
NetReceiveFrame(
    IN                      DEVICE_ID       DeviceId,
    OUT_WRITES_BYTES(Size)  PVOID           Buffer,
    IN                      DWORD           Size,
    OUT                     DWORD*          BytesWritten
    )
{
    STATUS status;
    PIRP pIrp;
    PETHERNET_FRAME pFrame;
    PNETWORK_DEVICE pNetDevice;

    if (NULL == Buffer)
    {
        return STATUS_INVALID_PARAMETER2;
    }

    if (0 == Size)
    {
        return STATUS_INVALID_PARAMETER3;
    }

    if (NULL == BytesWritten)
    {
        return STATUS_INVALID_PARAMETER4;
    }

    status = STATUS_NOT_IMPLEMENTED;
    pIrp = NULL;
    pFrame = NULL;

    pNetDevice = NetOpGetDeviceById(DeviceId);
    if (NULL == pNetDevice)
    {
        return STATUS_DEVICE_DOES_NOT_EXIST;
    }

    __try
    {
        pIrp = IoBuildDeviceIoControlRequest(IOCTL_NET_RECEIVE_FRAME,
                                             pNetDevice->PhysicalDevice,
                                             NULL,
                                             0,
                                             Buffer,
                                             Size
        );
        ASSERT(NULL != pIrp);

        status = IoCallDriver(pNetDevice->PhysicalDevice,
                              pIrp
        );
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("IoCallDriver", status);
            __leave;
        }

        status = pIrp->IoStatus.Status;
        ASSERT(pIrp->IoStatus.Information <= MAX_DWORD);
        *BytesWritten = (DWORD)pIrp->IoStatus.Information;
        if (!SUCCEEDED(status))
        {
            if (STATUS_BUFFER_TOO_SMALL != status)
            {
                LOG_FUNC_ERROR("IoCallDriver", status);
                __leave;
            }
            LOG_WARNING("IoCallDriver failed before buffer of size %u was too small, %u bytes are required\n", Size, *BytesWritten);
        }
    }
    __finally
    {
        if (NULL != pIrp)
        {
            IoFreeIrp(pIrp);
            pIrp = NULL;
        }
    }

    return status;
}

STATUS
NetGetNetworkDevices(
    OUT_WRITES_OPT(*NumberOfDevices)
            PNETWORK_DEVICE_INFO            DeviceObjects,
    _When_(NULL == DeviceObjects, OUT)
    _When_(NULL != DeviceObjects, INOUT)
            DWORD*                          NumberOfDevices
    )
{
    STATUS status;
    INTR_STATE intrState;
    DWORD i;

    if (NULL == NumberOfDevices)
    {
        return STATUS_INVALID_PARAMETER2;
    }

    status = STATUS_SUCCESS;
    i = 0;

    RwSpinlockAcquireShared(&m_netStackData.DeviceLock, &intrState);
    
    __try
    {
        if (NULL != DeviceObjects)
        {
            if (*NumberOfDevices < m_netStackData.NumberOfDevices)
            {
                status = STATUS_BUFFER_TOO_SMALL;
            }
            else
            {
                PLIST_ENTRY pEntry;

                for (pEntry = m_netStackData.NetworkDeviceList.Flink;
                     pEntry != &m_netStackData.NetworkDeviceList;
                     pEntry = pEntry->Flink)
                {
                    PNETWORK_DEVICE pNetDevice;

                    pNetDevice = CONTAINING_RECORD(pEntry, NETWORK_DEVICE, NextDevice);

                    status = NetOpGetDeviceStatus(pNetDevice->PhysicalDevice,
                                                  &pNetDevice->Info.DeviceStatus
                    );
                    if (!SUCCEEDED(status))
                    {
                        LOG_FUNC_ERROR("NetOpGetDeviceStatus", status);
                        __leave;
                    }

                    status = NetOpGetLinkStatus(pNetDevice->PhysicalDevice,
                                                &pNetDevice->Info.LinkStatus
                    );
                    if (!SUCCEEDED(status))
                    {
                        LOG_FUNC_ERROR("NetOpGetDeviceStatus", status);
                        __leave;
                    }

                    memcpy(&DeviceObjects[i], &pNetDevice->Info, sizeof(NETWORK_DEVICE_INFO));
                    ++i;
                }
            }
        }
        *NumberOfDevices = m_netStackData.NumberOfDevices;
    }
    __finally
    {
        RwSpinlockReleaseShared(&m_netStackData.DeviceLock, intrState);
    }

    return status;
}

STATUS
NetChangeDeviceStatus(
    IN              DEVICE_ID                       DeviceId,
    IN              PNETWORK_DEVICE_STATUS          DeviceStatus
    )
{
    STATUS status;
    PNETWORK_DEVICE pNetDevice;

    if (NULL == DeviceStatus)
    {
        return STATUS_INVALID_PARAMETER2;
    }

    pNetDevice = NetOpGetDeviceById(DeviceId);
    if (NULL == pNetDevice)
    {
        return STATUS_DEVICE_DOES_NOT_EXIST;
    }

    status = NetOpSetDeviceStatus(pNetDevice->PhysicalDevice,
                                  DeviceStatus
                                  );
    if (!SUCCEEDED(status))
    {
        LOG_FUNC_ERROR("NetOpSetDeviceStatus", status );
        return status;
    }

    return status;
}