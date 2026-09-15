# 기본 베이스는 최신 OS로 사용한다
FROM ubuntu:24.04

# 초기 패키지 설치 시, 질문은 모두 생략한다.
ENV DEBIAN_FRONTEND=noninteractive

# Install compiler(g++), build tools(make, cmake), packet tools(iproute2, tcpdump ...)
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    clangd \
    g++ \
    curl \
    wget \
    ca-certificates \
    git \
    iproute2 \
    net-tools \
    iputils-ping \
    tcpdump \
    && rm -rf /var/lib/apt/lists/*

# Container 내부 기본 작업 디렉터리 설정
WORKDIR /app

# cpprun(crun) 등록하기
RUN printf '#!/bin/bash\nTARGET=$1\nshift\nEXE="/tmp/$(basename "$TARGET" .cpp)"\ng++ -std=c++20 -I /app/include "$TARGET" -o "$EXE" && "$EXE" "$@"\n' > /usr/local/bin/cpprun \
    && chmod +x /usr/local/bin/cpprun


# Default execute command (Linux bash shell)
CMD ["/bin/bash"]