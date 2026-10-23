cmake_minimum_required(VERSION 3.20)
project(KingSejong VERSION 0.1.0 LANGUAGES CXX)

# C++23 표준 사용
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 컴파일러 경고 설정
if(MSVC)
    # MSVC 는 경고를 에러로 올리지 않는다. 기존 코드에 MSVC 전용 경고(C4244 등)가 남아 있어
    # /WX 를 켜면 Windows 빌드가 한 번도 통과하지 못했다. 경고를 없앤 뒤 /WX 를 다시 켠다.
    # 소스는 UTF-8 이고 식별자에 한글이 있다(builtin_파일_읽기 등). /utf-8 이 없으면 MSVC 가 코드 페이지로 읽어 C3872 를 낸다.
    add_compile_options(/W4 /utf-8)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

# 빌드 타입 설정
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# 출력 디렉토리 설정
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# 라이브러리 소스 파일
set(LIB_SOURCES
    src/lexer/Token.cpp
    src/lexer/JosaRecognizer.cpp
    src/lexer/Lexer.cpp
    src/ast/Node.cpp
    src/ast/Visitor.cpp
    src/parser/Parser.cpp
    src/semantic/SemanticAnalyzer.cpp
    src/types/Type.cpp
    src/evaluator/Value.cpp
    src/evaluator/Environment.cpp
    src/evaluator/Evaluator.cpp
    src/evaluator/Builtin.cpp
    src/evaluator/EventLoop.cpp
    src/module/ModuleLoader.cpp
    src/repl/Repl.cpp
    src/memory/GC.cpp
    src/bytecode/OpCode.cpp
    src/bytecode/Chunk.cpp
    src/bytecode/Compiler.cpp
    src/bytecode/VM.cpp
    src/jit/HotPathDetector.cpp
    src/security/SecurityManager.cpp
    src/security/NetworkSecurityManager.cpp
    src/error/ErrorReporter.cpp
    src/debugger/BreakpointManager.cpp
    src/debugger/CallStack.cpp
    src/debugger/WatchpointManager.cpp
    src/debugger/CommandParser.cpp
    src/debugger/SourceCodeViewer.cpp
    src/debugger/Debugger.cpp
    src/lsp/JsonRpc.cpp
    src/lsp/DocumentManager.cpp
    src/lsp/LanguageServer.cpp
    src/lsp/CompletionProvider.cpp
    src/lsp/DiagnosticsProvider.cpp
    src/lsp/SymbolTable.cpp
    src/lsp/SymbolCollector.cpp
    src/lsp/LspUtils.cpp
    src/linter/Linter.cpp
    src/linter/Rule.cpp
    src/linter/rules/UnusedVariableRule.cpp
    src/linter/rules/DeadCodeRule.cpp
    src/linter/rules/NoSelfComparisonRule.cpp
    src/linter/rules/ConstantConditionRule.cpp
    src/linter/rules/EmptyBlockRule.cpp
    src/linter/rules/NoUnusedParameterRule.cpp
    src/linter/rules/NoShadowingRule.cpp
    src/linter/rules/NoMagicNumberRule.cpp
    src/linter/rules/NamingConventionRule.cpp
    src/linter/rules/FunctionComplexityRule.cpp
    src/linter/rules/UnusedImportRule.cpp
    src/linter/rules/ImportOrderRule.cpp
    src/linter/rules/JosaAgreementRule.cpp
    src/formatter/Formatter.cpp
    src/profiler/Profiler.cpp
    src/config/ConfigLoader.cpp
    src/morphology/Dictionary.cpp
    src/morphology/MorphologicalAnalyzer.cpp
    src/common/Logger.cpp
)

# 라이브러리 생성 (kingsejong_lib)
add_library(kingsejong_lib STATIC ${LIB_SOURCES})
target_include_directories(kingsejong_lib PUBLIC
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/include
)
target_link_libraries(kingsejong_lib PUBLIC nlohmann_json::nlohmann_json)

