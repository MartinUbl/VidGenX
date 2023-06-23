#include "consts.h"
#include <iostream>
#include <algorithm>
#include <string>

#include <spdlog/spdlog.h>

CConsts::CConsts() {
	//
}

bool CConsts::Build(CBlock* block) {
	auto* cmd = block->Get_Content();
	auto subc = cmd->Get_Subcommands();

	for (auto& sc : subc) {
		if (sc->Get_Identifier().empty()) {
			spdlog::error("Constants must have a name!");
			return false;
		}
		
		std::string namecopy(sc->Get_Identifier());
		std::transform(namecopy.begin(), namecopy.end(), namecopy.begin(), [](char c) { return std::tolower(c); });
		
		if (mConsts.find(namecopy) != mConsts.end()) {
			spdlog::error("Multiple definition of constant '{}'!", sc->Get_Identifier());
			return false;
		}
		else if (!sc->Get_Value().has_value()) {
			spdlog::error("Constant '{}' does not have a valid value", sc->Get_Identifier());
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
