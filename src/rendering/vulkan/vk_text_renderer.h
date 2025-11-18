#ifndef PHANTOM_VK_TEXT_RENDERER_H
#define PHANTOM_VK_TEXT_RENDERER_H

#include <vulkan/vulkan.h>
#include <phantom_writer/types.h>
#include <string>
#include <vector>

namespace phantom {

class VulkanRenderer;
class GlyphFragmenter;
struct FontAtlas;

struct TextVertex {
    float position[2];    // x, y
    float texCoord[2];    // u, v
    uint32_t fragmentMode; // 0=top, 1=bottom
};

class VulkanTextRenderer {
public:
    VulkanTextRenderer();
    ~VulkanTextRenderer();

    bool initialize(VulkanRenderer* renderer, VkRenderPass renderPass, const FontAtlas& atlas);
    void cleanup();

    // Render text at specified position
    void renderText(VkCommandBuffer commandBuffer, const std::string& text, float x, float y, float scale = 1.0f);

    // Update projection matrix (call when window resizes)
    void updateProjection(int width, int height);

private:
    bool createPipeline();
    bool createFontTexture();
    bool createDescriptorSet();
    bool loadShader(const std::string& filename, VkShaderModule& shaderModule);

    VulkanRenderer* renderer_ = nullptr;
    VkDevice device_ = VK_NULL_HANDLE;
    VkRenderPass renderPass_ = VK_NULL_HANDLE;

    // Pipeline
    VkPipelineLayout pipelineLayout_ = VK_NULL_HANDLE;
    VkPipeline pipeline_ = VK_NULL_HANDLE;

    // Shaders
    VkShaderModule vertShaderModule_ = VK_NULL_HANDLE;
    VkShaderModule fragShaderModule_ = VK_NULL_HANDLE;

    // Font texture
    VkImage fontImage_ = VK_NULL_HANDLE;
    VkDeviceMemory fontImageMemory_ = VK_NULL_HANDLE;
    VkImageView fontImageView_ = VK_NULL_HANDLE;
    VkSampler fontSampler_ = VK_NULL_HANDLE;

    // Descriptor set
    VkDescriptorSetLayout descriptorSetLayout_ = VK_NULL_HANDLE;
    VkDescriptorPool descriptorPool_ = VK_NULL_HANDLE;
    VkDescriptorSet descriptorSet_ = VK_NULL_HANDLE;

    // Vertex buffer for text quads
    VkBuffer vertexBuffer_ = VK_NULL_HANDLE;
    VkDeviceMemory vertexBufferMemory_ = VK_NULL_HANDLE;
    size_t vertexBufferSize_ = 0;

    // Projection matrix
    float projectionMatrix_[16];

    // Font atlas reference
    const FontAtlas* atlas_ = nullptr;

    // Glyph fragmenter
    GlyphFragmenter* fragmenter_ = nullptr;

    bool initialized_ = false;
};

} // namespace phantom

#endif // PHANTOM_VK_TEXT_RENDERER_H
