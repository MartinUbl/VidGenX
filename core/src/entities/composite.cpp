#include "composite.h"

#include "../factory.h"
#include "../scene.h"
#include <spdlog/spdlog.h>

void CComposite::Apply_Parameters(const CParams* params) {
	CScene_Object::Apply_Parameters(params);

	CParams p;
	params->Merge_To(&p);
	p.Remove_Parameter("x");
	p.Remove_Parameter("y");

	auto& rp = p.Get_Parameters();
	for (auto& rs : mResolvable_Attributes) {
		if (rp.find(rs) != rp.end())
			mDefault_Value_Store.Add(rs, rp.at(rs));
	}

	for (auto& obj : mObjects)
		obj->Apply_Parameters(&p);
}

void CComposite::Apply_Body(CCommand* command) {

	for (auto sct : command->Get_Subcommands()) {

		for (auto sc : sct->Get_Subcommands()) {

			std::string namecopy(sc->Get_Entity_Name());
			std::transform(namecopy.begin(), namecopy.end(), namecopy.begin(), [](char c) { return std::tolower(c); });

			auto obj = sFactory.Create(namecopy);

			if (!obj)
				continue;

			if (!sc->Get_Subcommands().empty()) {
				obj->Apply_Body(sc);
			}

			if (sc->Get_Params()) {
				obj->Apply_Parameters(sc->Get_Params());
			}

			if (sc->Get_Attributes()) {
				obj->Apply_Attribute_Block(sc->Get_Attributes());
			}

			if (!sc->Get_Object_Reference().empty()) {
				obj->Set_Object_Reference(sc->Get_Object_Reference());
			}

			mObjects.push_back(std::move(obj));
		}
	}

}

bool CComposite::Render(BLContext& context, const CTransform& transform) const {

	CTransform_Guard _(transform, context);
	{
		CTransform tr(Get_X(), Get_Y(), Get_Rotate(), Get_Scale());

		for (auto& obj : mObjects) {

			obj->Get_Value_Store().Merge_With(mDefault_Value_Store);

			auto* object = dynamic_cast<CScene_Object*>(obj.get());
			if (object)
				object->Render(context, tr);
		}
	}

	return true;
}
