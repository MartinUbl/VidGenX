#include "prototypes.h"
#include "factory.h"

#include <iostream>
#include <spdlog/spdlog.h>

CPrototypes::CPrototypes() {
	//
}

bool CPrototypes::Build(CBlock* block) {

	for (auto& sc : block->Get_Content()->Get_Subcommands()) {
		auto name = sc->Get_Entity_Name();

		if (sc->Get_Identifier().empty()) {
			spdlog::error("Prototype must have an identifier");
			return false;
		}

		std::string namecopy(sc->Get_Identifier());
		std::transform(namecopy.begin(), namecopy.end(), namecopy.begin(), [](char c) { return std::tolower(c); });

		auto obj = sFactory.Create(name);

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

		sFactory.Register_Prototype(namecopy, std::move(obj));
	}

	return true;
}

bool CPrototypes::Is_Initialized() const {
	return mInitialized;
}
