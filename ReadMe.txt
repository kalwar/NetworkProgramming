-----------------------------------------------------------------------------------------------------------------------------------------------------------
Author and References
This program is done by
Student name:: Santosh Kalwar
Student number:: 0331927
Reference taken from 
http://www.it.lut.fi/kurssit/07-08/CT30A5000/home-exam.html
http://www.faqs.org/rfcs/rfc959.html
Stevens, W.: TCP/IP Illustrated Volume 1, page 419
http://beej.us/guide/bgnet/output/html/singlepage/bgnet.html
-----------------------------------------------------------------------------------------------------------------------------------------------------------
Synopsis

The task was to make a FTP Client based on the RFC 959.It must be able to transfer a  file of up 10 MB using the FTP Protocol Type of the file can be either ASCII or Binary.The client should communicate with standard FTP servers. Access to FTP Server is given in the assignment.

-----------------------------------------------------------------------------------------------------------------------------------------------------------
Commands Implemented

The client recognizes the following commands, Which are basically front-end commands.

get: get file from server

put: save file on server

ls: show listing of files on server

cd: change remote working directory

ascii/binary: Set the file transfer type to ASCII or Binary (only binary has been implemented for file and ASCII for directory listings.)

open: open server connection, user needs to send user id and password (tries for 3 times and quits)

close: close connection with server

quit: exit the FTPClient

passive: Active and passive connection mode is been implemented. 

cd.. : Change the directory to one level up
-----------------------------------------------------------------------------------------------------------------------------------------------------------
Structure of FTP Program



  User at terminal   --------->FTP Client------>Control connection                 ------>FTP Server
  enters commands                             (FTP commands, replies)   
  open, ls, close,
   quit, passive,get                        ------->Data Connection
   put                                               (File data)


-----------------------------------------------------------------------------------------------------------------------------------------------------------
Output

preeti:~/network programming/ass7> make
gcc -g -Wall FTPClient.c -o FTPClient -lnsl -lm


Step 1:

./FTPClient
Use: ./FTPClient -p <FTP server port> -P <data transfer port> -h <ftp server address>


 ./FTPClient -p 21 -P 20005 -h venla.it.lut.fi

*******************************************************************
 Welcome to File Transfer Protocol (FTP Client) based on RFC 959
 Write open,to open the connection.
 Give default Username and Password
 Client recognizes following commands-
 Commands are: get, put, ls, cd,cd.. ,open, close, quit, passive
 Wish you have nice, File transfer session !!!
*******************************************************************

Step 2:

open
Connection ready
220-
220- **********************************************************************
220- *                                                                    *
220- *  Anonymous ftp server for datacommunication practise works        *
220- *                                                                    *
220- **********************************************************************
220 venla.it.lut.fi FTP server (Version 6.4/OpenBSD/Linux-ftpd-0.17) ready.
Give user name:


Step 3:

Give user name: anonymous
---> USER anonymous
331 Guest login ok, send your complete e-mail address as password.
Give password: anonymous
---> PASS anonymous
230 Guest login ok, access restrictions apply.


Step 4:

ls
---> TYPE A
200 Type set to A.
---> PORT 157,24,55,222,78,37
200 PORT command successful.
---> LIST
150 Opening ASCII mode data connection for '/bin/ls'.
Data connection created
total 24
-rw-r--r-- 1 0    0   75 Jan 21 12:19 .message
d--x--x--x 2 0    0 4096 Jan 21 09:44 bin
dr-x--x--x 2 0    0 4096 Jan 21 12:52 etc
dr-x--x--x 2 0    0 4096 Jan 21 12:53 lib
dr-xr-xr-x 3 0    0 4096 Jan 21 12:48 pub
drwxrwxr-x 2 0 1001 4096 Feb 14 13:08 upload
226 Transfer complete.

Step 5:

cd upload
---> CWD upload
250- File upload area for anonymous users
250-
250 CWD command successful.

get 97.wmv.mp4
---> TYPE I
200 Type set to I.
---> PORT 157,24,55,222,78,39
200 PORT command successful.
---> RETR 97.wmv.mp4
150 Opening BINARY mode data connection for '97.wmv.mp4' (188034898 bytes).
Data connection created
226 Transfer complete.

Step 6:

put 12345
---> TYPE I
200 Type set to I.
---> PORT 157,24,55,222,78,42
200 PORT command successful.
---> STOR 12345
150 Opening BINARY mode data connection for '12345'.
Data connection created
Transfer speed: 9197783.203336
Time taken: 0.046283
226 Transfer complete.

Step 7:

passive
Entering passive mode
get 111
---> PASV
227 Entering Passive Mode (157,24,53,46,166,194)
---> TYPE I
200 Type set to I.
---> RETR 111
Passive data connection opened
150 Opening BINARY mode data connection for '111' (2 bytes).
Data connection created
226 Transfer complete.


Step 8:

put 123456
---> PASV
227 Entering Passive Mode (157,24,53,46,190,141)
---> TYPE I
200 Type set to I.
---> STOR 123456
150 Opening BINARY mode data connection for '123456'.
Transfer speed: 23791482.702733
Time taken: 0.017893
226 Transfer complete.

Step 9:

cd..
---> CDUP
250- root folder of anonymous ftp server for datacommunications practise works
250-
250 CWD command successful.

Step 10:

close
---> QUIT
221 Goodbye.

Step 11:

quit
preeti:~/network programming/ass7>

-----------------------------------------------------------------------------------------------------------------------------------------------------------










