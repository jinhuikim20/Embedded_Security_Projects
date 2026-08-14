/**
 * @file TlsServer.h
 * @brief Secure TLS server implementation for the ESP32 using mbedTLS and POSIX sockets.
 * 
 * Handles certificate parsing, private key configuration, cryptographic session caching, 
 * and network socket binding for secure HTTPS connections.
 */

#ifndef TLS_SERVER_H
#define TLS_SERVER_H

#include <Arduino.h>
#include "lwip/sockets.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/x509_crt.h"
#include "mbedtls/pk.h"
#include "mbedtls/net_sockets.h"
#include "mbedtls/ssl_cache.h"

class TlsServer {
private:
    mbedtls_ssl_config      conf;       ///< SSL/TLS configuration context.
    mbedtls_entropy_context entropy;    ///< Entropy context for cryptographic randomness.
    mbedtls_ctr_drbg_context ctr_drbg;  ///< Deterministic random bit generator context.
    mbedtls_x509_crt        srvcert;    ///< Server X.509 certificate structure.
    mbedtls_pk_context      pkey;       ///< Server private key context.
    mbedtls_ssl_cache_context cache;    ///< Session cache context to enable fast TLS cryptographic resumption.
    int                     listenFd = -1; ///< File descriptor for the listening socket server.

public:
    /**
     * @brief Construct a new Tls Server object and initialize all underlying mbedTLS structures.
     */
    TlsServer() {
        mbedtls_ssl_config_init(&conf);
        mbedtls_entropy_init(&entropy);
        mbedtls_ctr_drbg_init(&ctr_drbg);
        mbedtls_x509_crt_init(&srvcert);
        mbedtls_pk_init(&pkey);
        mbedtls_ssl_cache_init(&cache);
    }

    /**
     * @brief Destroy the Tls Server object and safely free allocated mbedTLS contexts and sockets.
     */
    ~TlsServer() {
        mbedtls_x509_crt_free(&srvcert);
        mbedtls_pk_free(&pkey);
        mbedtls_ssl_cache_free(&cache);
        mbedtls_ssl_config_free(&conf);
        mbedtls_ctr_drbg_free(&ctr_drbg);
        mbedtls_entropy_free(&entropy);
        if (listenFd >= 0) close(listenFd);
    }

    /**
     * @brief Initializes cryptographic contexts, parses certificates/keys, enables session caching, and starts listening.
     * 
     * @param port Network port to listen on (e.g., 443 for HTTPS).
     * @param certDer Pointer to the DER-formatted server certificate byte array.
     * @param certLen Length of the certificate byte array.
     * @param keyDer Pointer to the DER-formatted private key byte array.
     * @param keyLen Length of the private key byte array.
     * @return true If initialization, parsing, and socket binding succeed.
     * @return false If any cryptographic or socket setup step fails.
     */
    bool begin(uint16_t port, const unsigned char* certDer, size_t certLen, 
               const unsigned char* keyDer, size_t keyLen) {
        
        // 1. Seed the CTR_DRBG entropy-based random generator
        const char *pers = "esp32_mbedtls_server";
        int ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, 
                                        reinterpret_cast<const unsigned char*>(pers), strlen(pers));
        if (ret != 0) return false;

        // 2. Parse server certificate and private key from DER buffers
        if (mbedtls_x509_crt_parse(&srvcert, certDer, certLen) != 0) return false;
        if (mbedtls_pk_parse_key(&pkey, keyDer, keyLen, NULL, 0, mbedtls_ctr_drbg_random, &ctr_drbg) != 0) return false;

        // 3. Configure default SSL server parameters
        mbedtls_ssl_config_defaults(&conf, MBEDTLS_SSL_IS_SERVER, 
                                    MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT);
        mbedtls_ssl_conf_rng(&conf, mbedtls_ctr_drbg_random, &ctr_drbg);
        mbedtls_ssl_conf_own_cert(&conf, &srvcert, &pkey);

        // 4. Attach session caching callbacks to enable TLS session resumption without full handshakes
        mbedtls_ssl_conf_session_cache(&conf, &cache, mbedtls_ssl_cache_get, mbedtls_ssl_cache_set);

        // 5. Create, bind, and listen on the TCP socket server
        listenFd = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
        if (listenFd < 0) return false;

        int opt = 1;
        setsockopt(listenFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

        struct sockaddr_in serverAddr{};
        serverAddr.sin_family      = AF_INET;
        serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
        serverAddr.sin_port        = htons(port);

        if (bind(listenFd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0 ||
            listen(listenFd, 5) < 0) {
            close(listenFd);
            return false;
        }

        return true;
    }

    /**
     * @brief Accepts an incoming client connection on the TLS server socket.
     * 
     * @return int Client file descriptor if successful, or negative value if no connection is pending.
     */
    int acceptClient() const {
        struct sockaddr_in clientAddr;
        socklen_t clientLen = sizeof(clientAddr);
        return accept(listenFd, (struct sockaddr *)&clientAddr, &clientLen);
    }

    /**
     * @brief Retrieves a pointer to the internal mbedTLS configuration structure.
     * 
     * @return mbedtls_ssl_config* Pointer to the SSL configuration context.
     */
    mbedtls_ssl_config* getConfig() {
        return &conf;
    }
};

#endif // TLS_SERVER_H