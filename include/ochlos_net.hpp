#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>

#ifdef _WIN32
#include <WinSock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#endif

#pragma pack(push, 1)

// TCP 표준 6대 제어 플래그 비트마스크 상수 정의 (RFC 793)
enum TCPFlag : uint8_t {
    FLAG_FIN = 0x01, // 0000 0001 (연결 종료)
    FLAG_SYN = 0x02, // 0000 0010 (연결 동기화)
    FLAG_RST = 0x04, // 0000 0100 (연결 즉시 리셋)
    FLAG_PSH = 0x08, // 0000 1000 (버퍼 즉시 푸시)
    FLAG_ACK = 0x10, // 0001 0000 (승인 응답)
    FLAG_URG = 0x20  // 0010 0000 (긴급 데이터)
};

// RFC 792
enum ICMPType : uint8_t {
    ICMP_TYPE_ECHO_REPLY = 0,  // Ping 응답
    ICMP_TYPE_ECHO_REQUEST = 8 // Ping 요청
};

// ICMP(Internet Control Message Protocol) Header
struct ICMPHeader {
    uint8_t type;
    uint8_t code;
    uint16_t checksum;
    uint16_t id;
    uint16_t sequence;
};

// IPv4 표준 헤더 구조체 - 20바이트 (IPv4 Standard Header structure - 20 Bytes)
struct IPHeader {
    uint8_t ihl : 4;     // 헤더 길이 (Header Length: 5 = 20바이트)
    uint8_t version : 4; // 버전 (Version: 4 = IPv4)
    uint8_t tos;         // 서비스 유형 (Type of Service)
    uint16_t tot_len;    // 전체 패킷 길이 (Total Length)
    uint16_t id;         // 식별자 (Identification)
    uint16_t frag_off;   // 단편화 플래그/오프셋 (Fragment Offset)
    uint8_t ttl;         // 생존 시간 (Time to Live)
    uint8_t protocol;    // 상위 프로토콜 (Protocol, TCP=6)
    uint16_t check;      // IP 체크섬 (IP Header Checksum)
    uint32_t saddr;      // 출발지 IP 주소 (Source IP Address)
    uint32_t daddr;      // 목적지 IP 주소 (Destination IP Address)
};

// TCP 표준 헤더 구조체 - 20바이트 (TCP Standard Header structure - 20 Bytes)
struct TCPHeader {
    uint16_t source;  // 출발지 포트 (Source Port)
    uint16_t dest;    // 목적지 포트 (Destination Port)
    uint32_t seq;     // 순서 번호 (Sequence Number)
    uint32_t ack_seq; // 확인 응답 번호 (Acknowledgment Number)
    uint8_t res1 : 4; // 예약 영역 (Reserved bits)
    uint8_t doff : 4; // 데이터 오프셋/헤더 길이 (Data Offset: 5 = 20바이트)
    union {
        uint8_t flags;
        struct {
            uint8_t fin : 1;
            uint8_t syn : 1;
            uint8_t rst : 1;
            uint8_t psh : 1;
            uint8_t ack : 1;
            uint8_t urg : 1;
            uint8_t res2 : 2; // 예약 비트
        };
    };
    uint16_t window;  // 수신 윈도우 크기 (Window Size)
    uint16_t check;   // TCP 체크섬 (TCP Checksum)
    uint16_t urg_ptr; // 긴급 포인터 (Urgent Pointer)
};

// TCP 체크섬 계산 전용 의조 헤더 구조체 - 12바이트 (TCP Pseudo Header for checksum - 12 Bytes)
struct PseudoHeader {
    uint32_t src_ip;  // 출발지 IP 주소 (Source IP)
    uint32_t dst_ip;  // 목적지 IP 주소 (Destination IP)
    uint8_t reserved; // 0x00 정렬 패딩 (Zero Padding)
    uint8_t protocol; // 프로토콜 번호 (Protocol TCP = 6)
    uint16_t tcp_len; // TCP 헤더 + 데이터 길이 (TCP Header + Payload Length)
};

#pragma pack(pop)

// 16비트 인터넷 체크섬 계산 함수 (Compute 16-bit Internet Checksum)
inline uint16_t calculate_checksum(uint16_t *ptr, int nbytes) {
    uint32_t sum = 0;
    uint16_t oddbyte = 0;

    while (nbytes > 1) {
        sum += *ptr++;
        nbytes -= 2;
    }

    if (nbytes == 1) {
        *(uint8_t *)&oddbyte = *(uint8_t *)ptr;
        sum += oddbyte;
    }

    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    return static_cast<uint16_t>(~sum);
}

