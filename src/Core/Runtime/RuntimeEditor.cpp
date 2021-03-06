// Copyright(c) 2019 - 2020, #Momo
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met :
// 
// 1. Redistributions of source code must retain the above copyright notice, this
// list of conditions and the following disclaimer.
// 
// 2. Redistributions in binary form must reproduce the above copyright notice,
// this list of conditions and the following disclaimer in the documentation
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

#include "RuntimeEditor.h"
#include "Utilities/ImGui/GraphicConsole.h"
#include "Utilities/ImGui/EventLogger.h"
#include "Utilities/Profiler/Profiler.h"
#include "Utilities/ImGui/ImGuiUtils.h"
#include "Utilities/Format/Format.h"
#include "Utilities/Logging/Logger.h"
#include "Utilities/FileSystem/FileManager.h"
#include "Core/Events/WindowResizeEvent.h"
#include "Core/Events/UpdateEvent.h"
#include "Core/Application/Event.h"
#include "Core/Application/Rendering.h"
#include "Platform/Window/WindowManager.h"
#include "Platform/Window/Input.h"
#include "Core/Events/FpsUpdateEvent.h"
#include "Core/Components/Instancing/Instance.h"

namespace MxEngine
{
	RuntimeEditor::~RuntimeEditor()
	{
		Free(this->console);
		Free(this->logger);
	}

	void RuntimeEditor::LogToConsole(const MxString& message)
	{
		this->console->PrintLog(message.c_str()); //-V111
	}

	void RuntimeEditor::ClearConsoleLog()
	{
		this->console->ClearLog();
	}

	void RuntimeEditor::PrintCommandHistory()
	{
		this->console->PrintHistory();
	}

	void InitDockspace(ImGuiID dockspaceId)
	{
		static bool inited = false;
		auto node = ImGui::DockBuilderGetNode(dockspaceId);

		if (inited || (node != nullptr && node->IsSplitNode()))
			return;

		inited = true;
		const float viewportRatio = 0.7f;
		const float editorRatio = 0.15f;
		const float objectListRatio = 0.5f;

		ImGuiID viewportDockspace = 0; 
		ImGuiID editorDockspace = 0;
		ImGui::DockBuilderSplitNode(dockspaceId, ImGuiDir_Left, viewportRatio, &viewportDockspace, &editorDockspace);

		ImGuiID viewportId = 0;
		ImGuiID profilerId = 0;
		ImGui::DockBuilderSplitNode(viewportDockspace, ImGuiDir_Up, viewportRatio, &viewportId, &profilerId);

		ImGuiID applicationId = 0;
		ImGuiID objectEditorDockspace = 0;
		ImGui::DockBuilderSplitNode(editorDockspace, ImGuiDir_Up, editorRatio, &applicationId, &objectEditorDockspace);

		ImGuiID objectListId = 0;
		ImGuiID objectEditorId = 0;
		ImGui::DockBuilderSplitNode(objectEditorDockspace, ImGuiDir_Up, objectListRatio, &objectListId, &objectEditorId);

		ImGui::DockBuilderDockWindow("Viewport", viewportId);
		ImGui::DockBuilderDockWindow("Profiling Tools", profilerId);
		ImGui::DockBuilderDockWindow("Application Editor", applicationId);
		ImGui::DockBuilderDockWindow("Object Editor", objectEditorId);
		ImGui::DockBuilderDockWindow("Object List", objectListId);
		ImGui::DockBuilderDockWindow("Developer Console", objectListId);
		ImGui::DockBuilderDockWindow("Render Editor", objectListId);
		ImGui::DockBuilderDockWindow("Texture Viewer", objectListId);

		ImGui::DockBuilderFinish(dockspaceId);
	}

