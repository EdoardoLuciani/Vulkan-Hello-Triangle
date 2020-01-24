#define VK_USE_PLATFORM_WIN32_KHR
#define VOLK_IMPLEMENTATION
#define GLFW_EXPOSE_NATIVE_WIN32

#include <iostream>
#include <cassert>
#include <fstream>
#include <filesystem>
#include <chrono>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "volk.h"
#include "vulkan_helper.h"

class VulkanTriangle {
private:
	void create_instance();
    void setup_debug_callback();
    void create_window();
    void create_surface();
    void create_logical_device();
    void create_swapchain();
    void create_command_pool();
    void allocate_command_buffers();
    void create_host_buffers();
    void create_device_buffers();
    void create_descriptor_pool();
    void allocate_descriptor_sets();
    void create_renderpass();
    void create_framebuffers();
    void create_pipeline();
    void upload_input_data();
    void record_command_buffers();
    void create_semaphores();
    void frame_loop();

    void on_window_resize();

    VkInstance instance;
    VkDebugReportCallbackEXT debug_report_callback;

    VkExtent2D window_size = { 800,800 };
    GLFWwindow* window;
    VkSurfaceKHR surface;

    VkPhysicalDevice physical_device;
    VkPhysicalDeviceMemoryProperties physical_device_memory_properties;
    uint32_t queue_family_index;
    VkDevice device;
    VkQueue queue;

    VkSwapchainKHR old_swapchain = VK_NULL_HANDLE;
    VkSwapchainCreateInfoKHR swapchain_create_info;
    VkSwapchainKHR swapchain;
    uint32_t swapchain_images_count;
    std::vector<VkImage> swapchain_images;

    VkCommandPool command_pool;
    std::vector<VkCommandBuffer> command_buffers;

    VkBuffer host_vertex_buffer;
    VkBuffer host_m_matrix_buffer;
    VkMemoryRequirements host_memory_requirements[2];
    VkDeviceMemory host_memory;
    void* host_data_pointer;
    VkBuffer device_vertex_buffer;
    VkBuffer device_m_matrix_buffer;
    VkMemoryRequirements device_memory_requirements[2];
    VkDeviceMemory device_memory;

    VkDescriptorPool descriptor_pool;
    VkDescriptorSetLayout descriptor_set_layout;
    VkDescriptorSet descriptor_set;

    VkRenderPass render_pass;
    std::vector<VkFramebuffer> framebuffers;
    std::vector<VkImageView> swapchain_images_views;

    VkPipelineLayout pipeline_layout;
    VkPipeline pipeline;

    std::vector<VkSemaphore> semaphores;
    glm::mat4 mv_matrix;
    double start_time;
    uint32_t rendered_frames = 0;
    uint32_t modulus_result = 0;

    std::chrono::steady_clock::time_point t1;
    std::chrono::steady_clock::time_point t2;
    std::chrono::duration<double> time_span;

    // input data format: XYZ - RGB / XYZ - RGB / XYZ - RGB
    std::vector<glm::vec3> input_data = { {-0.2f,-0.2f,0.5f},{0.5f,0.8f,0.72f},{0.2f,-0.2f,0.5f},{0.0f,0.3f,0.1f},{0.0f,0.2f,0.5f},{0.4f,0.1f,0.8f} };

public:
	VulkanTriangle();
    void start_main_loop();
    ~VulkanTriangle();

    typedef enum Errors {
        VOLK_INITIALIZATION_FAILED = -1,
        GLFW_INITIALIZATION_FAILED = -2,
        INSTANCE_CREATION_FAILED = -3,
        GLFW_WINDOW_CREATION_FAILED = -4,
        SURFACE_CREATION_FAILED = -5,
        DEVICE_CREATION_FAILED = -6,
        SWAPCHAIN_CREATION_FAILED = -7,
        COMMAND_POOL_CREATION_FAILED = -8,
        COMMAND_BUFFER_CREATION_FAILED = -9,
        MEMORY_ALLOCATION_FAILED = -10,
        SHADER_MODULE_CREATION_FAILED = -11,
        ACQUIRE_NEXT_IMAGE_FAILED = -12,
        QUEUE_PRESENT_FAILED = -13
    } Errors;
};

