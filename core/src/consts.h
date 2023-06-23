#pragma once

#include "parser_entities.h"

#include <map>

/*
 * Global constants store
 */
class CConsts {
	private:
		// is the const store initialized?
		bool mInitialized = false;
		// stored constants
		std::map<std::string, TValue_Spec> mConsts;

		// private constructor to avoid multiple instantiation
		CConsts();

	public:
		// static singletor retrieval method
		static CConsts& Instance() {
			static CConsts gInstance;
			return gInstance;
		}

		// builds the constants store from given consts block
		bool Build(CBlock* block);

		// is the const store initialized?
		bool Is_Initialized() const;
		// retrieves constant from the store, if there is any
		const TValue_Spec& Get_Constant(const std::string& key) const;
};

#define sConsts CConsts::Instance()
