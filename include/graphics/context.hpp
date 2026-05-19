#pragma once

#include <vulkan/vulkan.h>
#include <vector>

namespace voxyl::math {
    class Vector3;
    class Vector4;
    class Matrix4;
    class Quaternion;
}

namespace voxyl::graphics {
    class Pipeline;
    class Buffer;
    class Texture;
}

namespace voxyl::graphics {

    struct Lens {
        bool flat;
        float angle;
    };

    struct Model {
        const Texture* skin;
    };

    struct Sprite {
        const Texture* skin;
        float x;
        float y;
        float width;
        float height;
    };

    class Context {
    public:
        Context(int width, int height, const char* title);
        ~Context();

        Context(const Context&) = delete;
        Context& operator=(const Context&) = delete;

        [[nodiscard]] bool active() const;
        void poll();

        void begin();
        void end();

        void view(const Lens& lens);
        void draw(const Model& model);
        void draw(const Sprite& sprite);

        [[nodiscard]] VkDevice device() const { return core; }
        [[nodiscard]] VkPhysicalDevice hardware() const { return silicon; }

    private:
        void boot(const char* title);
        void link();
        void clean() const;

        void* window;

        VkInstance instance;
        VkSurfaceKHR surface;
        VkPhysicalDevice silicon;
        VkDevice core;
        VkQueue graphics;
        VkQueue present;

        VkSwapchainKHR swapchain;
        std::vector<VkImage> images;
        std::vector<VkImageView> views;
        VkFormat format;
        VkExtent2D extent;

        VkCommandPool pool;
        std::vector<VkCommandBuffer> buffers;
        std::vector<VkSemaphore> available;
        std::vector<VkSemaphore> finished;
        std::vector<VkFence> fences;

        uint32_t tick;
        uint32_t index;
        bool resized;
    };

}