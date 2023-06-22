#include "scene.h"
#include "factory.h"

#include <stdexcept>
#include <iostream>

CScene::CScene() {
	//
}

std::unique_ptr<CScene> CScene::Build_From(CBlock* block) {

	std::unique_ptr<CScene> ret = std::make_unique<CScene>();

	for (auto& sc : block->Get_Content()->Get_Subcommands()) {
		auto name = sc->Get_Entity_Name();

		auto obj = sFactory.Create(name);
		if (sc->Get_Params()) {
			obj->Apply_Parameters(sc->Get_Params());
		}
		if (sc->Get_Attributes()) {
			obj->Apply_Attribute_Block(sc->Get_Attributes());
		}

		std::string objId = sc->Get_Identifier();
		if (objId.empty()) {
			objId = "object" + std::to_string(ret->mObject_Counter++);
		}

		ret->mEntities.push_back(std::move(obj));
		ret->mScene_Objects[objId] = ret->mEntities.size() - 1;
	}

	auto* pars = block->Get_Parameters();
	if (pars) {
		auto& mp = pars->Get_Parameters();

		auto itr = mp.find("duration");
		if (itr != mp.end()) {
			ret->mMax_Frame = (std::get<int>(itr->second.value) / 1000) * sConfig.Get_FPS();
		}
	}

	return ret;
}

const std::unique_ptr<CScene_Entity>& CScene::Get_Object_By_Name(const std::string& name) {

	auto itr = mScene_Objects.find(name);

	if (itr == mScene_Objects.end())
		throw std::invalid_argument{ "No object with name" };

	return mEntities[itr->second];
}

void CScene::Begin() {
	mCurrent_Entity = 0;
	Update_Scene();
}

void CScene::Update_Scene() {
	for (; mCurrent_Entity < mEntities.size(); mCurrent_Entity++) {

		if (mEntities[mCurrent_Entity]->Execute(*this) == NExecution_Result::Suspend) {
			break;
		}

		if (mEntities[mCurrent_Entity]->Get_Type() == NEntity_Type::Object) {
			mWorking_Entites.push_back(mCurrent_Entity);
		}
	}
}

bool CScene::Next_Frame() {
	mFrame_Counter++;

	if (mFrame_Counter >= mMax_Frame) {
		return false;
	}

	Update_Scene();

	return true;
}

void CScene::Render_Frame(BLContext& context) {

	for (size_t idx : mWorking_Entites) {
		auto* obj = dynamic_cast<CScene_Object*>(mEntities[idx].get());
		if (obj) {
			obj->Render(context, CTransform::Identity());
		}
	}

}
