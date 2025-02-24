#pragma once

#ifdef CACHE_EXPORTS
#define CACHE_API __declspec(dllexport)
#else
#define CACHE_API __declspec(dllimport)
#endif

#include <sys/types.h>

// Открытие файла по заданному пути файла, доступного для чтения
CACHE_API int lab2_open(const char *path);

// Закрытие файла по хэндлу
CACHE_API int lab2_close(int fd);

// Чтение данных из файла
CACHE_API size_t lab2_read(int fd, void *buf, size_t count);

// Запись данных в файл
CACHE_API size_t lab2_write(int fd, const void *buf, size_t count);

// Перестановка позиции указателя на данные файла
CACHE_API off_t lab2_lseek(int fd, off_t offset, int whence);

// Синхронизация данных из кэша с диском
CACHE_API int lab2_fsync(int fd);
