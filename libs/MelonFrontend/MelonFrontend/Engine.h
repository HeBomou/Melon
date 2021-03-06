#pragma once

#include <MelonFrontend/MeshBuffer.h>
#include <MelonFrontend/RenderBatch.h>
#include <MelonFrontend/Renderer.h>
#include <MelonFrontend/SwapChain.h>
#include <MelonFrontend/Window.h>
#include <MelonTask/TaskManager.h>

#include <cstddef>
#include <glm/gtc/quaternion.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <memory>
#include <vector>

namespace Melon {

class Engine {
  public:
    void initialize(TaskManager* taskManager, const char* title, const unsigned int& width, const unsigned int& height);
    void terminate();

    void beginFrame();
    MeshBuffer createMeshBuffer(std::vector<Vertex> vertices, std::vector<uint16_t> indices);
    void destroyMeshBuffer(const MeshBuffer& meshBuffer);
    void beginBatches();
    void addBatch(std::vector<glm::mat4> const& models, const MeshBuffer& meshBuffer);
    void endBatches();
    void renderFrame(const glm::mat4& projection, const glm::vec3& cameraTranslation, const glm::quat& cameraRotation, const glm::vec3& lightDirection);
    void endFrame();

    float windowAspectRatio() const { return m_Window.aspectRatio(); }
    bool windowClosed() const { return m_Window.closed(); }

    const std::vector<KeyDownEvent>& keyDownEvents() { return m_Window.keyDownEvents(); }
    const std::vector<KeyUpEvent>& keyUpEvents() { return m_Window.keyUpEvents(); }
    const std::vector<MouseButtonDownEvent>& mouseButtonDownEvents() { return m_Window.mouseButtonDownEvents(); }
    const std::vector<MouseButtonUpEvent>& mouseButtonUpEvents() { return m_Window.mouseButtonUpEvents(); }
    const std::vector<MouseScrollEvent>& mouseScrollEvents() { return m_Window.mouseScrollEvents(); }

  private:
    void notifyWindowResized();
    void notifyWindowClosed();

    Window m_Window;
    bool m_WindowResized{};
    bool m_WindowClosed{};

    std::unique_ptr<Renderer> m_Renderer;
};

}  // namespace Melon
