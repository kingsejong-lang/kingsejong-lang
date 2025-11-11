/**
 * @file JsonRpc.cpp
 * @brief JSON-RPC 2.0 프로토콜 구현
 * @author KingSejong Team
 * @date 2025-11-12
 */

#include "lsp/JsonRpc.h"
#include <sstream>
#include <stdexcept>

namespace kingsejong {
namespace lsp {

nlohmann::json JsonRpc::parse(const std::string& message)
{
    try {
        return nlohmann::json::parse(message);
    } catch (const nlohmann::json::parse_error& e) {
        throw std::runtime_error("JSON parse error: " + std::string(e.what()));
    }
}

std::string JsonRpc::serialize(const nlohmann::json& json)
{
    return json.dump();
}

nlohmann::json JsonRpc::readMessage(std::istream& input)
{
    // Content-Length 헤더 파싱
    int contentLength = parseContentLength(input);

    // 빈 줄 읽기 (\r\n)
    std::string emptyLine;
    std::getline(input, emptyLine);
    if (!emptyLine.empty() && emptyLine != "\r") {
        throw std::runtime_error("Expected empty line after Content-Length header");
    }

    // 메시지 본문 읽기
    std::string content(contentLength, '\0');
    input.read(&content[0], contentLength);

    if (input.gcount() != contentLength) {
        throw std::runtime_error("Failed to read complete message body");
    }

    // JSON 파싱
    return parse(content);
}

void JsonRpc::writeMessage(std::ostream& output, const nlohmann::json& json)
{
    std::string content = serialize(json);

    // Content-Length 헤더 쓰기
    output << "Content-Length: " << content.length() << "\r\n";
    output << "\r\n";

    // 메시지 본문 쓰기
    output << content;
    output.flush();
}

nlohmann::json JsonRpc::createError(int id, int code, const std::string& message)
{
    nlohmann::json error;
    error["jsonrpc"] = "2.0";
    error["id"] = id;
    error["error"]["code"] = code;
    error["error"]["message"] = message;
    return error;
}

nlohmann::json JsonRpc::createResponse(int id, const nlohmann::json& result)
{
    nlohmann::json response;
    response["jsonrpc"] = "2.0";
    response["id"] = id;
    response["result"] = result;
    return response;
}

int JsonRpc::parseContentLength(std::istream& input)
{
    std::string line;
    std::getline(input, line);

    // \r 제거 (Windows 스타일 줄 바꿈)
    if (!line.empty() && line.back() == '\r') {
        line.pop_back();
    }

    // "Content-Length: " 접두사 확인
    const std::string prefix = "Content-Length: ";
    if (line.find(prefix) != 0) {
        throw std::runtime_error("Expected Content-Length header, got: " + line);
    }

    // 길이 파싱
    std::string lengthStr = line.substr(prefix.length());
    try {
        return std::stoi(lengthStr);
    } catch (const std::exception& e) {
        throw std::runtime_error("Invalid Content-Length value: " + lengthStr);
    }
}

} // namespace lsp
} // namespace kingsejong
