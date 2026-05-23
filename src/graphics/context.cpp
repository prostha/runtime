#include "../include/graphics/context.hpp"

namespace voxyl::graphics {

    Context::Context(int width, int height, const char* title)
        : window(nullptr), instance(VK_NULL_HANDLE), surface(VK_NULL_HANDLE),
          silicon(VK_NULL_HANDLE), core(VK_NULL_HANDLE), graphics(VK_NULL_HANDLE),
          present(VK_NULL_HANDLE), swapchain(VK_NULL_HANDLE), format(VK_FORMAT_UNDEFINED),
          extent{0, 0}, pool(VK_NULL_HANDLE), tick(0), index(0), resized(false) {
        boot(title);
        link();
    }

    Context::~Context() {
        clean();
    }

    bool Context::active() const {
        return true;
    }

    void Context::poll() {
    }

    void Context::begin() {
        vkAcquireNextImageKHR(core, swapchain, UINT64_MAX, available[tick], VK_NULL_HANDLE, &index);
        vkResetCommandBuffer(buffers[index], 0);

        VkCommandBufferBeginInfo info{};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        vkBeginCommandBuffer(buffers[index], &info);
    }

    void Context::end() {
        vkEndCommandBuffer(buffers[index]);

        VkPipelineStageFlags flags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        VkSubmitInfo info{};
        info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        info.waitSemaphoreCount = 1;
        info.pWaitSemaphores = &available[tick];
        info.pWaitDstStageMask = &flags;
        info.commandBufferCount = 1;
        info.pCommandBuffers = &buffers[index];
        info.signalSemaphoreCount = 1;
        info.pSignalSemaphores = &finished[tick];

        vkQueueSubmit(graphics, 1, &info, fences[tick]);

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &finished[tick];
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &swapchain;
        presentInfo.pImageIndices = &index;

        vkQueuePresentKHR(present, &presentInfo);
        tick = (tick + 1) % 2;
    }

    void Context::view(const Camera& camera) {
    }

    void Context::draw(const Model& model) {
    }

    void Context::draw(const Sprite& sprite) {
    }

    void Context::boot(const char* title) {
    }

    void Context::link() {
    }

    void Context::clean() const {
        vkDestroyCommandPool(core, pool, nullptr);
        vkDestroyDevice(core, nullptr);
        vkDestroyInstance(instance, nullptr);
    }

}