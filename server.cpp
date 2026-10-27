// system
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

//stdlib
#include <cstdlib>
#include <cerrno>
#include <cstdio>
#include <cstring>

static void die(const char *msg)
{
    int err = errno;
    fprintf(stderr, "[%d] %s: %s\n", err, msg, strerror(err));
    abort();
}

static void msg(const char *msg)
{
    fprintf(stderr, "[errno: %d] %s\n", errno, msg);
}

static void do_something(int connfd)
{
    char rbuf[64] = {};
    ssize_t n = read(connfd, rbuf, sizeof(rbuf) - 1);
    if (n < 0)
    {
        msg("read() error");
        return;
    }
    
    printf("client says: %s\n", rbuf);
    char wbuf[] = "world";
    ssize_t m = write(connfd, wbuf, strlen(wbuf));
    if (m < 0)
    {
        msg("write() error");
        return;
    }
}

int main()
{
    // listening socket using IPv4 with TCP
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
    {
        die("socket()");
    }

    int val = 1;
    // SO_REUSEADDR server program bind to same IP:port it was using before
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) < 0) // configure the socket
    {
        die("setsockopt()");
    }

    //bind
    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(1234); // port(host to network: convert 16bit value into network bytes order)
    addr.sin_addr.s_addr = htonl(0); // wildcard IP 0.0.0.0(convert 32bit int to network byte order)
    int rv = bind(fd, (const struct sockaddr *)&addr, sizeof(addr));
    if (rv) 
    {
        die("bind()");
    }

    // listen
    rv = listen(fd, SOMAXCONN); // size of queue, 4096 on LINUX 
    if (rv)
    {
        die("listen()");
    }

    while (true)
    {
        // accept
        struct sockaddr_in client_addr = {};
        socklen_t addrlen = sizeof(client_addr);
        int connfd = accept(fd, (struct sockaddr *)&client_addr, &addrlen);
        if (connfd < 0)
        {
            continue; // error
        }

        do_something(connfd);
        close(connfd);
    }
}