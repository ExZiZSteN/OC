#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <fstream>
#include <cstdlib>

using Matrix = std::vector<std::vector<long long>>;


void multiplyPart(const Matrix& A, const Matrix& B, Matrix& C, int startRow, int endRow) {
    int n = A.size();
    for (int i = startRow; i < endRow; i++) {
        for (int j = 0; j < n; j++) {
            long long sum = 0;
            for (int k = 0; k < n; k++) {
                sum += A[i][k] * B[k][j];
            }
            C[i][j] = sum;
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: ./threads <N> <num_threads>\n";
        return 1;
    }

    int N = std::atoi(argv[1]);
    int T = std::atoi(argv[2]);

    if (T < 1) T = 1;
    if (T > N) T = N; 

    
    Matrix A(N, std::vector<long long>(N));
    Matrix B(N, std::vector<long long>(N));
    Matrix C(N, std::vector<long long>(N, 0));

    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++) {
            A[i][j] = rand() % 10;
            B[i][j] = rand() % 10;
        }

    std::vector<std::thread> threads;
    threads.reserve(T);

    int rowsPerThread = N / T;
    int extra = N % T;

    int current = 0;

    auto start = std::chrono::high_resolution_clock::now();

    for (int t = 0; t < T; t++) {
        int startRow = current;
        int endRow = startRow + rowsPerThread + (t < extra ? 1 : 0);
        current = endRow;

        threads.emplace_back(multiplyPart, std::cref(A), std::cref(B), std::ref(C), startRow, endRow);
    }

    
    for (auto& th : threads) th.join();

    auto end = std::chrono::high_resolution_clock::now();
    long long elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    std::cout << "Matrix multiplied. First row result:\n";
    for (auto x : C[0]) std::cout << x << " ";
    std::cout << "\n";


    std::ofstream file("threads.csv", std::ios::app);
    file << elapsed << "," << T << "\n";
    file.close();

    std::cout << "Time: " << elapsed << " miilliseconds\n";
}