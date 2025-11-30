#include <iostream>
#include <vector>
#include <thread>


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
    if (A.empty() || B.empty()) {
        throw std::invalid_argument("Матрицы не должны быть пустыми.");
    }

    std::size_t n = A.size();             

    Matrix C(n, std::vector<long long>(n, 0));
    std::vector<std::thread> threads;

    for (std::size_t i = 0; i < n; ++i) {
        threads.emplace_back(multiplyRow, std::cref(A), std::cref(B), std::ref(C), i);
    }
    for (auto& t : threads) t.join();

    return C;
}

int main() {
    Matrix A = {
        {1, 2, 3},
        {4, 5, 6},
        {7, 8, 9}
    };
    Matrix B = {
        {2, 0, 0},
        {0, 2, 0},
        {0, 0, 2}
    };
    Matrix C = multiplyMatrixThreaded(A, B);
    std::cout << "Result:\n";
    for (const auto& row : C) {
        for (auto v : row) std::cout << v << "\t";
        std::cout << "\n";
    }
}