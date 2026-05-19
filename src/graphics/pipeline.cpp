#include "../include/graphics/pipeline.hpp"
#include "../include/graphics/context.hpp"
#include <fstream>
#include <stdexcept>

namespace voxyl::graphics {

    Pipeline::Pipeline(const Context& context, const std::string& vertex, const std::string& fragment, const Configuration& configuration)
        : core(context.device()), pipeline(VK_NULL_HANDLE), shape(configuration.layout) {
        build(vertex, fragment, configuration);
    }

    Pipeline::~Pipeline() {
        if (pipeline != VK_NULL_HANDLE) {
            vkDestroyPipeline(core, pipeline, nullptr);
        }
    }

    void Pipeline::bind(VkCommandBuffer buffer) {
        vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    }

    std::vector<char> Pipeline::read(const std::string& path) {
        std::ifstream file(path, std::ios::ate | std::ios::binary);
        if (!file.is_open()) {
            throw std::runtime_error("error");
        }
        std::size_t size = static_cast<std::size_t>(file.tellg());
        std::vector<char> buffer(size);
        file.seekg(0);
        file.read(buffer.data(), size);
        return buffer;
    }

    void Pipeline::compile(const std::vector<char>& code, VkShaderModule* module) {
        VkShaderModuleCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        info.codeSize = code.size();
        info.pCode = reinterpret_cast<const uint32_t*>(code.data());
        if (vkCreateShaderModule(core, &info, nullptr, module) != VK_SUCCESS) {
            throw std::runtime_error("error");
        }
    }

    void Pipeline::build(const std::string& vertex, const std::string& fragment, const Configuration& configuration) {
        auto vcode = read(vertex);
        auto fcode = read(fragment);

        VkShaderModule vmodule;
        VkShaderModule fmodule;

        compile(vcode, &vmodule);
        compile(fcode, &fmodule);

        VkPipelineShaderStageCreateInfo vstage{};
        vstage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vstage.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vstage.module = vmodule;
        vstage.pName = "main";

        VkPipelineShaderStageCreateInfo fstage{};
        fstage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fstage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fstage.module = fmodule;
        fstage.pName = "main";

        VkPipelineShaderStageCreateInfo stages[] = {vstage, fstage};

        VkPipelineVertexInputStateCreateInfo input{};
        input.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

        VkPipelineDynamicStateCreateInfo dynamic{};
        dynamic.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamic.dynamicStateCount = static_cast<uint32_t>(configuration.states.size());
        dynamic.pDynamicStates = configuration.states.data();

        VkGraphicsPipelineCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        info.stageCount = 2;
        info.pStages = stages;
        info.pVertexInputState = &input;
        info.pInputAssemblyState = &configuration.assembly;
        info.pViewportState = &configuration.viewport;
        info.pRasterizationState = &configuration.raster;
        info.pMultisampleState = &configuration.multisample;
        info.pDepthStencilState = &configuration.depth;
        info.pColorBlendState = &configuration.blend;
        info.pDynamicState = configuration.states.empty() ? nullptr : &dynamic;
        info.layout = configuration.layout;
        info.renderPass = configuration.pass;
        info.subpass = configuration.subpass;

        if (vkCreateGraphicsPipelines(core, VK_NULL_HANDLE, 1, &info, nullptr, &pipeline) != VK_SUCCESS) {
            throw std::runtime_error("error");
        }

        vkDestroyShaderModule(core, fmodule, nullptr);
        vkDestroyShaderModule(core, vmodule, nullptr);
    }

    void Pipeline::solid(Configuration& configuration) {
        configuration.assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        configuration.assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        configuration.assembly.primitiveRestartEnable = VK_FALSE;

        configuration.raster.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        configuration.raster.depthClampEnable = VK_FALSE;
        configuration.raster.rasterizerDiscardEnable = VK_FALSE;
        configuration.raster.polygonMode = VK_POLYGON_MODE_FILL;
        configuration.raster.lineWidth = 1.0f;
        configuration.raster.cullMode = VK_CULL_MODE_BACK_BIT;
        configuration.raster.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        configuration.raster.depthBiasEnable = VK_FALSE;

        configuration.multisample.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        configuration.multisample.sampleShadingEnable = VK_FALSE;
        configuration.multisample.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        configuration.depth.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        configuration.depth.depthTestEnable = VK_TRUE;
        configuration.depth.depthWriteEnable = VK_TRUE;
        configuration.depth.depthCompareOp = VK_COMPARE_OP_LESS;

        configuration.attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        configuration.attachment.blendEnable = VK_FALSE;

        configuration.blend.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        configuration.blend.logicOpEnable = VK_FALSE;
        configuration.blend.attachmentCount = 1;
        configuration.blend.pAttachments = &configuration.attachment;
    }

    void Pipeline::flat(Configuration& configuration) {
        solid(configuration);
        configuration.depth.depthTestEnable = VK_FALSE;
        configuration.depth.depthWriteEnable = VK_FALSE;
        configuration.attachment.blendEnable = VK_TRUE;
        configuration.attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        configuration.attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        configuration.attachment.colorBlendOp = VK_BLEND_OP_ADD;
    }

}