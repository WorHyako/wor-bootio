// SPDX-License-Identifier: MIT
// Copyright (c) 2025 WorHyako

#pragma once

#include <stdint.h>

/**
 * \enum BootIoError
 * \brief
 */
enum BootIoError
#if __STDC_VERSION__ == 202311L
    : int8_t
#endif
{
    /**
     * \brief Success.
     */
    BootIoError_Success = 0,
    /**
     * \brief Invalid parameter.
     */
    BootIoError_InvalidParam = -98,
    /**
     * \brief Other error.
     */
    BootIoError_Other = -99
};

/**
 * \enum BootIoError_File
 * \brief
 */
enum BootIoError_File
#if __STDC_VERSION__ == 202311L
    : int8_t
#endif
{
    BootIoError_File_UcdSignature = -100,
    BootIoError_File_Crc = -101,
    BootIoError_File_SuffixLength = -102,
    BootIoError_File_PrefixType = -103
};

/**
 * \enum BootIoError_Configuration
 * \brief
 */
enum BootIoError_Configuration
#if __STDC_VERSION__ == 202311L
    : int16_t
#endif
{
    BootIoError_Configuration_DescriptorLength = -200,
    BootIoError_Configuration_Crc = -201,
    BootIoError_Configuration_SuffixLength = -202,
    BootIoError_Configuration_PrefixType = -203,
    BootIoError_Configuration_DescriptorString = -204
};

/**
 * \enum BootIoError_Device
 * \brief
 */
enum BootIoError_Device
#if __STDC_VERSION__ == 202311L
    : int16_t
#endif
{
    BootIoError_Device_Success = -300,
    BootIoError_Device_Open = -301,
    BootIoError_Device_ClaimInterface = -302,
    BootIoError_Device_Find = -303
};

/**
 * \enum BootIoError_Transfer
 * \brief
 */
enum BootIoError_Transfer
#if __STDC_VERSION__ == 202311L
    : int16_t
#endif
{
    /**
     * \brief Input/output error.
     */
    BootIoError_Transfer_Io = -1,
    /**
     * \brief Invalid parameter.
     */
    BootIoError_Transfer_InvalidParam = -2,
    /**
     * \brief Access denied (insufficient permissions).
     */
    BootIoError_Transfer_Access = -3,
    /**
     * \brief No such device (it may have been disconnected).
     */
    BootIoError_Transfer_NoDevice = -4,
    /**
     * \brief Entity not found.
     */
    BootIoError_Transfer_NotFound = -5,
    /**
     * \brief Resource busy.
     */
    BootIoError_Transfer_Busy = -6,
    /**
     * \brief Operation timed out.
     */
    BootIoError_Transfer_Timeout = -7,
    /**
     * \brief Overflow.
     */
    BootIoError_Transfer_Overflow = -8,
    /**
     * \brief Pipe error.
     */
    BootIoError_Transfer_Pipe = -9,
    /**
     * \brief System call interrupted (perhaps due to signal).
     */
    BootIoError_Transfer_Interrupted = -10,
    /**
     * \brief Insufficient memory.
     */
    BootIoError_Transfer_Memory = -11,
    /**
     * \brief Operation isn't supported or unimplemented on this platform.
     */
    BootIoError_Transfer_NotSupported = -12
};
