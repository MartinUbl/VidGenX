#pragma once

#include "parser_entities.h"

#include <map>

class CConsts {
	private:
		bool mInitialized = false;
		std::map<std::string, TValue_Spec> mConsts;

		CConsts();

	public:
		static CConsts& Instance() {
			static CConsts gInstance;
			return gInstance;
		}

		bool Build(CBlock* block);

		bool Is_Initialized() const {
			return mInitialized;
		}

		const TValue_Spec& Get_Constant(const std::string& key) const {
			return mConsts.at(key);
		}
};

#define sConsts CConsts::Instance()
