/**
 * @file SimpleJITTest.cpp
 * @brief 가장 간단한 JIT 테스트 - asmjit 동작 확인
 */

#include <iostream>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wgnu-anonymous-struct"
#pragma GCC diagnostic ignored "-Wnested-anon-types"
#include <asmjit/core.h>
#include <asmjit/x86.h>
#include <asmjit/a64.h>
#pragma GCC diagnostic pop

using namespace asmjit;

int main()
{
    std::cout << "=== Simple JIT Test ===\n";

    // 1. JitRuntime 생성 (native architecture)
    JitRuntime rt;
    std::cout << "1. JitRuntime created for native arch\n";

    // 2. CodeHolder 생성 및 초기화
    CodeHolder code;
    code.init(rt.environment());
    std::cout << "2. CodeHolder initialized\n";

    // 3. Platform-specific assembler
    Arch arch = rt.environment().arch();

    if (arch == Arch::kX64 || arch == Arch::kX86) {
        x86::Assembler a;
        code.attach(&a);
        std::cout << "3. x86/x64 Assembler attached\n";

        a.mov(x86::eax, 42);
        a.ret();
        std::cout << "4. x86 Instructions emitted\n";

        // Check and execute
        std::cout << "5. Code size: " << code.text_section()->buffer_size() << " bytes\n";

        typedef int (*FuncType)();
        FuncType func;
        Error err = rt.add(&func, &code);
        if (err == kErrorOk) {
            int result = func();
            std::cout << "6. Function executed, result: " << result << "\n";
            if (result == 42) {
                std::cout << "\n✅ SUCCESS! x64 JIT is working!\n";
                return 0;
            }
        }
    }
    else if (arch == Arch::kAArch64) {
        a64::Assembler a;
        code.attach(&a);
        std::cout << "3. ARM64 Assembler attached\n";

        a.mov(a64::w0, 42);  // w0 is the return register on ARM64
        a.ret(a64::x30);     // x30 is the link register
        std::cout << "4. ARM64 Instructions emitted\n";

        // Check and execute
        std::cout << "5. Code size: " << code.text_section()->buffer_size() << " bytes\n";

        typedef int (*FuncType)();
        FuncType func;
        Error err = rt.add(&func, &code);
        if (err == kErrorOk) {
            int result = func();
            std::cout << "6. Function executed, result: " << result << "\n";
            if (result == 42) {
                std::cout << "\n✅ SUCCESS! ARM64 JIT is working!\n";
                return 0;
            }
        }
    }
    else {
        std::cerr << "Unsupported architecture: " << static_cast<int>(arch) << "\n";
        return 1;
    }

    std::cerr << "\n❌ FAIL!\n";
    return 1;
}
