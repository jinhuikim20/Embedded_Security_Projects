In Embedded Cybersecurity, Secureboot and Flash encryption are playing important roles to keep the integrity and confidentiality of the FW.
Secureboot can ensure only properly signed FW can be loaded on the system. --> Integrity
Flash encryption can ensure that intruders cannot read out the content of the FW --> Confidentiality

For these functions ESP32 supports eFuse, which once burned, cannot return to the previous state. 
That is why it is important to test these functions using QEMU emulator first and read the Espressif documents carefully.
I bought cheap ESP32-C3 boards for these purposes. 

The following image explains the trust chain during Secureboot from ESP32.
Since ROM cannot be rewritten, it is the trusted root of the chain.
Rom loads the 2nd Bootloader and check its validity.
When the validity check succeeds, the 2nd Bootloader takes over the control and check the validity of the application image.  
<img width="700" height="250" alt="image" src="https://github.com/user-attachments/assets/b2748eaf-c3c8-4b52-bb72-8c065419a0e4" />

The following shows how ROM can validate the 2nd stage bootloader. 
1. Checking whether the public key is correct or not.
2. Calculating the digest of the image
3. Decrypting the signature of the image using the public key
4. Comparing the digest from step 2 and 3 and checking if they are the same
<img width="1050" height="500" alt="image" src="https://github.com/user-attachments/assets/54632268-aa0e-4928-b95b-716578c9390e" />


Similar steps are performed when the 2nd stage bootloader validates the application image.
<img width="1050" height="500" alt="image" src="https://github.com/user-attachments/assets/67050a58-f25e-4ad2-9bbe-1f547c350606" />

When flash encryption is enabled, the ESP32 encrypts the FW on its first writing and when it reboots it decrypts it. This process is done using HW. And the encryption key is stored in the eFuse with read/write protection.
<img width="550" height="310" alt="image" src="https://github.com/user-attachments/assets/94eb4c80-8369-4bcd-9c39-4b597b5cd893" />

I tested these functions using ESP32-C3.  
<img width="600" height="500" alt="image" src="https://github.com/user-attachments/assets/bd2e5dd2-619e-49c3-bf31-0936dd4e2fb2" />




