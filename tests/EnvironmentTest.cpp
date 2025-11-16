/**
 * @file EnvironmentTest.cpp
 * @brief Environment (변수 환경/스코프) 시스템 테스트
 * @author KingSejong Team
 * @date 2025-11-07
 */

#include <gtest/gtest.h>
#include "evaluator/Environment.h"
#include "evaluator/Value.h"

using namespace kingsejong::evaluator;

/**
 * @test 기본 환경 생성
 */
TEST(EnvironmentTest, ShouldCreateDefaultEnvironment)
{
    auto env = std::make_shared<Environment>();

    EXPECT_EQ(env->size(), 0);
    EXPECT_EQ(env->outer(), nullptr);
}

/**
 * @test 변수 설정 및 조회
 */
TEST(EnvironmentTest, ShouldSetAndGetVariable)
{
    auto env = std::make_shared<Environment>();

    env->set("x", Value::createInteger(42));

    Value val = env->get("x");
    EXPECT_TRUE(val.isInteger());
    EXPECT_EQ(val.asInteger(), 42);
}

/**
 * @test 여러 변수 저장
 */
TEST(EnvironmentTest, ShouldStoreMultipleVariables)
{
    auto env = std::make_shared<Environment>();

    env->set("a", Value::createInteger(10));
    env->set("b", Value::createString("안녕"));
    env->set("c", Value::createBoolean(true));

    EXPECT_EQ(env->size(), 3);
    EXPECT_EQ(env->get("a").asInteger(), 10);
    EXPECT_EQ(env->get("b").asString(), "안녕");
    EXPECT_TRUE(env->get("c").asBoolean());
}

/**
 * @test 변수 덮어쓰기
 */
TEST(EnvironmentTest, ShouldOverwriteVariable)
{
    auto env = std::make_shared<Environment>();

    env->set("x", Value::createInteger(100));
    EXPECT_EQ(env->get("x").asInteger(), 100);

    env->set("x", Value::createString("변경됨"));
    EXPECT_EQ(env->get("x").asString(), "변경됨");

    EXPECT_EQ(env->size(), 1);  // 변수 개수는 그대로
}

/**
 * @test 존재하지 않는 변수 조회 시 예외 발생
 */
TEST(EnvironmentTest, ShouldThrowWhenGettingUndefinedVariable)
{
    auto env = std::make_shared<Environment>();

    EXPECT_THROW(env->get("undefined"), std::runtime_error);
}

/**
 * @test exists() - 현재 스코프만 확인
 */
TEST(EnvironmentTest, ExistsShouldCheckCurrentScopeOnly)
{
    auto env = std::make_shared<Environment>();

    env->set("local", Value::createInteger(42));

    EXPECT_TRUE(env->exists("local"));
    EXPECT_FALSE(env->exists("nothere"));
}

/**
 * @test 중첩 스코프 생성
 */
TEST(EnvironmentTest, ShouldCreateEnclosedEnvironment)
{
    auto outer = std::make_shared<Environment>();
    auto inner = outer->createEnclosed();

    EXPECT_NE(inner, nullptr);
    EXPECT_EQ(inner->outer(), outer);
    EXPECT_EQ(outer->outer(), nullptr);
}

/**
 * @test 스코프 체인을 통한 변수 조회
 */
TEST(EnvironmentTest, ShouldGetVariableFromOuterScope)
{
    auto outer = std::make_shared<Environment>();
    outer->set("global", Value::createInteger(100));

    auto inner = outer->createEnclosed();

    // 내부 스코프에서 외부 변수 접근
    Value val = inner->get("global");
    EXPECT_TRUE(val.isInteger());
    EXPECT_EQ(val.asInteger(), 100);
}

/**
 * @test 변수 섀도잉 (Shadowing)
 */
TEST(EnvironmentTest, ShouldShadowVariableInInnerScope)
{
    auto outer = std::make_shared<Environment>();
    outer->set("x", Value::createInteger(10));

    auto inner = outer->createEnclosed();
    inner->set("x", Value::createInteger(20));

    // 내부 스코프에서는 섀도잉된 값
    EXPECT_EQ(inner->get("x").asInteger(), 20);

    // 외부 스코프는 원래 값 유지
    EXPECT_EQ(outer->get("x").asInteger(), 10);
}

/**
 * @test existsInChain() - 스코프 체인 전체 확인
 */
