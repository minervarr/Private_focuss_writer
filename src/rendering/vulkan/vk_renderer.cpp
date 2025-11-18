#include "vk_renderer.h"
#include "platform/platform_interface.h"
#include "utils/logger.h"
#include <algorithm>
#include <set>
#include <cstring>

namespace phantom {

// ============================================================================
// Debug callback
// ============================================================================

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {

    if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        LOG_WARN(LogCategory::VULKAN, "Validation layer: %s", pCallbackData->pMessage);
    } else {
        LOG_DEBUG(LogCategory::VULKAN, "Validation layer: %s", pCallbackData->pMessage);
    }

    return VK_FALSE;
}

// ============================================================================
// Constructor / Destructor
// ============================================================================

VulkanRenderer::VulkanRenderer() {
    LOG_TRACE(LogCategory::VULKAN, "VulkanRenderer constructor");
}

VulkanRenderer::~VulkanRenderer() {
    LOG_TRACE(LogCategory::VULKAN, "VulkanRenderer destructor");
    cleanup();
}

// ============================================================================
// Initialization
// ============================================================================

bool VulkanRenderer::initialize(IPlatformWindow* window) {
    LOG_INFO(LogCategory::VULKAN, "Initializing Vulkan renderer");

    window_ = window;

    if (!createInstance()) {
        LOG_ERROR(LogCategory::VULKAN, "Failed to create Vulkan instance");
        return false;
    }

    if (enableValidationLayers) {
        if (!setupDebugMessenger()) {
            LOG_WARN(LogCategory::VULKAN, "Failed to setup debug messenger");
        }
    }

    if (!createSurface()) {
        LOG_ERROR(LogCategory::VULKAN, "Failed to create surface");
        return false;
    }

    if (!selectPhysicalDevice()) {
        LOG_ERROR(LogCategory::VULKAN, "Failed to select physical device");
        return false;
    }

    if (!createLogicalDevice()) {
        LOG_ERROR(LogCategory::VULKAN, "Failed to create logical device");
        return false;
    }

    if (!createSwapChain()) {
        LOG_ERROR(LogCategory::VULKAN, "Failed to create swap chain");
        return false;
    }

    if (!createImageViews()) {
        LOG_ERROR(LogCategory::VULKAN, "Failed to create image views");
        return false;
    }

    if (!createRenderPass()) {
        LOG_ERROR(LogCategory::VULKAN, "Failed to create render pass");
        return false;
    }

    if (!createFramebuffers()) {
        LOG_ERROR(LogCategory::VULKAN, "Failed to create framebuffers");
        return false;
    }

    if (!createCommandPool()) {
        LOG_ERROR(LogCategory::VULKAN, "Failed to create command pool");
        return false;
    }

    if (!createCommandBuffers()) {
        LOG_ERROR(LogCategory::VULKAN, "Failed to create command buffers");
        return false;
    }

    if (!createSyncObjects()) {
        LOG_ERROR(LogCategory::VULKAN, "Failed to create sync objects");
        return false;
    }

    initialized_ = true;
    LOG_INFO(LogCategory::VULKAN, "Vulkan renderer initialized successfully");
    return true;
}

