#pragma once

#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>

#if __has_include(<filesystem>)
#include <filesystem>
namespace golden_fs = std::filesystem;
#else
#include <experimental/filesystem>
namespace golden_fs = std::experimental::filesystem;
#endif

namespace golden {

inline std::string envOr(const char *name, const char *fallback) {
  const char *v = std::getenv(name);
  return (v && *v) ? std::string(v) : std::string(fallback);
}

inline bool updateMode() {
  const std::string v = envOr("TV_UPDATE_GOLDEN", "0");
  return v == "1" || v == "true" || v == "TRUE" || v == "yes";
}

inline golden_fs::path approvedDir() {
  return golden_fs::path(envOr("TV_GOLDEN_APPROVED_DIR",
#ifdef GOLDEN_APPROVED_DIR
                               GOLDEN_APPROVED_DIR
#else
                               "test/golden/approved"
#endif
                               ));
}

inline golden_fs::path receivedDir() {
  return golden_fs::path(envOr("TV_GOLDEN_RECEIVED_DIR",
#ifdef GOLDEN_RECEIVED_DIR
                               GOLDEN_RECEIVED_DIR
#else
                               "test/golden/received"
#endif
                               ));
}

inline std::string readFile(const golden_fs::path &path) {
  std::ifstream in(path, std::ios::binary);
  if (!in) {
    return {};
  }
  std::ostringstream ss;
  ss << in.rdbuf();
  return ss.str();
}

inline void writeFile(const golden_fs::path &path, const std::string &content) {
  golden_fs::create_directories(path.parent_path());
  std::ofstream out(path, std::ios::binary | std::ios::trunc);
  out << content;
}

inline std::string normalizeNewlines(std::string s) {
  std::string out;
  out.reserve(s.size());
  for (std::size_t i = 0; i < s.size(); ++i) {
    if (s[i] == '\r') {
      if (i + 1 < s.size() && s[i + 1] == '\n') {
        ++i;
      }
      out.push_back('\n');
    } else {
      out.push_back(s[i]);
    }
  }
  return out;
}

struct GoldenResult {
  bool match = false;
  std::string message;
};

inline GoldenResult assertGolden(const std::string &actual,
                                 const std::string &scenarioName) {
  const std::string normalized = normalizeNewlines(actual);
  const golden_fs::path approvedPath =
      approvedDir() / (scenarioName + ".approved.txt");
  const golden_fs::path receivedPath =
      receivedDir() / (scenarioName + ".received.txt");

  if (updateMode()) {
    writeFile(approvedPath, normalized);
    return {true, "Updated golden: " + approvedPath.string()};
  }

  const std::string expected = normalizeNewlines(readFile(approvedPath));
  if (expected.empty() && !golden_fs::exists(approvedPath)) {
    writeFile(receivedPath, normalized);
    return {false, "Missing approved file: " + approvedPath.string() +
                        "\nRun with TV_UPDATE_GOLDEN=1 to create it.\n"
                        "Received written to: " +
                        receivedPath.string()};
  }

  if (expected == normalized) {
    return {true, {}};
  }

  writeFile(receivedPath, normalized);
  std::ostringstream msg;
  msg << "Golden mismatch for '" << scenarioName << "'\n"
      << "  approved: " << approvedPath.string() << "\n"
      << "  received: " << receivedPath.string() << "\n"
      << "Set TV_UPDATE_GOLDEN=1 after reviewing the diff.\n"
      << "--- expected ---\n"
      << expected << "--- actual ---\n"
      << normalized;
  return {false, msg.str()};
}

} // namespace golden