TEST(EnvironmentTest, ExistsInChainShouldCheckFullChain)
{
    auto outer = std::make_shared<Environment>();
    outer->set("global", Value::createInteger(42));

    auto inner = outer->createEnclosed();
    inner->set("local", Value::createString("지역"));

    // 현재 스코프 변수
    EXPECT_TRUE(inner->existsInChain("local"));

    // 외부 스코프 변수
    EXPECT_TRUE(inner->existsInChain("global"));

    // 존재하지 않는 변수
    EXPECT_FALSE(inner->existsInChain("nothere"));
}

/**
 * @test exists()와 existsInChain()의 차이
 */
TEST(EnvironmentTest, ExistsVsExistsInChain)
{
    auto outer = std::make_shared<Environment>();
    outer->set("global", Value::createInteger(42));

    auto inner = outer->createEnclosed();

    // exists()는 현재 스코프만
    EXPECT_FALSE(inner->exists("global"));

    // existsInChain()은 전체 체인
    EXPECT_TRUE(inner->existsInChain("global"));
}

/**
 * @test 3단계 중첩 스코프
 */
TEST(EnvironmentTest, ShouldHandleMultipleLevelsOfNesting)
{
    auto level1 = std::make_shared<Environment>();
    level1->set("a", Value::createInteger(1));

    auto level2 = level1->createEnclosed();
    level2->set("b", Value::createInteger(2));

    auto level3 = level2->createEnclosed();
    level3->set("c", Value::createInteger(3));

    // 가장 내부에서 모든 레벨 접근 가능
    EXPECT_EQ(level3->get("a").asInteger(), 1);
    EXPECT_EQ(level3->get("b").asInteger(), 2);
    EXPECT_EQ(level3->get("c").asInteger(), 3);

    // 중간 레벨에서는 외부만 접근 가능
    EXPECT_EQ(level2->get("a").asInteger(), 1);
    EXPECT_EQ(level2->get("b").asInteger(), 2);
    EXPECT_THROW(level2->get("c"), std::runtime_error);

    // 최외곽에서는 자신의 변수만
    EXPECT_EQ(level1->get("a").asInteger(), 1);
    EXPECT_THROW(level1->get("b"), std::runtime_error);
    EXPECT_THROW(level1->get("c"), std::runtime_error);
}

/**
 * @test 복잡한 섀도잉 시나리오
 */
TEST(EnvironmentTest, ComplexShadowingScenario)
{
    auto outer = std::make_shared<Environment>();
    outer->set("x", Value::createInteger(1));
    outer->set("y", Value::createInteger(2));

    auto middle = outer->createEnclosed();
    middle->set("x", Value::createInteger(10));  // x 섀도잉
    middle->set("z", Value::createInteger(30));

    auto inner = middle->createEnclosed();
    inner->set("x", Value::createInteger(100));  // x 다시 섀도잉

    // 각 레벨에서 x 값 확인
    EXPECT_EQ(inner->get("x").asInteger(), 100);
    EXPECT_EQ(middle->get("x").asInteger(), 10);
    EXPECT_EQ(outer->get("x").asInteger(), 1);

    // y는 섀도잉 없음
    EXPECT_EQ(inner->get("y").asInteger(), 2);
    EXPECT_EQ(middle->get("y").asInteger(), 2);

    // z는 middle에서만
    EXPECT_EQ(inner->get("z").asInteger(), 30);
    EXPECT_EQ(middle->get("z").asInteger(), 30);
    EXPECT_THROW(outer->get("z"), std::runtime_error);
}

/**
 * @test keys() - 변수 이름 목록 반환
 */
TEST(EnvironmentTest, ShouldReturnVariableKeys)
{
    auto env = std::make_shared<Environment>();

    env->set("a", Value::createInteger(1));
    env->set("b", Value::createInteger(2));
    env->set("c", Value::createInteger(3));

    auto keys = env->keys();

    EXPECT_EQ(keys.size(), 3);
    EXPECT_NE(std::find(keys.begin(), keys.end(), "a"), keys.end());
    EXPECT_NE(std::find(keys.begin(), keys.end(), "b"), keys.end());
    EXPECT_NE(std::find(keys.begin(), keys.end(), "c"), keys.end());
}

/**
 * @test keys()는 현재 스코프만 반환
 */
TEST(EnvironmentTest, KeysShouldReturnCurrentScopeOnly)
{
    auto outer = std::make_shared<Environment>();
    outer->set("global", Value::createInteger(1));

    auto inner = outer->createEnclosed();
    inner->set("local", Value::createInteger(2));

    auto keys = inner->keys();

    EXPECT_EQ(keys.size(), 1);
    EXPECT_NE(std::find(keys.begin(), keys.end(), "local"), keys.end());
    EXPECT_EQ(std::find(keys.begin(), keys.end(), "global"), keys.end());
}