	void RuntimeEditor::OnUpdate()
	{
		if (this->shouldRender)
		{
			MAKE_SCOPE_PROFILER("RuntimeEditor::OnUpdate()");
			auto dockspaceID = ImGui::DockSpaceOverViewport();
			InitDockspace(dockspaceID);

			static bool isRenderEditorOpened = false;
			static bool isObjectListOpened = false;
			static bool isObjectEditorOpened = false;
			static bool isApplicationEditorOpened = true;
			static bool isTextureListOpened = true;
			static bool isDeveloperConsoleOpened = true;
			static bool isProfilerOpened = true;
			static bool isViewportOpened = true;

			this->console->Draw("Developer Console", &isDeveloperConsoleOpened);

			GUI::DrawRenderEditor("Render Editor", &isRenderEditorOpened);
			GUI::DrawApplicationEditor("Application Editor", &isApplicationEditorOpened);
			GUI::DrawTextureList("Texture Viewer", &isTextureListOpened);

			GUI::DrawViewportWindow("Viewport", this->cachedViewportSize, this->cachedViewportPosition, &isViewportOpened);

			this->DrawMxObjectList(&isObjectListOpened);
			this->DrawMxObjectEditorWindow(&isObjectEditorOpened);

			{
				ImGui::Begin("Profiling Tools", &isProfilerOpened);
				
				GUI_TREE_NODE("Profiler", GUI::DrawProfiler("fps profiler"));
				GUI_TREE_NODE("Render Statistics", GUI::DrawRenderStatistics("render statistics"));
				this->logger->Draw("Event Logger", 20);

				ImGui::End();
			}
			
			ImGui::End();
		}
	}

	void RuntimeEditor::AddEventEntry(const MxString& entry)
	{
		this->logger->AddEventEntry(entry);
	}

	void RuntimeEditor::Toggle(bool isVisible)
	{
		this->shouldRender = isVisible;

		if (!this->shouldRender) // if developer environment was turned off, we should notify application that viewport returned to normal
		{
			Rendering::SetRenderToDefaultFrameBuffer(this->cachedUseDefaultFrameBufferVariable);
			auto windowSize = WindowManager::GetSize();
			Event::AddEvent(MakeUnique<WindowResizeEvent>(this->cachedViewportSize, windowSize));
			this->cachedViewportSize = windowSize;
		}
		else
		{
			this->cachedUseDefaultFrameBufferVariable = Rendering::IsRenderedToDefaultFrameBuffer();
			Rendering::SetRenderToDefaultFrameBuffer(false);
		}
	}

	void RuntimeEditor::AddKeyBinding(KeyCode openKey)
	{
		MXLOG_INFO("MxEngine::ConsoleBinding", MxFormat("bound console to keycode: {0}", EnumToString(openKey)));
		Event::AddEventListener<UpdateEvent>("RuntimeEditor", 
		[cursorPos = Vector2(), cursorModeCached = CursorMode::DISABLED, openKey, savedStateKeyHeld = false](auto& event) mutable
		{
			bool isHeld = Application::GetImpl()->GetWindow().IsKeyHeldUnchecked(openKey);
			if (isHeld != savedStateKeyHeld) savedStateKeyHeld = false;

			if (isHeld && !savedStateKeyHeld)
			{
				savedStateKeyHeld = true;
				if (Application::GetImpl()->GetRuntimeEditor().IsActive())
				{
					Input::SetCursorMode(cursorModeCached);
					Application::GetImpl()->ToggleRuntimeEditor(false);
					Input::SetCursorPosition(cursorPos);
				}
				else
				{
					cursorPos = Input::GetCursorPosition();
					cursorModeCached = Input::GetCursorMode();
					Input::SetCursorMode(CursorMode::NORMAL);
					Application::GetImpl()->ToggleRuntimeEditor(true);
					Input::SetCursorPosition(WindowManager::GetSize() * 0.5f);
				}
			}
		});
	}

	bool IsFileUpdated(FileSystemTime oldTime, const FilePath& filepath)
	{
		auto newTime = File::LastModifiedTime(filepath);
		return oldTime < newTime;
	}

	bool IsAnyFileUpdated(MxVector<FilePath>& filepaths, MxVector<FileSystemTime> updateTimes)
	{
		for (size_t i = 0; i < filepaths.size(); i++)
		{
			if (IsFileUpdated(updateTimes[i], filepaths[i]))
				return true;
		}
		return false;
	}

	FilePath GetShaderLookupDirectory(const ShaderHandle& shader)
	{
		return ToFilePath(shader->GetDebugFilePath(Shader::PipelineStage::FRAGMENT)).parent_path();
	}

