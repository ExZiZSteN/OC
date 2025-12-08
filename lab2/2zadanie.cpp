#include <iostream>
#include <vector>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <unistd.h>
#include <chrono>
#include <fstream>

using Matrix = std::vector<std::vector<int>>;

void multiplyRow(const Matrix& A, const Matrix& B, int* shared, int row) {
    int n = A.size();
    for (int j = 0; j < n; ++j) {
        int sum = 0;
        for (int k = 0; k < n; ++k) {
            sum += A[row][k] * B[k][j];
        }
        shared[row * n + j] = sum;
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: ./processes <matrix_size> <num_processes>\n";
        return 1;
    }

    int n = std::stoi(argv[1]);
    int p = std::stoi(argv[2]);

    if (p <= 0) {
        std::cerr << "num_processes must be > 0\n";
        return 1;
    }

    Matrix A(n, std::vector<int>(n));
    Matrix B(n, std::vector<int>(n));
    
    int value = 1;
    for (int i = 0; i < n; i++) 
        for (int j = 0; j < n; j++) 
            A[i][j] = value++;

    for (int i = 0; i < n; i++)
        B[i][i] = 1;

    int shm_id = shmget(IPC_PRIVATE, sizeof(int) * n * n, IPC_CREAT | 0666);
    int* shared = (int*) shmat(shm_id, nullptr, 0);

    std::vector<pid_t> pids;

    auto start = std::chrono::high_resolution_clock::now();

    int rows_per_proc = (n + p - 1) / p;

    int rows_per_proc = (n + p - 1) / p;

    for (int proc = 0; proc < p; proc++) {
        pid_t pid = fork();

        if (pid == 0) {
            int start_row = proc * rows_per_proc;
            int end_row = std::min(start_row + rows_per_proc, n);

            for (int r = start_row; r < end_row; r++)
                multiplyRow(A, B, shared, r);

            shmdt(shared);
            exit(0);
        }
        else {
            pids.push_back(pid);
        }
    }

    for (pid_t pid : pids)
        waitpid(pid, nullptr, 0);

    auto end = std::chrono::high_resolution_clock::now();
    long long elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    std::ofstream file("processes.csv", std::ios::app);
    file << elapsed << "," << p << "\n";
    file.close();

    std::cout << "Time: " << elapsed << " ms\n";

    shmdt(shared);
    shmctl(shm_id, IPC_RMID, nullptr);

    return 0;
}
