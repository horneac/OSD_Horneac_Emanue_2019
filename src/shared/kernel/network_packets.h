#pragma once

#pragma pack(push,1)

#pragma warning(push)

// warning C4201: nonstandard extension used: nameless struct/union
#pragma warning(disable:4201)

// warning C4200: nonstandard extension used: zero-sized array in struct/union
#pragma warning(disable:4200)

// warning C4214: nonstandard extension used: bit field types other than int
#pragma warning(disable:4214)

//////////////////////////////////////////////////////////////////////////////////////
//////                               Address Types                             ///////
//////////////////////////////////////////////////////////////////////////////////////
#define MAC_ADDRESS_SIZE            6
#define IP4_ADDRESS_SIZE            4
#define IP6_ADDRESS_SIZE            16

typedef struct _MAC_ADDRESS
{
    BYTE                    Value[MAC_ADDRESS_SIZE];
} MAC_ADDRESS, *PMAC_ADDRESS;
STATIC_ASSERT(sizeof(MAC_ADDRESS) == MAC_ADDRESS_SIZE );

extern const MAC_ADDRESS MAC_BROADCAST;

typedef union _IP4_ADDRESS
{
    struct
    {
        BYTE                ByteAddress[IP4_ADDRESS_SIZE];
    };
    DWORD                   DwordAddress;
} IP4_ADDRESS, *PIP4_ADDRESS;
STATIC_ASSERT(sizeof(IP4_ADDRESS) == IP4_ADDRESS_SIZE);

typedef struct _IP6_ADDRESS
{
    BYTE            Address[IP6_ADDRESS_SIZE];
} IP6_ADDRESS, *PIP6_ADDRESS;

//////////////////////////////////////////////////////////////////////////////////////
//////                             Layer 2 Structures                          ///////
//////////////////////////////////////////////////////////////////////////////////////

#define ETHERNET_FRAME_SIZE                 14
#define ARP_PACKET_SIZE                     28
#define IEEE_802_3_MINIMUM_FRAME_SIZE       64

typedef WORD        ETHERNET_FRAME_TYPE;

#define ETHERNET_FRAME_TYPE_IP4         __pragma(warning(suppress: 4310)) ((WORD)0x0800ui16)
#define ETHERNET_FRAME_TYPE_ARP         __pragma(warning(suppress: 4310)) ((WORD)0x0806ui16)
#define ETHERNET_FRAME_TYPE_RARP        __pragma(warning(suppress: 4310)) ((WORD)0x8035ui16)
#define ETHERNET_FRAME_TYPE_VLAN        __pragma(warning(suppress: 4310)) ((WORD)0x8100ui16)
#define ETHERNET_FRAME_TYPE_IP6         __pragma(warning(suppress: 4310)) ((WORD)0x86DDui16)
#define ETHERNET_FRAME_TYPE_LLDP        __pragma(warning(suppress: 4310)) ((WORD)0x88CCui16)

typedef struct _ETHERNET_FRAME
{
    MAC_ADDRESS             Destination;
    MAC_ADDRESS             Source;

    ETHERNET_FRAME_TYPE     Type;
    BYTE                    Data[0];
} ETHERNET_FRAME, *PETHERNET_FRAME;
STATIC_ASSERT(sizeof(ETHERNET_FRAME) == ETHERNET_FRAME_SIZE);

typedef WORD        HARDWARE_TYPE;

#define HARDWARE_TYPE_ETHERNET          1

typedef WORD        ARP_OPERATION;

#define ARP_OPERATION_REQUEST           1
#define ARP_OPERATION_REPLY             2

typedef struct _ARP_PACKET
{
    HARDWARE_TYPE           HardwareType;

    ETHERNET_FRAME_TYPE     ProtocolType;

    BYTE                    HardwareAddressLength;
    BYTE                    ProtocolAddressLength;

    ARP_OPERATION           Operation;

    MAC_ADDRESS             SenderHardwareAddress;
    IP4_ADDRESS             SenderProtocolAddress;

    MAC_ADDRESS             TargetHardwareAddress;
    IP4_ADDRESS             TargetProtocolAddress;
} ARP_PACKET, *PARP_PACKET;
STATIC_ASSERT(sizeof(ARP_PACKET) == ARP_PACKET_SIZE);

//////////////////////////////////////////////////////////////////////////////////////
//////                             Layer 3 Structures                          ///////
//////////////////////////////////////////////////////////////////////////////////////

#define IP4_PACKET_SIZE             20
#define IP6_PACKET_SIZE             40

typedef BYTE        IP_PROTOCOL;

#define IP_PROTOCOL_ICMP                1
#define IP_PROTOCOL_IGMP                2
#define IP_PROTOCOL_TCP                 6
#define IP_PROTOCOL_UDP                 17

typedef struct _IP4_PACKET
{
    // size in DWORDs
    BYTE            InternetHeaderLength    : 4;

    BYTE            Version                 : 4;

    BYTE            QoS;

    WORD            Length;

    WORD            Id;

    WORD            __Reserved0;

    BYTE            TimeToLive;

    IP_PROTOCOL     Protocol;

    WORD            Checksum;

    IP4_ADDRESS     Source;

    IP4_ADDRESS     Destination;
} IP4_PACKET, *PIP4_PACKET;
STATIC_ASSERT(sizeof(IP4_PACKET) == IP4_PACKET_SIZE);

typedef struct _IP6_PACKET
{
    DWORD           Version                 : 4;
    DWORD           TrafficClass            : 8;
    DWORD           FlowLabel               : 20;

    WORD            PayloadLength;
    BYTE            NextHeader;
    BYTE            HopLimit;
    IP6_ADDRESS     Source;
    IP6_ADDRESS     Destination;
} IP6_PACKET, *PIP6_PACKET;
STATIC_ASSERT(sizeof(IP6_PACKET) == IP6_PACKET_SIZE);

//////////////////////////////////////////////////////////////////////////////////////
//////                             Layer 4 Structures                          ///////
//////////////////////////////////////////////////////////////////////////////////////

#define UDP_DATAGRAM_SIZE               8
#define TCP_SEGMENT_SIZE                20

typedef WORD    PORT_NUMBER;

typedef struct _UDP_DATAGRAM
{
    PORT_NUMBER         Source;
    PORT_NUMBER         Destination;
    WORD                Length;
    WORD                Checksum;
} UDP_DATAGRAM, *PUDP_DATAGRAM;
STATIC_ASSERT(sizeof(UDP_DATAGRAM) == UDP_DATAGRAM_SIZE);

typedef struct _TCP_SEGMENT
{
    PORT_NUMBER         Source;
    PORT_NUMBER         Destination;
    DWORD               SequenceNumber;
    DWORD               AckNumber;
    struct
    {
        // 1st byte
        BYTE            CWR             : 1;
        BYTE            ECE             : 1;
        BYTE            URG             : 1;
        BYTE            ACK             : 1;
        BYTE            PSH             : 1;
        BYTE            RST             : 1;
        BYTE            SYN             : 1;
        BYTE            FIN             : 1;

        // 2nd byte
        BYTE            DataOffset      : 4;
        BYTE            __Reserved0     : 3;
        BYTE            NS              : 1;
    };
    WORD                WindowSize;
    WORD                Checksum;
    WORD                UrgentPointer;
} TCP_SEGMENT, *PTCP_SEGMENT;
STATIC_ASSERT(sizeof(TCP_SEGMENT) == TCP_SEGMENT_SIZE);

#pragma warning(pop)
#pragma pack(pop)
