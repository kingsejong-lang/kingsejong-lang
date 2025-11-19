/**
 * @file CallStack.cpp
 * @brief 함수 호출 스택 추적 시스템 구현
 * @author KingSejong Team
 * @date 2025-11-11
 */

#include "debugger/CallStack.h"
#include <stdexcept>
#include "error/ErrorMessages.h"

namespace kingsejong {
namespace debugger {

void CallStack::pop()
{
    if (frames_.empty()) {
        throw std::runtime_error(std::string(error::debugger::CALLSTACK_POP_EMPTY));
    }
    frames_.pop_back();
}

CallStack::StackFrame& CallStack::current()
{
    if (frames_.empty()) {
        throw std::runtime_error(std::string(error::debugger::CALLSTACK_CURRENT_EMPTY));
    }
    return frames_.back();
}

const CallStack::StackFrame& CallStack::current() const
{
    if (frames_.empty()) {
        throw std::runtime_error(std::string(error::debugger::CALLSTACK_CURRENT_EMPTY));
    }
    return frames_.back();
}

} // namespace debugger
} // namespace kingsejong