void VulkanTriangle::create_instance() {
    if (volkInitialize() != VK_SUCCESS) { throw VOLK_INITIALIZATION_FAILED; }
    if (glfwInit() != GLFW_TRUE) { throw GLFW_INITIALIZATION_FAILED; }

    VkApplicationInfo application_info = {
        VK_STRUCTURE_TYPE_APPLICATION_INFO,
        nullptr,
        "Vulkan Triangle",
        VK_MAKE_VERSION(1,0,0),
        "Pure Vulkan",
        VK_MAKE_VERSION(1,0,0),
        VK_MAKE_VERSION(1,0,0)
    };

#ifdef NDEBUG
    std::vector<const char*> desired_validation_layers = {};
    std::vector<const char*> desired_instance_level_extensions = { "VK_KHR_surface","VK_KHR_win32_surface" };
#else
    std::vector<const char*> desired_validation_layers = { "VK_LAYER_LUNARG_standard_validation" };
    std::vector<const char*> desired_instance_level_extensions = { "VK_KHR_surface","VK_KHR_win32_surface","VK_EXT_debug_report" };
#endif

    VkInstanceCreateInfo instance_create_info = {
        VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        nullptr,
        0,
        &application_info,
        desired_validation_layers.size(),
        desired_validation_layers.data(),
        desired_instance_level_extensions.size(),
        desired_instance_level_extensions.data()
    };

    if (vkCreateInstance(&instance_create_info, nullptr, &instance)) {
        instance = VK_NULL_HANDLE;
        throw INSTANCE_CREATION_FAILED;
    }
    volkLoadInstance(instance);
}

void VulkanTriangle::create_window() {
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    window = glfwCreateWindow(window_size.width, window_size.height, "Vulkan", nullptr, nullptr);
    if (window == NULL) { throw GLFW_WINDOW_CREATION_FAILED; }
}

void VulkanTriangle::setup_debug_callback() {
    VkDebugReportCallbackCreateInfoEXT debug_report_callback_create_info_EXT = {
        VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT,
        nullptr,
        VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT,
        vulkan_helper::debug_callback,
        nullptr
    };
    vkCreateDebugReportCallbackEXT(instance, &debug_report_callback_create_info_EXT, nullptr, &debug_report_callback);
}

void VulkanTriangle::create_surface() {
    VkWin32SurfaceCreateInfoKHR surface_create_info = {
        VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
        nullptr,
        0,
        GetModuleHandle(NULL),
        glfwGetWin32Window(window)
    };
    if (vkCreateWin32SurfaceKHR(instance, &surface_create_info, nullptr, &surface) != VK_SUCCESS) { throw SURFACE_CREATION_FAILED; }
}

void VulkanTriangle::create_logical_device() {
    uint32_t devices_number;
    vkEnumeratePhysicalDevices(instance, &devices_number, nullptr);
    std::vector<VkPhysicalDevice> devices(devices_number);
    std::vector<VkPhysicalDeviceProperties> devices_properties(devices_number);
    std::vector<VkPhysicalDeviceFeatures> devices_features(devices_number);
    vkEnumeratePhysicalDevices(instance, &devices_number, devices.data());

    for (uint32_t i = 0; i < devices.size(); i++) {
        vkGetPhysicalDeviceProperties(devices[i], &devices_properties[i]);
        vkGetPhysicalDeviceFeatures(devices[i], &devices_features[i]);
    }
    // TODO: check features of all available physical devices to find the one best suited for our usage
    size_t selected_device_number = 0;

    // we get the device memory properties because we need them later for allocations
    physical_device = devices[selected_device_number];
    vkGetPhysicalDeviceMemoryProperties(physical_device, &physical_device_memory_properties);

    uint32_t families_count;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &families_count, nullptr);
    std::vector<VkQueueFamilyProperties> queue_families_properties(families_count);
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &families_count, queue_families_properties.data());

    queue_family_index = -1;
    VkBool32 does_queue_family_support_surface = VK_FALSE;
    while (does_queue_family_support_surface == VK_FALSE) {
        queue_family_index++;
        vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, queue_family_index, surface, &does_queue_family_support_surface);
    }
    // TODO: check for other properties we require

    //logical device creation
    std::vector<float> queue_priorities = { 1.0f };
    std::vector<VkDeviceQueueCreateInfo> queue_create_info;
    queue_create_info.push_back({
        VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        nullptr,
        0,
        static_cast<uint32_t>(queue_family_index),
        static_cast<uint32_t>(queue_priorities.size()),
        queue_priorities.data()
        });

    std::vector<const char*> desired_device_level_extensions = { "VK_KHR_swapchain" };
    VkPhysicalDeviceFeatures selected_device_features = { 0 };
    // TODO: enable here features we need
    VkDeviceCreateInfo device_create_info = {
        VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        nullptr,
        0,
        queue_create_info.size(),
        queue_create_info.data(),
        0,
        nullptr,
        desired_device_level_extensions.size(),
        desired_device_level_extensions.data(),
        &selected_device_features
    };

    if (vkCreateDevice(physical_device, &device_create_info, nullptr, &device)) { throw DEVICE_CREATION_FAILED; }
    vkGetDeviceQueue(device, queue_family_index, 0, &queue);
    volkLoadDevice(device);
}