# SQLite 는 선택이다. 없으면 DB_* 내장 함수가 등록되지 않는다 (Windows 러너처럼 헤더가 없는 환경).
find_package(SQLite3)
if(SQLite3_FOUND)
    target_link_libraries(kingsejong_lib PUBLIC SQLite::SQLite3)
    target_compile_definitions(kingsejong_lib PUBLIC KINGSEJONG_HAS_SQLITE=1)
    message(STATUS "SQLite3: ${SQLite3_VERSION}")
else()
    message(STATUS "SQLite3 not found: DB_* builtins disabled")
endif()

# 실행 파일 소스
set(SOURCES
    src/main.cpp
)

# 실행 파일 생성
add_executable(kingsejong ${SOURCES})

# 인클루드 디렉토리 설정
target_include_directories(kingsejong PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/include
)

# 라이브러리 링크
target_link_libraries(kingsejong PRIVATE kingsejong_lib)

# 플랫폼별 설정
if(WIN32)
    # Windows 특정 설정
    target_compile_definitions(kingsejong PRIVATE UNICODE _UNICODE)
elseif(APPLE)
    # macOS 특정 설정
    target_compile_options(kingsejong PRIVATE -stdlib=libc++)
elseif(UNIX)
    # Linux 특정 설정
    target_link_libraries(kingsejong PRIVATE pthread)
endif()

# 테스트 활성화
enable_testing()

