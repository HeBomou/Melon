#include <MelonCore/Archetype.h>
#include <MelonCore/Entity.h>
#include <MelonCore/EntityCommandBufferChunkTask.h>
#include <MelonCore/EntityManager.h>
#include <MelonCore/Instance.h>
#include <MelonCore/SystemBase.h>
#include <MelonCore/TaskHandle.h>
#include <MelonCore/TaskManager.h>
#include <MelonCore/Time.h>
#include <MelonCore/Translation.h>

#include <array>
#include <cstdio>
#include <memory>

struct MonsterHealth : public MelonCore::Component {
    unsigned int value;
};

struct PersistentDamage : public MelonCore::Component {
    unsigned int value;
};

struct ManualDamageCounter : public MelonCore::ManualComponent {
    unsigned int index;
    // The count of damage taken
    unsigned int damageTakenCount;
};

class MonsterDamageCounterSystem : public MelonCore::SystemBase {
   protected:
    class DamageEntityCommandBufferChunkTask : public MelonCore::EntityCommandBufferChunkTask {
       public:
        DamageEntityCommandBufferChunkTask(const unsigned int& monsterHealthComponentId, const unsigned int& persistentDamageComponentId, const unsigned int& manualDamageCounterComponentId) : _monsterHealthComponentId(monsterHealthComponentId), _persistentDamageComponentId(persistentDamageComponentId), _manualDamageCounterComponentId(manualDamageCounterComponentId) {}
        virtual void execute(const MelonCore::ChunkAccessor& chunkAccessor, const unsigned int& chunkIndex, const unsigned int& firstEntityIndex, MelonCore::EntityCommandBuffer* entityCommandBuffer) override {
            const MelonCore::Entity* entities = chunkAccessor.entityArray();
            MonsterHealth* monsterHealths = chunkAccessor.componentArray<MonsterHealth>(_monsterHealthComponentId);
            PersistentDamage* persistentDamages = chunkAccessor.componentArray<PersistentDamage>(_persistentDamageComponentId);
            ManualDamageCounter* manualDamageCounters = chunkAccessor.componentArray<ManualDamageCounter>(_manualDamageCounterComponentId);
            for (unsigned int i = 0; i < chunkAccessor.entityCount(); i++) {
                if (monsterHealths[i].value <= persistentDamages[i].value)
                    entityCommandBuffer->destroyEntity(entities[i]);
                else
                    monsterHealths[i].value -= persistentDamages[i].value;
                manualDamageCounters[i].damageTakenCount++;
            }
        }

        const unsigned int& _monsterHealthComponentId;
        const unsigned int& _persistentDamageComponentId;
        const unsigned int& _manualDamageCounterComponentId;
    };

    class CollectCounterCommandBufferChunkTask : public MelonCore::EntityCommandBufferChunkTask {
       public:
        CollectCounterCommandBufferChunkTask(const unsigned int& manualDamageCounterComponentId, std::vector<unsigned int>& damageTakenCounts) : _manualDamageCounterComponentId(manualDamageCounterComponentId), _damageTakenCounts(damageTakenCounts) {}
        virtual void execute(const MelonCore::ChunkAccessor& chunkAccessor, const unsigned int& chunkIndex, const unsigned int& firstEntityIndex, MelonCore::EntityCommandBuffer* entityCommandBuffer) override {
            const MelonCore::Entity* entities = chunkAccessor.entityArray();
            ManualDamageCounter* manualDamageCounters = chunkAccessor.componentArray<ManualDamageCounter>(_manualDamageCounterComponentId);
            for (unsigned int i = 0; i < chunkAccessor.entityCount(); i++) {
                _damageTakenCounts[manualDamageCounters[i].index] = manualDamageCounters[i].damageTakenCount;
                entityCommandBuffer->removeComponent<ManualDamageCounter>(entities[i]);
            }
        }

        const unsigned int& _manualDamageCounterComponentId;

        std::vector<unsigned int>& _damageTakenCounts;
    };

    void onEnter() override {
        MelonCore::Archetype* archetype = entityManager()->createArchetypeBuilder().markComponents<MonsterHealth, PersistentDamage, ManualDamageCounter>().createArchetype();

        std::array<MelonCore::Entity, 4> entities;
        for (unsigned int i = 0; i < entities.size(); i++)
            entities[i] = entityManager()->createEntity(archetype);

        entityManager()->setComponent(entities[0], MonsterHealth{.value = 3});
        entityManager()->setComponent(entities[0], PersistentDamage{.value = 2});
        entityManager()->setComponent(entities[0], ManualDamageCounter{.index = 0});

        entityManager()->setComponent(entities[1], MonsterHealth{.value = 8});
        entityManager()->setComponent(entities[1], PersistentDamage{.value = 3});
        entityManager()->setComponent(entities[1], ManualDamageCounter{.index = 1});

        entityManager()->setComponent(entities[2], MonsterHealth{.value = 4});
        entityManager()->setComponent(entities[2], PersistentDamage{.value = 1});
        entityManager()->setComponent(entities[2], ManualDamageCounter{.index = 2});

        entityManager()->setComponent(entities[3], MonsterHealth{.value = 6});
        entityManager()->setComponent(entities[3], PersistentDamage{.value = 2});
        entityManager()->setComponent(entities[3], ManualDamageCounter{.index = 3});

        _monsterEntityFilter = entityManager()->createEntityFilterBuilder().requireComponents<MonsterHealth, PersistentDamage, ManualDamageCounter>().createEntityFilter();
        _collectCounterEntityFilter = entityManager()->createEntityFilterBuilder().requireComponents<ManualDamageCounter>().rejectComponents<MonsterHealth>().createEntityFilter();
        _monsterHealthComponentId = entityManager()->componentId<MonsterHealth>();
        _persistentDamageComponentId = entityManager()->componentId<PersistentDamage>();
        _manualDamageCounterComponentId = entityManager()->componentId<ManualDamageCounter>();

        _damageTakenCounts.resize(4);
    }

    void onUpdate() override {
        printf("Delta time : %f\n", MelonCore::Time::instance()->deltaTime());
        std::shared_ptr<MelonCore::TaskHandle> damageTaskHandle = schedule(std::make_shared<DamageEntityCommandBufferChunkTask>(_monsterHealthComponentId, _persistentDamageComponentId, _manualDamageCounterComponentId), _monsterEntityFilter, predecessor());
        std::shared_ptr<MelonCore::TaskHandle> counterTaskHandle = schedule(std::make_shared<CollectCounterCommandBufferChunkTask>(_manualDamageCounterComponentId, _damageTakenCounts), _collectCounterEntityFilter, predecessor());
        predecessor() = MelonCore::TaskManager::instance()->combine({damageTaskHandle, counterTaskHandle});
        if (entityManager()->entityCount(_monsterEntityFilter) == 0 && entityManager()->entityCount(_collectCounterEntityFilter) == 0) {
            for (const unsigned int& damageTakenCount : _damageTakenCounts)
                printf("%d ", damageTakenCount);
            MelonCore::Instance::instance()->quit();
        }
    }

    void onExit() override {}

   private:
    MelonCore::EntityFilter _monsterEntityFilter;
    MelonCore::EntityFilter _collectCounterEntityFilter;
    unsigned int _monsterHealthComponentId;
    unsigned int _persistentDamageComponentId;
    unsigned int _manualDamageCounterComponentId;

    std::vector<unsigned int> _damageTakenCounts;
};

int main() {
    MelonCore::Instance::instance()->registerSystem<MonsterDamageCounterSystem>();
    MelonCore::Instance::instance()->start();
    return 0;
}