# KingSejong 클래스 시스템 설계

**작성일**: 2025-11-17
**버전**: v0.5.0 (Phase 7.1)
**상태**: 설계 단계

---

## 📋 목차

1. [개요](#개요)
2. [문법 설계](#문법-설계)
3. [아키텍처](#아키텍처)
4. [구현 계획](#구현-계획)
5. [예제](#예제)

---

## 개요

### 목표

KingSejong 언어에 **객체지향 프로그래밍(OOP)** 기능을 추가합니다.

### 핵심 기능

- ✅ 클래스 정의
- ✅ 생성자 (초기화)
- ✅ 메서드 (공개/비공개)
- ✅ 필드 (공개/비공개)
- ✅ this 키워드
- ✅ 객체 생성 및 메서드 호출
- 🔄 단일 상속 (Phase 7.2)
- 🔄 특수 메서드 (toString, equals, hashCode) (Phase 7.2)

---

## 문법 설계

### 1. 클래스 정의

```ksj
클래스 사람 {
    비공개 문자열 이름
    비공개 정수 나이

    생성자(이름, 나이) {
        자신.이름 = 이름
        자신.나이 = 나이
    }

    공개 함수 인사하기() {
        출력("안녕하세요, 저는 " + 자신.이름 + "입니다")
    }

    공개 함수 나이_가져오기() {
        반환 자신.나이
    }

    비공개 함수 내부함수() {
        출력("비공개 메서드입니다")
    }
}
```

### 2. 객체 생성 및 사용

```ksj
변수 홍길동 = 사람("홍길동", 30)
홍길동.인사하기()  # "안녕하세요, 저는 홍길동입니다"

변수 나이 = 홍길동.나이_가져오기()
출력(나이)  # 30

# 에러: 비공개 필드 접근 불가
# 홍길동.이름

# 에러: 비공개 메서드 호출 불가
# 홍길동.내부함수()
```

### 3. 클래스 상속 (Phase 7.2)

```ksj
클래스 학생 상속 사람 {
    비공개 문자열 학교

    생성자(이름, 나이, 학교) {
        상위(이름, 나이)  # 부모 생성자 호출
        이.학교 = 학교
    }

    공개 함수 소개하기() {
        출력("저는 " + 이.학교 + "의 학생입니다")
    }
}
```

---

## 아키텍처

### 1. 새로운 TokenType (Token.h)

```cpp
// 클래스 관련 키워드
KEULLAESU,       // 클래스 (class)
SAENGSEONGJA,    // 생성자 (constructor)
GONGGAE,         // 공개 (public)
BIGONGGAE,       // 비공개 (private)
JASIN,           // 자신 (this)
SANGSOK,         // 상속 (extends) - Phase 7.2
SANGWI,          // 상위 (super) - Phase 7.2
```

### 2. 새로운 AST 노드

#### ClassStatement (Statement.h)

```cpp
class ClassStatement : public Statement {
private:
    std::string className_;
    std::vector<std::unique_ptr<FieldDeclaration>> fields_;
    std::unique_ptr<ConstructorDeclaration> constructor_;
    std::vector<std::unique_ptr<MethodDeclaration>> methods_;
    std::string superClass_;  // Phase 7.2

public:
    ClassStatement(
        const std::string& className,
        std::vector<std::unique_ptr<FieldDeclaration>> fields,
        std::unique_ptr<ConstructorDeclaration> constructor,
        std::vector<std::unique_ptr<MethodDeclaration>> methods
    );

    NodeType type() const override { return NodeType::CLASS_STATEMENT; }
    std::string toString() const override;

    const std::string& className() const { return className_; }
    const std::vector<std::unique_ptr<FieldDeclaration>>& fields() const { return fields_; }
    const ConstructorDeclaration* constructor() const { return constructor_.get(); }
    const std::vector<std::unique_ptr<MethodDeclaration>>& methods() const { return methods_; }
};
```

#### FieldDeclaration

```cpp
enum class AccessModifier {
    PUBLIC,    // 공개
    PRIVATE    // 비공개
};

class FieldDeclaration : public Node {
private:
    AccessModifier access_;
    std::string typeName_;
    std::string fieldName_;
    std::unique_ptr<Expression> initializer_;

public:
    FieldDeclaration(
        AccessModifier access,
        const std::string& typeName,
        const std::string& fieldName,
        std::unique_ptr<Expression> initializer = nullptr
    );

    AccessModifier access() const { return access_; }
    const std::string& typeName() const { return typeName_; }
    const std::string& fieldName() const { return fieldName_; }
    const Expression* initializer() const { return initializer_.get(); }
};
```

#### MethodDeclaration

```cpp
class MethodDeclaration : public Node {
private:
    AccessModifier access_;
    std::string returnType_;
    std::string methodName_;
    std::vector<Parameter> parameters_;
    std::unique_ptr<BlockStatement> body_;

public:
    MethodDeclaration(
        AccessModifier access,
        const std::string& returnType,
        const std::string& methodName,
        std::vector<Parameter> parameters,
        std::unique_ptr<BlockStatement> body
    );

    AccessModifier access() const { return access_; }
    const std::string& returnType() const { return returnType_; }
    const std::string& methodName() const { return methodName_; }
    const std::vector<Parameter>& parameters() const { return parameters_; }
    const BlockStatement* body() const { return body_.get(); }
};
```

#### ConstructorDeclaration

```cpp
class ConstructorDeclaration : public Node {
private:
    std::vector<Parameter> parameters_;
    std::unique_ptr<BlockStatement> body_;

public:
    ConstructorDeclaration(
        std::vector<Parameter> parameters,
        std::unique_ptr<BlockStatement> body
    );

    const std::vector<Parameter>& parameters() const { return parameters_; }
    const BlockStatement* body() const { return body_.get(); }
};
```

### 3. 새로운 Expression 노드

#### MemberAccessExpression (Expression.h)

```cpp
class MemberAccessExpression : public Expression {
private:
    std::unique_ptr<Expression> object_;
    std::string memberName_;

public:
    MemberAccessExpression(
        std::unique_ptr<Expression> object,
        const std::string& memberName
    );

    NodeType type() const override { return NodeType::MEMBER_ACCESS_EXPRESSION; }
    const Expression* object() const { return object_.get(); }
    const std::string& memberName() const { return memberName_; }
};
```

#### ThisExpression

```cpp
class ThisExpression : public Expression {
public:
    ThisExpression() = default;
    NodeType type() const override { return NodeType::THIS_EXPRESSION; }
    std::string toString() const override { return "이"; }
};
```

#### NewExpression (클래스 인스턴스화)

```cpp
class NewExpression : public Expression {
private:
    std::string className_;
    std::vector<std::unique_ptr<Expression>> arguments_;

public:
    NewExpression(
        const std::string& className,
        std::vector<std::unique_ptr<Expression>> arguments
    );

    NodeType type() const override { return NodeType::NEW_EXPRESSION; }
    const std::string& className() const { return className_; }
    const std::vector<std::unique_ptr<Expression>>& arguments() const { return arguments_; }
};
```

### 4. NodeType 추가 (Node.h)

```cpp
enum class NodeType {
    // ... 기존 타입들 ...

    // 클래스 관련
    CLASS_STATEMENT,
    FIELD_DECLARATION,
    METHOD_DECLARATION,
    CONSTRUCTOR_DECLARATION,
    MEMBER_ACCESS_EXPRESSION,
    THIS_EXPRESSION,
    NEW_EXPRESSION,
};
```

### 5. 새로운 OpCode (OpCode.h)

```cpp
// ========================================
// 클래스 (Phase 7.1)
// ========================================
CLASS_DEF,          ///< 클래스 정의: CLASS_DEF [class_name_index] [field_count] [method_count]
NEW_INSTANCE,       ///< 객체 생성: NEW_INSTANCE [class_name_index] [arg_count]
LOAD_FIELD,         ///< 필드 로드: pop instance, push instance.field
STORE_FIELD,        ///< 필드 저장: pop value, pop instance, instance.field = value
CALL_METHOD,        ///< 메서드 호출: CALL_METHOD [method_name_index] [arg_count]
LOAD_THIS,          ///< this 로드
```

### 6. Object 시스템 (evaluator/Value.h)

#### ClassDefinition

```cpp
struct FieldInfo {
    AccessModifier access;
    std::string typeName;
    std::string fieldName;
    Value defaultValue;
};

struct MethodInfo {
    AccessModifier access;
    std::string returnType;
    std::string methodName;
    std::shared_ptr<Function> function;
};

class ClassDefinition {
private:
    std::string className_;
    std::unordered_map<std::string, FieldInfo> fields_;
    std::unordered_map<std::string, MethodInfo> methods_;
    std::shared_ptr<Function> constructor_;

public:
    ClassDefinition(
        const std::string& className,
        std::unordered_map<std::string, FieldInfo> fields,
        std::unordered_map<std::string, MethodInfo> methods,
        std::shared_ptr<Function> constructor
    );

    const std::string& className() const { return className_; }
    const FieldInfo* getField(const std::string& name) const;
    const MethodInfo* getMethod(const std::string& name) const;
    const Function* constructor() const { return constructor_.get(); }
};
```

#### ClassInstance

```cpp
class ClassInstance {
private:
    std::shared_ptr<ClassDefinition> classDef_;
    std::unordered_map<std::string, Value> fields_;

public:
    ClassInstance(std::shared_ptr<ClassDefinition> classDef);

    Value getField(const std::string& name, AccessContext context) const;
    void setField(const std::string& name, const Value& value, AccessContext context);
    Value callMethod(const std::string& name, const std::vector<Value>& args, AccessContext context);

    const ClassDefinition* classDefinition() const { return classDef_.get(); }
};
```

#### Value 타입 추가

```cpp
enum class ValueType {
    // ... 기존 타입들 ...
    CLASS_DEFINITION,
    CLASS_INSTANCE,
};

class Value {
    // ...
    static Value createClassDefinition(std::shared_ptr<ClassDefinition> classDef);
    static Value createClassInstance(std::shared_ptr<ClassInstance> instance);

    ClassDefinition* asClassDefinition() const;
    ClassInstance* asClassInstance() const;
};
```

---

## 구현 계획

### Phase 7.1.1: Lexer & Token (1일)

**작업**:
- Token.h에 새로운 TokenType 추가
- Lexer.cpp에 키워드 매핑 추가
- 테스트 작성

**파일**:
- `src/lexer/Token.h`
- `src/lexer/Lexer.cpp`
- `tests/LexerTest.cpp`

**테스트 예시**:
```cpp
TEST(LexerTest, ShouldTokenizeClassKeywords) {
    std::string input = "클래스 생성자 공개 비공개 자신";
    Lexer lexer(input);

    EXPECT_EQ(lexer.nextToken().type, TokenType::KEULLAESU);
    EXPECT_EQ(lexer.nextToken().type, TokenType::SAENGSEONGJA);
    EXPECT_EQ(lexer.nextToken().type, TokenType::GONGGAE);
    EXPECT_EQ(lexer.nextToken().type, TokenType::BIGONGGAE);
    EXPECT_EQ(lexer.nextToken().type, TokenType::JASIN);
}
```

### Phase 7.1.2: AST 노드 (1-2일)

**작업**:
- Node.h에 NodeType 추가
- Statement.h에 ClassStatement, FieldDeclaration, MethodDeclaration, ConstructorDeclaration 추가
- Expression.h에 MemberAccessExpression, ThisExpression, NewExpression 추가
- toString() 구현

**파일**:
- `src/ast/Node.h`
- `src/ast/Statement.h`
- `src/ast/Expression.h`

### Phase 7.1.3: Parser (2-3일)

**작업**:
- Parser.cpp에 parseClassStatement() 추가
- parseFieldDeclaration() 구현
- parseMethodDeclaration() 구현
- parseConstructorDeclaration() 구현
- parseMemberAccess() (Pratt parsing infix 함수)
- parseThisExpression() (prefix 함수)
- parseNewExpression() (prefix 함수)
- 테스트 작성

**파일**:
- `src/parser/Parser.cpp`
- `src/parser/Parser.h`
- `tests/ParserTest.cpp`

**파싱 우선순위**:
- 멤버 접근(`.`): `Precedence::CALL` (함수 호출과 동일)

**테스트 예시**:
```cpp
TEST(ParserTest, ShouldParseClassDefinition) {
    std::string input = R"(
        클래스 사람 {
            비공개 문자열 이름

            생성자(이름) {
                자신.이름 = 이름
            }

            공개 함수 인사하기() {
                출력("안녕")
            }
        }
    )";

    Lexer lexer(input);
    Parser parser(lexer);
    auto program = parser.parseProgram();

    ASSERT_EQ(parser.errors().size(), 0);
    ASSERT_EQ(program->statements().size(), 1);

    auto* classStmt = dynamic_cast<ClassStatement*>(program->statements()[0].get());
    ASSERT_NE(classStmt, nullptr);
    EXPECT_EQ(classStmt->className(), "사람");
    EXPECT_EQ(classStmt->fields().size(), 1);
    EXPECT_NE(classStmt->constructor(), nullptr);
    EXPECT_EQ(classStmt->methods().size(), 1);
}
```

### Phase 7.1.4: SemanticAnalyzer (1-2일)

**작업**:
- ClassStatement 검증
  - 필드 중복 체크
  - 메서드 중복 체크
  - 생성자 존재 여부 (선택적)
- MemberAccessExpression 검증
  - 클래스 인스턴스인지 확인
  - 필드/메서드 존재 여부
  - 접근 제어 검증
- ThisExpression 검증
  - 클래스 메서드/생성자 내에서만 사용 가능
- 테스트 작성

**파일**:
- `src/semantic/SemanticAnalyzer.cpp`
- `tests/SemanticAnalyzerTest.cpp`

### Phase 7.1.5: Bytecode Compiler (2-3일)

**작업**:
- OpCode.h에 새로운 OpCode 추가
- Compiler.cpp에 컴파일 로직 추가
  - compileClassStatement()
  - compileMemberAccess()
  - compileThisExpression()
  - compileNewExpression()
- 테스트 작성

**파일**:
- `src/bytecode/OpCode.h`
- `src/bytecode/OpCode.cpp`
- `src/bytecode/Compiler.cpp`
- `tests/CompilerTest.cpp`

**바이트코드 예시**:
```
클래스 사람 {
    비공개 문자열 이름

    생성자(이름) {
        자신.이름 = 이름
    }
}

변수 홍길동 = 사람("홍길동")

↓ 컴파일

CLASS_DEF "사람"
  FIELD "이름" PRIVATE
  CONSTRUCTOR:
    LOAD_THIS
    LOAD_VAR "이름"
    STORE_FIELD "이름"
    RETURN

LOAD_CONST "홍길동"
NEW_INSTANCE "사람" 1
STORE_VAR "홍길동"
```

### Phase 7.1.6: VM 실행 로직 (2-3일)

**작업**:
- VM.cpp에 OpCode 실행 추가
- ClassDefinition 관리 (클래스 테이블)
- ClassInstance 생성 및 관리
- 필드 접근 (LOAD_FIELD, STORE_FIELD)
- 메서드 호출 (CALL_METHOD)
- this 바인딩
- 테스트 작성

**파일**:
- `src/bytecode/VM.cpp`
- `src/evaluator/Value.h`
- `src/evaluator/Value.cpp`
- `tests/VMClassTest.cpp`

### Phase 7.1.7: 통합 테스트 (1-2일)

**작업**:
- End-to-end 테스트 작성
- 예제 파일 작성
- 버그 수정

**파일**:
- `tests/ClassIntegrationTest.cpp`
- `examples/class_example.ksj`
- `examples/class_person.ksj`
- `examples/class_rectangle.ksj`

### Phase 7.1.8: 문서화 (1일)

**작업**:
- ROADMAP.md 업데이트
- 사용자 가이드 작성
- API 문서 업데이트

**파일**:
- `docs/ROADMAP.md`
- `docs/USER_GUIDE.md`
- `docs/CLASS_SYSTEM_DESIGN.md`

---

## 예제

### 예제 1: 간단한 클래스

```ksj
클래스 계좌 {
    비공개 정수 잔액

    생성자(초기잔액) {
        이.잔액 = 초기잔액
    }

    공개 함수 입금하기(금액) {
        이.잔액 = 이.잔액 + 금액
    }

    공개 함수 출금하기(금액) {
        만약 금액 > 이.잔액 {
            출력("잔액 부족")
            반환 거짓
        }
        이.잔액 = 이.잔액 - 금액
        반환 참
    }

    공개 함수 잔액조회() {
        반환 이.잔액
    }
}

변수 내계좌 = 계좌(10000)
내계좌.입금하기(5000)
내계좌.출금하기(3000)
출력(내계좌.잔액조회())  # 12000
```

### 예제 2: 사각형 클래스

```ksj
클래스 사각형 {
    비공개 실수 너비
    비공개 실수 높이

    생성자(너비, 높이) {
        이.너비 = 너비
        이.높이 = 높이
    }

    공개 함수 넓이() {
        반환 이.너비 * 이.높이
    }

    공개 함수 둘레() {
        반환 2.0 * (이.너비 + 이.높이)
    }
}

변수 직사각형 = 사각형(5.0, 3.0)
출력("넓이: " + 직사각형.넓이())  # 15.0
출력("둘레: " + 직사각형.둘레())  # 16.0
```

### 예제 3: 접근 제어

```ksj
클래스 비밀상자 {
    비공개 문자열 비밀

    생성자(비밀) {
        이.비밀 = 비밀
    }

    공개 함수 확인하기(추측) {
        만약 추측 == 이.비밀 {
            반환 참
        }
        반환 거짓
    }

    비공개 함수 내부함수() {
        출력("비공개 메서드입니다")
    }
}

변수 상자 = 비밀상자("1234")
출력(상자.확인하기("1234"))  # 참

# 에러: 비공개 필드 접근 불가
# 출력(상자.비밀)

# 에러: 비공개 메서드 호출 불가
# 상자.내부함수()
```

---

## 타임라인

| 단계 | 기간 | 담당 |
|------|------|------|
| 7.1.1: Lexer & Token | 1일 | 완료 예정 |
| 7.1.2: AST 노드 | 1-2일 | 완료 예정 |
| 7.1.3: Parser | 2-3일 | 완료 예정 |
| 7.1.4: SemanticAnalyzer | 1-2일 | 완료 예정 |
| 7.1.5: Bytecode Compiler | 2-3일 | 완료 예정 |
| 7.1.6: VM 실행 로직 | 2-3일 | 완료 예정 |
| 7.1.7: 통합 테스트 | 1-2일 | 완료 예정 |
| 7.1.8: 문서화 | 1일 | 완료 예정 |
| **전체** | **12-17일** | **2-3주** |

---

## 참고 자료

- [Python Classes](https://docs.python.org/3/tutorial/classes.html)
- [JavaScript Classes](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Classes)
- [Crafting Interpreters - Classes](https://craftinginterpreters.com/classes.html)
- [ROADMAP.md](./ROADMAP.md)

---

**마지막 업데이트**: 2025-11-17
**다음 단계**: Phase 7.1.1 Lexer & Token 구현
