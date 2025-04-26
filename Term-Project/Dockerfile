FROM ubuntu:22.04

# Install build tools
RUN apt-get update && apt-get install -y g++ cmake git wget unzip python3

# Copy OR-Tools source
WORKDIR /opt
COPY or-tools-stable /opt/or-tools

# Build OR-Tools
WORKDIR /opt/or-tools
RUN cmake -S . -B build -DBUILD_DEPS=ON
RUN cmake --build build --parallel

# Copy your C++ code
WORKDIR /app
COPY solve.cpp .

# Compile linking against static library
RUN g++ solve.cpp /opt/or-tools/build/libortools.a -std=c++17 -I/opt/or-tools/build -I/opt/or-tools/include -lpthread -o solver

# Set entrypoint
ENTRYPOINT ["./solver"]