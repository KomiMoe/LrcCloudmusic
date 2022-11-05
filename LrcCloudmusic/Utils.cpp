#include <intrin.h>
#include <string>

#include "Utils.h"

const unsigned char* SearchVirtualMemory(const unsigned char* virtualAddress, size_t virtualLength, const unsigned char* sigPattern, const char* sigMask)
{
    if (sigMask == nullptr || sigMask[0] == 0) {
        CHAR tmpMask[0x1000];
        auto sigLen = strlen(sigMask);
        if (sigLen > 0x1000 - 1) sigLen = 0x1000 - 1;
        memset(tmpMask, 'x', sigLen);
        tmpMask[sigLen] = 0;
        sigMask = tmpMask;
    }

    const auto maxAddress = virtualAddress + virtualLength;

    std::string sigMaskString;
    sigMaskString.append(sigMask);
    const auto firstHitPos = sigMaskString.find_first_of('x');
    sigPattern += firstHitPos;
    sigMask += firstHitPos;

    const __m128i sigHead = _mm_set1_epi8(static_cast<CHAR>(sigPattern[0]));
    ULONG idxComp;

    for (ULONGLONG i = 0; i <= virtualLength - 16; i += 16)
    {
        const __m128i curHead = _mm_loadu_si128(reinterpret_cast<const __m128i*>(virtualAddress + i));
        const __m128i curComp = _mm_cmpeq_epi8(sigHead, curHead);
        ULONG mskComp = _mm_movemask_epi8(curComp);

        const auto baseAddress = virtualAddress + i;
        ULONGLONG j = 0;
        while (_BitScanForward(&idxComp, mskComp))
        {
            auto currentAddress = baseAddress + j + idxComp;
            auto currentPattern = sigPattern;
            auto currentMask = sigMask;
            for (; currentAddress <= maxAddress; currentAddress++, currentPattern++, currentMask++)
            {
                const UCHAR currentUChar = *currentPattern;
                const BOOLEAN currentEqual = (*currentAddress == currentUChar);
                if (!currentEqual)
                {
                    if (*currentMask == 'x') break;
                }
                if (*currentMask == 0)
                {
                    return (baseAddress + j + idxComp - firstHitPos);
                }
            }

            ++idxComp;
            if (idxComp == 32)
            {
                mskComp = 0;
            }
            else
            {
                mskComp = mskComp >> idxComp;
            }
            j += idxComp;
        }
    }

    return nullptr;
}
