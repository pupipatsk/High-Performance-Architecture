# Use official GCC image
FROM gcc:13.1.0

# Set working directory
WORKDIR /app

# Copy your source file into the container
COPY main-solver.cpp ./solver.cpp

# Compile the solver with full optimization
RUN g++ -O3 -march=native -funroll-loops -std=c++17 solver.cpp -o solver

# Entry point matches grading system expectations
ENTRYPOINT ["./solver"]
