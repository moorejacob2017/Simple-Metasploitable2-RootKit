# Simple Metasploitable2 RootKit
A condensed, single file rootkit (```upload.php```) used to establish persistance within a Metasploitable2 machine.

## How to Use upload.php
1. Navigate to the File Upload section of DVWA (```http://<ms2 ipaddr>/dvwa/vulnerabilities/upload/```) and upload the file as ```upload.php.jpeg```. This will subvert the filetype filter.
2. Send an HTTP request to the uploaded file, resulting in server-side PHP code execution. This may be done with any prefered method; browser, curl, wget, etc.
    - ```wget http://<ms2 ipaddr>/dvwa/hackable/uploads/upload.php.jpeg```
3. Connect to the server on port 8080.
    - ```nc <ms2 ipaddr> 8080```
4. Enter ```yvnS65uxnW9pQJJZ``` for the backdoor password when asked to authenticate.
5. When finished, type ```exit``` and then ```^C``` to terminate the connection. Failure to type ```exit``` before terminating the connection will result in the backdoor closing.
6. Repeat steps 3-5 to reconnect to the backdoor

__WARNING:__ It is recommended to clean tracks after exploiting, as the HTTP request to ```upload.php.jpeg``` is visible in ```/var/log/apache2/access.log```
<br>

## How upload.php Works

This particular rootkit takes advantage of two vulnerabilities; the intial File Upload Vulnerability as an attack vector, and [CVE-2009-1185](https://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2009-1185) for privilege escalation, a vulnerability where udev < 1.4.1 does not verify whether a NETLINK message originates from kernel space.

1. ```upload.php.jpeg``` recieves an HTTP request and begins to executes PHP code.
2. ```upload.php.jpeg``` exports a base64 encoded zip file to ```/tmp``` and extracts four files:
    - ```8572``` - x86 ELF used for privilege escalation ([exploit-db.com](https://www.exploit-db.com/exploits/8572))
    - ```run``` - Bash Script run with elevated privleges by ```8572```, sets up persistence and removes rootkit files
    - ```.conf``` - x86 ELF Backdoor
    - ```samba``` - Modified version of the Samba File Server init.d Service Script (```/etc/init.d/samba```), starts ```.conf``` on service start
3. ```upload.php.jpeg``` gets the proper NETLINK PID and runs ```8572``` with said PID as an argument before finishing.
4. ```8572``` uses [CVE-2009-1185](https://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2009-1185) to escalate privileges and runs ```run``` before finishing.
5. ```run``` alters file privileges and ownership of ```.conf``` before moving it to ```/bin```, then runs it.
6. ```run``` alters file privileges and ownership of ```samba``` before moving it to ```/etc/init.d/```, replacing the original ```/etc/init.d/samba``` file.
7. ```run``` removes the remaining extracted files from ```/tmp```, including itself, as well as ```upload.php.jpeg``` before finishing.
8. Exploitation is complete and the modified version of ```/etc/init.d/samba``` will run ```/bin/.conf``` anytime the Samba Service is started. Because the Samba Service is automatically started on boot, the backdoor will also be run on boot, allowing for persistance.

