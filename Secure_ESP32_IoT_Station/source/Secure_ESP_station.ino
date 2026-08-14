/**
 * ============================================================================
 * ESP32 Clean Architecture HTTPS Server (mbedTLS) with Keep-Alive & Session Caching
 * ============================================================================
 */
/* SD card connection
    Connections:
     * SD Card | ESP32(TTGO T)
     * CS         33
     * MOSI       26
     * MISO       27
     * CLK        25
     * VCC        5V
*/
#include <WiFi.h>
#include "cert.h"          // DER Certificate byte array (example_crt_der)[cite: 10]
#include "private_key.h"   // DER Private Key byte array (example_key_der)[cite: 13]

#include "AuthService.h"
#include "ViewTemplates.h"
#include "HttpParser.h"
#include "TlsServer.h"
#include <stdio.h>
#include <stdlib.h>
#include <SPI.h>
#include <FS.h>
#include "SD.h"
#include "MyDatabank.h"
#include <WiFi.h>
#include "time.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
SPIClass spiSD(HSPI);
#define SD_CS 33
#define SDSPEED 27000000

// Configuration Parameters
namespace Config {
    constexpr char SSID[]     = "SmartHome-M&J";
    constexpr char PASSWORD[] = "69587753228372994597";
    constexpr uint16_t PORT   = 443;
    constexpr uint8_t LED_PIN = 2;
}

TlsServer tlsServer;

// NTP Server Einstellungen 
const char* ntpServer = "de.pool.ntp.org"; // Deutscher Server-Pool für bessere Latenz
const long  gmtOffset_sec = 3600;          // Zeitzone: Deutschland ist UTC+1 (3600 Sekunden)
const int   daylightOffset_sec = 3600;     // Sommerzeit: Weitere 3600 Sekunden (1 Stunde)
int LoginTry = 0;
long BlockTimeBegin = 0; 
long PassedTime = 0;
String loginattackID = "";
// -----------------------------------------------

/**
 * @brief Handles incoming secure HTTPS client connections, performing handshakes, 
 *        socket timeouts, and persistent request processing loops (Keep-Alive).
 * 
 * @param clientFd Connected POSIX client socket file descriptor.
 */
 

