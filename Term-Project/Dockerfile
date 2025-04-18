# Image-name: power-plant-solver
FROM gcc:12

WORKDIR /app

COPY solve.cpp .

# RUN g++ -O3 -std=c++17 solve.cpp -o solve
# RUN g++ -O3 -std=c++17 -fopenmp solve.cpp -o solve
RUN g++ -O3 -march=native -funroll-loops -ftree-vectorize -fopenmp solve.cpp -o solve
# RUN g++ -O3 solve.cpp -o solve -std=c++17 -I /usr/local/include -L /usr/local/lib -lortools -lpthread

ENTRYPOINT ["./solve"]
