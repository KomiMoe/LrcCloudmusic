#pragma once
#include <Windows.h>

const unsigned char* SearchVirtualMemory(const unsigned char* virtualAddress, size_t virtualLength, const unsigned char* sigPattern, const char* sigMask);
