#pragma once

#include "freertos/FreeRTOS.h"

#include <algorithm>
#include <cstdint>

template <typename T>
uint8_t invLerpPercentage(const T& v, const T& min, const T& max) {
    const T& clampedV = std::clamp(v, min, max);
    const T& inV = clampedV - min;
    const T& range = max - min;
    return static_cast<uint8_t>((inV * 100) / range);
}

template <typename T>
T lerpPercentage(const T& min, const T& max, uint8_t p) {
    return min + (p * (max - min)) / 100;
}

inline const char* freeRTOSErrorString(const BaseType_t& err) {
    switch (err) {
        case pdFREERTOS_ERRNO_NONE: return "pdFREERTOS_ERRNO_NONE";
        case pdFREERTOS_ERRNO_ENOENT: return "pdFREERTOS_ERRNO_ENOENT";
        case pdFREERTOS_ERRNO_EINTR: return "pdFREERTOS_ERRNO_EINTR";
        case pdFREERTOS_ERRNO_EIO: return "pdFREERTOS_ERRNO_EIO";
        case pdFREERTOS_ERRNO_ENXIO: return "pdFREERTOS_ERRNO_ENXIO";
        case pdFREERTOS_ERRNO_EBADF: return "pdFREERTOS_ERRNO_EBADF";
        case pdFREERTOS_ERRNO_EAGAIN: return "pdFREERTOS_ERRNO_EAGAIN";
        case pdFREERTOS_ERRNO_ENOMEM: return "pdFREERTOS_ERRNO_ENOMEM";
        case pdFREERTOS_ERRNO_EACCES: return "pdFREERTOS_ERRNO_EACCES";
        case pdFREERTOS_ERRNO_EFAULT: return "pdFREERTOS_ERRNO_EFAULT";
        case pdFREERTOS_ERRNO_EBUSY: return "pdFREERTOS_ERRNO_EBUSY";
        case pdFREERTOS_ERRNO_EEXIST: return "pdFREERTOS_ERRNO_EEXIST";
        case pdFREERTOS_ERRNO_EXDEV: return "pdFREERTOS_ERRNO_EXDEV";
        case pdFREERTOS_ERRNO_ENODEV: return "pdFREERTOS_ERRNO_ENODEV";
        case pdFREERTOS_ERRNO_ENOTDIR: return "pdFREERTOS_ERRNO_ENOTDIR";
        case pdFREERTOS_ERRNO_EISDIR: return "pdFREERTOS_ERRNO_EISDIR";
        case pdFREERTOS_ERRNO_EINVAL: return "pdFREERTOS_ERRNO_EINVAL";
        case pdFREERTOS_ERRNO_ENOSPC: return "pdFREERTOS_ERRNO_ENOSPC";
        case pdFREERTOS_ERRNO_ESPIPE: return "pdFREERTOS_ERRNO_ESPIPE";
        case pdFREERTOS_ERRNO_EROFS: return "pdFREERTOS_ERRNO_EROFS";
        case pdFREERTOS_ERRNO_EUNATCH: return "pdFREERTOS_ERRNO_EUNATCH";
        case pdFREERTOS_ERRNO_EBADE: return "pdFREERTOS_ERRNO_EBADE";
        case pdFREERTOS_ERRNO_EFTYPE: return "pdFREERTOS_ERRNO_EFTYPE";
        case pdFREERTOS_ERRNO_ENMFILE: return "pdFREERTOS_ERRNO_ENMFILE";
        case pdFREERTOS_ERRNO_ENOTEMPTY: return "pdFREERTOS_ERRNO_ENOTEMPTY";
        case pdFREERTOS_ERRNO_ENAMETOOLONG: return "pdFREERTOS_ERRNO_ENAMETOOLONG";
        case pdFREERTOS_ERRNO_EOPNOTSUPP: return "pdFREERTOS_ERRNO_EOPNOTSUPP";
        case pdFREERTOS_ERRNO_ENOBUFS: return "pdFREERTOS_ERRNO_ENOBUFS";
        case pdFREERTOS_ERRNO_ENOPROTOOPT: return "pdFREERTOS_ERRNO_ENOPROTOOPT";
        case pdFREERTOS_ERRNO_EADDRINUSE: return "pdFREERTOS_ERRNO_EADDRINUSE";
        case pdFREERTOS_ERRNO_ETIMEDOUT: return "pdFREERTOS_ERRNO_ETIMEDOUT";
        case pdFREERTOS_ERRNO_EINPROGRESS: return "pdFREERTOS_ERRNO_EINPROGRESS";
        case pdFREERTOS_ERRNO_EALREADY: return "pdFREERTOS_ERRNO_EALREADY";
        case pdFREERTOS_ERRNO_EADDRNOTAVAIL: return "pdFREERTOS_ERRNO_EADDRNOTAVAIL";
        case pdFREERTOS_ERRNO_EISCONN: return "pdFREERTOS_ERRNO_EISCONN";
        case pdFREERTOS_ERRNO_ENOTCONN: return "pdFREERTOS_ERRNO_ENOTCONN";
        case pdFREERTOS_ERRNO_ENOMEDIUM: return "pdFREERTOS_ERRNO_ENOMEDIUM";
        case pdFREERTOS_ERRNO_EILSEQ: return "pdFREERTOS_ERRNO_EILSEQ";
        case pdFREERTOS_ERRNO_ECANCELED: return "pdFREERTOS_ERRNO_ECANCELED";
    }
    return "<Unknown Error>";
}
