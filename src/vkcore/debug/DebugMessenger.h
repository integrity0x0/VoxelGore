#pragma once
#include <iostream>

#include "../instances/Instance.h"

namespace vkcore {

static VKAPI_ATTR VkBool32 VKAPI_CALL
defaultDebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                     VkDebugUtilsMessageTypeFlagsEXT messageType,
                     const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
  if (messageSeverity & (VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                         VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)) {
    std::cerr << "validation layer: " << pCallbackData->pMessage << "\n\n";
    return false;
  }

  return VK_FALSE;
}

class DebugMessenger {
 public:
  DebugMessenger(const vkcore::Instance& instance,
                 PFN_vkDebugUtilsMessengerCallbackEXT callbackFunction = defaultDebugCallback);

 private:
  const vkcore::Instance* instance;
  UniqueDebugUtilsMessengerEXT debugMessenger;
};
}  // namespace vkcore
