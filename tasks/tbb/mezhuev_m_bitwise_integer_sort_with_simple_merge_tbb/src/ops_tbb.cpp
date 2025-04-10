#include "tbb/mezhuev_m_bitwise_integer_sort_with_simple_merge_tbb/include/ops_tbb.hpp"

#include <algorithm>
#include <cmath>
#include <ranges>
#include <tbb/tbb.h>
#include <vector>

#include "core/task/include/task.hpp"
#include "oneapi/tbb/task_arena.h"
#include "oneapi/tbb/task_group.h"

namespace mezhuev_m_bitwise_integer_sort_tbb {

bool SortTBB::PreProcessingImpl() {
  unsigned int input_size = task_data->inputs_count[0];
  auto* in_ptr = reinterpret_cast<int*>(task_data->inputs[0]);

  if (input_size == 0) {
    output_ = input_;
    return true;
  }

  input_ = std::vector<int>(in_ptr, in_ptr + input_size);
  unsigned int output_size = task_data->outputs_count[0];
  output_ = std::vector<int>(output_size, 0);

  max_value_ = *std::ranges::max_element(input_, [](int a, int b) { return std::abs(a) < std::abs(b); });
  max_value_ = std::abs(max_value_);

  return true;
}

bool SortTBB::ValidationImpl() { return task_data->inputs_count[0] == task_data->outputs_count[0]; }

bool SortTBB::RunImpl() {
  if (input_.empty()) {
    output_ = input_;
    return true;
  }

  std::vector<int> negative;
  std::vector<int> positive;

  for (int num : input_) {
    if (num < 0) {
      negative.push_back(-num);
    } else {
      positive.push_back(num);
    }
  }

  oneapi::tbb::task_arena arena(1);
  arena.execute([&] {
    tbb::task_group tg;

    tg.run([&] {
      int exp = 1;
      while (max_value_ / exp > 0) {
        std::vector<int> count(10, 0);
        std::vector<int> output(positive.size());

        for (int i = 0; i < static_cast<int>(positive.size()); ++i) {
          int digit = (positive[i] / exp) % 10;
          count[digit]++;
        }

        for (int j = 1; j < 10; ++j) {
          count[j] += count[j - 1];
        }

        for (int i = positive.size() - 1; i >= 0; --i) {
          int digit = (positive[i] / exp) % 10;
          output[--count[digit]] = positive[i];
        }

        for (int i = 0; i < static_cast<int>(positive.size()); ++i) {
          positive[i] = output[i];
        }

        exp *= 10;
      }
    });

    tg.run([&] {
      int exp = 1;
      while (max_value_ / exp > 0) {
        std::vector<int> count(10, 0);
        std::vector<int> output(negative.size());

        for (int i = 0; i < static_cast<int>(negative.size()); ++i) {
          int digit = (negative[i] / exp) % 10;
          count[digit]++;
        }

        for (int j = 1; j < 10; ++j) {
          count[j] += count[j - 1];
        }

        for (int i = negative.size() - 1; i >= 0; --i) {
          int digit = (negative[i] / exp) % 10;
          output[--count[digit]] = negative[i];
        }

        for (int i = 0; i < static_cast<int>(negative.size()); ++i) {
          negative[i] = output[i];
        }

        exp *= 10;
      }
    });

    tg.wait();
  });

  std::ranges::reverse(negative);
  for (int& num : negative) {
    num = -num;
  }

  output_.clear();
  output_.insert(output_.end(), negative.begin(), negative.end());
  output_.insert(output_.end(), positive.begin(), positive.end());

  return true;
}

bool SortTBB::PostProcessingImpl() {
  if (input_.empty()) {
    return true;
  }

  auto* out_ptr = reinterpret_cast<int*>(task_data->outputs[0]);
  std::ranges::copy(output_, out_ptr);
  return true;
}

}  // namespace mezhuev_m_bitwise_integer_sort_tbb
