#pragma once

#include "shared.h"

/*
 * Composite entity - an entity containing one or more other entities
 */
class CComposite : public CBasic_Clonable_Scene_Object<CComposite> {
	private:
		// list of object instances, that are encapsulated within this composite entity
		std::list<std::unique_ptr<CScene_Entity>> mObjects;

	public:
		CComposite() : CBasic_Clonable_Scene_Object(NObject_Type::Composite) {}

		// it is important to perform deep copy on copy-semantic invocation
		CComposite(const CComposite& other) : CBasic_Clonable_Scene_Object(other) {
			for (auto& obj : other.mObjects)
				mObjects.push_back(obj->Clone());
		}

		void Apply_Body(CCommand* command) override;
		void Apply_Parameters(const CParams* params) override;
		bool Render(BLContext& context, const CTransform& transform) const override;
};
