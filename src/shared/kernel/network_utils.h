#pragma once

#include "network.h"

#define TEXT_MAC_ADDRESS_CHARS_REQUIRED             18
#define TEXT_IP4_ADDRESS_CHARS_REQUIRED             16
#define TEXT_IP6_ADDRESS_CHARS_REQUIRED             40

char*
NetUtilMacAddressToText(
    IN                                                          MAC_ADDRESS         Address,
    OUT_WRITES_BYTES_ALL(TEXT_MAC_ADDRESS_CHARS_REQUIRED)       char*               Buffer
    );

char*
NetUtilIp4AddressToText(
    IN                                                          IP4_ADDRESS         Address,
    OUT_WRITES_BYTES_ALL(TEXT_IP4_ADDRESS_CHARS_REQUIRED)       char*               Buffer
    );