# GoogleTest 설정
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
)
# Windows에서 gtest 빌드 설정
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# 가져온 서드파티 헤더는 시스템 헤더로 취급한다. 우리 코드는 -Werror 로 빌드하지만
# googletest 1.14 헤더가 Apple clang 21 의 -Wcharacter-conversion 에 걸려 빌드가 깨진다.
foreach(_dep gtest gmock gtest_main gmock_main)
    if(TARGET ${_dep})
        get_target_property(_inc ${_dep} INTERFACE_INCLUDE_DIRECTORIES)
        if(_inc)
            set_target_properties(${_dep} PROPERTIES INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${_inc}")
        endif()
        # googletest 자체를 컴파일할 때도 위의 -Werror 가 적용된다. 서드파티 코드의 경고로
        # 우리 빌드가 깨지지 않게 한다.
        if(NOT MSVC)
            target_compile_options(${_dep} PRIVATE -Wno-error)
        endif()
    endif()
endforeach()

# nlohmann/json 설정 (LSP용)
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(json)

# 테스트 파일 수집
file(GLOB_RECURSE TEST_SOURCES "tests/*.cpp")
# manual_lexer_test.cpp 는 main 함수가 있으므로 제외
list(FILTER TEST_SOURCES EXCLUDE REGEX "manual_lexer_test\\.cpp$")

if(TEST_SOURCES)
    # 테스트 실행 파일 생성
    add_executable(kingsejong_tests ${TEST_SOURCES})

    # 테스트에 필요한 라이브러리 링크
    target_link_libraries(kingsejong_tests
        PRIVATE
        kingsejong_lib
        GTest::gtest_main
        GTest::gmock_main
    )

    # 테스트 인클루드 디렉토리
    target_include_directories(kingsejong_tests PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/tests
    )

    # CTest에 테스트 추가
    include(GoogleTest)
    gtest_discover_tests(kingsejong_tests
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
endif()

# examples/*.ksj 를 실제 바이너리로 실행해 examples/expected/*.out 과 비교한다.
# 문서의 예제가 거짓이 되는 순간 빌드가 깨지게 하는 종단 테스트다.
find_package(Python3 COMPONENTS Interpreter)
if(Python3_Interpreter_FOUND)
    # SQLite 가 없는 빌드(Windows 러너)에는 DB_* 내장 함수가 없어 stdlib_db 예제를 돌릴 수 없다.
    set(EXAMPLES_EXCLUDE_ARGS)
    if(NOT SQLite3_FOUND)
        set(EXAMPLES_EXCLUDE_ARGS --exclude stdlib_db.ksj)
    endif()
    add_test(
        NAME examples
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tests/examples/run_examples.py
                --interpreter $<TARGET_FILE:kingsejong> ${EXAMPLES_EXCLUDE_ARGS}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
    # 같은 예제를 --vm 으로도 돌린다. 평가기와 달라야 정상인 예제는 KNOWN_FAILURES_VM 에 적는다.
    add_test(
        NAME examples-vm
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tests/examples/run_examples.py
                --interpreter $<TARGET_FILE:kingsejong> --interpreter-flag=--vm
                --known-failures ${CMAKE_SOURCE_DIR}/examples/expected/KNOWN_FAILURES_VM
                ${EXAMPLES_EXCLUDE_ARGS}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
endif()

# 설치 규칙
# 설치된 바이너리가 stdlib 를 찾을 수 있게 위치를 컴파일 시점에 알려 준다
target_compile_definitions(kingsejong_lib PUBLIC KINGSEJONG_STDLIB_DIR="${CMAKE_INSTALL_PREFIX}/share/kingsejong")
install(DIRECTORY stdlib DESTINATION share/kingsejong)

install(TARGETS kingsejong
    RUNTIME DESTINATION bin
)

# ============================================================================
# WebAssembly 빌드 (Emscripten)
# ============================================================================
if(EMSCRIPTEN)
    message(STATUS "Building for WebAssembly with Emscripten")

    # WASM 용 라이브러리 소스: 네이티브 라이브러리와 같은 목록에서 브라우저에 없는 것만 뺀다.
    # lsp·debugger·repl(stdin/소켓 기반 도구), linter·formatter·profiler·config(CLI 전용).
    set(WASM_LIB_SOURCES ${LIB_SOURCES})
    list(FILTER WASM_LIB_SOURCES EXCLUDE REGEX "src/(jit|lsp|debugger|repl|linter|formatter|profiler|config)/")
    # 핫 패스 감지기는 평가기가 부르는 카운터라 남긴다.
    list(APPEND WASM_LIB_SOURCES src/jit/HotPathDetector.cpp)

    # WASM 라이브러리 생성
    add_library(kingsejong_wasm_lib STATIC ${WASM_LIB_SOURCES})
    target_include_directories(kingsejong_wasm_lib PUBLIC
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/include
    )
    target_link_libraries(kingsejong_wasm_lib PUBLIC nlohmann_json::nlohmann_json)
    target_compile_definitions(kingsejong_wasm_lib PUBLIC KINGSEJONG_WASM=1)

    # WASM 바인딩 타겟
    add_executable(kingsejong_wasm
        wasm/src/bindings.cpp
    )

    target_include_directories(kingsejong_wasm PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/wasm/include
    )

    target_link_libraries(kingsejong_wasm PRIVATE kingsejong_wasm_lib)

    # Emscripten 링커 플래그
    set_target_properties(kingsejong_wasm PROPERTIES
        LINK_FLAGS "\
            -s WASM=1 \
            -s MODULARIZE=1 \
            -s EXPORT_NAME='createKingSejong' \
            -s ALLOW_MEMORY_GROWTH=1 \
            -s EXPORTED_RUNTIME_METHODS='[\"ccall\",\"cwrap\"]' \
            -s EXPORTED_FUNCTIONS='[\"_malloc\",\"_free\"]' \
            -s ENVIRONMENT=web \
            -s FILESYSTEM=0 \
            -s ASSERTIONS=1 \
            --bind \
            -O3"
    )

    # 출력 파일 설정
    set_target_properties(kingsejong_wasm PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/wasm
        OUTPUT_NAME "kingsejong"
    )

    message(STATUS "WASM output: ${CMAKE_BINARY_DIR}/wasm/kingsejong.js")
    message(STATUS "WASM module: ${CMAKE_BINARY_DIR}/wasm/kingsejong.wasm")
endif()

# 프로젝트 정보 출력
message(STATUS "KingSejong Language Interpreter")
message(STATUS "Version: ${PROJECT_VERSION}")
message(STATUS "C++ Standard: C++${CMAKE_CXX_STANDARD}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
