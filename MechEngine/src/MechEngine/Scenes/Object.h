#pragma once
#include "Scene.h"
#include "MechEngine/Core/UUID.h"
#include "entt.hpp"

namespace MechEngine {
	class Object {
	public:

		Object() = default;
		Object(entt::entity handle, Scene* scene);
		Object(const Object& other) = default;

		UUID GetUUID() {
			return m_uuid;
		}

		template<typename T>
		bool HasComponent() {
			return m_Scene->m_Registry.all_of<T>(m_ObjectHandle);
		}

		template<typename T>
		T& GetComponent() {
			ME_CORE_ASSERT(HasComponent<T>(), "Object does not have component!");
			return m_Scene->m_Registry.get<T>(m_ObjectHandle);
		}

		template<typename T, typename... Args>
		T& AddComponent(Args&&... args) {
			ME_CORE_ASSERT(!HasComponent<T>(), "Object already has component!");
			return m_Scene->m_Registry.emplace<T>(m_ObjectHandle, std::forward<Args>(args)...);
		}

		template <typename T>
		void RemoveComponent(){
			ME_CORE_ASSERT(HasComponent<T>(), "Object does not have component!");
			m_Scene->m_Registry.remove<T>(m_ObjectHandle);
		}

		operator bool() const { return m_ObjectHandle != entt::null; }
		operator entt::entity() const { return m_ObjectHandle; }
		operator uint32_t() const { return (uint32_t)m_ObjectHandle; }

		bool operator==(const Object& other) const
		{
			return m_ObjectHandle == other.m_ObjectHandle && m_Scene == other.m_Scene;
		}

		bool operator!=(const Object& other) const
		{
			return !(*this == other);
		}


	private:
		entt::entity m_ObjectHandle{ entt::null };
		Scene* m_Scene = nullptr;
		UUID m_uuid;
	};
}