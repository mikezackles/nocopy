#include <catch.hpp>

#include <iostream>
#include <nocopy.hpp>

#include <chrono>
#include <iterator>
#include <unordered_map>
#include <random>

constexpr unsigned long long operator "" _KB(unsigned long long val) {
  return val << 10;
}

struct measurement {
  NOCOPY_FIELD(delta, float);
  NOCOPY_FIELD(first, uint32_t);
  NOCOPY_FIELD(second, uint8_t);
  NOCOPY_FIELD(third, NOCOPY_ARRAY(int8_t, 4));
  NOCOPY_FIELD(coords, NOCOPY_ARRAY(uint8_t, 10));
  NOCOPY_FIELD(locations, NOCOPY_ARRAY(uint32_t, 20));
  using type = nocopy::datapack<delta, first, second, coords, locations>;
};
using measurement_t = measurement::type;

TEST_CASE("deref", "[heap]") {
  std::array<unsigned char, 1_KB> buffer;
  auto heap = nocopy::heap64::create(
    buffer.data(), sizeof(buffer)
  , [](auto heap) { return heap; }
  , [](std::error_code) -> nocopy::heap64 { throw std::runtime_error{"shouldn't happen"}; }
  );
  auto result = heap.malloc_range<measurement_t>(
    2
  , [](auto result) { return result; }
  , [](std::error_code) -> nocopy::heap64::range_reference<measurement_t> {
      throw std::runtime_error{"shouldn't happen"};
    }
  );
  auto m = heap.deref(result);
  m[1].get<measurement::first>() = 2000;
  heap.free(result);
}

TEST_CASE("raw heap corruption", "[heap]") {
  using offset_t = nocopy::heap32::offset_t;
  std::array<unsigned char, 100_KB> buffer;

  auto seed = std::chrono::system_clock::now().time_since_epoch().count();
  std::default_random_engine generator{seed};
  auto rand_block_size = std::bind(std::uniform_int_distribution<offset_t>{1, 1_KB}, generator);
  auto flip_coin = std::bind(std::uniform_int_distribution<std::size_t>{0, 3}, generator);

  nocopy::heap32::create(
    buffer.data(), sizeof(buffer)
  , [&](nocopy::heap32 heap) {
      std::unordered_map<offset_t, offset_t> alloc_lookup{};
      std::vector<nocopy::heap32::range_reference<uint8_t>> allocs;
      auto free_random = [&]() {
        auto rand_alloc_offset = std::bind(std::uniform_int_distribution<std::size_t>{0, allocs.size()-1}, generator);
        auto alloc_offset = rand_alloc_offset();
        auto heap_ref = allocs[alloc_offset];
        heap.free(heap_ref);
        auto it = allocs.begin();
        std::advance(it, alloc_offset);
        allocs.erase(it);
        alloc_lookup.erase(static_cast<offset_t>(heap_ref));
      };
      for(auto i = 0u; i < 1000u; ++i) {
        if (allocs.size() == 0 || flip_coin() < 3) {
          auto block_size = rand_block_size();
          bool success = false;
          while (!success) {
            heap.malloc_range<uint8_t>(block_size
            , [&](auto result) {
                allocs.push_back(result);
                alloc_lookup.insert({{static_cast<offset_t>(result), block_size}});
                success = true;
              }
            , [&](std::error_code) {
                free_random();
              }
            );
          }
        } else {
          free_random();
        }
      }
      std::unordered_map<offset_t, offset_t> free_offsets;
      std::unordered_map<offset_t, offset_t> alloced_offsets;
      heap.each_block([&](auto size, bool is_free, auto offset) {
        if (is_free) {
          free_offsets.insert({{offset, size}});
        } else {
          alloced_offsets.insert({{offset, size}});
        }
        constexpr auto alignment = sizeof(uint32_t);
        REQUIRE(size % alignment == 0);
        REQUIRE(offset % alignment == 0);
      });
      for (auto pair : alloc_lookup) {
        // Note that actual size is at the very least aligned, so it is often
        // larger than the requested size. It may be even bigger than that if
        // the block was too small to split off the remainder.
        offset_t offset = pair.first;
        auto requested_size = pair.second;
        auto actual_size = alloced_offsets[offset];
        REQUIRE(actual_size >= requested_size);
      }
      heap.each_free_block([&](auto size, auto offset) {
        auto it = free_offsets.find(offset);
        REQUIRE(it != free_offsets.end());
        REQUIRE(it->second == size);
        free_offsets.erase(it);
      });
      REQUIRE(free_offsets.size() == 0);
      while(allocs.size() > 0) {
        free_random();
      }
    }
  , [](std::error_code) { REQUIRE(false); }
  );
}
