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

#include "Utilities/Math/Math.h"
#include "core/Macro/Macro.h"

#include <unordered_map>

namespace MxEngine
{
	class Shader
	{
		#if defined(MXENGINE_DEBUG)
		std::string vertexShaderPath, geometryShaderPath, fragmentShaderPath;
		#endif
		using UniformType = int;
		using UniformCache = std::unordered_map<std::string, UniformType>;
		using ShaderId = unsigned int;
		using BindableId = unsigned int;

		BindableId id = 0;
		mutable UniformCache uniformCache;

		ShaderId CompileShader(unsigned int type, const std::string& source, const std::string& name) const;
		BindableId CreateProgram(ShaderId vertexShader, ShaderId fragmentShader) const;
		BindableId CreateProgram(ShaderId vertexShader, ShaderId geometryShader, ShaderId fragmentShader) const;
		UniformType GetUniformLocation(const std::string& uniformName) const;
	public:
		Shader();
		Shader(const std::string& vertexShaderPath, const std::string& fragmentShaderPath);
		Shader(const std::string& vertexShaderPath, const std::string& geometryShaderPath, const std::string& fragmentShaderPath);
		Shader(const Shader&) = delete;
		Shader(Shader&& shader) noexcept;
		Shader& operator=(const Shader&) = delete;
		Shader& operator=(Shader&& shader) noexcept;
		~Shader();

		void Bind() const;
		void Unbind() const;
		BindableId GetNativeHandle() const;
		void Load(const std::string& vertex, const std::string& fragment);
		void Load(const std::string& vertex, const std::string& geometry, const std::string& fragment);
		void LoadFromString(const std::string& vertex, const std::string& fragment);
		void LoadFromString(const std::string& vertex, const std::string& geometry, const std::string& fragment);
		void SetUniformFloat(const std::string& name, float f) const;
		void SetUniformVec2(const std::string& name, const Vector2& vec) const;
		void SetUniformVec3(const std::string& name, const Vector3& vec) const;
		void SetUniformVec4(const std::string& name, const Vector4& vec) const;
		void SetUniformMat4(const std::string& name, const Matrix4x4& matrix) const;
		void SetUniformMat3(const std::string& name, const Matrix3x3& matrix) const;
		void SetUniformInt(const std::string& name, int i) const;
	};
}