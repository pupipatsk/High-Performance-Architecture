FROM ubuntu:22.04

RUN apt-get update && apt-get install -y g++ cmake

WORKDIR /opt

COPY or-tools_amd64_ubuntu-22.04_cpp_v9.11.4210.tar.gz .

RUN tar -xzf or-tools_amd64_ubuntu-22.04_cpp_v9.11.4210.tar.gz
RUN mkdir /opt/ortools
RUN mv or-tools_*/* /opt/ortools/
RUN rm or-tools_amd64_ubuntu-22.04_cpp_v9.11.4210.tar.gz

ENV LD_LIBRARY_PATH="/opt/ortools/lib:${LD_LIBRARY_PATH}"

WORKDIR /app
COPY solve.cpp .

RUN g++ solve.cpp -std=c++17 -O3 -march=native -funroll-loops -flto -fomit-frame-pointer \
    -I/opt/ortools/include -L/opt/ortools/lib -lortools -lpthread -o solver

ENTRYPOINT ["./solver"]