void VulkanTriangle::create_swapchain() {

    uint32_t presentation_modes_number;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &presentation_modes_number, nullptr);
    std::vector<VkPresentModeKHR> presentation_modes(presentation_modes_number);
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &presentation_modes_number, presentation_modes.data());
    VkPresentModeKHR selected_present_mode = vulkan_helper::select_presentation_mode(presentation_modes, VK_PRESENT_MODE_MAILBOX_KHR);

    VkSurfaceCapabilitiesKHR surface_capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &surface_capabilities);

    uint32_t number_of_images = vulkan_helper::select_number_of_images(surface_capabilities);
    VkExtent2D size_of_images = vulkan_helper::select_size_of_images(surface_capabilities, window_size);
    VkImageUsageFlags image_usage = vulkan_helper::select_image_usage(surface_capabilities, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    VkSurfaceTransformFlagBitsKHR surface_transform = vulkan_helper::select_surface_transform(surface_capabilities, VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR);

    uint32_t formats_count = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &formats_count, nullptr);
    std::vector<VkSurfaceFormatKHR> surface_formats(formats_count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &formats_count, surface_formats.data());
    VkSurfaceFormatKHR surface_format = vulkan_helper::select_surface_format(surface_formats, { VK_FORMAT_B8G8R8A8_UNORM ,VK_COLOR_SPACE_SRGB_NONLINEAR_KHR });

    swapchain_create_info = {
        VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        nullptr,
        0,
        surface,
        number_of_images,
        surface_format.format,
        surface_format.colorSpace,
        size_of_images,
        1,
        image_usage,
        VK_SHARING_MODE_EXCLUSIVE,
        0,
        nullptr,
        surface_transform,
        VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        selected_present_mode,
        VK_TRUE,
        old_swapchain
    };

    if (vkCreateSwapchainKHR(device, &swapchain_create_info, nullptr, &swapchain) != VK_SUCCESS) { throw SWAPCHAIN_CREATION_FAILED; }
    
    vkGetSwapchainImagesKHR(device, swapchain, &swapchain_images_count, nullptr);
    swapchain_images.resize(swapchain_images_count);
    vkGetSwapchainImagesKHR(device, swapchain, &swapchain_images_count, swapchain_images.data());
}

void VulkanTriangle::create_command_pool() {
    VkCommandPoolCreateInfo command_pool_create_info = {
        VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        nullptr,
        0,
        queue_family_index
    };
    if (vkCreateCommandPool(device, &command_pool_create_info, nullptr, &command_pool)) { throw COMMAND_POOL_CREATION_FAILED; }
}

void VulkanTriangle::allocate_command_buffers() {
    VkCommandBufferAllocateInfo command_buffer_allocate_info = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        nullptr,
        command_pool,
        VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        swapchain_images_count
    };
    command_buffers.resize(swapchain_images_count);
    if (vkAllocateCommandBuffers(device, &command_buffer_allocate_info, command_buffers.data())) { throw COMMAND_BUFFER_CREATION_FAILED; }
}

