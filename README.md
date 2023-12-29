# INFinityDrive

INFinityDrive is a Dropbox-like service, that allows ***file sharing*** with automatic ***synchronization*** between ***different devices***.

It is implemented using the ***Transmission Control Protocol (TCP) Socket API*** with C++. In its core functions, important concepts such as programming with ***multiple processes/threads***, communication and ***concurrency control*** were used.


Another important mechanism implemented is the ***replication*** system with ***leader election***. Using the ***Ring Election Algorithm*** it makes sure that when the primary server crashes, a backup server takes the responsibility. 


It was tested with 5 computers connected to a network. One as the main server, 2 as backup servers and 2 as clients. Tests included:
- Client file upload, check if it is propagated correctly
- Kill the primary server, check the election of a backup server and the reconnection of the clients to it
- Client file upload through the elected server
- Kill the current elected server
- ...


This project was developed for the Operating Systems II course at [INF](https://inf.ufrgs.br)-[UFRGS](https://ufrgs.br).

### Working example

![Working example](infinitydrive_example.gif)


## Authors
- [Arthur P Baumgardt](https://github.com/4rthb)
- [Gabriel Madeira](http://gabrielmadeira.com)
- [Arthur Brackmann Pires](https://github.com/ArthurBPires)
- [João Bedin](https://github.com/jmbedin)