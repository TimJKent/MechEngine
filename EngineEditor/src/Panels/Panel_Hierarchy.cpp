#include "Panel_Hierarchy.h"

#include "SurfEngine/Scenes/Components.h"
#include "SurfEngine/Scenes/Object.h"
#include "SurfEngine/Scenes/Scene.h"
#include "../Util/ProjectManager.h"
#include "SurfEngine/Scenes/ObjectSerializer.h"

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

namespace SurfEngine {

	Panel_Hierarchy::Panel_Hierarchy() {
		add_icon = Texture2D::Create("res\\textures\\icon_add.png");

	}

	void Panel_Hierarchy::OnImGuiRender(){

		if (ImGui::Begin("Hierarchy",NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDecoration)) {
			if (ProjectManager::IsActiveScene()) {
				char* buff = new char[40];
				float buttonWidth = 12.f;
				std::strcpy(buff, "search");
				ImGui::InputText("##search",buff, 40);
				ImGui::SameLine();
				ImGui::SetCursorPosX(ImGui::GetWindowWidth() - (buttonWidth + 18.f));
				if (ImGui::ImageButton((ImTextureID)(uint64_t)add_icon->GetRendererID(), { buttonWidth ,buttonWidth })) {
					auto& scene = ProjectManager::GetActiveScene();
					scene->CreateObject("New Game Object");
				}
				ImGui::Separator();
				auto group = ProjectManager::GetActiveScene()->m_Registry.group<TagComponent>(entt::get<TransformComponent>);
				group.sort<TagComponent>([](const TagComponent& lhs, const TagComponent& rhs) {
					return lhs.Tag.compare(rhs.Tag) < 0;
					});
				for (auto entity : group) {
					auto [tag, transform] = group.get<TagComponent, TransformComponent>(entity);
					if (ProjectManager::GetActiveScene()->m_Registry.valid(entity)) {
						if(transform.parent == nullptr)
							DrawObjectNode(Object(entity, ProjectManager::GetActiveScene().get()));
					}
				}
				delete[] buff;
				ImGui::PushID("rspace");
				ImGui::InvisibleButton("##dd",ImGui::GetContentRegionAvail());

				if (ImGui::BeginDragDropTarget())
				{

					char* data;
					const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("objectitem");
					if (payload) {
						data = new char[payload->DataSize + 1];
						memcpy((char*)&data[0], payload->Data, payload->DataSize);

						UUID uuid = std::stoull(data);
						TransformComponent* ref = &ProjectManager::GetActiveScene()->GetObjectByUUID(uuid).GetComponent<TransformComponent>();
						
						ref->SetParent(nullptr);
					}
					ImGui::EndDragDropTarget();
				}
				ImGui::PopID();
			}
		}
		ImGui::End();
	}

	void Panel_Hierarchy::DrawObjectNode(Object object) {
		TagComponent& tag = ProjectManager::GetActiveScene()->m_Registry.get<TagComponent>(object);
		TransformComponent& tc = ProjectManager::GetActiveScene()->m_Registry.get<TransformComponent>(object);

		ImGui::PushID(tag.Tag.c_str());

		bool entityDeleted = false;
		bool duplicateEntity = false;
		
		bool selectHighlight = false;

		if (ProjectManager::GetSelectedObject().get()) {
			selectHighlight = *ProjectManager::GetSelectedObject().get() == object;
		}

		
		ImGuiTreeNodeFlags flags = ((selectHighlight) ? ImGuiTreeNodeFlags_Selected : 0) | ((tc.children.size() > 0) ? ImGuiTreeNodeFlags_OpenOnArrow : ImGuiTreeNodeFlags_Leaf) | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_DefaultOpen;

		
		if (ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)object, flags, tag.Tag.c_str()))
		{
			if (ImGui::IsItemClicked())
			{
				ProjectManager::SetSelectedObject(std::make_shared<Object>(Object(object, ProjectManager::GetActiveScene().get())));
			}
			
			if (ImGui::BeginDragDropSource()) {
				ImGui::Text(tag.Tag.c_str());
				std::string data = tag.uuid.ToString();
				data += '\0';
				ImGui::SetDragDropPayload("objectitem", data.c_str(), data.length());
				ImGui::EndDragDropSource();
			}

			if (ImGui::BeginDragDropTarget())
			{
				char* data;
				const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("objectitem");
				if (payload) {
					data = new char[payload->DataSize + 1];
					memcpy((char*)&data[0], payload->Data, payload->DataSize);

					UUID uuid = std::stoull(data);
					TransformComponent* ref = &ProjectManager::GetActiveScene()->GetObjectByUUID(uuid).GetComponent<TransformComponent>();
					ref->SetParent(&tc);
				}
				ImGui::EndDragDropTarget();
			}

			if (ImGui::BeginPopupContextItem(tag.uuid.ToString().c_str()))
			{
				if (ImGui::MenuItem("Create Asset")) {
					ObjectSerializer serializer = ObjectSerializer();
					serializer.Serialize(
						ProjectManager::GetPath()+"\\" + tag.Tag + ".asset",
						object
					);
				}
				if (ImGui::MenuItem("Duplicate")) {
					ProjectManager::GetActiveScene()->DuplicateObject(object);
				}
				ImGui::Separator();
				if (ImGui::MenuItem("Delete")) {
					ProjectManager::GetActiveScene()->DeleteObject(object);
					ProjectManager::ClearSelectedObject();
				}
				
				ImGui::EndPopup();
			}
			for (int i = 0; i < tc.children.size(); i++) {
				DrawObjectNode(tc.children[i]->gameObject);
			}

			ImGui::TreePop();
		}
		ImGui::PopID();
	}
}
