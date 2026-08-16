📂 Project Structure & File 

AuthService.h: Hash calculation and authentication verification logic.

HttpParser.h: Extracting information and parsing parameters from HTTP request messages.

MyDatabank.cpp: Database access layer for managing users and system logs.

**Secure_ESP_station.ino**: Main Arduino source sketch governing server initialization and routing.

TlsServer.h: TLS server implementation for the ESP32 using mbedTLS and POSIX sockets.

ViewTemplates.h: HTML view generation and HTTP response rendering helpers for the secure HTTPS web server.

cert.h: SSL/TLS server public certificate storage (PEM format) used for the secure HTTPS handshake.

private_key.h: SSL/TLS server private key storage (PEM format) matching the public certificate for cryptographic authentication and encryption.