void VulkanTriangle::create_host_buffers() {
    VkBufferCreateInfo buffer_create_info = {
        VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        nullptr,
        0,
        input_data.size() * sizeof(decltype(input_data[0])),
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_SHARING_MODE_EXCLUSIVE,
        0,
        nullptr
    };
    vkCreateBuffer(device, &buffer_create_info, nullptr, &host_vertex_buffer);

    buffer_create_info.size = sizeof(glm::mat4);
    vkCreateBuffer(device, &buffer_create_info, nullptr, &host_m_matrix_buffer);

    vkGetBufferMemoryRequirements(device, host_vertex_buffer, &host_memory_requirements[0]);
    vkGetBufferMemoryRequirements(device, host_m_matrix_buffer, &host_memory_requirements[1]);

    VkMemoryAllocateInfo memory_allocate_info = {
        VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        nullptr,
        host_memory_requirements[0].size + host_memory_requirements[1].size,
        vulkan_helper::select_memory_index(physical_device_memory_properties,host_memory_requirements[0],VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
    };
    if (vkAllocateMemory(device, &memory_allocate_info, nullptr, &host_memory) != VK_SUCCESS) { throw MEMORY_ALLOCATION_FAILED; }

    vkBindBufferMemory(device, host_vertex_buffer, host_memory, 0);
    vkBindBufferMemory(device, host_m_matrix_buffer, host_memory, host_memory_requirements[0].size);

    vkMapMemory(device, host_memory, 0, VK_WHOLE_SIZE, 0, &host_data_pointer);
    memcpy(host_data_pointer, input_data.data(), input_data.size() * sizeof(decltype(input_data[0])));
    VkMappedMemoryRange mapped_memory_range = { VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE, nullptr, host_memory,0,VK_WHOLE_SIZE };
    vkFlushMappedMemoryRanges(device, 1, &mapped_memory_range);
}

void VulkanTriangle::create_device_buffers() {
    VkBufferCreateInfo buffer_create_info = {
        VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        nullptr,
        0,
        input_data.size() * sizeof(decltype(input_data[0])),
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_SHARING_MODE_EXCLUSIVE,
        0,
        nullptr
    };
    vkCreateBuffer(device, &buffer_create_info, nullptr, &device_vertex_buffer);

    buffer_create_info.size = sizeof(glm::mat4);
    buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    vkCreateBuffer(device, &buffer_create_info, nullptr, &device_m_matrix_buffer);

    vkGetBufferMemoryRequirements(device, device_m_matrix_buffer, &device_memory_requirements[0]);
    vkGetBufferMemoryRequirements(device, device_vertex_buffer, &device_memory_requirements[1]);

    VkMemoryAllocateInfo memory_allocate_info = {
        VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        nullptr,
        device_memory_requirements[0].size + device_memory_requirements[1].size,
        vulkan_helper::select_memory_index(physical_device_memory_properties,device_memory_requirements[0],VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
    };
    if (vkAllocateMemory(device, &memory_allocate_info, nullptr, &device_memory) != VK_SUCCESS) { throw MEMORY_ALLOCATION_FAILED; }

    vkBindBufferMemory(device, device_vertex_buffer, device_memory, 0);
    vkBindBufferMemory(device, device_m_matrix_buffer, device_memory, device_memory_requirements[0].size);
}

void VulkanTriangle::create_descriptor_pool() {
    VkDescriptorPoolSize descriptor_pool_size = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1 };
    VkDescriptorPoolCreateInfo descriptor_pool_create_info = {
        VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        nullptr,
        0,
        1,
        1,
        &descriptor_pool_size
    };
    vkCreateDescriptorPool(device, &descriptor_pool_create_info, nullptr, &descriptor_pool);
}

void VulkanTriangle::allocate_descriptor_sets() {
    VkDescriptorSetLayoutBinding descriptor_set_layout_binding = {
    0,
    VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
    1,
    VK_SHADER_STAGE_VERTEX_BIT,
    nullptr
    };
    VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info = {
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        nullptr,
        0,
        1,
        &descriptor_set_layout_binding
    };
    vkCreateDescriptorSetLayout(device, &descriptor_set_layout_create_info, nullptr, &descriptor_set_layout);

    VkDescriptorSetAllocateInfo descriptor_set_allocate_info = {
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        nullptr,
        descriptor_pool,
        1,
        &descriptor_set_layout
    };
    vkAllocateDescriptorSets(device, &descriptor_set_allocate_info, &descriptor_set);

    VkDescriptorBufferInfo descriptor_buffer_info = { device_m_matrix_buffer,0,sizeof(glm::mat4) };
    VkWriteDescriptorSet write_descriptor_set = {
        VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        nullptr,
        descriptor_set,
        0,
        0,
        1,
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        nullptr,
        &descriptor_buffer_info,
        nullptr
    };
    vkUpdateDescriptorSets(device, 1, &write_descriptor_set, 0, nullptr);
}

void VulkanTriangle::create_renderpass() {
    VkAttachmentDescription attachment_description = {
        0,
        swapchain_create_info.imageFormat,
        VK_SAMPLE_COUNT_1_BIT,
        VK_ATTACHMENT_LOAD_OP_CLEAR,
        VK_ATTACHMENT_STORE_OP_STORE,
        VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        VK_ATTACHMENT_STORE_OP_DONT_CARE,
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
    };

    VkAttachmentReference attachment_reference = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
    VkSubpassDescription subpass_description = {
        0,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        0,
        nullptr,
        1,
        &attachment_reference,
        nullptr,
        nullptr,
        0,
        nullptr
    };

    VkRenderPassCreateInfo render_pass_create_info = {
        VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        nullptr,
        0,
        1,
        &attachment_description,
        1,
        &subpass_description,
        0,
        nullptr
    };
    vkCreateRenderPass(device, &render_pass_create_info, nullptr, &render_pass);
}

void VulkanTriangle::create_framebuffers() {
    framebuffers.resize(swapchain_images_count);
    swapchain_images_views.resize(swapchain_images_count);

    for (int i = 0; i < swapchain_images_count; i++) {
        VkImageViewCreateInfo image_view_create_info = {
            VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            nullptr,
            0,
            swapchain_images[i],
            VK_IMAGE_VIEW_TYPE_2D,
            swapchain_create_info.imageFormat,
            {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY },
            {VK_IMAGE_ASPECT_COLOR_BIT, 0, VK_REMAINING_MIP_LEVELS, 0 , VK_REMAINING_ARRAY_LAYERS}
        };
        vkCreateImageView(device, &image_view_create_info, nullptr, &swapchain_images_views[i]);

        VkFramebufferCreateInfo framebuffer_create_info = {
            VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            nullptr,
            0,
            render_pass,
            1,
            &swapchain_images_views[i],
            swapchain_create_info.imageExtent.width,
            swapchain_create_info.imageExtent.height,
            swapchain_create_info.imageArrayLayers
        };
        vkCreateFramebuffer(device, &framebuffer_create_info, nullptr, &framebuffers[i]);
    }
}

void VulkanTriangle::create_pipeline() {
    std::ifstream shader_file("shader//spirv.vert", std::ios::in | std::ios::binary);
    std::vector<char> shader_contents(std::filesystem::file_size("shader//spirv.vert"));
    shader_file.read(shader_contents.data(), std::filesystem::file_size("shader//spirv.vert"));
    VkShaderModuleCreateInfo shader_module_create_info = {
        VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        nullptr,
        0,
        std::filesystem::file_size("shader//spirv.vert"),
        reinterpret_cast<uint32_t*>(shader_contents.data())
    };
    VkShaderModule vertex_shader_module;
    if (vkCreateShaderModule(device, &shader_module_create_info, nullptr, &vertex_shader_module)) { throw SHADER_MODULE_CREATION_FAILED; }
    shader_file.close();

    shader_file.open("shader//spirv.frag", std::ios::in | std::ios::binary);
    shader_contents.resize(std::filesystem::file_size("shader//spirv.frag"));
    shader_file.read(shader_contents.data(), std::filesystem::file_size("shader//spirv.frag"));
    shader_module_create_info.codeSize = std::filesystem::file_size("shader//spirv.frag");
    shader_module_create_info.pCode = reinterpret_cast<uint32_t*>(shader_contents.data());
    VkShaderModule fragment_shader_module;
    if (vkCreateShaderModule(device, &shader_module_create_info, nullptr, &fragment_shader_module)) { throw SHADER_MODULE_CREATION_FAILED; }
    shader_file.close();

    VkPipelineShaderStageCreateInfo pipeline_shaders_stage_create_info[2] = {
        {
            VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            nullptr,
            0,
            VK_SHADER_STAGE_VERTEX_BIT,
            vertex_shader_module,
            "main",
            nullptr
        },
        {
            VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            nullptr,
            0,
            VK_SHADER_STAGE_FRAGMENT_BIT,
            fragment_shader_module,
            "main",
            nullptr
        }
    };

    VkVertexInputBindingDescription vertex_input_binding_description = {
        0,
        6 * sizeof(float),
        VK_VERTEX_INPUT_RATE_VERTEX
    };
    VkVertexInputAttributeDescription vertex_input_attribute_description[] = { {
        0,
        0,
        VK_FORMAT_R32G32B32_SFLOAT,
        0
    },
    {
        1,
        0,
        VK_FORMAT_R32G32B32_SFLOAT,
        3 * sizeof(float)
    }
    };
    VkPipelineVertexInputStateCreateInfo pipeline_vertex_input_state_create_info = {
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        nullptr,
        0,
        1,
        &vertex_input_binding_description,
        2,
        vertex_input_attribute_description
    };

    VkPipelineInputAssemblyStateCreateInfo pipeline_input_assembly_create_info = {
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        nullptr,
        0,
        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        VK_FALSE
    };

    VkViewport viewport = {
        0.0f,
        0.0f,
        swapchain_create_info.imageExtent.width,
        swapchain_create_info.imageExtent.height,
        0.0f,
        1.0f
    };
    VkRect2D scissor = {
        {0,0},
        swapchain_create_info.imageExtent
    };
    VkPipelineViewportStateCreateInfo pipeline_viewport_state_create_info = {
        VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        nullptr,
        0,
        1,
        &viewport,
        1,
        &scissor
    };

    VkPipelineRasterizationStateCreateInfo pipeline_rasterization_state_create_info = {
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        nullptr,
        0,
        VK_FALSE,
        VK_FALSE,
        VK_POLYGON_MODE_FILL,
        VK_CULL_MODE_NONE,
        VK_FRONT_FACE_COUNTER_CLOCKWISE,
        VK_FALSE,
        0.0f,
        0.0f,
        0.0f,
        1.0f
    };

    VkPipelineMultisampleStateCreateInfo pipeline_multisample_state_create_info = {
        VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        nullptr,
        0,
        VK_SAMPLE_COUNT_1_BIT,
        VK_FALSE,
        1.0f,
        nullptr,
        VK_FALSE,
        VK_FALSE
    };

    VkPipelineColorBlendAttachmentState pipeline_color_blend_attachment_state = {
        VK_FALSE,
        VK_BLEND_FACTOR_ONE,
        VK_BLEND_FACTOR_ZERO,
        VK_BLEND_OP_ADD,
        VK_BLEND_FACTOR_ONE,
        VK_BLEND_FACTOR_ZERO,
        VK_BLEND_OP_ADD,
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
    };
    VkPipelineColorBlendStateCreateInfo pipeline_color_blend_state_create_info = {
        VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        nullptr,
        0,
        VK_FALSE,
        VK_LOGIC_OP_COPY,
        1,
        &pipeline_color_blend_attachment_state,
        {0.0f,0.0f,0.0f,0.0f}
    };

    VkPipelineLayoutCreateInfo pipeline_layout_create_info = {
        VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        nullptr,
        0,
        1,
        &descriptor_set_layout,
        0,
        nullptr
    };
    vkCreatePipelineLayout(device, &pipeline_layout_create_info, nullptr, &pipeline_layout);

    VkGraphicsPipelineCreateInfo graphics_pipeline_create_info = {
        VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        nullptr,
        0,
        2,
        pipeline_shaders_stage_create_info,
        &pipeline_vertex_input_state_create_info,
        &pipeline_input_assembly_create_info,
        nullptr,
        &pipeline_viewport_state_create_info,
        &pipeline_rasterization_state_create_info,
        &pipeline_multisample_state_create_info,
        nullptr,
        &pipeline_color_blend_state_create_info,
        nullptr,
        pipeline_layout,
        render_pass,
        0,
        VK_NULL_HANDLE,
        -1
    };
    vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &graphics_pipeline_create_info, nullptr, &pipeline);
    vkDestroyShaderModule(device, vertex_shader_module, nullptr);
    vkDestroyShaderModule(device, fragment_shader_module, nullptr);
}