void VulkanRenderer::cleanup() {
    if (!initialized_) {
        return;
    }

    LOG_INFO(LogCategory::VULKAN, "Cleaning up Vulkan renderer");

    vkDeviceWaitIdle(device_);

    // Cleanup sync objects
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(device_, imageAvailableSemaphores_[i], nullptr);
        vkDestroySemaphore(device_, renderFinishedSemaphores_[i], nullptr);
        vkDestroyFence(device_, inFlightFences_[i], nullptr);
    }

    // Cleanup command pool
    if (commandPool_ != VK_NULL_HANDLE) {
        vkDestroyCommandPool(device_, commandPool_, nullptr);
    }

    // Cleanup framebuffers
    for (auto framebuffer : swapChainFramebuffers_) {
        vkDestroyFramebuffer(device_, framebuffer, nullptr);
    }

    // Cleanup render pass
    if (renderPass_ != VK_NULL_HANDLE) {
        vkDestroyRenderPass(device_, renderPass_, nullptr);
    }

    // Cleanup image views
    for (auto imageView : swapChainImageViews_) {
        vkDestroyImageView(device_, imageView, nullptr);
    }

    // Cleanup swap chain
    if (swapChain_ != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(device_, swapChain_, nullptr);
    }

    // Cleanup device
    if (device_ != VK_NULL_HANDLE) {
        vkDestroyDevice(device_, nullptr);
    }

    // Cleanup debug messenger
    if (enableValidationLayers) {
        destroyDebugMessenger();
    }

    // Cleanup surface
    if (surface_ != VK_NULL_HANDLE) {
        vkDestroySurfaceKHR(instance_, surface_, nullptr);
    }

    // Cleanup instance
    if (instance_ != VK_NULL_HANDLE) {
        vkDestroyInstance(instance_, nullptr);
    }

    initialized_ = false;
    LOG_INFO(LogCategory::VULKAN, "Vulkan renderer cleaned up");
}

// ============================================================================
// Instance creation
// ============================================================================

bool VulkanRenderer::createInstance() {
    LOG_DEBUG(LogCategory::VULKAN, "Creating Vulkan instance");

    if (enableValidationLayers && !checkValidationLayerSupport()) {
        LOG_ERROR(LogCategory::VULKAN, "Validation layers requested but not available");
        return false;
    }

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Phantom Writer";
    appInfo.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(0, 1, 0);
    appInfo.apiVersion = VK_API_VERSION_1_2;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    auto extensions = getRequiredExtensions();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    LOG_DEBUG(LogCategory::VULKAN, "Required extensions: %zu", extensions.size());
    for (size_t i = 0; i < extensions.size(); i++) {
        LOG_TRACE(LogCategory::VULKAN, "  Extension[%zu]: %s", i, extensions[i]);
    }

    if (enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers_.size());
        createInfo.ppEnabledLayerNames = validationLayers_.data();
        LOG_DEBUG(LogCategory::VULKAN, "Validation layers enabled: %zu", validationLayers_.size());
    } else {
        createInfo.enabledLayerCount = 0;
    }

    VkResult result = vkCreateInstance(&createInfo, nullptr, &instance_);
    if (result != VK_SUCCESS) {
        LOG_ERROR(LogCategory::VULKAN, "Failed to create Vulkan instance: %d", result);
        return false;
    }

    LOG_INFO(LogCategory::VULKAN, "Vulkan instance created successfully");
    return true;
}

bool VulkanRenderer::checkValidationLayerSupport() {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName : validationLayers_) {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }

        if (!layerFound) {
            return false;
        }
    }

    return true;
}

std::vector<const char*> VulkanRenderer::getRequiredExtensions() {
    uint32_t count = 0;
    const char** platformExtensions = window_->getRequiredVulkanExtensions(count);

    std::vector<const char*> extensions(platformExtensions, platformExtensions + count);

    if (enableValidationLayers) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

// ============================================================================
// Debug messenger
// ============================================================================

bool VulkanRenderer::setupDebugMessenger() {
    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;

    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance_, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        VkResult result = func(instance_, &createInfo, nullptr, &debugMessenger_);
        if (result == VK_SUCCESS) {
            LOG_DEBUG(LogCategory::VULKAN, "Debug messenger setup successfully");
            return true;
        }
    }

    return false;
}

void VulkanRenderer::destroyDebugMessenger() {
    if (debugMessenger_ != VK_NULL_HANDLE) {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance_, "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr) {
            func(instance_, debugMessenger_, nullptr);
        }
    }
}

// ============================================================================
// Surface creation
// ============================================================================

bool VulkanRenderer::createSurface() {
    LOG_DEBUG(LogCategory::VULKAN, "Creating Vulkan surface");

    surface_ = window_->createVulkanSurface(instance_);
    if (surface_ == VK_NULL_HANDLE) {
        LOG_ERROR(LogCategory::VULKAN, "Failed to create Vulkan surface");
        return false;
    }

    LOG_INFO(LogCategory::VULKAN, "Vulkan surface created successfully");
    return true;
}

