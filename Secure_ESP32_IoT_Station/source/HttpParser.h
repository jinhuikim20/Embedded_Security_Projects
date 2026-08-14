#ifndef HTTP_PARSER_H
#define HTTP_PARSER_H

#include <Arduino.h>

struct HttpRequest {
    String method;
    String path;
    String body;
    String clientIP;
};

class HttpParser {
public:
    // Helper to extract a form field value from a body like "key=1234&user=admin"
    static String getFormField(const String& body, const String& fieldName) {
        String keyToFind = fieldName + "=";
        int start = body.indexOf(keyToFind);
        if (start == -1) return ""; // Not found

        start += keyToFind.length();
        int end = body.indexOf('&', start);
        if (end == -1) {
            end = body.length(); // End of string if no more parameters
        }

        return body.substring(start, end);
    }

    static HttpRequest parsePostRequest(const String& rawRequest) {
        HttpRequest req;
    // 1. Separate Headers and Body (divided by double CRLF: \r\n\r\n)
        int bodyIndex = rawRequest.indexOf("\r\n\r\n");
        if (bodyIndex != -1) {
            // Body starts right after "\r\n\r\n" (4 characters)
            req.body = rawRequest.substring(bodyIndex + 4);
        }

        // 2. Parse the First Line (e.g., "POST /login HTTP/1.1")
        int firstSpace  = rawRequest.indexOf(' ');
        int secondSpace = rawRequest.indexOf(' ', firstSpace + 1);

        if (firstSpace != -1 && secondSpace != -1) {
            req.method = rawRequest.substring(0, firstSpace);
            req.path   = rawRequest.substring(firstSpace + 1, secondSpace);
        }

        return req;
    }
};







#endif // HTTP_PARSER_H