cmake_minimum_required(VERSION 3.20)
project(KingSejong VERSION 0.1.0 LANGUAGES CXX)

# C++23 표준 사용
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 컴파일러 경고 설정
if(MSVC)
    add_compile_options(/W4 /WX)
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
    src/parser/Parser.cpp
    src/semantic/SemanticAnalyzer.cpp
    src/types/Type.cpp
    src/evaluator/Value.cpp
    src/evaluator/Environment.cpp
    src/evaluator/Evaluator.cpp
    src/evaluator/Builtin.cpp
    src/module/ModuleLoader.cpp
    src/repl/Repl.cpp
    src/memory/GC.cpp
    src/bytecode/OpCode.cpp
    src/bytecode/Chunk.cpp
    src/bytecode/Compiler.cpp
    src/bytecode/VM.cpp
    src/jit/HotPathDetector.cpp
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
    src/formatter/Formatter.cpp
)

# 라이브러리 생성 (kingsejong_lib)
add_library(kingsejong_lib STATIC ${LIB_SOURCES})
target_include_directories(kingsejong_lib PUBLIC
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/include
)
target_link_libraries(kingsejong_lib PUBLIC nlohmann_json::nlohmann_json)

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

# nlohmann/json 설정 (LSP용)
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(json)

# 테스트 파일 수집
file(GLOB_RECURSE TEST_SOURCES "tests/*.cpp")
# manual_lexer_test.cpp는 main 함수가 있으므로 제외
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
    gtest_discover_tests(kingsejong_tests)
endif()

# 설치 규칙
install(TARGETS kingsejong
    RUNTIME DESTINATION bin
)

# ============================================================================
# WebAssembly 빌드 (Emscripten)
# ============================================================================
if(EMSCRIPTEN)
    message(STATUS "Building for WebAssembly with Emscripten")

    # WASM용 라이브러리 소스 (웹에서 필요한 것만)
    set(WASM_LIB_SOURCES
        src/lexer/Token.cpp
        src/lexer/JosaRecognizer.cpp
        src/lexer/Lexer.cpp
        src/ast/Node.cpp
        src/parser/Parser.cpp
        src/semantic/SemanticAnalyzer.cpp
        src/types/Type.cpp
        src/evaluator/Value.cpp
        src/evaluator/Environment.cpp
        src/evaluator/Evaluator.cpp
        src/evaluator/Builtin.cpp
        src/error/ErrorReporter.cpp
        src/bytecode/OpCode.cpp
        src/bytecode/Chunk.cpp
        src/bytecode/Compiler.cpp
        src/bytecode/VM.cpp
        src/memory/GC.cpp
    )

    # WASM 라이브러리 생성
    add_library(kingsejong_wasm_lib STATIC ${WASM_LIB_SOURCES})
    target_include_directories(kingsejong_wasm_lib PUBLIC
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/include
    )

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
