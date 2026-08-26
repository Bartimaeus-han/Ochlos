# Ochlos (오클로스)

> **Red Team Attack Simulation & Offensive Security Testing Environment**  
> 백엔드 웹 애플리케이션 및 보안 시스템([Bartimaeus](https://github.com/Bartimaeus-han/Bartimaeus_app))의 방어선(Hardening) 검증을 위한 C++20 기반 레드팀 DoS/PoC 시뮬레이션 프레임워크

---

## 🎯 1. 프로젝트 개요 (Overview)

**Ochlos**는 정보보안기사 국가기술자격 및 KISA 주요정보통신기반시설 기술적 취약점 분석·평가 기준을 바탕으로 **OSI 7계층(L3 Network ~ L7 Application)** 공격 도구를 C++20으로 직접 구현하고 계측하는 공격자 C2 / 레드팀 시뮬레이션 환경입니다.

블루팀 방어선(Screening Router, Rate Limiter 등)을 선행 타격하여 방어 기제의 유효성을 실증적으로 검증하고 시스템 리소스(소켓 고갈, 메모리, CPU, 지연시간)를 정밀 계측합니다.

---

## 🏗️ 2. 기술 스택 (Tech Stack)

* **언어 및 표준**: C++20 (Modern C++)
* **빌드 시스템**: CMake (3.15 이상), MSVC 2026 / GCC (Linux)
* **네트워크 인터페이스**:
  * **Windows 호스트**: Winsock2 (`ws2_32.lib`) - `SOCK_STREAM` Connection Starvation
  * **Linux 컨테이너**: POSIX Sockets, `CAP_NET_RAW` - `SOCK_RAW`, 수제 IP/TCP 패킷 조작 및 Half-Open SYN Flooding

---

## 📂 3. 디렉터리 구조 (Directory Layout)

```text
Ochlos/
├── CMakeLists.txt        # 하위 소스 파일 자동 탐색 및 빌드 타깃 생성
├── GEMINI.md             # Ochlos 프로젝트 AI 운영 지침
├── OCHLOS.md             # 공격 시나리오 실증 이력 및 진단 로그
├── README.md             # 프로젝트 개요 및 가이드
├── include/              # 공통 네트워크 헤더 및 체크섬 라이브러리
│   └── ochlos_net.hpp    # L3 IPHeader, L4 TCPHeader, PseudoHeader, Checksum
└── DoS/                  # 서비스 거부 공격(DoS/DDoS) 실증 도구
    ├── raw_tcp_syn_flooding.cpp  # L3/L4 Raw Socket Half-Open SYN Flooding
    └── tcp_syn_flooding.cpp      # L4 TCP Connection Starvation
```

---

## 🛠️ 4. 빌드 및 실행 방법 (Build & Run)

### Windows (MSVC / CMake)

```powershell
# 1. 빌드 디렉터리 생성 및 CMake 구성
cmake -B build -S .

# 2. 타깃 빌드
cmake --build build --config Release

# 3. 도구 실행
./build/tcp_syn_flooding.exe
```

---

## 📝 5. 공격 실증 이력 (Simulation Log)

상세 공격 시나리오 및 블루팀 방어 권고 사항은 [OCHLOS.md](OCHLOS.md)를 참고하세요.

* **[SCENARIO-01]** L4 TCP Connection Starvation & Screening Router 실시간 패킷 탐지 실증 완료

---

## ⚠️ 6. 면책 조항 (Disclaimer)

본 프로젝트는 **보안 교육, 방어 아키텍처 실증, 학술적 연구 목적**으로만 제작되었습니다. 허가받지 않은 대상에 대한 무단 공격 및 악용 행위는 법적 처벌을 받을 수 있으며, 개발자는 오용으로 인한 어떠한 책임도 지지 않습니다.
