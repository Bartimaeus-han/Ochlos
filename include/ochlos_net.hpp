#pragma once

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

// IPv4 표준 헤더 구조체 - 20바이트 (IPv4 Standard Header structure - 20 Bytes)
struct IPHeader {
    uint8_t  ihl : 4;       // 헤더 길이 (Header Length: 5 = 20바이트)
    uint8_t  version : 4;   // 버전 (Version: 4 = IPv4)
    uint8_t  tos;           // 서비스 유형 (Type of Service)
    uint16_t tot_len;       // 전체 패킷 길이 (Total Length)
    uint16_t id;            // 식별자 (Identification)
    uint16_t frag_off;      // 단편화 플래그/오프셋 (Fragment Offset)
    uint8_t  ttl;           // 생존 시간 (Time to Live)
    uint8_t  protocol;      // 상위 프로토콜 (Protocol, TCP=6)
    uint16_t check;         // IP 체크섬 (IP Header Checksum)
    uint32_t saddr;         // 출발지 IP 주소 (Source IP Address)
    uint32_t daddr;         // 목적지 IP 주소 (Destination IP Address)
};

// TCP 표준 헤더 구조체 - 20바이트 (TCP Standard Header structure - 20 Bytes)
struct TCPHeader {
    uint16_t source;        // 출발지 포트 (Source Port)
    uint16_t dest;          // 목적지 포트 (Destination Port)
    uint32_t seq;           // 순서 번호 (Sequence Number)
    uint32_t ack_seq;       // 확인 응답 번호 (Acknowledgment Number)
    uint8_t  res1 : 4;      // 예약 영역 (Reserved bits)
    uint8_t  doff : 4;      // 데이터 오프셋/헤더 길이 (Data Offset: 5 = 20바이트)
    uint8_t  fin : 1;       // FIN 플래그 (FIN Flag)
    uint8_t  syn : 1;       // SYN 플래그 (SYN Flag)
    uint8_t  rst : 1;       // RST 플래그 (RST Flag)
    uint8_t  psh : 1;       // PSH 플래그 (PSH Flag)
    uint8_t  ack : 1;       // ACK 플래그 (ACK Flag)
    uint8_t  urg : 1;       // URG 플래그 (URG Flag)
    uint8_t  res2 : 2;      // 예약 영역 (Reserved bits)
    uint16_t window;        // 수신 윈도우 크기 (Window Size)
    uint16_t check;         // TCP 체크섬 (TCP Checksum)
    uint16_t urg_ptr;       // 긴급 포인터 (Urgent Pointer)
};

// TCP 체크섬 계산 전용 의조 헤더 구조체 - 12바이트 (TCP Pseudo Header for checksum - 12 Bytes)
struct PseudoHeader {
    uint32_t src_ip;        // 출발지 IP 주소 (Source IP)
    uint32_t dst_ip;        // 목적지 IP 주소 (Destination IP)
    uint8_t  reserved;      // 0x00 정렬 패딩 (Zero Padding)
    uint8_t  protocol;      // 프로토콜 번호 (Protocol TCP = 6)
    uint16_t tcp_len;       // TCP 헤더 + 데이터 길이 (TCP Header + Payload Length)
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
