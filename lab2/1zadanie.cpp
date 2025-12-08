#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <fstream>

using Matrix = std::vector<std::vector<long long>>;

void multiplyRow(const Matrix& A, const Matrix& B, Matrix& C, std::size_t row) {
    std::size_t n = A.size();
    for (std::size_t j = 0; j < n; ++j) {
        long long sum = 0;
        for (std::size_t k = 0; k < n; ++k) {
            sum += A[row][k] * B[k][j];
        }
        C[row][j] = sum;
    }
}

Matrix multiplyMatrixThreaded(const Matrix& A, const Matrix& B) {
    std::size_t n = A.size();             
    Matrix C(n, std::vector<long long>(n, 0));

    std::vector<std::thread> threads;
    for (std::size_t i = 0; i < n; ++i)
        threads.emplace_back(multiplyRow, std::cref(A), std::cref(B), std::ref(C), i);

    for (auto& t : threads) t.join();
    return C;
}

int main() {
    // Matrix A = {
    //     {1,2},
    //     {3,4}
    // };
    // Matrix A = {
    //     {1, 2, 3},
    //     {4, 5, 6},
    //     {7, 8, 9}
    // };
    // Matrix A = {
    //     {1, 2, 3, 2},
    //     {4, 5, 6, 2},
    //     {7, 8, 9, 2},
    //     {4, 5, 6, 7}
    // };
    Matrix A = {
        {1,2,3,4,5},
        {6,7,8,9,10},
        {11,12,13,14,15},
        {16,17,18,19,20},
        {21,22,23,24,25}
    };
    // Matrix B = {
    //     {2, 0, 0, 0},
    //     {0, 2, 0, 0},
    //     {0, 0, 2, 0},
    //     {0, 0, 0, 2}
    // };
    // Matrix B = {
    //     {1,0,0},
    //     {0,1,0},
    //     {0,0,1}
    // };
    // Matrix B = {
    //     {1,2},
    //     {3,4}
    // };
    Matrix B = {
        {1,0,0,0,0},
        {0,1,0,0,0},
        {0,0,1,0,0},
        {0,0,0,1,0},
        {0,0,0,0,1}
    };
    auto start = std::chrono::high_resolution_clock::now();
    Matrix C = multiplyMatrixThreaded(A, B);
    auto end = std::chrono::high_resolution_clock::now();

    long long elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    std::cout << "Result:" << std::endl;
    for (auto& row : C){
        for (auto& x : row){
            std::cout << x << "\t";
        }
        std::cout << std::endl;
    }
    std::ofstream file("threads.csv", std::ios::app);
    file << elapsed << "," << A.size() << "\n";
    file.close();

    std::cout << "Time: " << elapsed << " microseconds\n";
}