// ============================================================================
// Physical device selection
// ============================================================================

bool VulkanRenderer::selectPhysicalDevice() {
    LOG_DEBUG(LogCategory::VULKAN, "Selecting physical device");

    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance_, &deviceCount, nullptr);

    if (deviceCount == 0) {
        LOG_ERROR(LogCategory::VULKAN, "Failed to find GPUs with Vulkan support");
        return false;
    }

    LOG_DEBUG(LogCategory::VULKAN, "Found %u physical device(s)", deviceCount);

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance_, &deviceCount, devices.data());

    for (const auto& device : devices) {
        if (isDeviceSuitable(device)) {
            physicalDevice_ = device;
            break;
        }
    }

    if (physicalDevice_ == VK_NULL_HANDLE) {
        LOG_ERROR(LogCategory::VULKAN, "Failed to find a suitable GPU");
        return false;
    }

    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(physicalDevice_, &deviceProperties);
    LOG_INFO(LogCategory::VULKAN, "Selected GPU: %s", deviceProperties.deviceName);

    return true;
}

bool VulkanRenderer::isDeviceSuitable(VkPhysicalDevice device) {
    int queueFamily = findQueueFamilies(device);
    if (queueFamily < 0) {
        return false;
    }

    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
    bool swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();

    return swapChainAdequate;
}

int VulkanRenderer::findQueueFamilies(VkPhysicalDevice device) {
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    int graphicsFamily = -1;
    int presentFamily = -1;

    for (uint32_t i = 0; i < queueFamilyCount; i++) {
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            graphicsFamily = i;
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface_, &presentSupport);
        if (presentSupport) {
            presentFamily = i;
        }

        if (graphicsFamily >= 0 && presentFamily >= 0) {
            graphicsQueueFamily_ = graphicsFamily;
            presentQueueFamily_ = presentFamily;
            return graphicsFamily;
        }
    }

    return -1;
}

SwapChainSupportDetails VulkanRenderer::querySwapChainSupport(VkPhysicalDevice device) {
    SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface_, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface_, &formatCount, nullptr);
    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface_, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface_, &presentModeCount, nullptr);
    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface_, &presentModeCount, details.presentModes.data());
    }

    return details;
}

// ============================================================================
// Logical device creation
// ============================================================================

bool VulkanRenderer::createLogicalDevice() {
    LOG_DEBUG(LogCategory::VULKAN, "Creating logical device");

    std::set<int> uniqueQueueFamilies = {graphicsQueueFamily_, presentQueueFamily_};
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

    float queuePriority = 1.0f;
    for (int queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.pEnabledFeatures = &deviceFeatures;

    const std::vector<const char*> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    if (enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers_.size());
        createInfo.ppEnabledLayerNames = validationLayers_.data();
    } else {
        createInfo.enabledLayerCount = 0;
    }

    VkResult result = vkCreateDevice(physicalDevice_, &createInfo, nullptr, &device_);
    if (result != VK_SUCCESS) {
        LOG_ERROR(LogCategory::VULKAN, "Failed to create logical device: %d", result);
        return false;
    }

    vkGetDeviceQueue(device_, graphicsQueueFamily_, 0, &graphicsQueue_);
    vkGetDeviceQueue(device_, presentQueueFamily_, 0, &presentQueue_);

    LOG_INFO(LogCategory::VULKAN, "Logical device created successfully");
    return true;
}

// ============================================================================
// Swap chain creation
// ============================================================================

