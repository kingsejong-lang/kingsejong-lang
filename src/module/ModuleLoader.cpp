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
#include <sstream>
#include <filesystem>

namespace kingsejong {
namespace module {

ModuleLoader::ModuleLoader(const std::string& basePath)
    : basePath_(basePath)
{
}

std::shared_ptr<evaluator::Environment> ModuleLoader::loadModule(const std::string& modulePath)
{
    // 경로 해석
    std::string resolvedPath = resolvePath(modulePath);

    // 캐시 확인
    auto cacheIt = cache_.find(resolvedPath);
    if (cacheIt != cache_.end())
    {
        return cacheIt->second.env;
    }

    // 순환 참조 확인
    if (loading_.find(resolvedPath) != loading_.end())
    {
        throw error::RuntimeError(
            "순환 참조가 감지되었습니다: " + resolvedPath + "\n" +
            "해결 방법: 모듈 간 순환 참조를 제거하세요."
        );
    }

    // 파일 존재 확인
    if (!fileExists(resolvedPath))
    {
        throw error::RuntimeError(
            "모듈 파일을 찾을 수 없습니다: " + resolvedPath + "\n" +
            "해결 방법: 파일 경로를 확인하거나 모듈 파일을 생성하세요."
        );
    }

    // 로딩 중 표시
    loading_.insert(resolvedPath);

    try
    {
        // 파일 읽기
        std::string source = readFile(resolvedPath);

        // 모듈 환경 생성
        auto moduleEnv = std::make_shared<evaluator::Environment>();

        // Lexer, Parser, Evaluator로 실행
        lexer::Lexer lexer(source);
        parser::Parser parser(lexer);
        auto program = parser.parseProgram();

        if (!parser.errors().empty())
        {
            throw error::ParserError(
                "모듈 파싱 오류: " + resolvedPath + "\n" +
                parser.errors()[0]
            );
        }

        evaluator::Evaluator evaluator(moduleEnv);
        evaluator.evalProgram(program.get());

        // 캐시에 저장 (AST와 Environment 모두 저장)
        // Function 객체가 AST 노드에 대한 raw pointer를 가지므로
        // AST를 살려두어야 heap-use-after-free를 방지할 수 있습니다.
        ModuleCacheEntry entry;
        entry.env = moduleEnv;
        entry.ast = std::move(program);
        cache_[resolvedPath] = std::move(entry);

        // 로딩 완료
        loading_.erase(resolvedPath);

        return moduleEnv;
    }
    catch (...)
    {
        // 에러 발생 시 로딩 중 표시 제거
        loading_.erase(resolvedPath);
        throw;
    }
}

std::string ModuleLoader::resolvePath(const std::string& modulePath) const
{
    // 이미 절대 경로이거나 .ksj로 끝나는 경우
    if (modulePath.find('/') == 0 || modulePath.substr(modulePath.length() - 4) == ".ksj")
    {
        return modulePath;
    }

    // 상대 경로인 경우
    if (modulePath.find("./") == 0 || modulePath.find("../") == 0)
    {
        return basePath_ + "/" + modulePath + ".ksj";
    }

    // 모듈 이름만 주어진 경우
    return basePath_ + "/" + modulePath + ".ksj";
}

void ModuleLoader::clearCache()
{
    cache_.clear();
}

bool ModuleLoader::isCached(const std::string& modulePath) const
{
    std::string resolvedPath = resolvePath(modulePath);
    return cache_.find(resolvedPath) != cache_.end();
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
