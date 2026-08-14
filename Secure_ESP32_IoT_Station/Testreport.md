**Feature 1. Secure Authentication**

<img width="681" height="266" alt="User DB" src="https://github.com/user-attachments/assets/5537a633-96d3-42f1-9544-00364fb31b48" />

In the DB, the passwords are saved in hash values. When the login request is recieved, the corresponding hash values are used to compare with the entered password.


**Feature 2. Failed login protection**

After 3 login tries, the server blocks login attemps for 1 minute. This can prevent brue-force attacks.

<img width="600" height="200" alt="Login Failed" src="https://github.com/user-attachments/assets/55ee5955-a997-49a4-8137-4386981a9fc1" />


**Feature 3. Encrypted communication With TLS**

<img width="1667" height="929" alt="TLS" src="https://github.com/user-attachments/assets/16eb6ee9-2ee2-4630-9d28-34c4c9ac6e5d" />

With TLS, all the data is encrypted during transmission. When the messages are captured in WireShark, the hacker cannot reconstruct the original text.

In comparison, in HTTP the hacker can see the plain text as it is.

<img width="1721" height="795" alt="HTTP_Login" src="https://github.com/user-attachments/assets/52908d90-d110-4c18-a985-74db96a31563" />

Here you can see the login information.


**Feature 4. Logging system**

<img width="515" height="368" alt="Login DB" src="https://github.com/user-attachments/assets/caae9d6b-d22c-49a6-a19a-894f8f2e386a" />

All the user interactions are logged with IP address.
