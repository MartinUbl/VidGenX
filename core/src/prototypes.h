#pragma once

#include "parser_entities.h"

/*
 * Prototypes store class
 */
class CPrototypes {
	private:
		// is the prototypes store initialized?
		bool mInitialized = false;

		// private singleton constructor to avoid multiple instantiation
		CPrototypes();

	public:
		// static singleton retrieval method
		static CPrototypes& Instance() {
			static CPrototypes gPrototypes;
			return gPrototypes;
		}

		// build prototypes store from given prototypes block
		bool Build(CBlock* block);
		// is the store properly initialized?
		bool Is_Initialized() const;
};

#define sPrototypes CPrototypes::Instance()
