This is for me to learn how the internals of sockets and protocols (HTTP, DNS) work  
about 50% of this code is hand written so expect some abnormal technique.

## Future work
- dynamic DNS: right now we hardcode the ip addr of client and server in the dns.c file. we should make it dynamic. have any container that connects register its ip addr and hostname with the dns server.
- server handle multiple connections
- long lived connections. right now we always close() the socket after 1 round of request and response, we should use the Connection header to determine whether we close or not.
- other protocols, websockets, ftp, http streaming.
- non text based protocols, audio or video would be cool to learn.