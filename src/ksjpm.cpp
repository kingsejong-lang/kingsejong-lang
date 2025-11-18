/**
 * @file ksjpm.cpp
 * @brief KingSejong 패키지 관리자 CLI 도구
 * @author KingSejong Team
 * @date 2025-11-19
 *
 * ksjpm - KingSejong Package Manager
 *
 * 사용법:
 *   ksjpm init [name] [version]     - 새 패키지 초기화
 *   ksjpm install                   - 모든 의존성 설치
 *   ksjpm add <package>[@version]   - 패키지 추가
 *   ksjpm remove <package>          - 패키지 제거
 *   ksjpm list                      - 설치된 패키지 목록
 *   ksjpm info [package]            - 패키지 정보 출력
 *   ksjpm help                      - 도움말 출력
 */

#include "package/PackageManager.h"
#include <iostream>
#include <string>
#include <vector>

using namespace kingsejong::package;

void printUsage() {
    std::cout << R"(
KingSejong Package Manager (ksjpm) v1.0.0

사용법:
  ksjpm <command> [options]

명령어:
  init [name] [version]       새 패키지 초기화
                              예: ksjpm init my-project 1.0.0

  install                     package.ksj의 모든 의존성 설치
                              예: ksjpm install

  add <package>[@version]     패키지 추가 및 설치
                              예: ksjpm add stdlib@^1.0.0
                              예: ksjpm add my-lib

  add-dev <package>[@version] 개발 의존성 패키지 추가
                              예: ksjpm add-dev test-framework@^2.0.0

  remove <package>            패키지 제거
                              예: ksjpm remove my-lib

  list                        설치된 패키지 목록 출력
                              예: ksjpm list

  info [package]              패키지 정보 출력
                              예: ksjpm info
                              예: ksjpm info stdlib

  help                        이 도움말 출력
                              예: ksjpm help

옵션:
  -h, --help                  도움말 출력
  -v, --version               버전 정보 출력

예제:
  # 새 프로젝트 초기화
  ksjpm init my-awesome-app 1.0.0

  # 의존성 추가
  ksjpm add stdlib@^1.0.0
  ksjpm add-dev test-framework

  # 모든 의존성 설치
  ksjpm install

  # 패키지 정보 확인
  ksjpm info stdlib

  # 설치된 패키지 목록
  ksjpm list
)" << std::endl;
}

void printVersion() {
    std::cout << "ksjpm version 1.0.0" << std::endl;
    std::cout << "KingSejong Package Manager" << std::endl;
}

std::pair<std::string, std::string> parsePackageSpec(const std::string& spec) {
    // package@version 형태를 파싱
    size_t atPos = spec.find('@');
    if (atPos != std::string::npos) {
        return {spec.substr(0, atPos), spec.substr(atPos + 1)};
    }
    return {spec, ""};
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printUsage();
        return 1;
    }

    std::string command = argv[1];

    // 도움말
    if (command == "help" || command == "-h" || command == "--help") {
        printUsage();
        return 0;
    }

    // 버전
    if (command == "-v" || command == "--version") {
        printVersion();
        return 0;
    }

    try {
        PackageManager pm(".");

        // init: 패키지 초기화
        if (command == "init") {
            std::string name = "my-package";
            std::string version = "1.0.0";
            std::string description = "";

            if (argc >= 3) name = argv[2];
            if (argc >= 4) version = argv[3];
            if (argc >= 5) description = argv[4];

            pm.initPackage(name, version, description);
            return 0;
        }

        // install: 모든 의존성 설치
        if (command == "install") {
            if (pm.installDependencies()) {
                return 0;
            } else {
                return 1;
            }
        }

        // add: 패키지 추가
        if (command == "add") {
            if (argc < 3) {
                std::cerr << "❌ 오류: 패키지 이름을 지정하세요" << std::endl;
                std::cerr << "사용법: ksjpm add <package>[@version]" << std::endl;
                return 1;
            }

            auto [pkgName, pkgVersion] = parsePackageSpec(argv[2]);

            if (pm.installPackage(pkgName, pkgVersion)) {
                return 0;
            } else {
                return 1;
            }
        }

        // add-dev: 개발 의존성 추가
        if (command == "add-dev") {
            if (argc < 3) {
                std::cerr << "❌ 오류: 패키지 이름을 지정하세요" << std::endl;
                std::cerr << "사용법: ksjpm add-dev <package>[@version]" << std::endl;
                return 1;
            }

            auto [pkgName, pkgVersion] = parsePackageSpec(argv[2]);

            if (pm.installDevPackage(pkgName, pkgVersion)) {
                return 0;
            } else {
                return 1;
            }
        }

        // remove: 패키지 제거
        if (command == "remove") {
            if (argc < 3) {
                std::cerr << "❌ 오류: 패키지 이름을 지정하세요" << std::endl;
                std::cerr << "사용법: ksjpm remove <package>" << std::endl;
                return 1;
            }

            std::string pkgName = argv[2];

            if (pm.uninstallPackage(pkgName)) {
                return 0;
            } else {
                return 1;
            }
        }

        // list: 설치된 패키지 목록
        if (command == "list") {
            pm.listInstalledPackages();
            return 0;
        }

        // info: 패키지 정보
        if (command == "info") {
            std::string pkgName = "";
            if (argc >= 3) {
                pkgName = argv[2];
            }

            pm.printPackageInfo(pkgName);
            return 0;
        }

        // 알 수 없는 명령어
        std::cerr << "❌ 오류: 알 수 없는 명령어 '" << command << "'" << std::endl;
        std::cerr << "사용 가능한 명령어를 보려면 'ksjpm help'를 실행하세요" << std::endl;
        return 1;

    } catch (const std::exception& e) {
        std::cerr << "❌ 오류: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
