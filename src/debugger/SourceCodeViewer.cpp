/**
 * @file SourceCodeViewer.cpp
 * @brief 소스 코드 표시 기능 구현
 * @author KingSejong Team
 * @date 2025-11-14
 */

#include "debugger/SourceCodeViewer.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace kingsejong {
namespace debugger {

// ============================================================================
// Public Methods
// ============================================================================

bool SourceCodeViewer::loadFile(const std::string& filePath)
{
    // 기존 내용 초기화
    lines_.clear();
    loadedFile_.clear();

    // 파일 열기
    std::ifstream file(filePath);
    if (!file.is_open()) {
        return false;
    }

    // 라인별로 읽기
    std::string line;
    while (std::getline(file, line)) {
        lines_.push_back(line);
    }

    file.close();

    // 파일 경로 저장
    loadedFile_ = filePath;

    return true;
}

std::vector<std::string> SourceCodeViewer::getLines(int lineNumber, int contextLines) const
{
    // 파일이 로드되지 않았으면 빈 벡터 반환
    if (lines_.empty()) {
        return {};
    }

    // 1-based -> 0-based 인덱스 변환
    int index = lineNumber - 1;

    // 범위 검증
    if (index < 0 || index >= static_cast<int>(lines_.size())) {
        return {};
    }

    // 시작/끝 인덱스 계산
    int startIdx = std::max(0, index - contextLines);
    int endIdx = std::min(static_cast<int>(lines_.size()) - 1, index + contextLines);

    // 라인 추출
    std::vector<std::string> result;
    for (int i = startIdx; i <= endIdx; i++) {
        result.push_back(lines_[i]);
    }

    return result;
}

void SourceCodeViewer::setCurrentLine(int lineNumber)
{
    currentLine_ = lineNumber;
}

void SourceCodeViewer::addBreakpoint(int lineNumber)
{
    breakpoints_.insert(lineNumber);
}

void SourceCodeViewer::removeBreakpoint(int lineNumber)
{
    breakpoints_.erase(lineNumber);
}

bool SourceCodeViewer::hasBreakpoint(int lineNumber) const
{
    return breakpoints_.find(lineNumber) != breakpoints_.end();
}

std::string SourceCodeViewer::format(int lineNumber, int contextLines) const
{
    // 파일이 로드되지 않았으면 빈 문자열 반환
    if (lines_.empty()) {
        return "";
    }

    // 라인 가져오기
    auto lines = getLines(lineNumber, contextLines);
    if (lines.empty()) {
        return "";
    }

    // 시작 라인 번호 계산
    int startLineNum = std::max(1, lineNumber - contextLines);

    // 최대 라인 번호 자릿수 계산 (정렬을 위해)
    int endLineNum = std::min(static_cast<int>(lines_.size()), lineNumber + contextLines);
    int maxDigits = std::to_string(endLineNum).length();

    // 포맷팅
    std::ostringstream oss;

    for (size_t i = 0; i < lines.size(); i++) {
        int currentLineNum = startLineNum + static_cast<int>(i);

        // 라인 번호 (오른쪽 정렬)
        oss << std::setw(maxDigits) << currentLineNum << " | ";

        // 마커 결정
        if (currentLineNum == currentLine_) {
            oss << "→ ";  // 현재 라인 (우선순위 높음)
        } else if (hasBreakpoint(currentLineNum)) {
            oss << "● ";  // 브레이크포인트
        } else {
            oss << "  ";  // 일반 라인
        }

        // 소스 코드
        oss << lines[i];

        // 마지막 라인이 아니면 개행
        if (i < lines.size() - 1) {
            oss << "\n";
        }
    }

    return oss.str();
}

void SourceCodeViewer::clear()
{
    lines_.clear();
    loadedFile_.clear();
    currentLine_ = 0;
    breakpoints_.clear();
}

} // namespace debugger
} // namespace kingsejong