void VulkanTriangle::upload_input_data() {
    VkCommandBufferBeginInfo command_buffer_begin_info = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,nullptr, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,nullptr };
    vkBeginCommandBuffer(command_buffers[0], &command_buffer_begin_info);
    VkBufferCopy buffer_copy = { 0,0,input_data.size() * sizeof(decltype(input_data[0])) };
    vkCmdCopyBuffer(command_buffers[0], host_vertex_buffer, device_vertex_buffer, 1, &buffer_copy);
    vkEndCommandBuffer(command_buffers[0]);

    VkFenceCreateInfo fence_create_info = { VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,nullptr,0 };
    VkFence fence;
    vkCreateFence(device, &fence_create_info, nullptr, &fence);

    VkPipelineStageFlags pipeline_stage_flags = { VK_PIPELINE_STAGE_TRANSFER_BIT };
    VkSubmitInfo submit_info = {
        VK_STRUCTURE_TYPE_SUBMIT_INFO,
        nullptr,
        0,
        nullptr,
        &pipeline_stage_flags,
        1,
        &command_buffers[0],
        0,
        nullptr
    };
    vkQueueSubmit(queue, 1, &submit_info, fence);

    vkWaitForFences(device, 1, &fence, VK_TRUE, 20000000);
    vkResetCommandPool(device, command_pool, 0);
    vkDestroyFence(device, fence, nullptr);
}

