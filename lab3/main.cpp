#define _GNU_SOURCE
#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include <chrono>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <time.h>

const size_t FILE_SIZE = 100ULL * 1024 * 1024;
const size_t PAGE_ACCESSES = 1000;
const int RUNS = 5;

long diff_ns(const timespec& a, const timespec& b) {
    return (b.tv_sec - a.tv_sec) * 1'000'000'000L +
           (b.tv_nsec - a.tv_nsec);
}

int main() {
    const long page_size = sysconf(_SC_PAGESIZE);
    const size_t total_pages = FILE_SIZE / page_size;

    std::cout << "Page size: " << page_size << " bytes\n";
    std::cout << "Total pages: " << total_pages << "\n";

    long total_first_ns = 0;
    long total_second_ns = 0;
    long total_major_1 = 0, total_minor_1 = 0;
    long total_major_2 = 0, total_minor_2 = 0;

    std::mt19937_64 rng(std::random_device{}());
    std::uniform_int_distribution<int> byte_dist(0, 255);
    std::uniform_int_distribution<size_t> page_dist(0, total_pages - 1);

    for (int run = 0; run < RUNS; ++run) {
        std::cout << "\nRun " << run + 1 << "\n";

        int fd = open("mmap_test.bin", O_CREAT | O_RDWR | O_TRUNC, 0600);
        if (fd < 0) {
            perror("open");
            return 1;
        }

        if (ftruncate(fd, FILE_SIZE) != 0) {
            perror("ftruncate");
            return 1;
        }

        std::vector<char> buffer(page_size);
        for (size_t i = 0; i < total_pages; ++i) {
            for (long j = 0; j < page_size; ++j)
                buffer[j] = static_cast<char>(byte_dist(rng));
            write(fd, buffer.data(), page_size);
        }

        char* map = static_cast<char*>(
            mmap(nullptr, FILE_SIZE, PROT_READ, MAP_PRIVATE, fd, 0)
        );

        posix_fadvise(fd,0,FILE_SIZE,POSIX_FADV_DONTNEED);

        if (map == MAP_FAILED) {
            perror("mmap");
            return 1;
        }

        std::vector<size_t> pages(PAGE_ACCESSES);
        for (auto& p : pages)
            p = page_dist(rng);

        std::shuffle(pages.begin(), pages.end(), rng);

        volatile char sink;

        struct rusage r1_before{}, r1_after{};
        struct rusage r2_before{}, r2_after{};
        timespec t1{}, t2{};

        
        getrusage(RUSAGE_SELF, &r1_before);
        clock_gettime(CLOCK_MONOTONIC, &t1);
 
        for (size_t p : pages) {
            sink = map[p * page_size];
        }

        clock_gettime(CLOCK_MONOTONIC, &t2);
        getrusage(RUSAGE_SELF, &r1_after);

        long first_ns = diff_ns(t1, t2);
        long major1 = r1_after.ru_majflt - r1_before.ru_majflt;
        long minor1 = r1_after.ru_minflt - r1_before.ru_minflt;

        
        getrusage(RUSAGE_SELF, &r2_before);
        clock_gettime(CLOCK_MONOTONIC, &t1);

        for (size_t p : pages) {
            sink = map[p * page_size];
        }

        clock_gettime(CLOCK_MONOTONIC, &t2);
        getrusage(RUSAGE_SELF, &r2_after);

        long second_ns = diff_ns(t1, t2);
        long major2 = r2_after.ru_majflt - r2_before.ru_majflt;
        long minor2 = r2_after.ru_minflt - r2_before.ru_minflt;

        std::cout << "First access:  " << first_ns
                  << " ns | major=" << major1
                  << " minor=" << minor1 << "\n";

        std::cout << "Second access: " << second_ns
                  << " ns | major=" << major2
                  << " minor=" << minor2 << "\n";

        total_first_ns += first_ns;
        total_second_ns += second_ns;
        total_major_1 += major1;
        total_minor_1 += minor1;
        total_major_2 += major2;
        total_minor_2 += minor2;

        munmap(map, FILE_SIZE);
        close(fd);
        unlink("mmap_test.bin");
    }


    std::cout << "\n=== AVERAGE ===\n";
    std::cout << "First access:  " << total_first_ns / RUNS << " ns\n";
    std::cout << "Second access: " << total_second_ns / RUNS << " ns\n";
    std::cout << "Major faults: first=" << total_major_1 / RUNS
              << " second=" << total_major_2 / RUNS << "\n";
    std::cout << "Minor faults: first=" << total_minor_1 / RUNS
              << " second=" << total_minor_2 / RUNS << "\n";

    return 0;
}