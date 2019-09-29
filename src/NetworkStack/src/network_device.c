#include "network_stack_base.h"
#include "network_internal.h"
#include "network_operations.h"

_No_competing_thread_
void
NetworkDevicePreinit(
    OUT     PNETWORK_DEVICE     Device,
    IN      PDEVICE_OBJECT      DeviceObject,
    IN      DEVICE_ID           DeviceId
    )
{
    ASSERT(NULL != Device);
    ASSERT(NULL != DeviceObject);

    Device->PhysicalDevice = DeviceObject;
    Device->Info.DeviceId = DeviceId;
}

_No_competing_thread_
STATUS
NetworkDeviceInitialize(
    INOUT    PNETWORK_DEVICE    Device
    )
{
    STATUS status;

    ASSERT(NULL != Device);

    LOG_FUNC_START;

    status = STATUS_SUCCESS;

    status = NetOpGetPhysicalAddress(Device->PhysicalDevice,
                                     &Device->Info.PhysicalAddress
                                     );
    if (!SUCCEEDED(status))
    {
        LOG_FUNC_ERROR("NetOpGetPhysicalAddress", status);
        return status;
    }

    status = NetOpGetDeviceStatus(Device->PhysicalDevice,
                                  &Device->Info.DeviceStatus
                                  );
    if (!SUCCEEDED(status))
    {
        LOG_FUNC_ERROR("NetOpGetDeviceStatus", status);
        return status;
    }

    LOG_FUNC_END;

    return status;
}