void VulkanTriangle::record_command_buffers() {
    VkClearValue clearColor = { { 0.0f, 0.0f, 0.0f, 1.0f } };

    for (int i = 0; i < swapchain_images_count; i++) {
        VkCommandBufferBeginInfo command_buffer_begin_info = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, nullptr, VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT, nullptr };
        vkBeginCommandBuffer(command_buffers[i], &command_buffer_begin_info);

        VkBufferCopy buffer_copy = { 0,0,sizeof(glm::mat4) };
        vkCmdCopyBuffer(command_buffers[i], host_m_matrix_buffer, device_m_matrix_buffer, 1, &buffer_copy);

        VkMemoryBarrier memory_barrier = { VK_STRUCTURE_TYPE_MEMORY_BARRIER, nullptr, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_UNIFORM_READ_BIT };
        vkCmdPipelineBarrier(command_buffers[i], VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, 0, 1, &memory_barrier, 0, nullptr, 0, nullptr);

        VkImageMemoryBarrier image_memory_barrier = {
            VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            nullptr,
            0,
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            VK_QUEUE_FAMILY_IGNORED,
            VK_QUEUE_FAMILY_IGNORED,
            swapchain_images[i],
            { VK_IMAGE_ASPECT_COLOR_BIT,0,1,0,1 }
        };
        vkCmdPipelineBarrier(command_buffers[i], VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr, 0, nullptr, 1, &image_memory_barrier);

        VkRenderPassBeginInfo render_pass_begin_info = {
            VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            nullptr,
            render_pass,
            framebuffers[i],
            {{0,0},{swapchain_create_info.imageExtent}},
            1,
            &clearColor
        };
        vkCmdBeginRenderPass(command_buffers[i], &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindDescriptorSets(command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, &descriptor_set, 0, nullptr);

        vkCmdBindPipeline(command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

        VkDeviceSize offset = 0;
        vkCmdBindVertexBuffers(command_buffers[i], 0, 1, &device_vertex_buffer, &offset);

        vkCmdDraw(command_buffers[i], 3, 1, 0, 0);

        vkCmdEndRenderPass(command_buffers[i]);

        vkEndCommandBuffer(command_buffers[i]);
    }
    vkDestroyPipelineLayout(device, pipeline_layout, nullptr);
}

void VulkanTriangle::create_semaphores() {
    VkSemaphoreCreateInfo semaphore_create_info = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, nullptr, 0 };
    semaphores.resize(2);
    for (int i = 0; i < semaphores.size(); i++) {
        vkCreateSemaphore(device, &semaphore_create_info, nullptr, &semaphores[i]);
    }
}

