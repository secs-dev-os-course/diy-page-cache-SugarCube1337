#include <random>
#include <iostream>
#include <chrono>
#include <fstream>
#include <Windows.h>
#include <vector>
#include <stdexcept>
#include "Cache/CacheAPI.h"

namespace os_lab2::app {

    void createFile(const std::string &filename, size_t size) {
        std::ofstream file(filename, std::ios::binary | std::ios::trunc);
        if (!file) {
            throw std::runtime_error("Failed to create file");
        }

        std::vector<char> buffer(1024, 'A');
        size_t remaining = size;
        while (remaining > 0) {
            size_t chunk = (((remaining) < (buffer.size())) ? (remaining) : (buffer.size()));
            file.write(buffer.data(), chunk);
            remaining -= chunk;
        }
    }

    void readFileWithCache() {
        const std::string filename = "testfile_with_cache.dat";
        const size_t fileSize = 1L * 1024 * 1024 * 1024; // 1 GB
        const size_t blockSize = 4096; // Размер блока чтения
        const int numReads = 10;

        static bool fileCreated = false;
        if (!fileCreated) {
            createFile(filename, fileSize);
            fileCreated = true;
        }

        // Открытие файла с включением кеширования (обычное поведение по умолчанию)
        HANDLE hFile = CreateFile(filename.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (hFile == INVALID_HANDLE_VALUE) {
            throw std::runtime_error("Failed to open file with cache");
        }

        std::vector<char> buffer(blockSize);
        std::mt19937 rng(42);
        std::uniform_int_distribution<size_t> dist(0, fileSize / blockSize - 1);

        auto start_time = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < numReads; ++i) {
            size_t blockIndex = dist(rng);
            LARGE_INTEGER li;
            li.QuadPart = blockIndex * blockSize;
            SetFilePointerEx(hFile, li, nullptr, FILE_BEGIN);

            DWORD bytesRead;
            if (!ReadFile(hFile, buffer.data(), static_cast<DWORD>(blockSize), &bytesRead, nullptr)) {
                CloseHandle(hFile);
                throw std::runtime_error("Error reading file with cache");
            }
        }
        auto end_time = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duration = end_time - start_time;

        CloseHandle(hFile);
        std::cout << "Reading with cache took " << duration.count() * 1e3 << " ms\n";
    }

    void readFileWithoutCache() {
        const std::string filename = "testfile_no_cache.dat";
        const size_t fileSize = 1L * 1024 * 1024 * 1024; // 1 GB
        const size_t blockSize = 4096; // Размер блока чтения
        const int numReads = 10;

        static bool fileCreated = false;
        if (!fileCreated) {
            createFile(filename, fileSize);
            fileCreated = true;
        }

        // Открытие файла без кеширования
        HANDLE hFile = CreateFile(filename.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_NO_BUFFERING, nullptr);
        if (hFile == INVALID_HANDLE_VALUE) {
            throw std::runtime_error("Failed to open file without cache");
        }

        std::vector<char> buffer(blockSize);
        std::mt19937 rng(42);
        std::uniform_int_distribution<size_t> dist(0, fileSize / blockSize - 1);

        auto start_time = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < numReads; ++i) {
            size_t blockIndex = dist(rng);
            LARGE_INTEGER li;
            li.QuadPart = blockIndex * blockSize;
            SetFilePointerEx(hFile, li, nullptr, FILE_BEGIN);

            DWORD bytesRead;
            if (!ReadFile(hFile, buffer.data(), static_cast<DWORD>(blockSize), &bytesRead, nullptr)) {
                CloseHandle(hFile);
                throw std::runtime_error("Error reading file without cache");
            }
        }
        auto end_time = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duration = end_time - start_time;

        CloseHandle(hFile);
        std::cout << "Reading without cache took " << duration.count() * 1e3 << " ms\n";
    }

    void readFileWithFIFOCache() {
        const std::string filename = "testfile_with_fifo_cache.dat";
        const size_t fileSize = 1L * 1024 * 1024 * 1024; // 1 GB
        const size_t blockSize = 4096; // Размер блока чтения
        const int numReads = 10;

        static bool fileCreated = false;
        if (!fileCreated) {
            createFile(filename, fileSize);
            fileCreated = true;
        }

        int fd = lab2_open(filename.c_str());
        if (fd == -1) {
            throw std::runtime_error("Failed to open file with cache");
        }

        std::vector<char> buffer(blockSize);
        std::mt19937 rng(42);
        std::uniform_int_distribution<size_t> dist(0, fileSize / blockSize - 1);

        auto start_time = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < numReads; ++i) {
            size_t blockIndex = dist(rng);
            if (lab2_lseek(fd, blockIndex * blockSize, SEEK_SET) == -1) {
                lab2_close(fd);
                throw std::runtime_error("Error seeking file with cache");
            }
            if (lab2_read(fd, buffer.data(), blockSize) == -1) {
                lab2_close(fd);
                throw std::runtime_error("Error reading file with cache");
            }
        }
        auto end_time = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duration = end_time - start_time;

        lab2_close(fd);
        std::cout << "Reading with FIFO cache took " << duration.count() * 1e3 << " ms\n";
    }


}

int main() {
    try {
        std::cout << "\nRunning without cache:\n";
        os_lab2::app::readFileWithoutCache();

        std::cout << "Running with cache:\n";
        os_lab2::app::readFileWithCache();

        std::cout << "Running with FIFO cache:\n";
        os_lab2::app::readFileWithFIFOCache();
    } catch (std::exception &e) {
        std::cout << e.what() << std::endl;
    }
    return 0;
}
