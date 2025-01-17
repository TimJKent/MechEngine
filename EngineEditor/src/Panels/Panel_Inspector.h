#pragma once
#include "SurfEngine/Core/Core.h"
#include "SurfEngine/Scenes/Scene.h"
#include "SurfEngine/Scenes/Object.h"
#include "Panel_Hierarchy.h"


namespace SurfEngine {
	class Panel_Inspector {
	public:
		Panel_Inspector(Ref<Panel_Hierarchy>& hierarchy) {
			m_panel_hierarchy = hierarchy;
		}

		void OnImGuiRender();

		void SetDebugMode(bool debugMode) { m_DebugMode = debugMode; }
		bool GetDebugMode() { return m_DebugMode; }

	private:
		Ref<Panel_Hierarchy> m_panel_hierarchy;
		bool m_DebugMode = false;
	private:
		void DrawComponentTag(Ref<Object>);
		void DrawComponentTransform(Ref<Object>);
		void DrawComponentSpriteRenderer(Ref<Object>);
		void DrawComponentAnimation(Ref<Object>);
		void DrawComponentCamera(Ref<Object>);
		void DrawComponentScript(Ref<Object>);
		void DrawComponentBoxCollider(Ref<Object>);
		void DrawComponentCircleCollider(Ref<Object>);
		void DrawComponentRigidBody(Ref<Object>);
	};
}