// Pseudo Header 생성 헬퍼 함수 (Create and populate PseudoHeader)
inline PseudoHeader create_pseudo_header(uint32_t src_ip, uint32_t dst_ip, uint16_t tcp_len) {
    PseudoHeader header;
    header.src_ip = src_ip;
    header.dst_ip = dst_ip;
    header.reserved = 0;
    header.protocol = IPPROTO_TCP;
    header.tcp_len = htons(tcp_len);
    return header;
}

// tcp packet
inline void craft_tcp_packet(char *buffer, uint32_t src_ip, uint32_t dst_ip, uint16_t src_port, uint16_t dst_port, uint8_t flags = FLAG_SYN, uint32_t seq = 0, uint32_t ack_seq = 0, uint16_t packet_id = 54321) {
    // 1. memory initialize to 0 & mapping L3/L4 header pointer
    std::memset(buffer, 0, sizeof(IPHeader) + sizeof(TCPHeader));
    auto *ip = reinterpret_cast<IPHeader *>(buffer);
    auto *tcp = reinterpret_cast<TCPHeader *>(buffer + sizeof(IPHeader));

    // 2. L3 IPv4 header field 구성 (20byte)
    ip->ihl = 5;                                               // Header Lengths는 5*4
    ip->version = 4;                                           // IPv4
    ip->tos = 0;                                               // Best Effort
    ip->tot_len = htons(sizeof(IPHeader) + sizeof(TCPHeader)); // Total Length : 40byte
    ip->id = htons(packet_id);
    ip->frag_off = 0;
    ip->ttl = 64;               // TTL Default value
    ip->protocol = IPPROTO_TCP; // TCP Protocol (6)
    ip->saddr = src_ip;
    ip->daddr = dst_ip;
    ip->check = 0;
    ip->check = calculate_checksum(reinterpret_cast<uint16_t *>(ip), sizeof(IPHeader));

    // 3. L4 TCP header field 구성 (20byte)
    tcp->source = htons(src_port);
    tcp->dest = htons(dst_port);
    tcp->seq = htonl(seq);
    tcp->ack_seq = htonl(ack_seq);
    tcp->doff = 5; // Data Offset: 5*4=20
    tcp->flags = flags;
    tcp->window = htons(65535);
    tcp->check = 0;

    // 4. calcuate TCP checksum via Pseudo Header
    PseudoHeader pseudo_header = create_pseudo_header(src_ip, dst_ip, sizeof(TCPHeader));
    char pseudo_buffer[sizeof(PseudoHeader) + sizeof(TCPHeader)];
    std::memcpy(pseudo_buffer, &pseudo_header, sizeof(PseudoHeader));
    std::memcpy(pseudo_buffer + sizeof(PseudoHeader), tcp, sizeof(TCPHeader));
    tcp->check = calculate_checksum(reinterpret_cast<uint16_t *>(pseudo_buffer), sizeof(pseudo_buffer));
}

inline void craft_icmp_packet(
    char *buffer,
    uint32_t src_ip,
    uint32_t dst_ip,
    uint8_t type = ICMP_TYPE_ECHO_REQUEST,
    uint8_t code = 0,
    uint16_t id = 1234,
    uint16_t sequence = 0,
    uint16_t packet_id = 54321) {

    //  1. initialize buffer & pointer mapping
    std::memset(buffer, 0, sizeof(IPHeader) + sizeof(ICMPHeader));
    auto *ip = reinterpret_cast<IPHeader *>(buffer);
    auto *icmp = reinterpret_cast<ICMPHeader *>(buffer + sizeof(IPHeader));

    // 2. L3 IPv4 Header 구성 (20 byte)
    ip->ihl = 5;
    ip->version = 4;
    ip->tos = 0;
    ip->tot_len = htons(sizeof(IPHeader) + sizeof(ICMPHeader)); // 28 bytes

    ip->id = htons(packet_id);
    ip->frag_off = 0;
    ip->ttl = 64;
    ip->protocol = IPPROTO_ICMP; // icmp protocol
    ip->saddr = src_ip;
    ip->daddr = dst_ip;
    ip->check = 0;
    ip->check = calculate_checksum(reinterpret_cast<uint16_t *>(ip), sizeof(IPHeader));

    // 3. ICMP Header 구성 (8byte)
    icmp->type = type;
    icmp->code = code;
    icmp->id = htons(id);
    icmp->sequence = htons(sequence);
    icmp->checksum = 0;
    // 위에서 먼저 0으로 초기화 하는 이유는 calculate 과정에서 checksum 값도 필요하기 때문이다.
    icmp->checksum = calculate_checksum(reinterpret_cast<uint16_t *>(icmp), sizeof(ICMPHeader));
}