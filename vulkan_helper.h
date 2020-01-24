#pragma once
#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

#include <iostream>
#include <vector>
#include <algorithm>

namespace vulkan_helper {
	VkPresentModeKHR select_presentation_mode(const std::vector<VkPresentModeKHR>& presentation_modes, VkPresentModeKHR desired_presentation_mode);
	uint32_t select_number_of_images(const VkSurfaceCapabilitiesKHR& surface_capabilities);
	VkExtent2D select_size_of_images(const VkSurfaceCapabilitiesKHR& surface_capabilities, VkExtent2D desired_size_of_images);
	VkImageUsageFlags select_image_usage(const VkSurfaceCapabilitiesKHR& surface_capabilities,VkImageUsageFlags desired_usages);
	VkSurfaceTransformFlagBitsKHR select_surface_transform(const VkSurfaceCapabilitiesKHR& surface_capabilities, VkSurfaceTransformFlagBitsKHR desired_transform);
	VkSurfaceFormatKHR select_surface_format(const std::vector<VkSurfaceFormatKHR>& surface_formats, VkSurfaceFormatKHR desired_surface_format);
	uint32_t select_memory_index(const VkPhysicalDeviceMemoryProperties& physical_device_memory_properties, const VkMemoryRequirements& memory_requirements, VkMemoryPropertyFlagBits memory_properties);
	VkBool32 debug_callback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t srcObject, size_t location, int32_t msgCode, const char* pLayerPrefix, const char* pMsg, void* pUserData);
}