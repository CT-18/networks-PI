#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <iostream>

int demon_routine()
{
    auto port = 8888;
    auto in = socket(AF_INET, SOCK_STREAM, 0);
    if (in < 0) {
        //std::cerr << "Socket creation failed: " << strerror(errno) << std::endl;
        return 1;
    } 

    struct sockaddr_in localhost;
    memset(&localhost, 0, sizeof(struct sockaddr_in));
    localhost.sin_family = AF_INET;
    localhost.sin_port = htons(port);
    localhost.sin_addr.s_addr = htonl(INADDR_ANY);
 
    int res = bind(in, (struct sockaddr*) &localhost, sizeof(localhost));
    if (res < 0) {
        //std::cerr << "Bind error: " << strerror(errno) << std::endl;
	return 1;    
    }
 
    struct linger linger_opt = { 1, 0 };
    setsockopt(in, SOL_SOCKET, SO_LINGER, &linger_opt, sizeof(linger_opt));
 
    res = listen(in, 1);
    if (res < 0) {
        //std::cerr << "Listen error: " << strerror(errno) << std::endl;
        return 1;
    }
 
    struct sockaddr_in peeraddr;
    socklen_t peeraddr_len;
    int peer = accept(in, (struct sockaddr*) &peeraddr, &peeraddr_len);
    if (peer < 0) {
        //std::cerr << "Accept error: " << strerror(errno) << std::endl;
        return 1;
    }
 
    
    /*std::cout << "Connection from IP "
              << ( ( ntohl(peeraddr.sin_addr.s_addr) >> 24) & 0xff ) << "."
              << ( ( ntohl(peeraddr.sin_addr.s_addr) >> 16) & 0xff ) << "."
              << ( ( ntohl(peeraddr.sin_addr.s_addr) >> 8) & 0xff )  << "."
              <<   ( ntohl(peeraddr.sin_addr.s_addr) & 0xff ) << ", port "
              << ntohs(peeraddr.sin_port);
    */
    char buffer[1024];
    res = read(peer, buffer, 1023);
    if (res < 0) {
        //std::cerr << "Read error: " << strerror(errno) << std::endl;
        return 1;
    }
    buffer[res] = 0;
    
    if (std::string(buffer) == "{'command': 'start'}") {
        //start stream
        std::string responce = "{'name': %имя машины%, 'link': 'http://ip-в-подсети-запросившего/live.m3u8'}";
        write(peer, responce.data(), responce.size());
	close(peer); 
    } else {
        // ignore
        close(peer);
    }

    return 0;
}

int main()
{
    auto pid = fork();

    if (pid == -1) {
        std::cerr << "Fork error: " << strerror(errno) << std::endl;
        return 1;
    }

    if (!pid) {
        umask(0);
        setsid();
        chdir("/");
        
        close(0); // in
        close(1); // out
        close(2); // err

        return demon_routine();
    }
    else {
        return 0;
    }
}