	FilePath GetShaderLookupDirectory(const ComputeShaderHandle& shader)
	{
		return ToFilePath(shader->GetDebugFilePath()).parent_path();
	}

	MxVector<MxString> GetTrackedFilePaths(const ShaderHandle& shader)
	{
		MxVector<MxString> paths;

		auto& V = shader->GetDebugFilePath(Shader::PipelineStage::VERTEX);
		auto& G = shader->GetDebugFilePath(Shader::PipelineStage::GEOMETRY);
		auto& F = shader->GetDebugFilePath(Shader::PipelineStage::FRAGMENT);
		auto& includes = shader->GetIncludedFilePaths();

		if (!V.empty()) paths.push_back(V);
		if (!G.empty()) paths.push_back(G);
		if (!F.empty()) paths.push_back(F);

		paths.insert(paths.end(), includes.begin(), includes.end());

		return paths;
	}

	MxVector<MxString> GetTrackedFilePaths(const ComputeShaderHandle& shader)
	{
		MxVector<MxString> paths;

		auto& C = shader->GetDebugFilePath();
		auto& includes = shader->GetIncludedFilePaths();

		if (!C.empty()) paths.push_back(C);

		paths.insert(paths.end(), includes.begin(), includes.end());

		return paths;
	}

	/*
	extracts filenames from filepaths list
	*/
	MxVector<FilePath> GetTrackedFileNames(const MxVector<MxString>& paths)
	{
		MxVector<FilePath> filenames;
		filenames.reserve(paths.size());
		for (const auto& path : paths)
			filenames.push_back(ToFilePath(path).filename());
		return filenames;
	}

	/*
	replaces filename with filepath from lookup directory (i.e. file.txt + directory -> directory/smth/file.txt)
	if functions fails, empty list is returned
	*/
	MxVector<FilePath> ResolveTrackedFileNames(MxVector<FilePath>& filenames, const FilePath& lookupDirectory)
	{
		MxVector<FilePath> resolved;
		resolved.reserve(filenames.size());
		for (const auto& filename : filenames)
		{
			auto resolvedFilePath = FileManager::SearchFileInDirectory(lookupDirectory, filename);
			if (resolvedFilePath.empty())
			{
				MXLOG_WARNING("MxEngine::RuntimeEditor", MxFormat("cannot find shader {} in directory {}", ToMxString(filename), ToMxString(lookupDirectory)));
				resolved.clear();
				break;
			}
			resolved.push_back(resolvedFilePath.lexically_normal());
		}
		return resolved;
	}

	MxVector<FileSystemTime> GetTrackedFilesUpdateTime(const MxVector<FilePath>& filepaths)
	{
		MxVector<FileSystemTime> updateTimes;
		updateTimes.reserve(filepaths.size());
		for (const auto& filepath : filepaths)
			updateTimes.push_back(File::LastModifiedTime(filepath));
		return updateTimes;
	}

	void LoadShader(ShaderHandle shader, const MxVector<FilePath>& filepaths)
	{
		auto optionalGeometryStage = shader->GetDebugFilePath(Shader::PipelineStage::GEOMETRY);
		if (optionalGeometryStage.empty()) // no geometry stage
		{
			shader->Load(filepaths[0], filepaths[1]);
		}
		else
		{
			shader->Load(filepaths[0], filepaths[1], filepaths[2]);
		}
	}

	void LoadShader(ComputeShaderHandle shader, const MxVector<FilePath>& filepaths)
	{
		shader->Load(filepaths[0]);
	}

	template<typename ShaderHandleType>
	void AddShaderUpdateListenerImpl(const ShaderHandleType& shader, const FilePath& lookupDirectory)
	{
		auto debugFilepaths = GetTrackedFilePaths(shader);
		auto filenames = GetTrackedFileNames(debugFilepaths);

		auto resolvedFilepaths = ResolveTrackedFileNames(filenames, lookupDirectory);
		if (resolvedFilepaths.empty()) return;

		auto updateTimes = GetTrackedFilesUpdateTime(resolvedFilepaths);

		Event::AddEventListener<UpdateEvent>("RuntimeEditor", 
			[shader, filepaths = std::move(resolvedFilepaths), updates = std::move(updateTimes)](auto&) mutable
			{
				if (IsAnyFileUpdated(filepaths, updates))
				{
					LoadShader(shader, filepaths);
					updates = GetTrackedFilesUpdateTime(filepaths);
				}
			});
	}

