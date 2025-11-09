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
    src/types/Type.cpp
    src/evaluator/Value.cpp
    src/evaluator/Environment.cpp
    src/evaluator/Evaluator.cpp
    src/evaluator/Builtin.cpp
    src/repl/Repl.cpp
)

# 라이브러리 생성 (kingsejong_lib)
add_library(kingsejong_lib STATIC ${LIB_SOURCES})
target_include_directories(kingsejong_lib PUBLIC
    ${CMAKE_SOURCE_DIR}/src
)

# 실행 파일 소스
set(SOURCES
    src/main.cpp
)

# 실행 파일 생성
add_executable(kingsejong ${SOURCES})

# 인클루드 디렉토리 설정
target_include_directories(kingsejong PRIVATE
    ${CMAKE_SOURCE_DIR}/src
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

# 프로젝트 정보 출력
message(STATUS "KingSejong Language Interpreter")
message(STATUS "Version: ${PROJECT_VERSION}")
message(STATUS "C++ Standard: C++${CMAKE_CXX_STANDARD}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
