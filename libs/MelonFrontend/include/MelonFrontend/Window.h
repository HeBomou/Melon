#pragma once

#include <GLFW/glfw3.h>
#include <MelonFrontend/VulkanPlatform.h>

#include <vector>

namespace MelonFrontend {

// TODO: Process window events
class Window {
  public:
    void initialize(char const* title, unsigned int const& width, unsigned int const& height);
    void terminate();

    void pollEvents();

    void waitForResized();

    std::vector<char const*> requiredVulkanInstanceExtensions() const;

    float aspectRatio() const { return static_cast<float>(_extent.width) / static_cast<float>(_extent.height); }

    GLFWwindow* const& window() const { return _window; }

    VkExtent2D const& extent() const { return _extent; }

    bool resized() const { return _resized; }
    bool closed() const { return _closed; }

  private:
    static void framebufferResizeCallback(GLFWwindow* glfwWindow, int width, int height);
    static void windowCloseCallback(GLFWwindow* glfwWindow);

    void notifyResized();
    void notifyClosed();

    GLFWwindow* _window;

    VkExtent2D _extent;

    bool _resized{};
    bool _closed{};
};

}  // namespace MelonFrontend