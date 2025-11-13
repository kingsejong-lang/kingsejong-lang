/**
 * @file SourceCodeViewer.h
 * @brief 소스 코드 표시 기능
 * @author KingSejong Team
 * @date 2025-11-14
 *
 * 디버거에서 소스 코드를 표시하는 기능을 제공합니다.
 */

#ifndef KINGSEJONG_DEBUGGER_SOURCECODEVIEWER_H
#define KINGSEJONG_DEBUGGER_SOURCECODEVIEWER_H

#include <string>
#include <vector>
#include <optional>
#include <set>

namespace kingsejong {
namespace debugger {

/**
 * @class SourceCodeViewer
 * @brief 소스 코드 표시 클래스
 *
 * 소스 파일의 특정 라인 주변을 읽어서 표시합니다.
 * 현재 실행 라인과 브레이크포인트를 표시할 수 있습니다.
 *
 * Thread Safety: NOT thread-safe. 외부 동기화 필요.
 * Memory: RAII - 모든 리소스 자동 관리
 *
 * Example:
 * ```cpp
 * SourceCodeViewer viewer;
 *
 * // 소스 파일 로드
 * viewer.loadFile("test.ksj");
 *
 * // 10번 라인 주변 5줄 표시 (5-14번 라인)
 * auto lines = viewer.getLines(10, 5);
 *
 * // 현재 라인 설정
 * viewer.setCurrentLine(10);
 *
 * // 브레이크포인트 추가
 * viewer.addBreakpoint(12);
 *
 * // 표시 (현재 라인은 →, 브레이크포인트는 ●로 표시)
 * std::string display = viewer.format(10, 5);
 * ```
 */
class SourceCodeViewer {
public:
    /**
     * @brief 생성자
     */
    SourceCodeViewer() = default;

    /**
     * @brief 소멸자
     */
    ~SourceCodeViewer() = default;

    // RAII: Rule of Five
    SourceCodeViewer(const SourceCodeViewer&) = delete;
    SourceCodeViewer& operator=(const SourceCodeViewer&) = delete;
    SourceCodeViewer(SourceCodeViewer&&) = default;
    SourceCodeViewer& operator=(SourceCodeViewer&&) = default;

    /**
     * @brief 소스 파일 로드
     * @param filePath 파일 경로
     * @return true if 성공, false otherwise
     *
     * 파일을 읽어서 내부 버퍼에 저장합니다.
     *
     * Complexity: O(N) where N = 파일 라인 수
     */
    bool loadFile(const std::string& filePath);

    /**
     * @brief 특정 라인 주변 가져오기
     * @param lineNumber 중심 라인 번호 (1-based)
     * @param contextLines 주변에 표시할 라인 수
     * @return 라인 벡터 (비어있으면 파일 미로드 또는 범위 벗어남)
     *
     * lineNumber를 중심으로 앞뒤 contextLines 만큼 반환합니다.
     * 예: getLines(10, 2) -> 8, 9, 10, 11, 12번 라인 반환
     *
     * Complexity: O(C) where C = contextLines
     */
    std::vector<std::string> getLines(int lineNumber, int contextLines) const;

    /**
     * @brief 현재 실행 라인 설정
     * @param lineNumber 라인 번호 (1-based)
     *
     * Complexity: O(1)
     */
    void setCurrentLine(int lineNumber);

    /**
     * @brief 현재 실행 라인 가져오기
     * @return 현재 라인 번호 (설정 안 됨 = 0)
     *
     * Complexity: O(1)
     */
    int getCurrentLine() const {
        return currentLine_;
    }

    /**
     * @brief 브레이크포인트 추가
     * @param lineNumber 라인 번호 (1-based)
     *
     * Complexity: O(log B) where B = 브레이크포인트 수
     */
    void addBreakpoint(int lineNumber);

    /**
     * @brief 브레이크포인트 제거
     * @param lineNumber 라인 번호 (1-based)
     *
     * Complexity: O(log B) where B = 브레이크포인트 수
     */
    void removeBreakpoint(int lineNumber);

    /**
     * @brief 브레이크포인트 확인
     * @param lineNumber 라인 번호 (1-based)
     * @return true if 브레이크포인트 있음
     *
     * Complexity: O(log B) where B = 브레이크포인트 수
     */
    bool hasBreakpoint(int lineNumber) const;

    /**
     * @brief 포맷된 소스 코드 표시
     * @param lineNumber 중심 라인 번호 (1-based)
     * @param contextLines 주변에 표시할 라인 수
     * @return 포맷된 문자열
     *
     * 형식:
     * ```
     *    8  |   x = 10
     *    9  | ● y = 20
     *   10  | → z = 30
     *   11  |   결과 = x + y + z
     * ```
     *
     * 표시:
     * - ● : 브레이크포인트
     * - → : 현재 실행 라인
     * - 공백 : 일반 라인
     *
     * Complexity: O(C) where C = contextLines
     */
    std::string format(int lineNumber, int contextLines) const;

    /**
     * @brief 로드된 파일명 가져오기
     * @return 파일 경로 (로드 안 됨 = 빈 문자열)
     *
     * Complexity: O(1)
     */
    std::string getLoadedFile() const {
        return loadedFile_;
    }

    /**
     * @brief 총 라인 수 가져오기
     * @return 라인 수 (로드 안 됨 = 0)
     *
     * Complexity: O(1)
     */
    size_t getTotalLines() const {
        return lines_.size();
    }

    /**
     * @brief 모든 상태 초기화
     *
     * 파일 내용, 브레이크포인트, 현재 라인 모두 초기화합니다.
     *
     * Complexity: O(1)
     */
    void clear();

private:
    /// 소스 파일 내용 (각 라인)
    std::vector<std::string> lines_;

    /// 로드된 파일 경로
    std::string loadedFile_;

    /// 현재 실행 중인 라인 번호 (0 = 설정 안 됨)
    int currentLine_ = 0;

    /// 브레이크포인트 라인 번호 집합 (정렬 유지)
    std::set<int> breakpoints_;
};

} // namespace debugger
} // namespace kingsejong

#endif // KINGSEJONG_DEBUGGER_SOURCECODEVIEWER_H