/**
 * @test clear() - 모든 변수 제거
 */
TEST(EnvironmentTest, ShouldClearAllVariables)
{
    auto env = std::make_shared<Environment>();

    env->set("a", Value::createInteger(1));
    env->set("b", Value::createInteger(2));
    env->set("c", Value::createInteger(3));

    EXPECT_EQ(env->size(), 3);

    env->clear();

    EXPECT_EQ(env->size(), 0);
    EXPECT_THROW(env->get("a"), std::runtime_error);
}

/**
 * @test clear()는 현재 스코프만 영향
 */
TEST(EnvironmentTest, ClearShouldAffectCurrentScopeOnly)
{
    auto outer = std::make_shared<Environment>();
    outer->set("global", Value::createInteger(100));

    auto inner = outer->createEnclosed();
    inner->set("local", Value::createInteger(200));

    inner->clear();

    // 내부 스코프는 비어있음
    EXPECT_EQ(inner->size(), 0);
    EXPECT_THROW(inner->get("local"), std::runtime_error);

    // 외부 스코프는 그대로
    EXPECT_EQ(outer->size(), 1);
    EXPECT_EQ(outer->get("global").asInteger(), 100);

    // 내부에서 외부 변수는 여전히 접근 가능
    EXPECT_EQ(inner->get("global").asInteger(), 100);
}

/**
 * @test size() - 현재 스코프의 변수 개수
 */
TEST(EnvironmentTest, SizeShouldReturnCurrentScopeVariableCount)
{
    auto env = std::make_shared<Environment>();

    EXPECT_EQ(env->size(), 0);

    env->set("a", Value::createInteger(1));
    EXPECT_EQ(env->size(), 1);

    env->set("b", Value::createInteger(2));
    EXPECT_EQ(env->size(), 2);

    env->set("a", Value::createString("덮어쓰기"));  // 덮어쓰기
    EXPECT_EQ(env->size(), 2);  // 개수는 그대로
}

/**
 * @test 빈 환경에서 keys() 호출
 */
TEST(EnvironmentTest, KeysShouldReturnEmptyVectorForEmptyEnvironment)
{
    auto env = std::make_shared<Environment>();

    auto keys = env->keys();

    EXPECT_TRUE(keys.empty());
    EXPECT_EQ(keys.size(), 0);
}

/**
 * @test 모든 Value 타입 저장 및 조회
 */
TEST(EnvironmentTest, ShouldStoreAllValueTypes)
{
    auto env = std::make_shared<Environment>();

    env->set("int", Value::createInteger(42));
    env->set("float", Value::createFloat(3.14));
    env->set("string", Value::createString("안녕"));
    env->set("bool", Value::createBoolean(true));
    env->set("null", Value::createNull());

    EXPECT_EQ(env->size(), 5);

    EXPECT_EQ(env->get("int").asInteger(), 42);
    EXPECT_DOUBLE_EQ(env->get("float").asFloat(), 3.14);
    EXPECT_EQ(env->get("string").asString(), "안녕");
    EXPECT_TRUE(env->get("bool").asBoolean());
    EXPECT_TRUE(env->get("null").isNull());
}

/**
 * @test 한글 변수명 지원
 */
TEST(EnvironmentTest, ShouldSupportKoreanVariableNames)
{
    auto env = std::make_shared<Environment>();

    env->set("이름", Value::createString("세종대왕"));
    env->set("나이", Value::createInteger(32));
    env->set("활성화", Value::createBoolean(true));

    EXPECT_EQ(env->get("이름").asString(), "세종대왕");
    EXPECT_EQ(env->get("나이").asInteger(), 32);
    EXPECT_TRUE(env->get("활성화").asBoolean());
}

/**
 * @test 스코프 체인에서 가장 가까운 변수 반환
 */
TEST(EnvironmentTest, ShouldReturnClosestVariableInChain)
{
    auto level1 = std::make_shared<Environment>();
    level1->set("x", Value::createInteger(1));

    auto level2 = level1->createEnclosed();
    level2->set("x", Value::createInteger(2));

    auto level3 = level2->createEnclosed();
    level3->set("x", Value::createInteger(3));

    auto level4 = level3->createEnclosed();
    // level4에는 x가 없음

    // level4에서 x 조회 시 level3의 값 반환 (가장 가까운 스코프)
    EXPECT_EQ(level4->get("x").asInteger(), 3);
}

/**
 * @test 독립적인 환경들은 서로 간섭하지 않음
 */
