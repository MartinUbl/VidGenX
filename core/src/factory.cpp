#include "factory.h"

#include <algorithm>
#include <string>

#include <spdlog/spdlog.h>

CFactory::CFactory() {
	Register_Factory<CRectangle>("rectangle");
	Register_Factory<CCircle>("circle");
	Register_Factory<CComposite>("composite");
	Register_Factory<CEntity_Wait>("wait");
	Register_Factory<CEntity_Animate>("animate");
}

std::unique_ptr<CScene_Entity> CFactory::Create(const std::string& name) {

	std::string namecopy(name);
	std::transform(namecopy.begin(), namecopy.end(), namecopy.begin(), [](char c) { return std::tolower(c); });

	if (mFactories.find(namecopy) == mFactories.end()) {

		if (mPrototypes.find(namecopy) == mPrototypes.end()) {
			spdlog::warn("Cannot find a factory for object with name '{}'", namecopy);
			return nullptr;
		}

		return mPrototypes[namecopy]->Clone();
	}

	return mFactories[namecopy]();
}

void CFactory::Register_Prototype(const std::string& name, std::unique_ptr<CScene_Entity>&& prototype) {
	mPrototypes[name] = std::move(prototype);
}
