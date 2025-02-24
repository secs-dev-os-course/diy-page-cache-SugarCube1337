#include "CacheManager.h"
#include <iostream>
#include <algorithm>
#include <cstring>
#include <vector>

// Конструктор для инициализации CacheManager с заданным размером блока и максимальным количеством блоков
CacheManager::CacheManager(size_t blockSize, size_t maxBlocks)
        : blockSize(blockSize), maxBlocks(maxBlocks) {}

// Чтение блока из кэша или с диска, если блок не найден в кэше
void CacheManager::readBlock(HANDLE fileHandle, size_t blockIndex, char *buffer, size_t size) {
    // Поиск файла в кэше
    auto fileIt = fileCacheMap.find(fileHandle);
    if (fileIt != fileCacheMap.end()) {
        // Поиск блока внутри кэшированного файла
        auto blockIt = fileIt->second.find(blockIndex);
        if (blockIt != fileIt->second.end()) {
            std::memcpy(buffer, blockIt->second.data.get(), size); // Копирование данных из кэша в буфер
            return;
        }
    }

    // Блок не найден в кэше, читаем с диска
    LARGE_INTEGER offset;
    offset.QuadPart = blockIndex * blockSize; // Вычисление смещения в файле

    // Устанавливаем указатель файла в нужную позицию
    SetFilePointerEx(fileHandle, offset, nullptr, FILE_BEGIN);

    DWORD bytesRead;
    // Чтение блока с диска
    if (!ReadFile(fileHandle, buffer, size, &bytesRead, nullptr)) {
        std::cerr << "Failed to read block from disk. Error: " << GetLastError() << std::endl;
        return;
    }

    // Если чтение прошло успешно, записываем данные в кэш
    if (bytesRead > 0) {
        writeBlock(fileHandle, blockIndex, buffer, bytesRead);
    }
}

// Запись блока в кэш
void CacheManager::writeBlock(HANDLE fileHandle, size_t blockIndex, const char *buffer, size_t size) {
    // Если в кэше слишком много блоков, нужно освободить место
    if (fileCacheMap[fileHandle].size() >= maxBlocks) {
        evictBlock(fileHandle); // Выгрузка старого блока
    }

    // Создание нового блока и копирование данных в его память
    Block block;
    block.data = std::make_unique<char[]>(blockSize);
    std::memcpy(block.data.get(), buffer, size);

    // Сохранение блока в кэш
    fileCacheMap[fileHandle][blockIndex] = std::move(block);
    // Добавление блока в очередь FIFO для последующей выгрузки
    fifoQueue.push_back({fileHandle, blockIndex});
}

// Синхронизация данных из кэша на диск
void CacheManager::syncToDisk(HANDLE fileHandle) {
    // Проходим по всем блокам кэша для данного файла
    for (const auto &pair: fileCacheMap[fileHandle]) {
        size_t blockIndex = pair.first;
        const Block &block = pair.second;

        LARGE_INTEGER offset;
        offset.QuadPart = blockIndex * blockSize; // Вычисление смещения

        // Устанавливаем указатель файла в нужную позицию
        SetFilePointerEx(fileHandle, offset, nullptr, FILE_BEGIN);

        DWORD bytesWritten;
        // Запись данных из блока на диск
        if (!WriteFile(fileHandle, block.data.get(), blockSize, &bytesWritten, nullptr)) {
            std::cerr << "Failed to write block to disk. Error: " << GetLastError() << std::endl;
        }
    }

    // Очистка кэша только после успешной записи на диск
    fileCacheMap[fileHandle].clear();
    // Удаляем все блоки из очереди FIFO для данного файла
    fifoQueue.remove_if([fileHandle](const std::pair<HANDLE, size_t> &item) {
        return item.first == fileHandle;
    });
}

// Выгрузка самого старого блока из кэша (используется FIFO-очередь)
void CacheManager::evictBlock(HANDLE fileHandle) {
    if (fifoQueue.empty()) return; // Если очередь пуста, ничего не делаем

    // Получаем самый старый блок из очереди
    auto it = fifoQueue.front();
    size_t oldestBlockIndex = it.second;
    auto &blocks = fileCacheMap[fileHandle];

    // Ищем блок по индексу в кэше
    auto blockIt = blocks.find(oldestBlockIndex);
    if (blockIt != blocks.end()) {
        const Block &block = blockIt->second;

        // Записываем блок на диск, если он был изменен
        LARGE_INTEGER offset;
        offset.QuadPart = oldestBlockIndex * blockSize; // Вычисляем смещение

        SetFilePointerEx(fileHandle, offset, nullptr, FILE_BEGIN);

        DWORD bytesWritten;
        if (!WriteFile(fileHandle, block.data.get(), blockSize, &bytesWritten, nullptr)) {
            std::cerr << "Failed to write block to disk. Error: " << GetLastError() << std::endl;
        }

        // Удаляем блок из кэша после записи на диск
        blocks.erase(blockIt);
    }

    // Убираем блок из очереди FIFO
    fifoQueue.pop_front();
}
