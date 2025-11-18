/**
 * @file Package.cpp
 * @brief KingSejong 패키지 메타데이터 구현
 * @author KingSejong Team
 * @date 2025-11-18
 */

#include "package/Package.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace kingsejong {
namespace package {

Package::Package(const std::string& name, const std::string& version)
    : name_(name)
    , version_(version)
    , license_("MIT")
    , main_("index.ksj")
{
}

void Package::addAuthor(const Author& author)
{
    authors_.push_back(author);
}

void Package::addDependency(const Dependency& dep)
{
    dependencies_.push_back(dep);
}

void Package::addDevDependency(const Dependency& dep)
{
    devDependencies_.push_back(dep);
}

void Package::addScript(const std::string& name, const std::string& command)
{
    scripts_[name] = command;
}

bool Package::isValid() const
{
    return !name_.empty() && !version_.empty();
}

std::string Package::toJSON() const
{
    json j;

    j["name"] = name_;
    j["version"] = version_;

    if (!description_.empty()) {
        j["description"] = description_;
    }

    if (!license_.empty()) {
        j["license"] = license_;
    }

    if (!main_.empty()) {
        j["main"] = main_;
    }

    // Authors
    if (!authors_.empty()) {
        json authorsArray = json::array();
        for (const auto& author : authors_) {
            if (author.email.empty()) {
                authorsArray.push_back(author.name);
            } else {
                json authorObj;
                authorObj["name"] = author.name;
                authorObj["email"] = author.email;
                authorsArray.push_back(authorObj);
            }
        }
        j["authors"] = authorsArray;
    }

    // Dependencies
    if (!dependencies_.empty()) {
        json depsObj = json::object();
        for (const auto& dep : dependencies_) {
            depsObj[dep.name] = dep.version;
        }
        j["dependencies"] = depsObj;
    }

    // Dev Dependencies
    if (!devDependencies_.empty()) {
        json devDepsObj = json::object();
        for (const auto& dep : devDependencies_) {
            devDepsObj[dep.name] = dep.version;
        }
        j["dev_dependencies"] = devDepsObj;
    }

    // Scripts
    if (!scripts_.empty()) {
        json scriptsObj = json::object();
        for (const auto& [name, command] : scripts_) {
            scriptsObj[name] = command;
        }
        j["scripts"] = scriptsObj;
    }

    return j.dump(2);
}

Package Package::fromJSON(const std::string& jsonStr)
{
    try {
        json j = json::parse(jsonStr);

        Package pkg;

        // Required fields
        if (!j.contains("name") || !j.contains("version")) {
            throw std::runtime_error("패키지에 'name'과 'version' 필드가 필요합니다");
        }

        pkg.setName(j["name"].get<std::string>());
        pkg.setVersion(j["version"].get<std::string>());

        // Optional fields
        if (j.contains("description")) {
            pkg.setDescription(j["description"].get<std::string>());
        }

        if (j.contains("license")) {
            pkg.setLicense(j["license"].get<std::string>());
        }

        if (j.contains("main")) {
            pkg.setMain(j["main"].get<std::string>());
        }

        // Authors
        if (j.contains("authors")) {
            if (j["authors"].is_array()) {
                for (const auto& authorItem : j["authors"]) {
                    if (authorItem.is_string()) {
                        pkg.addAuthor(Author(authorItem.get<std::string>()));
                    } else if (authorItem.is_object()) {
                        std::string name = authorItem["name"].get<std::string>();
                        std::string email = authorItem.value("email", "");
                        pkg.addAuthor(Author(name, email));
                    }
                }
            }
        } else if (j.contains("author")) {
            // Single author (backward compatibility)
            if (j["author"].is_string()) {
                pkg.addAuthor(Author(j["author"].get<std::string>()));
            }
        }

        // Dependencies
        if (j.contains("dependencies") && j["dependencies"].is_object()) {
            for (auto& [name, version] : j["dependencies"].items()) {
                pkg.addDependency(Dependency(name, version.get<std::string>()));
            }
        }

        // Dev Dependencies
        if (j.contains("dev_dependencies") && j["dev_dependencies"].is_object()) {
            for (auto& [name, version] : j["dev_dependencies"].items()) {
                pkg.addDevDependency(Dependency(name, version.get<std::string>()));
            }
        }

        // Scripts
        if (j.contains("scripts") && j["scripts"].is_object()) {
            for (auto& [name, command] : j["scripts"].items()) {
                pkg.addScript(name, command.get<std::string>());
            }
        }

        return pkg;

    } catch (const json::exception& e) {
        throw std::runtime_error(std::string("JSON 파싱 오류: ") + e.what());
    }
}

Package Package::fromFile(const std::string& filePath)
{
    std::ifstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("파일을 열 수 없습니다: " + filePath);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();

    return fromJSON(buffer.str());
}

} // namespace package
} // namespace kingsejong
