Secure ESP32 IoT Station

<img width="707" height="505" alt="image" src="https://github.com/user-attachments/assets/86c923dd-2cb7-422b-84ea-b51bde935f75" />

```text
                  Wi-Fi Network
                        |
        +---------------+---------------+
        |                               |
        v                               v
      ESP32                           Laptop
    Secure Node                  Security Tester
     (Server)                    (Client/Attacker)
    - Authentication             - Login attempts
    - Encryption                 - Packet testing
    - Access control             - Security testing
    - Event logging              - Attack simulation             

```

**Feature 1. Secure Authentication**
Login passwords are saved in the server as hashwalues.
When a login is requested, ESP32 is checking the user database to get the hash value of the corresponding password.
When the entered password is correct then Access is allowed.
When not, Access is not allowed.
All login attemps are recorded in the log database.

**Feature 2. Failed login protection**
In order to prevent login Brute-Force attacks, login is not available for 1 minute when the login attemps are failed 3 times.

**Feature 3. Encrypted communication**
With TLS, the message is encrypted to prevent Man in the Middle Attacks.

**Feature 4. Logging system**
All interactions are logged with the client IP adddress. ex: login attemps, commands, logout
