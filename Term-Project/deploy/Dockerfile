# Docker-Image-name: power-plant-solver
FROM gcc:12
RUN apt-get update && apt-get install -y libgomp1
WORKDIR /app
COPY cpp-solver/ /app/cpp-solver/
COPY compile-cpp.sh /app/
RUN chmod +x compile-cpp.sh && ./compile-cpp.sh cpp-solver /app/bin
ENTRYPOINT ["/bin/bash"]