#pragma once

#include <vulkan/vulkan.h>
#include <string>
#include <vector>

namespace voxyl::graphics {

    class Context;

    struct Configuration {
        VkPipelineViewportStateCreateInfo viewport;
        VkPipelineInputAssemblyStateCreateInfo assembly;
        VkPipelineRasterizationStateCreateInfo raster;
        VkPipelineMultisampleStateCreateInfo multisample;
        VkPipelineColorBlendAttachmentState attachment;
        VkPipelineColorBlendStateCreateInfo blend;
        VkPipelineDepthStencilStateCreateInfo depth;
        std::vector<VkDynamicState> states;
        VkPipelineLayout layout;
        VkRenderPass pass;
        uint32_t subpass = 0;
    };

    class Pipeline {
    public:
        Pipeline(const Context& context, const std::string& vertex, const std::string& fragment, const Configuration& configuration);
        ~Pipeline();

        Pipeline(const Pipeline&) = delete;
        Pipeline& operator=(const Pipeline&) = delete;

        void bind(VkCommandBuffer buffer);

        [[nodiscard]] VkPipeline handle() const { return pipeline; }
        [[nodiscard]] VkPipelineLayout layout() const { return shape; }

        static void solid(Configuration& configuration);
        static void flat(Configuration& configuration);

    private:
        [[nodiscard]] std::vector<char> read(const std::string& path);
        void build(const std::string& vertex, const std::string& fragment, const Configuration& configuration);
        void compile(const std::vector<char>& code, VkShaderModule* module);

        VkDevice core;
        VkPipeline pipeline;
        VkPipelineLayout shape;
    };

}