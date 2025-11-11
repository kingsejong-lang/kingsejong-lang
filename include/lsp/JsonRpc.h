#pragma once

/**
 * @file JsonRpc.h
 * @brief JSON-RPC 2.0 프로토콜 구현
 * @author KingSejong Team
 * @date 2025-11-12
 *
 * LSP는 JSON-RPC 2.0을 사용하여 통신합니다.
 */

#include <nlohmann/json.hpp>
#include <iostream>
#include <string>

namespace kingsejong {
namespace lsp {

/**
 * @class JsonRpc
 * @brief JSON-RPC 2.0 메시지 파싱 및 직렬화
 *
 * LSP 프로토콜의 기본 통신 레이어입니다.
 * Content-Length 헤더를 포함한 메시지 읽기/쓰기를 지원합니다.
 *
 * Thread Safety: NOT thread-safe
 * Memory: RAII - 자동 관리
 *
 * Example:
 * ```cpp
 * JsonRpc rpc;
 *
 * // 메시지 읽기
 * auto request = rpc.readMessage(std::cin);
 * std::string method = request["method"];
 *
 * // 응답 쓰기
 * nlohmann::json response = rpc.createResponse(1, result);
 * rpc.writeMessage(std::cout, response);
 * ```
 */
class JsonRpc {
public:
    /**
     * @enum ErrorCode
     * @brief LSP 표준 에러 코드
     */
    enum ErrorCode {
        ParseError = -32700,       // JSON 파싱 실패
        InvalidRequest = -32600,   // 잘못된 요청
        MethodNotFound = -32601,   // 메서드 없음
        InvalidParams = -32602,    // 잘못된 파라미터
        InternalError = -32603     // 내부 에러
    };

    JsonRpc() = default;
    ~JsonRpc() = default;

    /**
     * @brief JSON 문자열 파싱
     * @param message JSON 문자열
     * @return 파싱된 JSON 객체
     * @throws std::runtime_error if parsing fails
     *
     * Complexity: O(n) where n = message length
     */
    nlohmann::json parse(const std::string& message);

    /**
     * @brief JSON 객체 직렬화
     * @param json JSON 객체
     * @return JSON 문자열
     *
     * Complexity: O(n) where n = object size
     */
    std::string serialize(const nlohmann::json& json);

    /**
     * @brief Content-Length 헤더를 포함한 메시지 읽기
     * @param input 입력 스트림 (stdin)
     * @return 파싱된 JSON 객체
     * @throws std::runtime_error if header missing or invalid
     *
     * 메시지 포맷:
     * ```
     * Content-Length: 123\r\n
     * \r\n
     * {"jsonrpc":"2.0",...}
     * ```
     */
    nlohmann::json readMessage(std::istream& input);

    /**
     * @brief Content-Length 헤더를 포함한 메시지 쓰기
     * @param output 출력 스트림 (stdout)
     * @param json JSON 객체
     *
     * 메시지 포맷:
     * ```
     * Content-Length: 123\r\n
     * \r\n
     * {"jsonrpc":"2.0",...}
     * ```
     */
    void writeMessage(std::ostream& output, const nlohmann::json& json);

    /**
     * @brief 에러 응답 생성
     * @param id 요청 ID
     * @param code 에러 코드 (LSP 표준)
     * @param message 에러 메시지
     * @return 에러 응답 JSON
     *
     * Example:
     * ```cpp
     * auto error = rpc.createError(1, ErrorCode::MethodNotFound, "Method not found");
     * rpc.writeMessage(std::cout, error);
     * ```
     */
    nlohmann::json createError(int id, int code, const std::string& message);

    /**
     * @brief 성공 응답 생성
     * @param id 요청 ID
     * @param result 결과 객체
     * @return 성공 응답 JSON
     *
     * Example:
     * ```cpp
     * nlohmann::json result = { {"status", "ok"} };
     * auto response = rpc.createResponse(1, result);
     * rpc.writeMessage(std::cout, response);
     * ```
     */
    nlohmann::json createResponse(int id, const nlohmann::json& result);

private:
    /**
     * @brief Content-Length 헤더 파싱
     * @param input 입력 스트림
     * @return 콘텐츠 길이
     * @throws std::runtime_error if header invalid
     */
    int parseContentLength(std::istream& input);
};

} // namespace lsp
} // namespace kingsejong
