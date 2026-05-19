#include <iostream>
#include <iomanip>
#include <string>
#include <utility>
#include <vector>
#include <map>
#include "../include/vm/core/sandbox.hpp"

class Parser {
    struct Option {
        std::string brief;
        std::string detail;
        std::string text;
        bool flag;
        bool required;
    };
    std::string description;
    std::string program;
    std::vector<Option> options;

public:
    explicit Parser(std::string text) : description(std::move(text)) {}

    void add(const std::string &brief, const std::string &detail, const std::string &text, const bool flag, const bool required = false) {
        options.push_back({brief, detail, text, flag, required});
    }

    std::map<std::string, std::string> parse(int count, char* inputs[]) {
        program = inputs[0];
        size_t slash = program.find_last_of("/\\");
        if (slash != std::string::npos) {
            program = program.substr(slash + 1);
        }

        if (count == 1) {
            for (const auto& option : options) {
                if (option.required) {
                    error("the following arguments are required: " + option.brief + "/" + option.detail);
                }
            }
        }

        std::map<std::string, std::string> results;

        if (count == 2 && (std::string(inputs[1]) == "-h" || std::string(inputs[1]) == "--help")) {
            help();
            std::exit(0);
        }

        for (int index = 1; index < count; ++index) {
            std::string current = inputs[index];
            bool matched = false;

            for (const auto& option : options) {
                if (current == option.brief || current == option.detail) {
                    matched = true;
                    if (option.flag) {
                        results[option.detail] = "true";
                    } else {
                        if (index + 1 < count && std::string(inputs[index + 1])[0] != '-') {
                            results[option.detail] = inputs[++index];
                        }
                    }
                    break;
                }
            }

            if (!matched) {
                if (current[0] != '-' && results["--path"].empty()) {
                    results["--path"] = current;
                } else {
                    std::cerr << program << ": error: unrecognized arguments: " << current << "\n";
                    std::exit(1);
                }
            }
        }

        // Validate required elements safely using .find()
        for (const auto& option : options) {
            if (option.required && results.find(option.detail) == results.end()) {
                error("the following arguments are required: " + option.brief + "/" + option.detail);
            }
        }

        return results;
    }

    void help() const {
        std::cout << "usage: " << program << " [-h]";
        for (const auto& option : options) {
            std::cout << (option.required ? " " : " [") << option.brief << ", " << option.detail;
            if (!option.flag) {
                std::string upper = option.detail.substr(2);
                for (char& character : upper) {
                    character = static_cast<char>(std::toupper(static_cast<unsigned char>(character)));
                }
                std::cout << " " << upper;
            }
            if (!option.required) {
                std::cout << "]";
            }
        }

        std::cout << "\n\n" << description << "\n\noptions:\n";
        std::cout << "  -h, --help                       show this help message and exit\n";

        for (const auto& option : options) {
            std::string left = option.brief + ", " + option.detail;
            if (!option.flag) {
                std::string upper = option.detail.substr(2);
                for (char& character : upper) {
                    character = static_cast<char>(std::toupper(static_cast<unsigned char>(character)));
                }
                left += " " + upper;
            }
            std::cout << "  " << std::left << std::setw(33) << left << option.text << "\n";
        }
    }

    void error(const std::string& message) const {
        std::cerr << "usage: " << program << " [-h]";
        for (const auto& option : options) {
            std::cerr << (option.required ? " " : " [") << option.brief << ", " << option.detail;
            if (!option.flag) {
                std::string upper = option.detail.substr(2);
                for (char& character : upper) {
                    character = static_cast<char>(std::toupper(static_cast<unsigned char>(character)));
                }
                std::cerr << " " << upper;
            }
            if (!option.required) {
                std::cerr << "]";
            }
        }
        std::cerr << "\n" << program << ": error: " << message << "\n";
        std::exit(1);
    }
};

int main(const int count, char* inputs[]) {
    Parser parser("Sandbox control utility for Polyblox environment tools.");

    parser.add("-m", "--memory", "Memory size for the execution context.hpp", false, false);
    parser.add("-i", "--instructions", "CPU step instruction limit quota", false, false);
    parser.add("-p", "--path", "Script path directory destination to run", false, true);
    parser.add("-t", "--timeout", "Maximum execution time duration in seconds", false, false);
    parser.add("-o", "--output", "File path destination to redirect sandbox logs", false, false);
    parser.add("-v", "--verbose", "Enable descriptive execution runtime profiling metrics", true, false);
    parser.add("-n", "--network", "Completely isolate network stack access from context.hpp", true, false);

    std::map<std::string, std::string> arguments = parser.parse(count, inputs);

    const std::string path = arguments["--path"];

    // Verbose reporting block
    if (!arguments["--verbose"].empty()) {
        std::cout << "Path: " << path << "\n";
        if (!arguments["--memory"].empty()) std::cout << "Memory: " << arguments["--memory"] << "\n";
        if (!arguments["--instructions"].empty()) std::cout << "Instructions: " << arguments["--instructions"] << "\n";
        if (!arguments["--timeout"].empty()) std::cout << "Timeout: " << arguments["--timeout"] << "\n";
        if (!arguments["--output"].empty()) std::cout << "Output: " << arguments["--output"] << "\n";
        if (!arguments["--network"].empty()) std::cout << "Network: true\n";
    }


    const sandbox::Sandbox sandbox;

    const int status = sandbox.run(path.c_str());

    return (status == 1) ? 0 : 1;
}