bool VulkanRenderer::createSwapChain() {
    LOG_DEBUG(LogCategory::VULKAN, "Creating swap chain");

    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice_);

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface_;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    uint32_t queueFamilyIndices[] = {static_cast<uint32_t>(graphicsQueueFamily_), static_cast<uint32_t>(presentQueueFamily_)};

    if (graphicsQueueFamily_ != presentQueueFamily_) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    VkResult result = vkCreateSwapchainKHR(device_, &createInfo, nullptr, &swapChain_);
    if (result != VK_SUCCESS) {
        LOG_ERROR(LogCategory::VULKAN, "Failed to create swap chain: %d", result);
        return false;
    }

    vkGetSwapchainImagesKHR(device_, swapChain_, &imageCount, nullptr);
    swapChainImages_.resize(imageCount);
    vkGetSwapchainImagesKHR(device_, swapChain_, &imageCount, swapChainImages_.data());

    swapChainImageFormat_ = surfaceFormat.format;
    swapChainExtent_ = extent;

    LOG_INFO(LogCategory::VULKAN, "Swap chain created: %u images, %ux%u", imageCount, extent.width, extent.height);
    return true;
}

VkSurfaceFormatKHR VulkanRenderer::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
            availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }
    return availableFormats[0];
}

VkPresentModeKHR VulkanRenderer::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentMode;
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanRenderer::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
    if (capabilities.currentExtent.width != UINT32_MAX) {
        return capabilities.currentExtent;
    } else {
        int width, height;
        window_->getFramebufferSize(width, height);

        VkExtent2D actualExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }
}

// ============================================================================
// Image views
// ============================================================================

