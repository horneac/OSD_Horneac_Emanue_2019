#pragma once

#include "list.h"
#include "pci_system.h"
#include "filesystem.h"
#include "network.h"
#include "cpu_structures.h"
#include "io_structures.h"
#include "mem_structures.h"

PTR_SUCCESS
PDEVICE_OBJECT
IoCreateDevice(
    INOUT   PDRIVER_OBJECT  DriverObject,
    IN      DWORD           DeviceExtensionSize,
    IN      DEVICE_TYPE     DeviceType
    );

PVOID
IoGetDeviceExtension(
    IN      PDEVICE_OBJECT      Device
    );

void
IoDeleteDevice(
    INOUT   PDEVICE_OBJECT      Device
    );

PTR_SUCCESS
PDRIVER_OBJECT
IoCreateDriver(
    IN_Z        char*                   DriverName,
    IN          PFUNC_DriverEntry       DriverEntry
    );

PVOID
IoGetDriverExtension(
    IN          PDEVICE_OBJECT  Device
    );

void
IoAttachDevice(
    INOUT   PDEVICE_OBJECT  SourceDevice,
    IN      PDEVICE_OBJECT  TargetDevice
    );

PTR_SUCCESS
PIRP
IoAllocateIrp(
    IN      BYTE            StackSize
    );

void
IoFreeIrp(
    IN      PIRP            Irp
    );

PTR_SUCCESS
PIO_STACK_LOCATION
IoGetCurrentIrpStackLocation(
    IN      PIRP            Irp
    );

PTR_SUCCESS
PIO_STACK_LOCATION
IoGetNextIrpStackLocation(
    IN      PIRP            Irp
    );

void
IoCopyCurrentStackLocationToNext(
    INOUT   PIRP            Irp
    );

STATUS
IoCallDriver(
    IN      PDEVICE_OBJECT  Device,
    INOUT   PIRP            Irp
    );

void
IoCompleteIrp(
    INOUT   PIRP            Irp
    );

#define IoIsIrpComplete(irp)        (TRUE==((irp)->Flags.Completed))

STATUS
IoGetPciDevicesMatchingSpecification(
    IN          PCI_SPEC        Specification,
    _When_(*NumberOfDevices > 0, OUT_PTR)
    _When_(*NumberOfDevices == 0, OUT_PTR_MAYBE_NULL)
                PPCI_DEVICE_DESCRIPTION**    PciDevices,
    OUT         DWORD*           NumberOfDevices
    );

STATUS
IoGetPciDevicesMatchingLocation(
    IN          PCI_SPEC_LOCATION           Specification,
    _When_(*NumberOfDevices > 0, OUT_PTR)
    _When_(*NumberOfDevices == 0, OUT_PTR_MAYBE_NULL)
                PPCI_DEVICE_DESCRIPTION**   PciDevices,
    OUT         DWORD*                      NumberOfDevices
    );

STATUS
IoGetPciSecondaryBusForBridge(
    IN          PCI_DEVICE_LOCATION         DeviceLocation,
    OUT         BYTE*                       Bus
    );

STATUS
IoGetDevicesByType(
    IN                      DEVICE_TYPE         DeviceType,
    _When_(*NumberOfDevices>0,OUT_PTR)
    _When_(*NumberOfDevices==0, OUT_PTR_MAYBE_NULL)
                            PDEVICE_OBJECT**    DeviceObjects,
    OUT                     DWORD*              NumberOfDevices
    );

void
IoFreeTemporaryData(
    IN          PVOID               Data
    );

PTR_SUCCESS
PIRP
IoBuildDeviceIoControlRequest(
    IN          DWORD               IoControlCode,
    IN          PDEVICE_OBJECT      DeviceObject,
    IN_OPT      PVOID               InputBuffer,
    IN          DWORD               InputBufferLength,
    OUT_OPT     PVOID               OutputBuffer,
    IN          DWORD               OutputBufferLength
    );

STATUS
IoReadDeviceEx(
    IN                          PDEVICE_OBJECT          DeviceObject,
    OUT_WRITES_BYTES(*Length)   PVOID                   Buffer,
    INOUT                       QWORD*                  Length,
    IN                          QWORD                   Offset,
    IN                          BOOLEAN                 Asynchronous
    );

#define IoReadDevice(Dev,Buf,Len,Off)                  IoReadDeviceEx((Dev),(Buf),(Len),(Off),FALSE)

