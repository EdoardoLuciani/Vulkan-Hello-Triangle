#include "vulkan_helper.h"

VkPresentModeKHR vulkan_helper::select_presentation_mode(const std::vector<VkPresentModeKHR>& presentation_modes, VkPresentModeKHR desired_presentation_mode) {
    VkPresentModeKHR selected_present_mode;
    if (std::find(presentation_modes.begin(), presentation_modes.end(), desired_presentation_mode) != presentation_modes.end()) {
        selected_present_mode = VK_PRESENT_MODE_MAILBOX_KHR;
    }
    else {
        selected_present_mode = VK_PRESENT_MODE_FIFO_KHR;
    }
    return selected_present_mode;
}

uint32_t vulkan_helper::select_number_of_images(const VkSurfaceCapabilitiesKHR& surface_capabilities) {
    uint32_t number_of_images = surface_capabilities.minImageCount + 1;
    if ((surface_capabilities.maxImageCount > 0) && (number_of_images > surface_capabilities.maxImageCount)) { number_of_images = surface_capabilities.maxImageCount; }
    return number_of_images;
}

VkExtent2D vulkan_helper::select_size_of_images(const VkSurfaceCapabilitiesKHR& surface_capabilities, VkExtent2D desired_size_of_images) {
    if (0xFFFFFFFF == surface_capabilities.currentExtent.width) {

        if (desired_size_of_images.width < surface_capabilities.minImageExtent.width) {
            desired_size_of_images.width = surface_capabilities.minImageExtent.width;
        }
        else if (desired_size_of_images.width > surface_capabilities.maxImageExtent.width) {
            desired_size_of_images.width = surface_capabilities.maxImageExtent.width;
        }

        if (desired_size_of_images.height < surface_capabilities.minImageExtent.height) {
            desired_size_of_images.height = surface_capabilities.minImageExtent.height;
        }
        else if (desired_size_of_images.height > surface_capabilities.maxImageExtent.height) {
            desired_size_of_images.height = surface_capabilities.maxImageExtent.height;
        }
    }
    else {
        desired_size_of_images = surface_capabilities.currentExtent;
    }
    return desired_size_of_images;
}

VkImageUsageFlags vulkan_helper::select_image_usage(const VkSurfaceCapabilitiesKHR& surface_capabilities, VkImageUsageFlags desired_usages) {
    VkImageUsageFlags image_usage;
    image_usage = desired_usages & surface_capabilities.supportedUsageFlags;
    if (desired_usages != image_usage) {
        image_usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    }
    return image_usage;
}

VkSurfaceTransformFlagBitsKHR vulkan_helper::select_surface_transform(const VkSurfaceCapabilitiesKHR& surface_capabilities, VkSurfaceTransformFlagBitsKHR desired_transform) {
    VkSurfaceTransformFlagBitsKHR surface_transform;
    if (surface_capabilities.supportedTransforms & desired_transform) {
        surface_transform = desired_transform;
    }
    else {
        surface_transform = surface_capabilities.currentTransform;
    }
    return surface_transform;
}

VkSurfaceFormatKHR vulkan_helper::select_surface_format(const std::vector<VkSurfaceFormatKHR>& surface_formats, VkSurfaceFormatKHR desired_surface_format) {
    VkSurfaceFormatKHR selected_surface_format;
    if ((1 == surface_formats.size()) &&
        (VK_FORMAT_UNDEFINED == surface_formats[0].format)) {
        selected_surface_format.format = desired_surface_format.format;
        selected_surface_format.colorSpace = desired_surface_format.colorSpace;
        return selected_surface_format;
    }

    for (auto& surface_format : surface_formats) {
        if ((desired_surface_format.format == surface_format.format) &&
            (desired_surface_format.colorSpace == surface_format.colorSpace)) {
            selected_surface_format.format = desired_surface_format.format;
            selected_surface_format.colorSpace = desired_surface_format.colorSpace;
            return surface_format;
        }
    }

    for (auto& surface_format : surface_formats) {
        if ((desired_surface_format.format == surface_format.format)) {
            selected_surface_format.format = desired_surface_format.format;
            selected_surface_format.colorSpace = surface_format.colorSpace;
            return selected_surface_format;
        }
    }

    selected_surface_format = surface_formats[0];
    return selected_surface_format;
}

uint32_t vulkan_helper::select_memory_index(const VkPhysicalDeviceMemoryProperties& physical_device_memory_properties, const VkMemoryRequirements& memory_requirements, VkMemoryPropertyFlagBits memory_properties) {
    for (uint32_t type = 0; type < physical_device_memory_properties.memoryTypeCount; ++type) {
        if ((memory_requirements.memoryTypeBits & (1 << type)) &&
            ((physical_device_memory_properties.memoryTypes[type].propertyFlags & memory_properties) == memory_properties)) {
            return type;
        }
    }
    return VK_MAX_MEMORY_TYPES;
}

VkBool32 vulkan_helper::debug_callback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t srcObject, size_t location, int32_t msgCode, const char* pLayerPrefix, const char* pMsg, void* pUserData) {
    if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) {
        std::cerr << "ERROR: [" << pLayerPrefix << "] Code " << msgCode << " : " << pMsg << std::endl;
    }
    else if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT) {
        std::cerr << "WARNING: [" << pLayerPrefix << "] Code " << msgCode << " : " << pMsg << std::endl;
    }
    return VK_FALSE;
}
