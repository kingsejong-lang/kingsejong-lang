/**
 * @file Debugger.cpp
 * @brief KingSejong 언어 대화형 디버거 구현
 * @author KingSejong Team
 * @date 2025-11-11
 */

#include "debugger/Debugger.h"

namespace kingsejong {
namespace debugger {

Debugger::Debugger()
    : breakpoints_(std::make_unique<BreakpointManager>())
    , callStack_(std::make_unique<CallStack>())
{
}

} // namespace debugger
} // namespace kingsejong