	template<>
	void RuntimeEditor::AddShaderUpdateListener<ShaderHandle, FilePath>(ShaderHandle shader, const FilePath& lookupDirectory)
	{
		AddShaderUpdateListenerImpl(shader, lookupDirectory);
	}

	template<>
	void RuntimeEditor::AddShaderUpdateListener<ComputeShaderHandle, FilePath>(ComputeShaderHandle shader, const FilePath& lookupDirectory)
	{
		AddShaderUpdateListenerImpl(shader, lookupDirectory);
	}

	template<>
	void RuntimeEditor::AddShaderUpdateListener<ShaderHandle>(ShaderHandle shader)
	{
		AddShaderUpdateListener(shader, GetShaderLookupDirectory(shader));
	}

	template<>
	void RuntimeEditor::AddShaderUpdateListener<ComputeShaderHandle>(ComputeShaderHandle shader)
	{
		AddShaderUpdateListener(shader, GetShaderLookupDirectory(shader));
	}

	void RuntimeEditor::DrawTransformManipulator(TransformComponent& transform)
	{
		static ImGuizmo::OPERATION currentOperation = ImGuizmo::TRANSLATE;

		auto viewport = Rendering::GetViewport();
		if (!viewport.IsValid()) return;
		auto viewportPosition = MxObject::GetByComponent(*viewport).Transform.GetPosition();

		auto view = viewport->GetViewMatrix(viewportPosition);
		auto projection = viewport->GetProjectionMatrix();
		auto matrix = transform.GetMatrix();
		
		// ImGuizmo does not work well with reversed perspective projection
		if (viewport->GetCameraType() == CameraType::PERSPECTIVE)
			projection = MakePerspectiveMatrix(
				viewport->Camera.GetZoom(),
				viewport->Camera.GetAspectRatio(),
				viewport->Camera.GetZNear(),
				viewport->Camera.GetZFar()
			);

		// TODO: add docs
		if (!ImGuizmo::IsUsing())
		{
			if (this->IsKeyHeld(KeyCode::T)) currentOperation = ImGuizmo::TRANSLATE;
			if (this->IsKeyHeld(KeyCode::R)) currentOperation = ImGuizmo::ROTATE;
			if (this->IsKeyHeld(KeyCode::S)) currentOperation = ImGuizmo::SCALE;
		}

		bool isSnapped = this->IsKeyHeld(KeyCode::LEFT_CONTROL);
		auto snapInterval = Vector3(currentOperation == ImGuizmo::ROTATE ? 45.0f : 0.5f);

		auto viewportWindow = ImGui::FindWindowByName("Viewport");
		MX_ASSERT(viewportWindow != nullptr);

		ImGuizmo::SetOrthographic(viewport->GetCameraType() == CameraType::ORTHOGRAPHIC);
		ImGuizmo::SetDrawlist(viewportWindow->DrawList);
		ImGuizmo::SetRect(viewportWindow->Pos.x, viewportWindow->Pos.y, viewportWindow->Size.x, viewportWindow->Size.y);
		ImGuizmo::Manipulate(&view[0][0], &projection[0][0], currentOperation, 
			ImGuizmo::MODE::LOCAL, &matrix[0][0], nullptr, isSnapped ? &snapInterval[0] : nullptr);

		auto oldRotation = DegreesVec(transform.GetEulerRotation());

		Vector3 position{ 0.0f };
		Vector3 rotation{ 0.0f };
		Vector3 scale{ 0.0f };
		ImGuizmo::DecomposeMatrixToComponents(&matrix[0][0], &position[0], &rotation[0], &scale[0]);

		auto rotationDelta = rotation - oldRotation;
		scale = VectorMax(scale, MakeVector3(0.001f));

		float maxRotationDelta = Max(std::abs(rotationDelta.x), std::abs(rotationDelta.y), std::abs(rotationDelta.z));
		maxRotationDelta = maxRotationDelta < 0.1f ? 0.0f : maxRotationDelta; // do not account small rotations

		if (currentOperation == ImGuizmo::TRANSLATE)
		{
			transform.SetPosition(position);
		}
		if (currentOperation == ImGuizmo::SCALE)
		{
			transform.SetScale(scale);
		}
		if (currentOperation == ImGuizmo::ROTATE)
		{
			     if (std::abs(rotationDelta.x) == maxRotationDelta) transform.RotateX(rotationDelta.x);
			else if (std::abs(rotationDelta.y) == maxRotationDelta) transform.RotateY(rotationDelta.y);
			else if (std::abs(rotationDelta.z) == maxRotationDelta) transform.RotateZ(rotationDelta.z);
		}
	}

