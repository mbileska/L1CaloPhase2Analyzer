#include <ap_int.h>

#include <array>
#include <cctype>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "algo_top.h"

namespace {
constexpr unsigned int kLinks = 6;
constexpr unsigned int kWordsPerLink = 9;

bool dataLine(const std::string& line) {
  std::istringstream input(line);
  std::string token;
  input >> token;
  if (token.empty() || token[0] == '#') return false;
  for (char c : token) {
    if (std::isxdigit(static_cast<unsigned char>(c)) == 0) return false;
  }
  return true;
}
}  // namespace

int main(int argc, char** argv) {
  if (argc != 3) {
    std::cerr << "Usage: gctsum_togt_firmware_replay INPUT OUTPUT\n";
    return 2;
  }
  std::ifstream input(argv[1]);
  if (!input.is_open()) {
    std::cerr << "Could not open " << argv[1] << '\n';
    return 2;
  }

  using Row = std::array<uint64_t, kLinks>;
  std::vector<Row> rows;
  std::string line;
  while (std::getline(input, line)) {
    if (!dataLine(line)) continue;
    std::istringstream parser(line);
    std::string token;
    parser >> token;
    Row row{};
    for (unsigned int link = 0; link < kLinks; ++link) {
      if (!(parser >> token)) {
        std::cerr << "Input row has fewer than 6 links\n";
        return 2;
      }
      row[link] = std::stoull(token, nullptr, 16);
    }
    rows.push_back(row);
  }
  if (rows.empty() || rows.size() % kWordsPerLink != 0) {
    std::cerr << "Input must contain a nonzero multiple of 9 data rows\n";
    return 2;
  }

  std::ofstream output(argv[2]);
  output << "WordCnt";
  for (unsigned int link = 0; link < kLinks; ++link) output << "    OUT_" << link;
  output << "\n#BeginData\n";

  for (unsigned int event = 0; event < rows.size() / kWordsPerLink; ++event) {
    ap_uint<576> linkInput[kLinks] = {};
    ap_uint<576> linkOutput[kLinks] = {};
    for (unsigned int word = 0; word < kWordsPerLink; ++word) {
      for (unsigned int link = 0; link < kLinks; ++link) {
        linkInput[link].range(word * 64 + 63, word * 64) = rows[event * kWordsPerLink + word][link];
      }
    }
    algo_top_GT(linkInput, linkOutput);
    for (unsigned int word = 0; word < kWordsPerLink; ++word) {
      output << std::setw(4) << std::setfill('0') << std::hex << word;
      for (unsigned int link = 0; link < kLinks; ++link) {
        output << "    " << std::setw(16) << std::setfill('0') << std::nouppercase
               << linkOutput[link].range(word * 64 + 63, word * 64).to_uint64();
      }
      output << '\n';
    }
  }
  return 0;
}
