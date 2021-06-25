#pragma once
#include "MechEngine.h"
#include "glm/glm.hpp"


class ScreenComponent : public MechEngine::Component{
public:
	ScreenComponent(MechEngine::Object* object);
	void OnUpdate() override;
	void DrawUI() override;
	
	void Save() override;
	void Load() override;

	std::string UniqueId() { return "SM_S"; }
	static std::string StaticId() { return "SM_S"; }


private:
	glm::vec2 m_Resolution;
};