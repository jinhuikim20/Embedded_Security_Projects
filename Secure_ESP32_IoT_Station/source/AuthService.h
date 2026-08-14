/**
 * @file AuthService.h
 * @brief Authentication and cryptographic utility module for credential validation and hashing.
 */

#ifndef AUTH_SERVICE_H
#define AUTH_SERVICE_H

#include <Arduino.h>
#include "mbedtls/sha256.h"

namespace AuthService {
 
    /**
     * @brief Computes the SHA-256 hash of an input string and returns it as a hex-encoded String.
     * 
     * @param input Raw text string to be hashed.
     * @return String 64-character lowercase hexadecimal hash digest.
     */
    inline String hashSHA256(const String& input) {
        unsigned char hash[32];
        mbedtls_sha256(reinterpret_cast<const unsigned char*>(input.c_str()), input.length(), hash, 0);

        char hexBuffer[65];
        for (int i = 0; i < 32; i++) {
            sprintf(hexBuffer + (i * 2), "%02x", hash[i]);
        }
        return String(hexBuffer);
    }

}

#endif // AUTH_SERVICE_H