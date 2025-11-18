#ifndef PHANTOM_VK_RENDERER_H
#define PHANTOM_VK_RENDERER_H

#include <vulkan/vulkan.h>
#include <vector>
#include <string>

namespace phantom {

class IPlatformWindow;

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

class VulkanRenderer {
public:
    VulkanRenderer();
    ~VulkanRenderer();

    bool initialize(IPlatformWindow* window);
    void cleanup();

    void beginFrame();
    void endFrame();

    bool isInitialized() const { return initialized_; }

    // Getters for text renderer
    VkDevice getDevice() const { return device_; }
    VkPhysicalDevice getPhysicalDevice() const { return physicalDevice_; }
    VkCommandPool getCommandPool() const { return commandPool_; }
    VkQueue getGraphicsQueue() const { return graphicsQueue_; }
    VkRenderPass getRenderPass() const { return renderPass_; }
    VkExtent2D getSwapChainExtent() const { return swapChainExtent_; }
    VkCommandBuffer getCurrentCommandBuffer() const { return commandBuffers_[currentFrame_]; }

private:
    // Initialization steps
    bool createInstance();
    bool selectPhysicalDevice();
    bool createLogicalDevice();
    bool createSurface();
    bool createSwapChain();
    bool createImageViews();
    bool createRenderPass();
    bool createFramebuffers();
    bool createCommandPool();
    bool createCommandBuffers();
    bool createSyncObjects();

    // Helper functions
    bool checkValidationLayerSupport();
    std::vector<const char*> getRequiredExtensions();
    bool isDeviceSuitable(VkPhysicalDevice device);
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
    int findQueueFamilies(VkPhysicalDevice device);

    // Vulkan objects
    VkInstance instance_ = VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice_ = VK_NULL_HANDLE;
    VkDevice device_ = VK_NULL_HANDLE;
    VkQueue graphicsQueue_ = VK_NULL_HANDLE;
    VkQueue presentQueue_ = VK_NULL_HANDLE;
    VkSurfaceKHR surface_ = VK_NULL_HANDLE;
    VkSwapchainKHR swapChain_ = VK_NULL_HANDLE;
    VkRenderPass renderPass_ = VK_NULL_HANDLE;
    VkCommandPool commandPool_ = VK_NULL_HANDLE;

    std::vector<VkImage> swapChainImages_;
    std::vector<VkImageView> swapChainImageViews_;
    std::vector<VkFramebuffer> swapChainFramebuffers_;
    std::vector<VkCommandBuffer> commandBuffers_;

    VkFormat swapChainImageFormat_;
    VkExtent2D swapChainExtent_;

    // Synchronization
    std::vector<VkSemaphore> imageAvailableSemaphores_;
    std::vector<VkSemaphore> renderFinishedSemaphores_;
    std::vector<VkFence> inFlightFences_;

    static constexpr int MAX_FRAMES_IN_FLIGHT = 2;
    uint32_t currentFrame_ = 0;
    uint32_t imageIndex_ = 0;

    // Queue family indices
    int graphicsQueueFamily_ = -1;
    int presentQueueFamily_ = -1;

    // Platform window
    IPlatformWindow* window_ = nullptr;

    // State
    bool initialized_ = false;

    // Validation layers
#ifdef DEBUG_BUILD
    static constexpr bool enableValidationLayers = true;
#else
    static constexpr bool enableValidationLayers = false;
#endif

    const std::vector<const char*> validationLayers_ = {
        "VK_LAYER_KHRONOS_validation"
    };

    VkDebugUtilsMessengerEXT debugMessenger_ = VK_NULL_HANDLE;
    bool setupDebugMessenger();
    void destroyDebugMessenger();
};

} // namespace phantom

#endif // PHANTOM_VK_RENDERER_H
