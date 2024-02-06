// Copyright (c) Google LLC 2019
//
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <ios>
#include <iterator>
#include <memory>
#include <string>
#include <tuple>
#include <vector>

#include "./test_utils.h"

#if !defined(TEST_DATA_PATH)
#include "tools/cpp/runfiles/runfiles.h"
#endif

namespace brunsli {

size_t StringOutputFunction(void* data, const uint8_t* buf, size_t count) {
  std::string* output = reinterpret_cast<std::string*>(data);
  output->append(reinterpret_cast<const char*>(buf), count);
  return count;
}

static const uint8_t kSmallBrunsliFile[] = {
  /* Signature */
  0x0a, 0x04,
        0x42, 0xd2, 0xd5, 0x4e,

  /* Header */
  0x12, 0x08,
        0x08, 0x10, 0x10, 0x10, 0x18, 0x02, 0x20, 0x00,

  /* Internal */
  0x22, 0xa8, 0x00,
        0xe0, 0xb6, 0x01, 0x04, 0x41, 0x10, 0x5a, 0x06,
        0xc5, 0x3a, 0xac, 0x18, 0xdd, 0x4c, 0x55, 0x67,
        0xb5, 0x4f, 0xe9, 0x91, 0xe4, 0xbe, 0x6f, 0x19,
        0x69, 0x37, 0x53, 0x35, 0xe3, 0x3a, 0x6e, 0x06,  // 020
        0xf8, 0x01, 0x04, 0x2a, 0x8b, 0x08, 0x01, 0x00,

  /* Metadata */
  0x1a, 0x06,
        0x01,
        0x03, 0x00, 0x80, 0x3a, 0x03,

  /* Quant */
  0x2a, 0x82, 0x00,
        0x29, 0x53,

  /* Histo */
  0x32, 0xb8, 0x80, 0x00,
        0x00, 0x0e, 0x0b, 0xf4, 0x56, 0xcd, 0x01, 0x90,
        0x00, 0x09, 0xca, 0x8f, 0xd6, 0x18, 0x12, 0x9b,
        0x76, 0x27, 0xfd, 0x70, 0x2c, 0xd7, 0xb3, 0x6d,
        0xdc, 0xf7, 0x1e, 0x8f, 0x80, 0xa1, 0x17, 0x92,  // 020
        0xfe, 0x14, 0xa0, 0x7e, 0x79, 0x2f, 0xef, 0xf5,
        0xbd, 0x62, 0x32, 0x9c, 0xa0, 0xef, 0x33, 0xf0,
        0x1e, 0xcf, 0x02, 0xdb, 0x56, 0x5f, 0x0d, 0x2a,

  /* Unknown */
  0x7a, 0x15,
        'k', 'o', 't', ' ', 'l', 'o', 'm',
        'o', 'm', ' ', 'k', 'o', 'l', 'o',
        'l', ' ', 's', 'l', 'o', 'n', 'a',

  /* DC */
  0x3a, 0x8e, 0x80, 0x00,
        0x13, 0x00, 0x00, 0x00, 0x3a, 0x40, 0x97, 0xa7,
        0xcf, 0xf5, 0x00, 0x00, 0xff, 0xff,

  /* AC */
  0x42, 0xc2, 0x82, 0x00,
        0xf3, 0x4c, 0xc4, 0xdc, 0x00, 0xd0, 0x5d, 0xfe,
        0xf9, 0x3e, 0xc1, 0x15, 0xec, 0x84, 0xad, 0xdf,
        0x0e, 0x04, 0x59, 0xdb, 0x81, 0x2a, 0x43, 0x3d,
        0x95, 0xc1, 0x2a, 0x94, 0x57, 0x8d, 0x67, 0xea,  // 020
        0x85, 0x0f, 0x73, 0xb8, 0x9d, 0x0a, 0xa8, 0x26,
        0xad, 0x7a, 0xa3, 0x55, 0xa4, 0x53, 0x29, 0xa9,
        0xf0, 0x7b, 0x40, 0xe8, 0x32, 0x77, 0x05, 0x48,
        0xf2, 0x51, 0x4f, 0x05, 0x0a, 0xa5, 0xb9, 0xc5,  // 040
        0x2b, 0x5e, 0x0c, 0xce, 0xe2, 0x0a, 0x6b, 0x6d,
        0x37, 0x93, 0x33, 0x75, 0x43, 0x98, 0x5d, 0xe6,
        0x54, 0x93, 0x2f, 0x5f, 0x11, 0x97, 0x94, 0x54,
        0xed, 0xff, 0x9b, 0x5d, 0x20, 0x28, 0x47, 0xe4,  // 060
        0x77, 0x9d, 0x14, 0x81, 0x66, 0x04, 0xb6, 0x72,
        0x05, 0xe0, 0x54, 0x10, 0x4c, 0x2f, 0x51, 0x72,
        0x9e, 0x4b, 0xa3, 0x3c, 0x27, 0x1f, 0x14, 0x20,
        0x5e, 0xb7, 0x87, 0x34, 0xa7, 0xc8, 0x90, 0xc3,  // 080
        0x2f, 0x80, 0x41, 0x0f, 0xd0, 0xa9, 0x8e, 0x72,
        0xf7, 0x09, 0x85, 0x67, 0xd2, 0xa0, 0xad, 0x3a,
        0xf4, 0x28, 0x47, 0x31, 0x36, 0xe4, 0xbe, 0xa3,
        0x73, 0x85, 0x55, 0xc8, 0x16, 0x98, 0x0e, 0xb3,  // 0A0
        0xd0, 0xcd, 0x00, 0x54, 0x93, 0x8f, 0x35, 0xf3,
        0xe9, 0x6b, 0xac, 0xde, 0xd6, 0x1f, 0x64, 0x61,
        0x18, 0xf7, 0x6d, 0x9c, 0x2b, 0xc8, 0x7e, 0xc0,
        0xa3, 0xa1, 0x87, 0xa2, 0x02, 0x06, 0x71, 0xf5,  // 0C0
        0x9b, 0xd4, 0xec, 0xdd, 0x6b, 0xef, 0x99, 0x3a,
        0x9c, 0xa2, 0xaa, 0x49, 0xc6, 0x46, 0x66, 0xa5,
        0xb8, 0x1f, 0x4a, 0x2a, 0xf5, 0xdb, 0x10, 0xfa,
        0x17, 0x29, 0x27, 0x50, 0xe5, 0xf8, 0x34, 0x4d,  // 0E0
        0xc2, 0x95, 0x9c, 0x15, 0xa9, 0x71, 0xea, 0xda,
        0x6a, 0xf4, 0x31, 0x93, 0x4c, 0x1d, 0x52, 0x98,
        0x09, 0xc7, 0xd5, 0x24, 0x11, 0x63, 0x0a, 0x2f,
        0x25, 0x15, 0xfb, 0x2f, 0x08, 0x4a, 0x56, 0xed,  // 100
        0x27, 0x93, 0x08, 0x5c, 0x3a, 0x6d, 0x79, 0x78,
        0x94, 0xdc, 0xe3, 0xbf, 0x4a, 0xc0, 0x06, 0xcd,
        0x46, 0xb4, 0x12, 0x17, 0x20, 0x58, 0x21, 0x1a,
        0xa7, 0x52, 0xb9, 0x66, 0x08, 0x80, 0x9d, 0xcf,  // 120
        0x5d, 0x9b, 0xcd, 0x05, 0xc9, 0x6f, 0x51, 0x8c,
        0x62, 0xe9, 0xef, 0x28, 0xdb, 0xec, 0x4b, 0x61,
        0x04, 0x15, 0x9f, 0xf5, 0xfc, 0x52, 0x9b, 0xdb,
        0xaf, 0xde, 0x0a, 0x94, 0xc0, 0x7e, 0x84, 0x52,  // 140
        0xc6, 0x00
};

static const uint8_t kFallbackBrunsliFile[] = {
  /* Signature */
  0x0a, 0x04,
        0x42, 0xd2, 0xd5, 0x4e,

  /* Header */
  0x12, 0x02,
        0x18, 0x04,

  /* Internal */
  0x4a, 0x04,
        0xde, 0xad, 0xbe, 0xef
};

std::vector<uint8_t> GetSmallBrunsliFile() {
  return std::vector<uint8_t>(kSmallBrunsliFile,
                              kSmallBrunsliFile + sizeof(kSmallBrunsliFile));
}

std::vector<uint8_t> GetFallbackBrunsliFile() {
  return std::vector<uint8_t>(
      kFallbackBrunsliFile,
      kFallbackBrunsliFile + sizeof(kFallbackBrunsliFile));
}

namespace {
uint32_t readU32(const uint8_t* data) {
  return data[3] | (data[2] << 8) | (data[1] << 16) | (data[0] << 24);
}

#if defined(TEST_DATA_PATH)
std::string GetTestDataPath(const std::string& filename) {
  return std::string(TEST_DATA_PATH "/") + filename;
}
#else
using bazel::tools::cpp::runfiles::Runfiles;
const std::unique_ptr<Runfiles> kRunfiles(Runfiles::Create(""));
std::string GetTestDataPath(const std::string& filename) {
  std::string root(BRUNSLI_ROOT_PACKAGE "/tests/testdata/");
  return kRunfiles->Rlocation(root + filename);
}
#endif
}  // namespace

std::vector<std::tuple<std::vector<uint8_t>>> ParseMar(const void* data,
                                                             size_t size) {
  std::vector<std::tuple<std::vector<uint8_t>>> result;
  const uint8_t* bytes = reinterpret_cast<const uint8_t*>(data);
  if (size < 8) std::abort();
  uint32_t sig = readU32(bytes);
  if (sig != 0x4D415231) std::abort();
  uint32_t index = readU32(bytes + 4);
  if ((index + 4 < index) || (index + 4 > size)) std::abort();
  uint32_t index_size = readU32(bytes + index);
  index += 4;
  if (index + index_size < index) std::abort();
  uint32_t index_end = index + index_size;
  if (index_end > size) std::abort();
  while (index < index_end) {
    if ((index + 13 < index) || (index + 13 > index_end)) std::abort();
    uint32_t offset = readU32(bytes + index);
    uint32_t len = readU32(bytes + index + 4);
    if (offset + len < offset || offset + len > size) std::abort();
    index += 12;
    while (bytes[index++]) {
      if (index == index_end) std::abort();
    }
    const uint8_t* start = bytes + offset;
    result.emplace_back(std::vector<uint8_t>(start, start + len));
  }
  return result;
}

std::vector<uint8_t> ReadTestData(const std::string& filename) {
  std::string full_path = GetTestDataPath(filename);
  std::ifstream file(full_path, std::ios::binary);
  std::vector<char> str((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
  if (!file.good()) std::abort();
  const uint8_t* raw = reinterpret_cast<const uint8_t*>(str.data());
  std::vector<uint8_t> data(raw, raw + str.size());
  return data;
}

}  // namespace brunsli