void handleHttpsClient(int clientFd) {
    // Get the sender's IP address using getpeername()
    // Declare address structure and length BEFORE calling getpeername()
    struct sockaddr_in address;
    socklen_t addrLen = sizeof(address);
    String clientIP = "Unknown";
    //Safely extract the client IP address from the file descriptor
    if (getpeername(clientFd, (struct sockaddr*)&address, &addrLen) == 0) {
        char ipStringBuffer[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &address.sin_addr, ipStringBuffer, sizeof(ipStringBuffer));
        clientIP = String(ipStringBuffer);
    }


    mbedtls_ssl_context ssl;
    mbedtls_ssl_init(&ssl);
    if (mbedtls_ssl_setup(&ssl, tlsServer.getConfig()) != 0) {
        close(clientFd);
        return;
    }

    // Attach network I/O callbacks to the mbedTLS context
    mbedtls_ssl_set_bio(&ssl, &clientFd, mbedtls_net_send, mbedtls_net_recv, NULL);

    // Perform TLS Handshake (automatically resumes cryptographic session if a valid cache ticket is presented)
    int ret;
    while ((ret = mbedtls_ssl_handshake(&ssl)) != 0) {
        if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE) {
            mbedtls_ssl_free(&ssl);
            close(clientFd);
            return;
        }
    }

    // Set a socket read timeout (5 seconds) to prevent idle Keep-Alive connections from blocking indefinitely
    struct timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    setsockopt(clientFd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));

    // Persistent Keep-Alive loop: multiplexes multiple requests over the single established TLS tunnel
    while (true) {
        unsigned char buffer[1024];
        int bytesRead = mbedtls_ssl_read(&ssl, buffer, sizeof(buffer) - 1);
        

        if (bytesRead <= 0) {
            break; // Connection closed by client or timed out
        }

        buffer[bytesRead] = '\0';
        String rawRequest(reinterpret_cast<char*>(buffer));

        HttpRequest req = HttpParser::parsePostRequest(rawRequest);
        req.clientIP = clientIP;
        
        char timebuffer[20];
        getLocalTimeString(timebuffer, sizeof(timebuffer));
        // Routing Logic
        // Preventing Brute-Force Login attack
        if (LoginTry == 2){
          BlockTimeBegin = getUnixTime(timebuffer);
          Serial.println("BlockTimeBegin");
          Serial.println(BlockTimeBegin);
          LoginTry = 0; 
          PassedTime = 0;
          log(timebuffer, req.clientIP.c_str(), loginattackID.c_str(),"3rd Login FAIL");
        }

        PassedTime = getUnixTime(timebuffer) - BlockTimeBegin;
        Serial.println(PassedTime);

        // When Login attempts are failed 3 times, Login is not available for 1 minute.
        if (PassedTime < 60){
    	  // Gracefully tear down the TLS session before closing the socket
          ViewTemplates::renderLoginAttack(&ssl, true);
          mbedtls_ssl_close_notify(&ssl);
          mbedtls_ssl_free(&ssl);
          close(clientFd);
          return;
        }


        // Application Routing Logic
    if (req.path == "/login") {
      String userID = HttpParser::getFormField(req.body,"ID");
      String password = HttpParser::getFormField(req.body, "key");
      String token = AuthService::hashSHA256(password);
      String DBkey;
      LoginDB(userID.c_str(), DBkey);
  
      if (DBkey.equalsIgnoreCase(token)) {
          log(timebuffer, req.clientIP.c_str(), userID.c_str(),"Login Success");
          Serial.println(userID);
          Serial.println("logged in");
          delay(100);
          ViewTemplates::renderControlPanel(&ssl, userID, token, Config::LED_PIN);
      } else {
          log(timebuffer, req.clientIP.c_str(), userID.c_str(),"Login FAIL");
          loginattackID = userID;
          Serial.println(userID);
          Serial.println("password not correct");
          delay(100);
          LoginTry++;

          ViewTemplates::renderLogin(&ssl, true);
      }
    } 
    else if (req.path == "/H" || req.path == "/L") {
        String userID = HttpParser::getFormField(req.body,"ID");
        String passHash = HttpParser::getFormField(req.body, "auth");
        String DBkey;
        LoginDB(userID.c_str(), DBkey);
        if (DBkey.equalsIgnoreCase(passHash)) {
            // Toggle the LED pin state
            if (req.path == "/H"){
              digitalWrite(Config::LED_PIN, HIGH);
              log(timebuffer, req.clientIP.c_str(), userID.c_str(),"On");
              Serial.println("On clicked");
              delay(100);
            } else if (req.path == "/L"){
              digitalWrite(Config::LED_PIN, LOW);
              log(timebuffer, req.clientIP.c_str(), userID.c_str(),"Off");
              Serial.println("Off clicked");
              delay(100);
            } else {
              digitalWrite(Config::LED_PIN, LOW);
              log(timebuffer, req.clientIP.c_str(), userID.c_str(),"invalid");
              Serial.println("invalid command");
              delay(100);
            }

            //digitalWrite(Config::LED_PIN, (req.path == "/H") ? HIGH : LOW);
            // Render the panel with the updated state
            ViewTemplates::renderControlPanel(&ssl, userID, passHash, Config::LED_PIN);
        } else {
            // Access denied due to invalid auth hash
            ViewTemplates::renderLogin(&ssl, true);
            log(timebuffer, req.clientIP.c_str(), userID.c_str(),"invalid access");
            Serial.println("invalid access");
            delay(100);
        }
    } else if (req.path == "/logout") {
        String userID = HttpParser::getFormField(req.body,"ID");
        log(timebuffer, req.clientIP.c_str(), userID.c_str(),"Logout Success");
        ViewTemplates::renderLogin(&ssl, false);
        Serial.println("Logout Success");
        delay(100);

    }
    else {
        // Base route / or unknown routes
        ViewTemplates::renderLogin(&ssl, false);
    }
    }
    // Cleanly tear down the TLS session and close socket when the keep-alive connection expires
    mbedtls_ssl_close_notify(&ssl);
    mbedtls_ssl_free(&ssl);
    close(clientFd);
}
void getLocalTimeString(char* buffer, size_t maxLen){
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)){
      Serial.println("Fehler beim Abrufen der Zeit");
      snprintf(buffer, maxLen, "0000-00-00 00:00:00");
    return;
  }
  strftime(buffer, maxLen, "%Y-%m-%d %H:%M:%S", &timeinfo);
  return;
}

long getUnixTime(char* buffer){
  struct tm timeinfo = {0};
  int year, month;
  sscanf(buffer, "%d-%d-%d %d:%d:%d", &year, &month, 
  &timeinfo.tm_mday, &timeinfo.tm_hour, &timeinfo.tm_min, &timeinfo.tm_sec);
  
  timeinfo.tm_year = year - 1900;
  timeinfo.tm_mon = month -1;
  timeinfo.tm_isdst = -1;

  time_t timestamp = mktime(&timeinfo);
  return (long) timestamp;
}

// === THESE MUST BE AT GLOBAL SCOPE ===
void setup() {
    Serial.begin(115200);
    pinMode(Config::LED_PIN, OUTPUT);

    // Initialize High-Speed SPI bus for the SD card module
    spiSD.begin(25, 27, 26, 33);
    if (!SD.begin(SD_CS, spiSD, SDSPEED)) {
      Serial.println("SD Card Mount Failed!");
      return;
    }
    Serial.println("SD Card Initialized.");

    // Initialize SQLite engine environment
    sqlite3_initialize();

    WiFi.begin(Config::SSID, Config::PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.printf("\nWiFi Connected! IP Address: %s\n", WiFi.localIP().toString().c_str());
    Serial.println("Warte auf Zeit-Synchronisation...");
    
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    struct tm timeinfo;
    while(!getLocalTime(&timeinfo)){
      Serial.print(".");
      delay(100);
    }
    Serial.println("\nZeit erfolgreich synchronisiert!");

    bool started = tlsServer.begin(
        Config::PORT, 
        example_crt_der, example_crt_der_len, 
        example_key_der, example_key_der_len
    );

    if (started) {
        Serial.printf("HTTPS Server listening securely on Port %d\n", Config::PORT);
    } else {
        Serial.println("HTTPS Server startup failed!");
    }
}

void loop() {
    int clientFd = tlsServer.acceptClient();
    if (clientFd >= 0) {
        handleHttpsClient(clientFd);
    }
    delay(5);
}