#include "../include/graphics/context.hpp"
#include <stdexcept>

namespace voxyl::graphics {

    Context::Context(int width, int height, const char* title)
        : window(nullptr), instance(VK_NULL_HANDLE), surface(VK_NULL_HANDLE), silicon(VK_NULL_HANDLE),
          core(VK_NULL_HANDLE), swapchain(VK_NULL_HANDLE), tick(0), index(0), resized(false) {
        (void)width; (void)height;
        boot(title);
        link();
    }

    Context::~Context() {
        clean();
    }

    bool Context::active() const {
        return window != nullptr;
    }

    void Context::poll() {}

    void Context::begin() {
        vkWaitForFences(core, 1, &fences[tick], VK_TRUE, UINT64_MAX);
        vkAcquireNextImageKHR(core, swapchain, UINT64_MAX, available[tick], VK_NULL_HANDLE, &index);
        vkResetFences(core, 1, &fences[tick]);
        vkResetCommandBuffer(buffers[tick], 0);

        VkCommandBufferBeginInfo info{};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        vkBeginCommandBuffer(buffers[tick], &info);
    }

    void Context::end() {
        vkEndCommandBuffer(buffers[tick]);

        VkSubmitInfo submit{};
        submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore wait[] = {available[tick]};
        VkPipelineStageFlags stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submit.waitSemaphoreCount = 1;
        submit.pWaitSemaphores = wait;
        submit.pWaitDstStageMask = stages;
        submit.commandBufferCount = 1;
        submit.pCommandBuffers = &buffers[tick];

        VkSemaphore signal[] = {finished[tick]};
        submit.signalSemaphoreCount = 1;
        submit.pSignalSemaphores = signal;

        if (vkQueueSubmit(graphics, 1, &submit, fences[tick]) != VK_SUCCESS) {
            throw std::runtime_error("error");
        }

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signal;

        VkSwapchainKHR chains[] = {swapchain};
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = chains;
        presentInfo.pImageIndices = &index;

        vkQueuePresentKHR(present, &presentInfo);

        tick = (tick + 1) % 2;
    }

    void Context::view(const Lens& lens) { (void)lens; }
    void Context::draw(const Model& model) { (void)model; }
    void Context::draw(const Sprite& sprite) { (void)sprite; }

    void Context::boot(const char* title) {
        VkApplicationInfo app{};
        app.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        app.pApplicationName = title;
        app.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        app.pEngineName = "voxyl";
        app.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        app.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        info.pApplicationInfo = &app;
        info.enabledExtensionCount = 0;
        info.enabledLayerCount = 0;

        if (vkCreateInstance(&info, nullptr, &instance) != VK_SUCCESS) {
            throw std::runtime_error("error");
        }

        uint32_t count = 0;
        vkEnumeratePhysicalDevices(instance, &count, nullptr);
        if (count == 0) throw std::runtime_error("error");
        std::vector<VkPhysicalDevice> devices(count);
        vkEnumeratePhysicalDevices(instance, &count, devices.data());
        silicon = devices[0];

        float priority = 1.0f;
        VkDeviceQueueCreateInfo queueInfo{};
        queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueInfo.queueFamilyIndex = 0;
        queueInfo.queueCount = 1;
        queueInfo.pQueuePriorities = &priority;

        VkDeviceCreateInfo deviceCreateInfo{};
        deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceCreateInfo.queueCreateInfoCount = 1;
        deviceCreateInfo.pQueueCreateInfos = &queueInfo;

        if (vkCreateDevice(silicon, &deviceCreateInfo, nullptr, &core) != VK_SUCCESS) {
            throw std::runtime_error("error");
        }

        vkGetDeviceQueue(core, 0, 0, &graphics);
        vkGetDeviceQueue(core, 0, 0, &present);
    }

    void Context::link() {
        extent = {800, 600};
        format = VK_FORMAT_B8G8R8A8_SRGB;

        available.resize(2);
        finished.resize(2);
        fences.resize(2);

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (std::size_t index = 0; index < 2; ++index) {
            vkCreateSemaphore(core, &semaphoreInfo, nullptr, &available[index]);
            vkCreateSemaphore(core, &semaphoreInfo, nullptr, &finished[index]);
            vkCreateFence(core, &fenceInfo, nullptr, &fences[index]);
        }
    }

    void Context::clean() const {
        if (core != VK_NULL_HANDLE) {
            vkDeviceWaitIdle(core);
            for (std::size_t index = 0; index < available.size(); ++index) {
                vkDestroySemaphore(core, available[index], nullptr);
                vkDestroySemaphore(core, finished[index], nullptr);
                vkDestroyFence(core, fences[index], nullptr);
            }
            if (swapchain != VK_NULL_HANDLE) vkDestroySwapchainKHR(core, swapchain, nullptr);
            vkDestroyDevice(core, nullptr);
        }
        if (surface != VK_NULL_HANDLE) vkDestroySurfaceKHR(instance, surface, nullptr);
        if (instance != VK_NULL_HANDLE) vkDestroyInstance(instance, nullptr);
    }

}