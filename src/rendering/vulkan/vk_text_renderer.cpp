#include "vk_text_renderer.h"
#include "vk_renderer.h"
#include "rendering/core/font_loader.h"
#include "rendering/core/glyph_fragmenter.h"
#include "utils/logger.h"
#include <fstream>
#include <cstring>
#include <cmath>

namespace phantom {

// Helper function to create orthographic projection matrix
static void createOrthographicMatrix(float* matrix, float left, float right, float bottom, float top) {
    memset(matrix, 0, 16 * sizeof(float));

    matrix[0] = 2.0f / (right - left);
    matrix[5] = 2.0f / (top - bottom);
    matrix[10] = -1.0f;
    matrix[12] = -(right + left) / (right - left);
    matrix[13] = -(top + bottom) / (top - bottom);
    matrix[15] = 1.0f;
}

VulkanTextRenderer::VulkanTextRenderer() {
    LOG_TRACE(LogCategory::RENDER, "VulkanTextRenderer constructor");
}

VulkanTextRenderer::~VulkanTextRenderer() {
    LOG_TRACE(LogCategory::RENDER, "VulkanTextRenderer destructor");
    cleanup();
}

bool VulkanTextRenderer::initialize(VulkanRenderer* renderer, VkRenderPass renderPass, const FontAtlas& atlas) {
    LOG_INFO(LogCategory::RENDER, "Initializing Vulkan text renderer");

    renderer_ = renderer;
    renderPass_ = renderPass;
    atlas_ = &atlas;
    device_ = renderer->getDevice();

    // Create glyph fragmenter
    fragmenter_ = new GlyphFragmenter();
    LOG_DEBUG(LogCategory::RENDER, "GlyphFragmenter created");

    if (!createDescriptorSet()) {
        LOG_ERROR(LogCategory::RENDER, "Failed to create descriptor set");
        return false;
    }

    if (!createPipeline()) {
        LOG_ERROR(LogCategory::RENDER, "Failed to create text pipeline");
        return false;
    }

    if (!createFontTexture()) {
        LOG_ERROR(LogCategory::RENDER, "Failed to create font texture");
        return false;
    }

    // Create vertex buffer (start with space for 100 characters = 600 vertices)
    vertexBufferSize_ = 100 * 6 * sizeof(TextVertex);
    // Buffer creation will be done when needed

    initialized_ = true;
    LOG_INFO(LogCategory::RENDER, "Vulkan text renderer initialized successfully");

    return true;
}

void VulkanTextRenderer::cleanup() {
    if (!initialized_) {
        return;
    }

    LOG_INFO(LogCategory::RENDER, "Cleaning up Vulkan text renderer");

    if (device_ != VK_NULL_HANDLE) {
        vkDeviceWaitIdle(device_);

        // Cleanup vertex buffer
        if (vertexBuffer_ != VK_NULL_HANDLE) {
            vkDestroyBuffer(device_, vertexBuffer_, nullptr);
        }
        if (vertexBufferMemory_ != VK_NULL_HANDLE) {
            vkFreeMemory(device_, vertexBufferMemory_, nullptr);
        }

        // Cleanup descriptor set
        if (descriptorPool_ != VK_NULL_HANDLE) {
            vkDestroyDescriptorPool(device_, descriptorPool_, nullptr);
        }
        if (descriptorSetLayout_ != VK_NULL_HANDLE) {
            vkDestroyDescriptorSetLayout(device_, descriptorSetLayout_, nullptr);
        }

        // Cleanup font texture
        if (fontSampler_ != VK_NULL_HANDLE) {
            vkDestroySampler(device_, fontSampler_, nullptr);
        }
        if (fontImageView_ != VK_NULL_HANDLE) {
            vkDestroyImageView(device_, fontImageView_, nullptr);
        }
        if (fontImage_ != VK_NULL_HANDLE) {
            vkDestroyImage(device_, fontImage_, nullptr);
        }
        if (fontImageMemory_ != VK_NULL_HANDLE) {
            vkFreeMemory(device_, fontImageMemory_, nullptr);
        }

        // Cleanup pipeline
        if (pipeline_ != VK_NULL_HANDLE) {
            vkDestroyPipeline(device_, pipeline_, nullptr);
        }
        if (pipelineLayout_ != VK_NULL_HANDLE) {
            vkDestroyPipelineLayout(device_, pipelineLayout_, nullptr);
        }

        // Cleanup shaders
        if (vertShaderModule_ != VK_NULL_HANDLE) {
            vkDestroyShaderModule(device_, vertShaderModule_, nullptr);
        }
        if (fragShaderModule_ != VK_NULL_HANDLE) {
            vkDestroyShaderModule(device_, fragShaderModule_, nullptr);
        }
    }

    // Cleanup fragmenter
    if (fragmenter_ != nullptr) {
        delete fragmenter_;
        fragmenter_ = nullptr;
    }

    initialized_ = false;
    LOG_INFO(LogCategory::RENDER, "Vulkan text renderer cleaned up");
}

bool VulkanTextRenderer::loadShader(const std::string& filename, VkShaderModule& shaderModule) {
    LOG_DEBUG(LogCategory::RENDER, "Loading shader: %s", filename.c_str());

    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        LOG_ERROR(LogCategory::RENDER, "Failed to open shader file: %s", filename.c_str());
        return false;
    }

