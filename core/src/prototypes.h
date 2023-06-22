#pragma once

#include "parser_entities.h"

class CPrototypes {
	private:
		bool mInitialized = false;

		CPrototypes();

	public:
		static CPrototypes& Instance() {
			static CPrototypes gPrototypes;
			return gPrototypes;
		}

		bool Build(CBlock* block);

		bool Is_Initialized() const {
			return mInitialized;
		}
};

#define sPrototypes CPrototypes::Instance()
