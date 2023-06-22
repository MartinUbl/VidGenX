#include "factory.h"

#include <algorithm>
#include <string>

CFactory::CFactory() {
	Register_Factory<CRectangle>("rectangle");
	Register_Factory<CCircle>("circle");
	Register_Factory<CComposite>("composite");
	Register_Factory<CEntity_Wait>("wait");
}

std::unique_ptr<CScene_Entity> CFactory::Create(const std::string& name) {

	std::string namecopy(name);
	std::transform(namecopy.begin(), namecopy.end(), namecopy.begin(), std::tolower);

	if (mFactories.find(namecopy) == mFactories.end()) {

		if (mPrototypes.find(namecopy) == mPrototypes.end())
			return nullptr;

		return mPrototypes[namecopy]->Clone();
	}

	return mFactories[namecopy]();
}
