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

#include "CompoundShape.h"
#include "Bullet3Utils.h"

namespace MxEngine
{
    void CompoundShape::AddShapeImpl(btCollisionShape* ptr, size_t userIndex, const TransformComponent& relativeTransform)
    {
        btTransform tr;
        ToBulletTransform(tr, relativeTransform);
        ((btCompoundShape*)this->collider)->addChildShape(tr, ptr);
    }

    CompoundShape::CompoundShape()
    {
        this->CreateShape<btCompoundShape>();
    }

    CompoundShape::CompoundShape(CompoundShape&& other) noexcept
    {
        this->collider = other.collider;
        other.collider = nullptr;
    }

    CompoundShape& CompoundShape::operator=(CompoundShape&& other) noexcept
    {
        this->DestroyShape();
        this->collider = other.collider;
        return *this;
    }

    CompoundShape::~CompoundShape()
    {
        this->DestroyShape();
    }

    size_t CompoundShape::GetShapeCount() const
    {
        return (size_t)((btCompoundShape*)this->collider)->getNumChildShapes();
    }

    void CompoundShape::RemoveShapeByIndex(size_t index)
    {
        MX_ASSERT(index < this->GetShapeCount());
        ((btCompoundShape*)this->collider)->removeChildShapeByIndex((int)index);
    }

    TransformComponent CompoundShape::GetShapeTransformByIndex(size_t index) const
    {
        MX_ASSERT(index < this->GetShapeCount());

        TransformComponent result;
        auto& tr = ((btCompoundShape*)this->collider)->getChildTransform((int)index);

        auto parentScale = FromBulletVector3(this->collider->getLocalScaling());

        FromBulletTransform(result, tr);
        result.SetPosition(result.GetPosition() / parentScale);
        return result;
    }

    void CompoundShape::SetShapeTransformByIndex(size_t index, const TransformComponent& relativeTransform)
    {
        MX_ASSERT(index < this->GetShapeCount());

        btTransform tr;
        ToBulletTransform(tr, relativeTransform);

        auto parentScale = this->collider->getLocalScaling();
        tr.setOrigin(tr.getOrigin() * parentScale);

        ((btCompoundShape*)this->collider)->updateChildTransform((int)index, tr);
        ((btCompoundShape*)this->collider)->getChildShape((int)index)->setLocalScaling(parentScale);
    }

    void CompoundShape::ClearShapes()
    {
        if (this->GetShapeCount() == 0) return;
        for (int index = (int)this->GetShapeCount() - 1; index >= 0; index--)
        {
            this->RemoveShapeByIndex(index);
        }
    }
}