void VulkanTriangle::frame_loop() {
    while (!glfwWindowShouldClose(window)) {
        rendered_frames++;
        modulus_result = rendered_frames % 1000;
        if (modulus_result == 0) {
            t1 = std::chrono::steady_clock::now();
        }

        mv_matrix = glm::rotate(static_cast<float>(glfwGetTime() * 0.2f), glm::vec3(0.0f, 0.0f, 1.0f));
        memcpy(static_cast<uint8_t*>(host_data_pointer) + host_memory_requirements[0].size, glm::value_ptr(mv_matrix), sizeof(mv_matrix));
        VkMappedMemoryRange mapped_memory_range = { VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE, nullptr, host_memory,host_memory_requirements[0].size,VK_WHOLE_SIZE };
        vkFlushMappedMemoryRanges(device, 1, &mapped_memory_range);

        uint32_t image_index = 0;
        VkResult res = vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, semaphores[0], VK_NULL_HANDLE, &image_index);
        if (res == VK_SUBOPTIMAL_KHR || res == VK_ERROR_OUT_OF_DATE_KHR) {
            on_window_resize();
        }
        else if(res != VK_SUCCESS) {
            throw ACQUIRE_NEXT_IMAGE_FAILED;
        }

        VkPipelineStageFlags pipeline_stage_flags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        VkSubmitInfo submit_info = {
            VK_STRUCTURE_TYPE_SUBMIT_INFO,
            nullptr,
            1,
            &semaphores[0],
            &pipeline_stage_flags,
            1,
            &command_buffers[image_index],
            1,
            &semaphores[1]
        };
        vkQueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE);

        VkPresentInfoKHR present_info = {
            VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            nullptr,
            1,
            &semaphores[1],
            1,
            &swapchain,
            &image_index
        };
        res = vkQueuePresentKHR(queue, &present_info);
        if (res == VK_SUBOPTIMAL_KHR || res == VK_ERROR_OUT_OF_DATE_KHR) {
            on_window_resize();
        }
        else if (res != VK_SUCCESS) {
            throw QUEUE_PRESENT_FAILED;
        }
        glfwPollEvents();

        if (modulus_result == 0) {
            t2 = std::chrono::steady_clock::now();
            time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
            std::cout << "Msec/frame: " << time_span.count()*1000 << std::endl;
        }
    }
}

