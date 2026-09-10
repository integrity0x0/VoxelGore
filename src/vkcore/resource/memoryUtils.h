#pragma once

#include <span>

#include "../commands/CommandPool.h"
#include "TransferContext.h"

namespace vkcore {
extern void LoadDataToBuffer(const Device& device, TransferContext& transferCtxt,
                             std::span<const std::byte> data, const Buffer& dstBuffer,
                             VkDeviceSize offset = 0);
}  // namespace vkcore