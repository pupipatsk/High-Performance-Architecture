# Dockerfile-cpp
FROM gcc:12

WORKDIR /project

# Copy everything into the container
COPY . .

# Install dependencies
RUN apt-get update && apt-get install -y time bc findutils

# Pre-compile all solvers and output them to /project/.tmp
RUN chmod +x compile-cpp.sh && ./compile-cpp.sh cpp-solver .tmp

# Optional: Build the dummy.cpp to pass dummy build ARG
ARG SOLVER=dummy.cpp
RUN g++ -O2 -fopenmp cpp-solver/${SOLVER} -o /dev/null || true

# Set default command to nothing — user runs container manually
CMD ["/bin/bash"]