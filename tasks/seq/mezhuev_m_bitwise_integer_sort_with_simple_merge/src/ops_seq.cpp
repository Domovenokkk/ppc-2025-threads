#include "seq/mezhuev_m_bitwise_integer_sort_with_simple_merge/include/ops_seq.hpp"

#include <algorithm>
#include <iostream>

namespace mezhuev_m_bitwise_integer_sort_seq {

bool TestTaskSequential::PreProcessingImpl() {
  if (!task_data) {
    std::cerr << "Error: Task data is null." << std::endl;
    return false;
  }

  unsigned int input_size = task_data->inputs_count[0];
  unsigned int output_size = task_data->outputs_count[0];

  if (input_size != output_size) {
    std::cerr << "Error: Input and output sizes do not match in PreProcessing." << std::endl;
    return false;
  }

  if (input_size > 0) {
    if (task_data->inputs[0] == nullptr || task_data->outputs[0] == nullptr) {
      std::cerr << "Error: Invalid input or output data for non-zero size." << std::endl;
      return false;
    }

    auto* in_ptr = reinterpret_cast<int*>(task_data->inputs[0]);
    input_ = std::vector<int>(in_ptr, in_ptr + input_size);
  } else {
    input_.clear();
  }

  output_.resize(output_size);
  return true;
}

bool TestTaskSequential::ValidationImpl() {
  if (!task_data) {
    std::cerr << "Error: Task data is null." << std::endl;
    return false;
  }

  if (task_data->inputs_count.size() < 1 || task_data->outputs_count.size() < 1) {
    std::cerr << "Error: inputs_count or outputs_count is empty." << std::endl;
    return false;
  }

  unsigned int input_size = task_data->inputs_count[0];
  unsigned int output_size = task_data->outputs_count[0];

  if (input_size != output_size) {
    std::cerr << "Error: Input and output sizes do not match." << std::endl;
    return false;
  }

  if (input_size > 0) {
    if (task_data->inputs[0] == nullptr) {
      std::cerr << "Error: Input data is null for non-zero size." << std::endl;
      return false;
    }
  }

  if (output_size > 0) {
    if (task_data->outputs[0] == nullptr) {
      std::cerr << "Error: Output data is null for non-zero size." << std::endl;
      return false;
    }
  }

  return true;
}

bool TestTaskSequential::RunImpl() {
  if (input_.empty()) {
    output_ = input_;
    return true;
  }

  std::vector<int> negative, positive;
  for (int num : input_) {
    if (num < 0) {
      negative.push_back(-num);
    } else {
      positive.push_back(num);
    }
  }

  auto radix_sort = [](std::vector<int>& nums) {
    if (nums.empty()) return;
    int max_num = *std::max_element(nums.begin(), nums.end());
    for (int exp = 1; max_num / exp > 0; exp *= 10) {
      std::vector<int> output(nums.size());
      std::vector<int> count(10, 0);
      for (int num : nums) count[(num / exp) % 10]++;
      for (int i = 1; i < 10; ++i) count[i] += count[i - 1];
      for (int i = nums.size() - 1; i >= 0; --i) {
        int digit = (nums[i] / exp) % 10;
        output[count[digit] - 1] = nums[i];
        count[digit]--;
      }
      nums = output;
    }
  };

  radix_sort(positive);
  radix_sort(negative);

  std::reverse(negative.begin(), negative.end());
  for (int& num : negative) num = -num;

  output_.clear();
  output_.insert(output_.end(), negative.begin(), negative.end());
  output_.insert(output_.end(), positive.begin(), positive.end());

  return true;
}

bool TestTaskSequential::PostProcessingImpl() {
  if (!task_data || (task_data->outputs[0] == nullptr)) {
    std::cerr << "Error: Invalid output data." << std::endl;
    return false;
  }

  if (output_.empty()) {
    std::cerr << "Error: Output data is empty." << std::endl;
    return false;
  }

  for (size_t i = 0; i < output_.size(); i++) {
    reinterpret_cast<int*>(task_data->outputs[0])[i] = output_[i];
  }

  return true;
}

}  // namespace mezhuev_m_bitwise_integer_sort_seq
