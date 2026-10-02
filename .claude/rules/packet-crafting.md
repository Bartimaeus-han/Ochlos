---
paths:
  - "DoS/**"
  - "include/**"
---

# 크로스 플랫폼 패킷 조작 및 바이너리 정렬 원칙

- **소켓/시스템 헤더 분기 (`#ifdef _WIN32` vs POSIX)**: Windows(`<WinSock2.h>`, `<ws2tcpip.h>`)와 Linux/POSIX(`<arpa/inet.h>`, `<netinet/in.h>`, `<unistd.h>`)를 명확히 분기해 MSVC, GCC, Clang에서 경고/오류 없이 빌드되게 합니다.
- **구조체 1바이트 패킹 (`#pragma pack(push, 1)`)**: 컴파일러 자동 패딩을 차단해 RFC 표준 IPv4(20B) + TCP(20B) = 정확히 40바이트 레이아웃을 강제 보장합니다.
- **리틀 엔디안 비트필드 배치**: x86_64/ARM64 메모리 저장 방식에 맞춰 IPv4 첫 바이트(`ihl: 4` 하위, `version: 4` 상위)를 LSB-first로 배치합니다. TCP 플래그는 `uint8_t flags` 공용체로 1바이트 통째 대입해 비트필드 해석 오차를 원천 차단합니다.
- **엔디안 변환 일관성**: 포트(`src_port`, `dest`)·시퀀스 번호 등 호스트 값은 `htons()`, `htonl()`로 Network Byte Order(Big Endian)로 변환합니다.
- **2단계 체크섬 계산 (RFC 791 / 768 / 793)**: IP 헤더 체크섬과 별도로, 12바이트 의사 헤더(Pseudo Header)를 구성해 TCP/UDP 체크섬을 계산합니다. 조립은 `craft_tcp_packet()` / `craft_udp_packet()`으로 캡슐화합니다.
- **타깃 대역 제한**: 목적지와 스푸핑 출발지는 사설(RFC 1918) · 루프백 · RFC 5737 문서 전용 대역 · Docker 컨테이너 이름만 씁니다. 공인 라우팅 대역은 어느 쪽으로도 쓰지 않습니다.