STATUS
IoWriteDeviceEx(
    IN                          PDEVICE_OBJECT          DeviceObject,
    IN_READS_BYTES(*Length)     PVOID                   Buffer,
    INOUT                       QWORD*                  Length,
    IN                          QWORD                   Offset,
    IN                          BOOLEAN                 Asynchronous
    );

#define IoWriteDevice(Dev,Buf,Len,Off)                  IoWriteDeviceEx((Dev),(Buf),(Len),(Off),FALSE)

STATUS
IoAllocateMdl(
    IN          PVOID           VirtualAddress,
    IN          DWORD           Length,
    IN_OPT      PIRP            Irp,
    OUT_PTR     struct _MDL**           Mdl
    );

void
IoFreeMdl(
    INOUT       struct _MDL*            Mdl
    );

SIZE_SUCCESS
DWORD
IoMdlGetNumberOfPairs(
    IN          PMDL            Mdl
    );

PTR_SUCCESS
PMDL_TRANSLATION_PAIR
IoMdlGetTranslationPair(
    IN          PMDL            Mdl,
    IN          DWORD           Index
    );

STATUS
IoRegisterInterruptEx(
    IN          PIO_INTERRUPT           Interrupt,
    IN_OPT      PDEVICE_OBJECT          DeviceObject,
    OUT_OPT     PBYTE                   Vector
    );

#define IoRegisterInterrupt(Int,Dev)    IoRegisterInterruptEx((Int),(Dev),NULL)

PTR_SUCCESS
PVOID
IoMapMemory(
    IN      PHYSICAL_ADDRESS        PhysicalAddress,
    IN      DWORD                   Size,
    IN      PAGE_RIGHTS             PageRights
    );

void
IoUnmapMemory(
    IN      PVOID                   VirtualAddress,
    IN      DWORD                   Size
    );

PTR_SUCCESS
PHYSICAL_ADDRESS
IoGetPhysicalAddress(
    IN      PVOID                   VirtualAddress
    );

#define IoAllocateContinuousMemory(Size)    IoAllocateContinuousMemoryEx((Size),FALSE)

PTR_SUCCESS
PVOID
IoAllocateContinuousMemoryEx(
    IN      DWORD                   AllocationSize,
    IN      BOOLEAN                 Uncacheable
    );

void
IoFreeContinuousMemory(
    IN      PVOID                   VirtualAddress
    );

DATETIME
IoGetCurrentDateTime(
    void
    );

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////                        FILE OPERATIONS                                        /////////
/////////////////////////////////////////////////////////////////////////////////////////////////

STATUS
IoCreateFile(
    OUT_PTR     PFILE_OBJECT*           Handle,
    IN_Z        char*                   FileName,
    IN          BOOLEAN                 Directory,
    IN          BOOLEAN                 Create,
    IN          BOOLEAN                 Asynchronous
    );

STATUS
IoCloseFile(
    IN          PFILE_OBJECT            FileHandle
    );

STATUS
IoReadFile(
    IN          PFILE_OBJECT            FileHandle,
    IN          QWORD                   BytesToRead,
    IN_OPT      QWORD*                  FileOffset,
    OUT_WRITES_BYTES(BytesToRead)
                PVOID                   Buffer,
    OUT         QWORD*                  BytesRead
    );

STATUS
IoWriteFile(
    IN          PFILE_OBJECT            FileHandle,
    IN          QWORD                   BytesToWrite,
    IN_OPT      QWORD*                  FileOffset,
    IN_READS_BYTES(BytesToWrite)
                PVOID                   Buffer,
    OUT         QWORD*                  BytesWritten
    );

STATUS
IoGetFileSize(
    IN          PFILE_OBJECT            FileHandle,
    OUT         QWORD*                  FileSize
    );

STATUS
IoQueryInformationFile(
    IN          PFILE_OBJECT            FileHandle,
    OUT         PFILE_INFORMATION       FileInformation
    );

STATUS
IoQueryDirectoryFile(
    IN          PFILE_OBJECT                    FileHandle,
    IN          DWORD                           BufferSize,
    _When_(0==BufferSize,OUT_OPT)
    _When_(0!=BufferSize,OUT)
                PFILE_DIRECTORY_INFORMATION     DirectoryInformation,
    OUT         DWORD*                          SizeRequired
    );