**Feature 1. Secure Authentication**

<img width="681" height="266" alt="User DB" src="https://github.com/user-attachments/assets/5537a633-96d3-42f1-9544-00364fb31b48" />

In the DB, the passwords are saved in hash values. When the login request is recieved, the corresponding hash values are used to compare with the entered password.

When the login succeeds, the user can see the control page as below.

<img width="600" height="250" alt="Control" src="https://github.com/user-attachments/assets/0cf77edc-7d94-42b5-8878-a1e72cb603b9" />


By clicking the On button, the user can access the control of the LED (Infrastructure).

<img width="350" height="250" alt="image" src="https://github.com/user-attachments/assets/b0853adc-703f-4dc4-9fff-9e6c46841deb" />

When the LED is On, the control page looks like the below.

<img width="600" height="250" alt="Control2" src="https://github.com/user-attachments/assets/778fd89a-fefa-4c11-9d6a-0473c7a8536a" />

When the user put the wrong password, Login failure page will be loaded.

<img width="600" height="350" alt="Loginfail" src="https://github.com/user-attachments/assets/2decc697-4236-4f90-b14d-1c7ffa780fbf" />



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
