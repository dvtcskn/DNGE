/* ---------------------------------------------------------------------------------------
* MIT License
*
* Copyright (c) 2023 Davut Coþkun.
* All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
* ---------------------------------------------------------------------------------------
*/

#pragma once

#include <stdio.h>
#include <string>
#include <windows.h>

namespace sCPU
{
    void InitCpuUsage();
    double GetCurrentCpuUsage();
    void InitCpuUsageBySystem();
    double GetCurrentCpuUsageBySystem();

    int getCpuidFamily();
    char* getCpuidVendor(char* vendor);
    void getProcessorCount(DWORD& cores, DWORD& logical);
    DWORD CountSetBits(ULONG_PTR bitMask);

    std::string Vendor(void);
    std::string Brand(void);

    bool SSE3(void);
    bool PCLMULQDQ(void);
    bool MONITOR(void);
    bool SSSE3(void);
    bool FMA(void);
    bool CMPXCHG16B(void);
    bool SSE41(void);
    bool SSE42(void);
    bool MOVBE(void);
    bool POPCNT(void);
    bool AES(void);
    bool XSAVE(void);
    bool OSXSAVE(void);
    bool AVX(void);
    bool F16C(void);
    bool RDRAND(void);

    bool MSR(void);
    bool CX8(void);
    bool SEP(void);
    bool CMOV(void);
    bool CLFSH(void);
    bool MMX(void);
    bool FXSR(void);
    bool SSE(void);
    bool SSE2(void);

    bool FSGSBASE(void);
    bool BMI1(void);
    bool HLE(void);
    bool AVX2(void);
    bool BMI2(void);
    bool ERMS(void);
    bool INVPCID(void);
    bool RTM(void);
    bool AVX512F(void);
    bool RDSEED(void);
    bool ADX(void);
    bool AVX512PF(void);
    bool AVX512ER(void);
    bool AVX512CD(void);
    bool SHA(void);

    bool PREFETCHWT1(void);

    bool LAHF(void);
    bool LZCNT(void);
    bool ABM(void);
    bool SSE4a(void);
    bool XOP(void);
    bool TBM(void);

    bool SYSCALL(void);
    bool MMXEXT(void);
    bool RDTSCP(void);
    bool _3DNOWEXT(void);
    bool _3DNOW(void);
}
