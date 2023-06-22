#include "prototypes.h"
#include "factory.h"

#include <iostream>

CPrototypes::CPrototypes() {
	//
}

bool CPrototypes::Build(CBlock* block) {

	for (auto& sc : block->Get_Content()->Get_Subcommands()) {
		auto name = sc->Get_Entity_Name();

		if (sc->Get_Identifier().empty()) {
			std::cerr << "Prototype must have an identifier" << std::endl;
			return false;
		}

		std::string namecopy(sc->Get_Identifier());
		std::transform(namecopy.begin(), namecopy.end(), namecopy.begin(), std::tolower);

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

		std::cout << "Name = " << name << std::endl;

		sFactory.Register_Prototype(namecopy, std::move(obj));
	}

	return true;
}
