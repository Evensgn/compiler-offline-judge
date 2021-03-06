#include "TestCase.h"

#include <cctype>
#include <fstream>
#include <iostream>
#include <sstream>
#include <ios>
#include <regex>

namespace sjtu {

TestCase parse(std::ifstream & fin) {
    std::string buffer(static_cast<std::stringstream const &>(std::stringstream() << fin.rdbuf()).str());

    TestCase result;
    result.src = readSource(buffer);

    CIter beg, end;

    std::tie(beg, end) = findBlock(buffer, "comment");
    result.comment = std::string(beg, end);

    std::tie(beg, end) = findBlock(buffer, "input");
    result.input = std::string(beg, end);

    std::tie(beg, end) = findBlock(buffer, "output");
    result.output = std::string(beg, end);

    std::tie(beg, end) = findBlock(buffer, "assert");
    if (beg != end) {
        std::string rawAssertion(beg, end);
        if (rawAssertion == "success_compile")
            result.assertion = AssertionType::SuccessCompile;
        else if (rawAssertion == "failure_compile")
            result.assertion = AssertionType::FailureCompile;
        else if (rawAssertion == "exitcode")
            result.assertion = AssertionType::Exitcode;
        else if (rawAssertion == "runtime_error")
            result.assertion = AssertionType::RuntimeError;
        else if (rawAssertion == "output")
            result.assertion = AssertionType::Output;
        else
            throw InvalidTestCase();
    }

    std::tie(beg, end) = findBlock(buffer, "timeout");
    if (beg != end) {
        std::string rawTimeout(beg, end);
        auto dTimeout = std::stod(rawTimeout, nullptr);
        result.timeout = (std::uint64_t) (dTimeout * 1000);
    }
    
    std::tie(beg, end) = findBlock(buffer, "exitcode");
    if (beg != end) {
        std::string rawExitcode(beg, end);
        result.exitcode = std::stoi(rawExitcode, nullptr);
    }

    std::tie(beg, end) = findBlock(buffer, "phase");
    result.phase = std::string(beg, end);
//    std::string rawPhase(beg, end);
//    if (rawPhase == "semantic pretest")
//        result.phase = TestCasePhase::SemanticPretest;
//    else if (rawPhase == "semantic extended")
//        result.phase = TestCasePhase::SemanticExtended;
//    else if (rawPhase == "codegen pretest")
//        result.phase = TestCasePhase::CodegenPretest;
//    else if (rawPhase == "codegen extended")
//        result.phase = TestCasePhase::CodegenExtended;
//    else if (rawPhase == "optim pretest")
//        result.phase = TestCasePhase::OptimPretest;
//    else if (rawPhase == "optim extended")
//        result.phase = TestCasePhase::OptimExtended;
//    else
//        throw InvalidTestCase();

    return result;
}

std::string readSource(const std::string &buffer) {
    std::smatch m;
    if (!std::regex_search(buffer, m, std::regex("/\\*!! metadata:")))
        throw InvalidTestCase();
    return {buffer.begin(), m[0].first};
}

std::pair<CIter, CIter> findBlock(const std::string &buffer, const std::string &name) {
    std::string title = "=== " + name + " ===";
    CIter beg = buffer.end(), end = buffer.end(), iter;
    for (iter = buffer.begin(); iter < buffer.end() - title.size(); ++iter) {
        if (std::equal(title.begin(), title.end(), iter)) {
            beg = iter + title.length();
            iter = beg;
            break;
        }
    }
    if (beg == buffer.end())
        return {buffer.end(), buffer.end()};

    std::string prefix = "===";
    std::string dataEnd = "!!*";
    for (; iter < buffer.end() - 3; ++iter) {
        if (std::equal(prefix.begin(), prefix.end(), iter)
            || std::equal(dataEnd.begin(), dataEnd.end(), iter)) {
            end = iter - 1;
            break;
        }
    }

    while (!std::isspace(*beg))
        ++beg;
//    while (!std::isspace(*end))
//        --end;
    while (std::isspace(*beg) && beg != end)
        ++beg;
    while (std::isspace(*(end - 1)) && beg != end)
        --end;
    return {beg, end};
}


} // namespace sjtu