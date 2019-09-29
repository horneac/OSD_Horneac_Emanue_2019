#pragma once

typedef struct _FILE_OBJECT_FLAGS
{
    // if set => IoManager will update file offset
    DWORD                   Asynchronous     :    1;

    // if set => file will be created
    // else   => file will be opened
    DWORD                   Create          :    1;

    // if set => file is a directory
    // else   => file is a normal file
    DWORD                   DirectoryFile   :    1;

    DWORD                   Reserved        :   29;
} FILE_OBJECT_FLAGS, *PFILE_OBJECT_FLAGS;

typedef struct _FILE_OBJECT
{
    // file system to which it belongs
    struct _DEVICE_OBJECT*  FileSystemDevice;
    FILE_OBJECT_FLAGS       Flags;

    /// must not be touched, used only by the
    /// filesystem to keep internal structure
    PVOID                   FsContext2;

    char*                   FileName;
    struct _FILE_OBJECT*    RelatedFileObject;

    QWORD                   CurrentByteOffset;

    // real file size
    QWORD                   FileSize;
} FILE_OBJECT, *PFILE_OBJECT;

typedef struct _VPB_FLAGS
{
    DWORD               Mounted     :    1;
    DWORD               SwapSpace   :    1;
    DWORD               Reserved    :   30;
} VPB_FLAGS, *PVPB_FLAGS;

// Provides an association between a logical volume
// and the corresponding file system
typedef struct _VPB
{
    struct _DEVICE_OBJECT*  FilesystemDevice;
    struct _DEVICE_OBJECT*  VolumeDevice;
    VPB_FLAGS               Flags;

    char                    VolumeLetter;
    LIST_ENTRY              NextVpb;
} VPB, *PVPB;