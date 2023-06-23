#include "consts.h"
#include <iostream>
#include <algorithm>
#include <string>

CConsts::CConsts() {
	//
}

bool CConsts::Build(CBlock* block) {
	auto* cmd = block->Get_Content();
	auto subc = cmd->Get_Subcommands();

	for (auto& sc : subc) {
		if (sc->Get_Identifier().empty()) {
			std::cerr << "Constants must have name!" << std::endl;
			return false;
		}
		
		std::string namecopy(sc->Get_Identifier());
		std::transform(namecopy.begin(), namecopy.end(), namecopy.begin(), std::tolower);
		
		if (mConsts.find(namecopy) != mConsts.end()) {
			std::cerr << "Multiple definition of constant '" << sc->Get_Identifier() << "'" << std::endl;
			return false;
		}
		else if (!sc->Get_Value().has_value()) {
			std::cerr << "Constant '" << sc->Get_Identifier() << "' does not have a valid value" << std::endl;
			return false;
		}

		mConsts[namecopy] = sc->Get_Value().value();
	}

	return true;
}

bool CConsts::Is_Initialized() const {
	return mInitialized;
}

const TValue_Spec& CConsts::Get_Constant(const std::string& key) const {
	return mConsts.at(key);
}