    void RuntimeEditor::DrawMxObject(const MxString& treeName, MxObject::Handle object)
    {
		GUI::DrawMxObjectEditor(treeName.c_str(), *object, this->componentAdderComponentNames, this->componentAdderCallbacks, this->componentEditorCallbacks);
		if (!IsInstanced(*object))
		{
			this->DrawTransformManipulator(object->Transform);
			GUI::DrawMxObjectBoundingBoxEditor(*object);
		}
		// instanciate by middle button click TODO: add docs
		if (ImGui::IsMouseClicked(ImGuiMouseButton_Middle))
		{
			if (IsInstance(*object))
			{
				this->currentlySelectedObject = Instanciate(*GetInstanceParent(*object));
			}
			else
			{
				this->currentlySelectedObject = Instanciate(*object);
			}
		}
	}

	Vector2 RuntimeEditor::GetViewportSize() const
	{
		return this->IsActive() ? this->cachedViewportSize : (Vector2)Rendering::GetViewportSize();
	}

	Vector2 RuntimeEditor::GetViewportPosition() const
	{
		return this->IsActive() ? this->cachedViewportPosition : MakeVector2(0.0f);
	}

	bool RuntimeEditor::IsActive() const
	{
		return this->shouldRender;
	}

	bool RuntimeEditor::IsKeyHeld(KeyCode key)
	{
		return Application::GetImpl()->GetWindow().IsKeyHeldUnchecked(key);
	}

	void RuntimeEditor::DrawMxObjectList(bool* isOpen)
	{
		ImGui::Begin("Object List", isOpen);

		static char filter[128] = { '\0' };
		ImGui::InputText("search filter", filter, std::size(filter));

		if (ImGui::Button("create new MxObject"))
			MxObject::Create();
		ImGui::SameLine();
		if (ImGui::Button("load model as MxObject"))
			GUI::CreateMxObjectFromModelFile();
		ImGui::SameLine();
		if (ImGui::Button("delete selected MxObject"))
			MxObject::Destroy(this->currentlySelectedObject);

		auto objects = MxObject::GetObjects();
		int id = 0;
		for (auto& object : objects)
		{
			bool filteredByName = object.Name.find(filter) != object.Name.npos;
			bool shouldDisplay = object.IsDisplayedInEditor && filteredByName && id < 10000;
			if (shouldDisplay) // do not display too much objects
			{
				ImGui::PushID(id++);
				bool isSelected = this->currentlySelectedObject == MxObject::GetHandle(object);
				if (ImGui::Selectable(object.Name.c_str(), &isSelected))
				{
					if (isSelected) this->currentlySelectedObject = MxObject::GetHandle(object);
					else this->currentlySelectedObject = { };
				}
				ImGui::PopID();
			}
		}
	}

	void RuntimeEditor::DrawMxObjectEditorWindow(bool* isOpen)
	{
		ImGui::Begin("Object Editor", isOpen);
		if (!this->currentlySelectedObject.IsValid())
			ImGui::Text("no object selected");
		else
			this->DrawMxObject(this->currentlySelectedObject->Name, this->currentlySelectedObject);
		ImGui::End();
	}

	RuntimeEditor::RuntimeEditor()
	{
		MAKE_SCOPE_PROFILER("DeveloperConsole::Init");
		MAKE_SCOPE_TIMER("MxEngine::DeveloperConsole", "DeveloperConsole::Init");
		this->console = Alloc<GraphicConsole>();
		this->logger = Alloc<EventLogger>();
	}
}