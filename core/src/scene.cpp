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
		//sc->Get_Attributes()

		std::cout << "Name = " << name << std::endl;

		std::string objId = sc->Get_Identifier();
		if (objId.empty()) {
			objId = "object" + std::to_string(ret->mObject_Counter++);
		}

		ret->mEntities.push_back(std::move(obj));
		ret->mScene_Objects[objId] = ret->mEntities.size() - 1;
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

	for (size_t i = 0; i < mEntities.size(); i++) {
		if (mEntities[i]->Get_Type() == NEntity_Type::Object) {
			mWorking_Entites.push_back(i);
		}
	}
}

void CScene::Next_Frame() {
	mFrame_Counter++;
}

void CScene::Render_Frame(BLContext& context) {

	

	for (size_t idx : mWorking_Entites) {
		dynamic_cast<CScene_Object*>(mEntities[idx].get())->Render(context, CTransform::Identity());
	}

	

}

void CScene::Wait(size_t frames) {
	//
}
