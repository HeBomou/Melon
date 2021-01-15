#pragma once

#include <libMelonCore/EntityManager.h>
#include <libMelonCore/Time.h>
#include <libMelonCore/World.h>
#include <libMelonTask/TaskManager.h>

#include <memory>

namespace Melon {

class EntityManager;
class World;

class Instance {
  public:
    Instance();

    template <typename Type, typename... Args>
    void registerSystem(Args&&... args);

    void start();
    void quit();

    const char* const& applicationName() const { return m_ApplicationName; }
    const char*& applicationName() { return m_ApplicationName; }

  private:
    void mainLoop();

    const char* m_ApplicationName{};

    MelonTask::TaskManager m_TaskManager;

    Time m_Time;

    std::unique_ptr<World> m_DefaultWorld;

    bool m_ShouldQuit{};
};

template <typename Type, typename... Args>
void Instance::registerSystem(Args&&... args) {
    m_DefaultWorld->registerSystem<Type>(std::forward<Args>(args)...);
}

}  // namespace Melon
