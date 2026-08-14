/**
 * @file ViewTemplates.h
 * @brief HTML view generation and HTTP response rendering helpers for the secure HTTPS web server.
 */

#ifndef VIEW_TEMPLATES_H
#define VIEW_TEMPLATES_H

#include <Arduino.h>
#include "mbedtls/ssl.h"

namespace ViewTemplates {
    /**
     * @brief Formats and sends an HTTP response over an active TLS secure socket connection with Keep-Alive headers.
     * 
     * @param ssl Pointer to the active mbedTLS context.
     * @param statusCode HTTP status code (e.g., 200, 401).
     * @param htmlBody Fully formatted HTML payload string.
     */
    inline void renderResponse(mbedtls_ssl_context* ssl, int statusCode, const String& htmlBody) {
        String response = "HTTP/1.1 " + String(statusCode) + " OK\r\n"
                         "Content-Type: text/html\r\n"
                         "Content-Length: " + String(htmlBody.length()) + "\r\n"
                         "Connection: keep-alive\r\n"  // Instruct client to keep TCP/TLS tunnel open for subsequent requests
                         "Keep-Alive: timeout=5, max=50\r\n\r\n" + htmlBody;

        mbedtls_ssl_write(ssl, reinterpret_cast<const unsigned char*>(response.c_str()), response.length());
    }

    /**
     * @brief Renders the login portal page, optionally displaying an error message banner.
     * 
     * @param ssl Pointer to the active mbedTLS context.
     * @param showError Flag indicating whether an invalid login warning should be displayed.
     */
    inline void renderLogin(mbedtls_ssl_context* ssl, bool showError = false) {
        String page = "<!DOCTYPE html><html><head>"
                      "<meta name='viewport' content='width=device-width, initial-scale=1'>"
                      "<style>body{font-family:sans-serif;text-align:center;padding:20px;}"
                      "input{padding:10px;margin:10px;font-size:16px;}</style></head><body>"
                      "<h1>ESP32 Secure Login</h1>";

        if (showError) {
            page += "<p style='color:red;'><strong>Access Denied!</strong> Invalid password.</p>";
        }

        page += "<form action='/login' method='POST'>"
                "<h2>ID</h2><input type='userid' name='ID' placeholder='Enter your ID' required><br>"
                "<h2>Password</h2><input type='password' name='key' placeholder='Password' required><br>"
                "<input type='submit' value='Login'></form></body></html>";

        renderResponse(ssl, showError ? 401 : 200, page);
    }

    inline void renderLoginAttack(mbedtls_ssl_context* ssl, bool showError = true) {
        String page = "<!DOCTYPE html><html><head>"
                      "<meta name='viewport' content='width=device-width, initial-scale=1'>"
                      "<style>body{font-family:sans-serif;text-align:center;padding:20px;}"
                      "input{padding:10px;margin:10px;font-size:16px;}</style></head><body>"
                      "<h1>ESP32 Secure Login</h1>";

        if (showError) {
            page += "<p style='color:red;'><strong>Login Faile 3 times!</strong> Login is not available for the next 1 minute.</p>";
        }

        page += "<form action='/loginattack' method='POST'>"
                "</form></body></html>";

        renderResponse(ssl, showError ? 401 : 200, page);
    }

    /**
     * @brief Renders the administrative control panel dashboard with hardware state controls.
     * 
     * @param ssl Pointer to the active mbedTLS context.
     * @param activeHash Current user session authentication token hash.
     * @param ledPin GPIO pin number driving the status LED.
     */
    inline void renderControlPanel(mbedtls_ssl_context* ssl, const String& ID, const String& activeHash, uint8_t ledPin) {
        bool isLedOn = (digitalRead(ledPin) == HIGH);
        String btnColor = isLedOn ? "#f44336" : "#4CAF50";
        String btnText  = isLedOn ? "Turn OFF" : "Turn ON";
        String targetRoute = isLedOn ? "/L" : "/H";

        String page = "<!DOCTYPE html><html><head>"
                      "<meta name='viewport' content='width=device-width, initial-scale=1'>"
                      "<style>body{font-family:sans-serif;text-align:center;padding:20px;}"
                      ".btn{padding:15px 32px;color:white;text-decoration:none;font-size:18px;display:inline-block;}</style></head><body>"
                      "<h1>ESP32 Control Panel</h1>"
                      "<p>LED Status: <strong>" + String(isLedOn ? "ON" : "OFF") + "</strong></p>"
                      "<form action='" + targetRoute + "' method='POST'>"
                            "<input type='hidden' name='ID' value='" + ID + "'>"
                            "<input type='hidden' name='auth' value='" + activeHash + "'>"
                            "<input type='submit' class='btn' style='background-color:" + btnColor + ";' value='" + btnText + "'>"
                      "</form>"
                      "<form action='/logout' method='POST' style='display:inline;'>"
                      "<input type='hidden' name='ID' value='" + ID + "'>"
                      "<input type='submit' class='link-btn' value='Logout'>"
                      "</form>"
                      "</body></html>";

        renderResponse(ssl, 200, page);
    }
}

#endif // VIEW_TEMPLATES_H