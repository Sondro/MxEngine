// Copyright(c) 2019 - 2020, #Momo
// All rights reserved.
// 
// Redistributionand use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met :
// 
// 1. Redistributions of source code must retain the above copyright notice, this
// list of conditionsand the following disclaimer.
// 
// 2. Redistributions in binary form must reproduce the above copyright notice,
// this list of conditionsand the following disclaimer in the documentation
// and /or other materials provided with the distribution.
// 
// 3. Neither the name of the copyright holder nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED.IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#pragma once

#include "Utilities/ImGui/ImGuiBase.h"
#include "Core/Application/Application.h"
#include "Utilities/ImGui/Layout/Layout.h"

namespace MxEngine::GUI
{
	inline void DrawCameraEditor()
	{
		auto context = Application::Get();
		static bool mesh = false;

		auto& camera = context->GetCurrentScene().Viewport;
		float speed = camera.GetMoveSpeed();
		float sensitivity = camera.GetRotateSpeed();
		float zoom = camera.GetZoom();
		Vector3 pos = camera.GetPosition();

		ImGui::Checkbox("display mesh", &mesh);
		ImGui::Text("position: (%f, %f, %f)", pos.x, pos.y, pos.z);

		context->ToggleMeshDrawing(mesh);

		if (ImGui::InputFloat("set speed", &speed))
			camera.SetMoveSpeed(speed);

		if (ImGui::InputFloat("set sensitivity", &sensitivity))
			camera.SetRotateSpeed(sensitivity);

		if (ImGui::InputFloat3("set position", &pos[0]))
			camera.SetPosition(pos);

		ImGui::Text("zoom / fov: %f", zoom);
		if (ImGui::DragFloat("zoom", &zoom, 0.1f, 0.1f, 20.0f))
			camera.SetZoom(zoom);

		GUI_TREE_NODE("skybox",
			auto& skybox = context->GetCurrentScene().SceneSkybox;
			if (skybox != nullptr)
			{
				auto rotation = DegreesVec(skybox->GetRotation());
				auto newRotation = rotation;
				if (ImGui::DragFloat("skybox rotate x", &newRotation.x))
					skybox->RotateX(newRotation.x - rotation.x);
				if (ImGui::DragFloat("skybox rotate y", &newRotation.y))
					skybox->RotateY(newRotation.y - rotation.y);
				if (ImGui::DragFloat("skybox rotate z", &newRotation.z))
					skybox->RotateZ(newRotation.z - rotation.z);

				static char buf[128];
				if (GUI::InputTextOnClick("skybox texture", buf, 128))
				{
					skybox->SkyboxTexture = context->GetCurrentScene().LoadCubeMap("SceneRuntimeSkybox", buf);
				}
			}
		);
	}
}