bool VulkanRenderer::createImageViews() {
    LOG_DEBUG(LogCategory::VULKAN, "Creating image views");

    swapChainImageViews_.resize(swapChainImages_.size());

    for (size_t i = 0; i < swapChainImages_.size(); i++) {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = swapChainImages_[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = swapChainImageFormat_;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        VkResult result = vkCreateImageView(device_, &createInfo, nullptr, &swapChainImageViews_[i]);
        if (result != VK_SUCCESS) {
            LOG_ERROR(LogCategory::VULKAN, "Failed to create image view %zu: %d", i, result);
            return false;
        }
    }

    LOG_INFO(LogCategory::VULKAN, "Created %zu image views", swapChainImageViews_.size());
    return true;
}

// ============================================================================
// Render pass
// ============================================================================

bool VulkanRenderer::createRenderPass() {
    LOG_DEBUG(LogCategory::VULKAN, "Creating render pass");

    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = swapChainImageFormat_;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    VkResult result = vkCreateRenderPass(device_, &renderPassInfo, nullptr, &renderPass_);
    if (result != VK_SUCCESS) {
        LOG_ERROR(LogCategory::VULKAN, "Failed to create render pass: %d", result);
        return false;
    }

    LOG_INFO(LogCategory::VULKAN, "Render pass created successfully");
    return true;
}

// ============================================================================
// Framebuffers
// ============================================================================

bool VulkanRenderer::createFramebuffers() {
    LOG_DEBUG(LogCategory::VULKAN, "Creating framebuffers");

    swapChainFramebuffers_.resize(swapChainImageViews_.size());

    for (size_t i = 0; i < swapChainImageViews_.size(); i++) {
        VkImageView attachments[] = {
            swapChainImageViews_[i]
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass_;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = swapChainExtent_.width;
        framebufferInfo.height = swapChainExtent_.height;
        framebufferInfo.layers = 1;

        VkResult result = vkCreateFramebuffer(device_, &framebufferInfo, nullptr, &swapChainFramebuffers_[i]);
        if (result != VK_SUCCESS) {
            LOG_ERROR(LogCategory::VULKAN, "Failed to create framebuffer %zu: %d", i, result);
            return false;
        }
    }

    LOG_INFO(LogCategory::VULKAN, "Created %zu framebuffers", swapChainFramebuffers_.size());
    return true;
}

// ============================================================================
// Command pool and buffers
// ============================================================================

bool VulkanRenderer::createCommandPool() {
    LOG_DEBUG(LogCategory::VULKAN, "Creating command pool");

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = graphicsQueueFamily_;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    VkResult result = vkCreateCommandPool(device_, &poolInfo, nullptr, &commandPool_);
    if (result != VK_SUCCESS) {
        LOG_ERROR(LogCategory::VULKAN, "Failed to create command pool: %d", result);
        return false;
    }

    LOG_INFO(LogCategory::VULKAN, "Command pool created successfully");
    return true;
}

bool VulkanRenderer::createCommandBuffers() {
    LOG_DEBUG(LogCategory::VULKAN, "Creating command buffers");

    commandBuffers_.resize(MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool_;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers_.size());

    VkResult result = vkAllocateCommandBuffers(device_, &allocInfo, commandBuffers_.data());
    if (result != VK_SUCCESS) {
        LOG_ERROR(LogCategory::VULKAN, "Failed to allocate command buffers: %d", result);
        return false;
    }

    LOG_INFO(LogCategory::VULKAN, "Command buffers created successfully");
    return true;
}

// ============================================================================
// Synchronization objects
// ============================================================================

bool VulkanRenderer::createSyncObjects() {
    LOG_DEBUG(LogCategory::VULKAN, "Creating synchronization objects");

    imageAvailableSemaphores_.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores_.resize(MAX_FRAMES_IN_FLIGHT);
    inFlightFences_.resize(MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(device_, &semaphoreInfo, nullptr, &imageAvailableSemaphores_[i]) != VK_SUCCESS ||
            vkCreateSemaphore(device_, &semaphoreInfo, nullptr, &renderFinishedSemaphores_[i]) != VK_SUCCESS ||
            vkCreateFence(device_, &fenceInfo, nullptr, &inFlightFences_[i]) != VK_SUCCESS) {
            LOG_ERROR(LogCategory::VULKAN, "Failed to create synchronization objects for frame %zu", i);
            return false;
        }
    }

    LOG_INFO(LogCategory::VULKAN, "Synchronization objects created successfully");
    return true;
}

// ============================================================================
// Rendering
// ============================================================================

void VulkanRenderer::beginFrame() {
    vkWaitForFences(device_, 1, &inFlightFences_[currentFrame_], VK_TRUE, UINT64_MAX);

    VkResult result = vkAcquireNextImageKHR(device_, swapChain_, UINT64_MAX,
        imageAvailableSemaphores_[currentFrame_], VK_NULL_HANDLE, &imageIndex_);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        // Recreate swap chain
        LOG_WARN(LogCategory::VULKAN, "Swap chain out of date");
        return;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        LOG_ERROR(LogCategory::VULKAN, "Failed to acquire swap chain image: %d", result);
        return;
    }

    vkResetFences(device_, 1, &inFlightFences_[currentFrame_]);

    vkResetCommandBuffer(commandBuffers_[currentFrame_], 0);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0;
    beginInfo.pInheritanceInfo = nullptr;

    if (vkBeginCommandBuffer(commandBuffers_[currentFrame_], &beginInfo) != VK_SUCCESS) {
        LOG_ERROR(LogCategory::VULKAN, "Failed to begin recording command buffer");
        return;
    }

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass_;
    renderPassInfo.framebuffer = swapChainFramebuffers_[imageIndex_];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = swapChainExtent_;

    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(commandBuffers_[currentFrame_], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void VulkanRenderer::endFrame() {
    vkCmdEndRenderPass(commandBuffers_[currentFrame_]);

    if (vkEndCommandBuffer(commandBuffers_[currentFrame_]) != VK_SUCCESS) {
        LOG_ERROR(LogCategory::VULKAN, "Failed to record command buffer");
        return;
    }

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {imageAvailableSemaphores_[currentFrame_]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffers_[currentFrame_];

    VkSemaphore signalSemaphores[] = {renderFinishedSemaphores_[currentFrame_]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(graphicsQueue_, 1, &submitInfo, inFlightFences_[currentFrame_]) != VK_SUCCESS) {
        LOG_ERROR(LogCategory::VULKAN, "Failed to submit draw command buffer");
        return;
    }

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {swapChain_};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex_;
    presentInfo.pResults = nullptr;

    vkQueuePresentKHR(presentQueue_, &presentInfo);

    currentFrame_ = (currentFrame_ + 1) % MAX_FRAMES_IN_FLIGHT;
}

} // namespace phantom