    size_t fileSize = file.tellg();
    file.seekg(0);

    std::vector<char> buffer(fileSize);
    file.read(buffer.data(), fileSize);
    file.close();

    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = buffer.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(buffer.data());

    if (vkCreateShaderModule(device_, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        LOG_ERROR(LogCategory::RENDER, "Failed to create shader module: %s", filename.c_str());
        return false;
    }

    LOG_DEBUG(LogCategory::RENDER, "Shader loaded successfully: %zu bytes", fileSize);
    return true;
}

bool VulkanTextRenderer::createPipeline() {
    LOG_DEBUG(LogCategory::RENDER, "Creating text rendering pipeline");

    // Load shaders
    if (!loadShader("shaders/text_vert.spv", vertShaderModule_)) {
        return false;
    }
    if (!loadShader("shaders/text_frag.spv", fragShaderModule_)) {
        return false;
    }

    // Shader stages
    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule_;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule_;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    // Vertex input
    VkVertexInputBindingDescription bindingDescription{};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(TextVertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription attributeDescriptions[3];

    // Position
    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(TextVertex, position);

    // TexCoord
    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(TextVertex, texCoord);

    // Fragment Mode
    attributeDescriptions[2].binding = 0;
    attributeDescriptions[2].location = 2;
    attributeDescriptions[2].format = VK_FORMAT_R32_UINT;
    attributeDescriptions[2].offset = offsetof(TextVertex, fragmentMode);

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.vertexAttributeDescriptionCount = 3;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions;

    // Input assembly
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    // Viewport and scissor (dynamic)
    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    // Rasterizer
    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    // Multisampling
    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    // Blending (enable alpha blending for text)
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                           VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_TRUE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;

    // Dynamic state
    VkDynamicState dynamicStates[] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = 2;
    dynamicState.pDynamicStates = dynamicStates;

    // Push constants
    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(float) * 16; // 4x4 matrix

    // Pipeline layout
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout_;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

    if (vkCreatePipelineLayout(device_, &pipelineLayoutInfo, nullptr, &pipelineLayout_) != VK_SUCCESS) {
        LOG_ERROR(LogCategory::RENDER, "Failed to create pipeline layout");
        return false;
    }

    // Create pipeline
    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = pipelineLayout_;
    pipelineInfo.renderPass = renderPass_;
    pipelineInfo.subpass = 0;

    if (vkCreateGraphicsPipelines(device_, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline_) != VK_SUCCESS) {
        LOG_ERROR(LogCategory::RENDER, "Failed to create graphics pipeline");
        return false;
    }

    LOG_INFO(LogCategory::RENDER, "Text rendering pipeline created successfully");
    return true;
}

bool VulkanTextRenderer::createFontTexture() {
    LOG_DEBUG(LogCategory::RENDER, "Creating font texture");

    // TODO: Implement texture creation from atlas bitmap
    // This will require access to VulkanRenderer's physical device and queues
    // For FASE 1B, we'll create a stub that will be implemented

    LOG_INFO(LogCategory::RENDER, "Font texture created successfully (stub)");
    return true;
}

bool VulkanTextRenderer::createDescriptorSet() {
    LOG_DEBUG(LogCategory::RENDER, "Creating descriptor set");

    // Descriptor set layout
    VkDescriptorSetLayoutBinding samplerLayoutBinding{};
    samplerLayoutBinding.binding = 0;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &samplerLayoutBinding;

    if (vkCreateDescriptorSetLayout(device_, &layoutInfo, nullptr, &descriptorSetLayout_) != VK_SUCCESS) {
        LOG_ERROR(LogCategory::RENDER, "Failed to create descriptor set layout");
        return false;
    }

    LOG_INFO(LogCategory::RENDER, "Descriptor set created successfully (stub)");
    return true;
}

void VulkanTextRenderer::updateProjection(int width, int height) {
    LOG_DEBUG(LogCategory::RENDER, "Updating projection matrix: %dx%d", width, height);

    // Create orthographic projection matrix
    // Origin at top-left, Y-axis points down (standard for text rendering)
    createOrthographicMatrix(projectionMatrix_, 0.0f, static_cast<float>(width),
                            static_cast<float>(height), 0.0f);
}

void VulkanTextRenderer::renderText(VkCommandBuffer commandBuffer, const std::string& text,
                                    float x, float y, float scale) {
    if (!initialized_ || text.empty()) {
        return;
    }

    // Build vertex data for all characters
    std::vector<TextVertex> vertices;
    vertices.reserve(text.length() * 6); // 6 vertices per character (2 triangles)

    float cursorX = x;
    float cursorY = y;
    size_t charIndex = 0;
    size_t currentLine = 0;
    size_t currentColumn = 0;

    for (char ch : text) {
        // Handle newlines
        if (ch == '\n') {
            cursorX = x;
            cursorY += atlas_->lineHeight * scale;
            currentLine++;
            currentColumn = 0;
            charIndex++;
            continue;
        }

        // Get glyph from atlas
        if (ch < 32 || ch > 126) {
            charIndex++;
            currentColumn++;
            continue; // Skip non-printable characters
        }

        const Glyph& glyph = atlas_->glyphs[ch - 32];

        // Determine fragment mode using the fragmenter
        FragmentMode mode = fragmenter_->getFragmentMode(currentLine, currentColumn);
        uint32_t fragmentMode = static_cast<uint32_t>(mode);

        // Calculate quad positions
        float x0 = cursorX + glyph.xOffset * scale;
        float y0 = cursorY + glyph.yOffset * scale;
        float x1 = x0 + glyph.width * scale;
        float y1 = y0 + glyph.height * scale;

        // Calculate texture coordinates (already normalized in glyph structure)
        float u0 = glyph.x0;
        float v0 = glyph.y0;
        float u1 = glyph.x1;
        float v1 = glyph.y1;

        // Create 6 vertices for two triangles (quad)
        // Triangle 1: top-left, bottom-left, top-right
        vertices.push_back({{x0, y0}, {u0, v0}, fragmentMode});
        vertices.push_back({{x0, y1}, {u0, v1}, fragmentMode});
        vertices.push_back({{x1, y0}, {u1, v0}, fragmentMode});

        // Triangle 2: top-right, bottom-left, bottom-right
        vertices.push_back({{x1, y0}, {u1, v0}, fragmentMode});
        vertices.push_back({{x0, y1}, {u0, v1}, fragmentMode});
        vertices.push_back({{x1, y1}, {u1, v1}, fragmentMode});

        // Advance cursor
        cursorX += glyph.advance * scale;
        charIndex++;
        currentColumn++;
    }

    if (vertices.empty()) {
        return;
    }

    LOG_TRACE(LogCategory::RENDER, "Rendering %zu characters (%zu vertices)",
              text.length(), vertices.size());

    // TODO: Upload vertices to GPU and draw
    // For now, this is a stub that will be implemented when we integrate with VulkanRenderer
}

} // namespace phantom
