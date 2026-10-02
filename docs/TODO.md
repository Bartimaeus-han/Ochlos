# Ochlos 인프라 및 공격 도구 개선 TODO (Backlog)

레드팀(Ochlos) 자체의 공격 도구·실행 인프라 백로그입니다. 표기 규칙은 `.claude/rules/todo.md`를 따릅니다.

공방전 전체의 다음 차례(어느 방어선을 다음에 때릴지, 블루팀이 무엇을 패치할지)는 이 파일에서 다루지 않습니다. 그 차례는 이 저장소 밖(상위 관제 폴더)에서 관리합니다.

---

- [x] **Ochlos 공격 도구의 Docker 컨테이너화 및 On-demand Runner 구성 (Infra-Attacker)**
  - **배경 및 목적**: 호스트 OS(Windows / macOS)의 Raw Socket 보안 제약(`SOCK_RAW`, IP Spoofing 차단 및 커널 차이)을 극복하고, 크로스 플랫폼 일관성 및 L3/L4 저수준 패킷 조작 환경 확보.
  - **개발 워크플로우**:
    - **코드 편집 & Git 관리**: 로컬 호스트(VS Code)에서 평소처럼 편집 및 GitHub 커밋 유지.
    - **실시간 바인드 마운트**: `docker-compose.yml`에 `ochlos` 서비스 정의 (`volumes: - .:/app`, `cap_add: - NET_RAW`).
    - **On-demand 실행**: `docker compose run --rm ochlos ...`으로 필요할 때만 컨테이너를 띄워 컴파일 & 타격 후 자동 소멸(`--rm`).
  - **기대 효과**: Linux 커널의 `CAP_NET_RAW` 권한을 활용해 순수 TCP SYN Flooding(Half-Open, 비정상 패킷 주입) 등 고도화된 DoS 공격을 제약 없이 실증 가능.

- [x] **L4 UDP Flooding 공격 도구 구현 및 3대 기초 Flooding 완성 (Attack-UDP)**
  - **배경 및 목적**: L3 ICMP Flooding(제어용), L4 TCP SYN Flooding(연결형)에 이어 L4 비연결형 프로토콜인 UDP Flooding 도구를 확보하여 네트워크 3대 기초 DoS 공격 라인업(TCP, ICMP, UDP)을 완전체로 구축.
  - **기술 구현 로드맵**:
    - `include/ochlos_net.hpp`에 RFC 768 표준 8바이트 `UDPHeader` 구조체 및 `craft_udp_packet()` 패킷 패킹 유틸리티 추가.
    - `DoS/raw_udp_flooding.cpp` 작성: 수제 Raw Socket 기반 비연결형 패킷 생성, 무작위 목적지/출발지 포트 변조, 동적 패킷 수 CLI 인자 지원, 고정밀 `steady_clock` 벤치마크 루프 적용.
    - 블루팀 스크리닝 라우터(웹 전용 HTTP 8080) 대상 UDP 무차별 패킷 주입 및 비인가 프로토콜 처리 거동(무차별 포워딩 vs Port Unreachable 유발 여부) 실측 완료 (SCENARIO-04).

- [ ] **미착수 스텁 정리 (Housekeeping)**
  - [ ] `DoS/smurf_attack.cpp` — 현재 주석 한 줄(`// Smurf Attack`)만 있는 빈 스텁. 착수하거나 삭제할지 결정 필요. `README.md`의 "구성" 항목은 이미 Smurf Attack을 구현된 것처럼 나열하고 있음
  - [ ] `ghidra.cpp` — 리버스 엔지니어링 실습용으로 루트에 둔 단독 파일. `CMakeLists.txt`의 자동 소스 탐색 대상에 포함되는지 확인하고, 유지한다면 둘 위치를 정할 필요 있음
