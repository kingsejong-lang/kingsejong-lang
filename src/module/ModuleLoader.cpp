/**
 * @file ModuleLoader.cpp
 * @brief KingSejong 언어 모듈 로더 구현
 * @author KingSejong Team
 * @date 2025-11-09
 */

#include "ModuleLoader.h"
#include "../lexer/Lexer.h"
#include "../parser/Parser.h"
#include "../evaluator/Evaluator.h"
#include "../error/Error.h"
#include <fstream>
#include <cstdlib>
#include <algorithm>
#include <sstream>
#include <filesystem>

namespace kingsejong {
namespace module {

ModuleLoader::ModuleLoader(const std::string& basePath)
    : basePath_(basePath)
{
}

std::shared_ptr<evaluator::Environment> ModuleLoader::loadModule(const std::string& modulePath, const std::string& fromFile)
{
    // 경로 해석. 캐시와 순환 검사는 정규화한 경로를 키로 쓴다 ("./x.ksj" 와 "x.ksj" 는 같은 모듈).
    std::string resolvedPath = resolvePath(modulePath, fromFile);
    std::string key = cacheKey(resolvedPath);

    // 캐시 확인
    auto cacheIt = cache_.find(key);
    if (cacheIt != cache_.end())
    {
        return cacheIt->second.env;
    }

    // 순환 참조 확인
    if (loading_.find(key) != loading_.end())
    {
        throw error::RuntimeError(
            "순환 참조가 감지되었습니다: " + resolvedPath + "\n" +
            "해결 방법: 모듈 간 순환 참조를 제거하세요."
        );
    }

    // 파일 존재 확인
    if (!fileExists(resolvedPath))
    {
        std::string tried;
        for (const auto& candidate : candidatePaths(modulePath, fromFile))
        {
            tried += "\n  - " + candidate;
        }
        throw error::RuntimeError(
            "모듈 파일을 찾을 수 없습니다: " + modulePath + "\n" +
            "찾아본 경로:" + tried + "\n" +
            "해결 방법: 파일 경로를 확인하거나 KINGSEJONG_PATH 환경변수에 모듈 디렉터리를 추가하세요."
        );
    }

    // 로딩 중 표시
    loading_.insert(key);

    try
    {
        // 파일 읽기
        std::string source = readFile(resolvedPath);

        // 모듈 환경 생성
        auto moduleEnv = std::make_shared<evaluator::Environment>();

        // Lexer, Parser, Evaluator 로 실행. 파일 이름을 넘겨야 모듈 안의 가져오기가
        // 이 파일 기준 상대 경로로 해석되고, 에러 위치에도 파일 이름이 남는다.
        lexer::Lexer lexer(source, resolvedPath);
        parser::Parser parser(lexer);
        auto program = parser.parseProgram();

        if (!parser.errors().empty())
        {
            const auto& first = parser.errorDetails()[0];
            throw error::ParserError(
                "모듈 파싱 오류: " + resolvedPath + ":" + std::to_string(first.location.line) + ":" +
                std::to_string(first.location.column) + "\n" + first.message
            );
        }

        // 모듈 안의 가져오기가 동작하려면 모듈 평가기도 같은 로더를 알아야 한다.
        evaluator::Evaluator evaluator(moduleEnv);
        evaluator.setModuleLoader(this);
        evaluator.evalProgram(program.get());

        // 캐시에 저장 (AST와 Environment 모두 저장)
        // Function 객체가 AST 노드에 대한 raw pointer를 가지므로
        // AST를 살려두어야 heap-use-after-free를 방지할 수 있습니다.
        ModuleCacheEntry entry;
        entry.env = moduleEnv;
        entry.ast = std::move(program);
        cache_[key] = std::move(entry);

        // 로딩 완료
        loading_.erase(key);

        return moduleEnv;
    }
    catch (...)
    {
        // 에러 발생 시 로딩 중 표시 제거
        loading_.erase(key);
        throw;
    }
}

void ModuleLoader::addSearchPath(const std::string& dir)
{
    if (!dir.empty())
    {
        searchPaths_.push_back(dir);
    }
}

std::vector<std::string> ModuleLoader::candidatePaths(const std::string& modulePath, const std::string& fromFile) const
{
    namespace fs = std::filesystem;

    auto withExtension = [](const std::string& path) {
        if (path.size() >= 4 && path.compare(path.size() - 4, 4, ".ksj") == 0)
        {
            return path;
        }
        return path + ".ksj";
    };

    std::vector<std::string> out;
    auto push = [&out](const fs::path& candidate) {
        // 구분자는 / 로 통일한다 (Windows 에서도 "dir/mod.ksj" 꼴). 파일 API 는 둘 다 받는다.
        std::string s = candidate.lexically_normal().generic_string();
        if (std::find(out.begin(), out.end(), s) == out.end())
        {
            out.push_back(s);
        }
    };

    std::string rel = withExtension(modulePath);
    if (fs::path(rel).is_absolute())
    {
        push(rel);
        return out;
    }

    // 1. 가져오기 문이 있는 파일의 디렉터리
    std::string fromDir;
    if (!fromFile.empty() && fromFile[0] != '<')
    {
        fromDir = fs::path(fromFile).parent_path().string();
    }
    if (!fromDir.empty())
    {
        push(fs::path(fromDir) / rel);
    }

    // "./x", "../x" 는 명시적 상대 경로: 호출 파일 디렉터리와 basePath 만 본다
    bool explicitRelative = modulePath.rfind("./", 0) == 0 || modulePath.rfind("../", 0) == 0;
    if (explicitRelative)
    {
        push(fs::path(basePath_) / rel);
        return out;
    }

    // 2. 명시적으로 추가한 검색 경로
    for (const auto& dir : searchPaths_)
    {
        push(fs::path(dir) / rel);
    }

    // 3. 기본 경로 (작업 디렉터리)
    push(fs::path(basePath_) / rel);

    // 4. KINGSEJONG_PATH 환경변수 (경로 구분자로 나눔)
    if (const char* env = std::getenv("KINGSEJONG_PATH"))
    {
#ifdef _WIN32
        const char separator = ';';
#else
        const char separator = ':';
#endif
        std::string value = env;
        size_t start = 0;
        while (start <= value.size())
        {
            size_t end = value.find(separator, start);
            std::string dir = value.substr(start, end == std::string::npos ? std::string::npos : end - start);
            if (!dir.empty())
            {
                push(fs::path(dir) / rel);
            }
            if (end == std::string::npos) break;
            start = end + 1;
        }
    }

    // 5. 설치된 표준 라이브러리 디렉터리 (CMake 가 정의)
#ifdef KINGSEJONG_STDLIB_DIR
    push(fs::path(KINGSEJONG_STDLIB_DIR) / rel);
#endif

    return out;
}

std::string ModuleLoader::resolvePath(const std::string& modulePath, const std::string& fromFile) const
{
    auto candidates = candidatePaths(modulePath, fromFile);
    for (const auto& candidate : candidates)
    {
        if (fileExists(candidate))
        {
            return candidate;
        }
    }
    // 아무것도 없으면 첫 후보를 돌려준다. 호출자가 존재 확인 뒤 후보 목록과 함께 에러를 낸다.
    return candidates.empty() ? modulePath : candidates.front();
}

void ModuleLoader::clearCache()
{
    // 모듈 환경은 자기 함수들에 붙잡혀 있어 그냥 지우면 남는다. 모듈 A 가 B 의 함수를
    // 들고 있으면 B 는 A 가 비워진 뒤에야 풀리므로, 더 풀리는 것이 없을 때까지 반복한다.
    bool progressed = true;
    while (progressed)
    {
        progressed = false;
        for (auto& [key, entry] : cache_)
        {
            if (entry.env && entry.env->releaseIfUnreachable(1))
            {
                progressed = true;
            }
        }
    }
    cache_.clear();
}

ModuleLoader::~ModuleLoader()
{
    clearCache();
}

bool ModuleLoader::isCached(const std::string& modulePath) const
{
    return cache_.find(cacheKey(resolvePath(modulePath))) != cache_.end();
}

std::string ModuleLoader::cacheKey(const std::string& resolvedPath) const
{
    std::error_code ec;
    auto canonical = std::filesystem::weakly_canonical(resolvedPath, ec);
    if (ec)
    {
        return resolvedPath;
    }
    return canonical.generic_string();
}

bool ModuleLoader::fileExists(const std::string& path) const
{
    std::ifstream file(path);
    return file.good();
}

std::string ModuleLoader::readFile(const std::string& path) const
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        throw error::RuntimeError("파일을 열 수 없습니다: " + path);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

} // namespace module
} // namespace kingsejong
