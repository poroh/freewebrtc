//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Murmur hash implementation
//

#include "util/util_hash_murmur.hpp"

namespace freewebrtc::util::hash {

uint64_t murmur64(const ConstBinaryView& view, uint64_t seed) noexcept {
  const uint64_t m = 0xc6a4a7935bd1e995ULL;
  const int r = 47;
  uint64_t h = seed ^ (view.size() * m);
  const uint64_t *data = (const uint64_t *)view.data();
  const uint64_t *end = data + view.size() / sizeof(*data);

  while (data != end) {
      uint64_t v = *data++;
      v *= m;
      v ^= v >> r;
      v *= m;

      h ^= v;
      h *= m;
  }

  const unsigned char * data2 = (const unsigned char*)data;

  switch (view.size() & 7) {
  case 7: h ^= uint64_t(data2[6]) << 48;
  case 6: h ^= uint64_t(data2[5]) << 40;
  case 5: h ^= uint64_t(data2[4]) << 32;
  case 4: h ^= uint64_t(data2[3]) << 24;
  case 3: h ^= uint64_t(data2[2]) << 16;
  case 2: h ^= uint64_t(data2[1]) << 8;
  case 1: h ^= uint64_t(data2[0]);
      h *= m;
  };
  h ^= h >> r;
  h *= m;
  h ^= h >> r;
  return h;
}

std::size_t murmur(const ConstBinaryView& view, uint64_t seed) noexcept {
    return murmur64(view, seed);
}

}
