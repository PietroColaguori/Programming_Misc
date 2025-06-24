#include "toralize.h"

// Usage: ./toralize ip-address port (e.g. ./toralize 1.2.3.4 80)

/*

1. TURN THE CLIENT INTO A SHARED LIB .SO FILE
2. TURN MAIN() INTO OUR OWN CONNECT()
3. REPLACE REGULAR CONNECT()
4. GRAB THE IP AND PORT FROM ORIGINAL CONNECT()

*/

Req *request(struct sockaddr_in *sock2) {
    Req *req;

    req = malloc(reqsize);

    req->vn = 4;
    req->cd = 1;
    req->dstport = sock2->sin_port;  // switch to network bytes order
    req->dstip = sock2->sin_addr.s_addr;  // switch to network bytes order
    strncpy(req->userid, USERNAME, 8);

    return req;
}

// Main turned in new connect function
int connect(int s2, const struct sockaddr *sock2, socklen_t addrlen) {
    int s;
    struct sockaddr_in sock;
    Req *req;
    char buf[ressize];
    Res *res;
    int success;  // used as predicate -> function that returns true or false
    char tmp[512];
    int (*p)(int, const struct sockaddr*, socklen_t); // signature of connect function

    p = dlsym(RTLD_NEXT, "connect");  // Recover address of connect from RTLD

    // Create connection to the proxy server at localhost:9050
    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) {
        perror("socket");
        return -1;
    }

    sock.sin_family = AF_INET;
    sock.sin_port = htons(PROXY_PORT);
    sock.sin_addr.s_addr = inet_addr(PROXY);

    if (p(s, (struct sockaddr *)&sock, sizeof(sock)) != 0) {
        perror("connect");
        return -1;
    }

    printf("Connected to proxy\n");

    // Make SOCKS 4 request to host:port
    req = request((struct sockaddr_in *)sock2);
    write(s, req, reqsize);

    memset(buf, 0, ressize);
    if (read(s, buf, ressize) < 1) {
        perror("read");
        free(req);
        close(s);

        return -1;
    }

    // Read SOCKS 4 response
    res = (Res *) buf;
    success = (res->cd == 90);
    if (!success) {
        fprintf(stderr, "Unable to traverse the proxy, error code: %d\n", res->cd);
        
        close(s);
        free(req);
        return -1;
    }

    printf("Connected through the proxy.\n");
    
    // Piping: everything being sent to s will also be sent to s2 and vice-versa
    // We act as a man in the middle

    dup2(s, s2);
    free(req);

    return 0;
}