void VulkanTriangle::on_window_resize() {
    vkDeviceWaitIdle(device);

    int width, height;
    glfwGetWindowSize(window, &width, &height);
    window_size = { static_cast<uint32_t>(width),static_cast<uint32_t>(height)};
    create_surface();
    old_swapchain = swapchain;
    create_swapchain();
    create_renderpass();
    create_framebuffers();
    create_pipeline();
    vkResetCommandPool(device,command_pool,0);
    record_command_buffers();
}

VulkanTriangle::VulkanTriangle() {
    create_instance();
#ifndef NDEBUG
    setup_debug_callback();
#endif
    create_window();
    create_surface();
    create_logical_device();
    create_swapchain();
    create_command_pool();
    allocate_command_buffers();
    create_host_buffers();
    create_device_buffers();
    create_descriptor_pool();
    allocate_descriptor_sets();
    create_renderpass();
    create_framebuffers();
    create_pipeline();
    upload_input_data();
    record_command_buffers();
    create_semaphores();
}

void VulkanTriangle::start_main_loop() {
    frame_loop();
}

VulkanTriangle::~VulkanTriangle() {
    vkDeviceWaitIdle(device);
    vkDestroyPipeline(device, pipeline, nullptr);
    for (int i = 0; i < framebuffers.size(); i++) {
        vkDestroyFramebuffer(device, framebuffers[i], nullptr);
        vkDestroyImageView(device, swapchain_images_views[i], nullptr);
    }
    vkDestroyRenderPass(device, render_pass, nullptr);
    vkDestroyDescriptorSetLayout(device, descriptor_set_layout, nullptr);
    vkDestroyDescriptorPool(device, descriptor_pool, nullptr);
    for (int i = 0; i < semaphores.size(); i++) {
        vkDestroySemaphore(device, semaphores[i], nullptr);
    }
    vkUnmapMemory(device, host_memory);
    vkDestroyBuffer(device, host_vertex_buffer, nullptr);
    vkDestroyBuffer(device, host_m_matrix_buffer, nullptr);
    vkFreeMemory(device, host_memory, nullptr);
    vkDestroyBuffer(device, device_vertex_buffer, nullptr);
    vkDestroyBuffer(device, device_m_matrix_buffer, nullptr);
    vkFreeMemory(device, device_memory, nullptr);
    vkFreeCommandBuffers(device, command_pool, command_buffers.size(), command_buffers.data());
    vkDestroyCommandPool(device, command_pool, nullptr);
    vkDestroySwapchainKHR(device, swapchain, nullptr);
    vkDestroyDevice(device, nullptr);
    vkDestroySurfaceKHR(instance, surface, nullptr);
    glfwDestroyWindow(window);
#ifndef NDEBUG
    vkDestroyDebugReportCallbackEXT(instance, debug_report_callback, nullptr);
#endif
    vkDestroyInstance(instance, nullptr);
}

int main() {
    VulkanTriangle vk_triangle;
    vk_triangle.start_main_loop();
    return 0;
}