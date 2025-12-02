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

int main() {
    Matrix A = {
        {1,2,3},
        {4,5,6},
        {7,8,9}
    };
    Matrix B = {
        {2,0,0},
        {0,2,0},
        {0,0,2}
    };

    int n = A.size();
    int shm_id = shmget(IPC_PRIVATE, sizeof(int) * n * n, IPC_CREAT | 0666);
    int* shared = (int*) shmat(shm_id, nullptr, 0);

    std::vector<pid_t> pids;

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < n; i++) {
        pid_t pid = fork();

        if (pid == 0) {
            multiplyRow(A, B, shared, i);
            shmdt(shared);
            exit(0);
        } else {
            pids.push_back(pid);
        }
    }

    for (pid_t pid : pids)
        waitpid(pid, nullptr, 0);

    auto end = std::chrono::high_resolution_clock::now();
    long long elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    std::ofstream file("processes.csv", std::ios::app);
    file << elapsed << "\n";
    file.close();
    
    std::cout << "Result:" << std::endl;
    
    for (int i = 0 ; i < n; i++){
        for (int j = 0;j < n; j++){
            std::cout << shared[i * n + j] << "\t";
        }
        std::cout << std::endl;
    }
    std::cout << "Time (processes): " << elapsed << " microseconds\n";

    shmdt(shared);
    shmctl(shm_id, IPC_RMID, nullptr);

    return 0;
}
