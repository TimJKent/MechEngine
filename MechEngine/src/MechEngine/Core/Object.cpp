#pragma once

#include "mepch.h"
#include "MechEngine/Core/Object.h"
#include "MechEngine/Core/Components/Component.h"
#include "MechEngine/Core/Components/TransformComponent.h"
#include "MechEngine/Core/Components/TransformComponent.cpp"
#include "MechEngine/Core/Components/MeshRendererComponent.h"
#include "MechEngine/Core/Components/MeshRendererComponent.cpp"

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

namespace MechEngine {

	Object::Object():
		m_Name("NewObject"),
		m_Enabled(true),
		m_Components()
	{}
	
	void Object::OnUpdate() {
		if (m_Enabled) {
			for (int i = 0; i < m_Components.size(); i++) {
				if (m_Components[i]->IsEnabled)
					m_Components[i]->OnUpdate();
			}
		}
	}

	const const std::string& Object::GetName() const {
		return m_Name;
	}

	void Object::SetName(const std::string& name) {
		m_Name = name;
	}

	void Object::Save() {
		if (!Serialization::ReadyForWrite()) {
			ME_ERROR("ERROR - ScreenMesh: Serializer not ready for Write");
			return;
		}
		Serialization::SERIAL_WRITE(m_Name);
		Serialization::SERIAL_WRITE(m_Enabled);
		Serialization::SERIAL_WRITE((int)m_Components.size());
		for (int i = 0; i < m_Components.size(); i++) {
			m_Components[i]->Save();
		}
	}

	int Object::GetNumberOfComponents() {
		return m_Components.size();
	}
}