TEST(EnvironmentTest, IndependentEnvironmentsShouldNotInterfere)
{
    auto env1 = std::make_shared<Environment>();
    auto env2 = std::make_shared<Environment>();

    env1->set("x", Value::createInteger(100));
    env2->set("x", Value::createInteger(200));

    EXPECT_EQ(env1->get("x").asInteger(), 100);
    EXPECT_EQ(env2->get("x").asInteger(), 200);

    env1->clear();

    EXPECT_EQ(env1->size(), 0);
    EXPECT_EQ(env2->size(), 1);
    EXPECT_EQ(env2->get("x").asInteger(), 200);
}

/**
 * @test setWithLookup() - 현재 스코프에 변수가 있으면 현재 스코프에서 업데이트
 */
TEST(EnvironmentTest, SetWithLookupShouldUpdateCurrentScope)
{
    auto env = std::make_shared<Environment>();

    env->set("x", Value::createInteger(10));
    env->setWithLookup("x", Value::createInteger(20));

    EXPECT_EQ(env->get("x").asInteger(), 20);
}

/**
 * @test setWithLookup() - 외부 스코프에 변수가 있으면 외부 스코프에서 업데이트
 */
TEST(EnvironmentTest, SetWithLookupShouldUpdateOuterScope)
{
    auto outer = std::make_shared<Environment>();
    outer->set("x", Value::createInteger(10));

    auto inner = outer->createEnclosed();

    // 내부 스코프에서 setWithLookup() 호출
    inner->setWithLookup("x", Value::createInteger(20));

    // 외부 스코프의 x가 업데이트되어야 함
    EXPECT_EQ(outer->get("x").asInteger(), 20);
    // 내부 스코프에는 x가 없어야 함
    EXPECT_FALSE(inner->exists("x"));
}

/**
 * @test setWithLookup() - 어디에도 없으면 현재 스코프에 새로 생성
 */
TEST(EnvironmentTest, SetWithLookupShouldCreateInCurrentScope)
{
    auto outer = std::make_shared<Environment>();
    outer->set("y", Value::createInteger(100));

    auto inner = outer->createEnclosed();

    // 외부에도 내부에도 없는 변수
    inner->setWithLookup("x", Value::createInteger(42));

    // 내부 스코프에 새로 생성되어야 함
    EXPECT_TRUE(inner->exists("x"));
    EXPECT_EQ(inner->get("x").asInteger(), 42);
    // 외부 스코프에는 없어야 함
    EXPECT_FALSE(outer->exists("x"));
}

/**
 * @test setWithLookup() - 중첩된 스코프에서 올바른 스코프 업데이트
 */
TEST(EnvironmentTest, SetWithLookupShouldUpdateCorrectScope)
{
    auto level1 = std::make_shared<Environment>();
    level1->set("x", Value::createInteger(1));
    level1->set("y", Value::createInteger(10));

    auto level2 = level1->createEnclosed();
    level2->set("y", Value::createInteger(20));  // y는 level2에도 있음

    auto level3 = level2->createEnclosed();

    // level3에서 x 업데이트 → level1의 x가 업데이트되어야 함
    level3->setWithLookup("x", Value::createInteger(100));
    EXPECT_EQ(level1->get("x").asInteger(), 100);
    EXPECT_FALSE(level2->exists("x"));
    EXPECT_FALSE(level3->exists("x"));

    // level3에서 y 업데이트 → level2의 y가 업데이트되어야 함 (가장 가까운 스코프)
    level3->setWithLookup("y", Value::createInteger(200));
    EXPECT_EQ(level2->get("y").asInteger(), 200);
    EXPECT_EQ(level1->get("y").asInteger(), 10);  // level1의 y는 그대로
    EXPECT_FALSE(level3->exists("y"));
}

/**
 * @test setWithLookup() - 섀도잉된 변수는 가장 가까운 스코프 업데이트
 */
TEST(EnvironmentTest, SetWithLookupShouldRespectShadowing)
{
    auto outer = std::make_shared<Environment>();
    outer->set("x", Value::createInteger(10));

    auto inner = outer->createEnclosed();
    inner->set("x", Value::createInteger(20));  // 섀도잉

    // inner에서 setWithLookup() 호출
    inner->setWithLookup("x", Value::createInteger(30));

    // inner의 x가 업데이트되어야 함
    EXPECT_EQ(inner->get("x").asInteger(), 30);
    // outer의 x는 그대로
    EXPECT_EQ(outer->get("x").asInteger(), 10);
}
