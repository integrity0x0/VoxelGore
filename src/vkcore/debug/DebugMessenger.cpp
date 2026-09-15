#include "DebugMessenger.h"

namespace vkcore {

DebugMessenger::DebugMessenger(const vkcore::Instance& instance,
                               PFN_vkDebugUtilsMessengerCallbackEXT callbackFunction)
    : instance(&instance) {
  VkDebugUtilsMessengerCreateInfoEXT debugMessengerCI = {
      VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT};
  debugMessengerCI.pfnUserCallback = callbackFunction;
  debugMessengerCI.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
                                     VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                     VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
  debugMessengerCI.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
  const auto& debugUtilsTable = instance.getDispatchTable().debugUtilsTable;

  if (!debugUtilsTable.has_value()) {
    throw SystemError(VK_ERROR_UNKNOWN,
                      "Failed to create debug messenger: functions are not loaded");
  }
  VkDebugUtilsMessengerEXT debugMessengerRaw = VK_NULL_HANDLE;
  SystemError::Check(debugUtilsTable->vkCreateDebugUtilsMessengerEXT(
                         instance.handle(), &debugMessengerCI, nullptr, &debugMessengerRaw),
                     "Failed to create VkDebugUtilsMessenger");
  debugMessenger = UniqueDebugUtilsMessengerEXT(
      debugMessengerRaw, {instance.handle(), debugUtilsTable->vkDestroyDebugUtilsMessengerEXT});
}

}  // namespace vkcore