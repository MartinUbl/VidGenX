#pragma once

#include "parser_entities.h"
#include "objects.h"

#include <thread>
#include <memory>
#include <map>
#include <vector>
#include <future>

/*
 * Entity factory class
 */
class CFactory {
	private:
		// scene entity factory for built-in objects
		std::map<std::string, std::function<std::unique_ptr<CScene_Entity>()>> mFactories;
		// prototypes builder - stores a template, that gets cloned upon request
		std::map<std::string, std::unique_ptr<CScene_Entity>> mPrototypes;

		// private singleton constructor to avoid multiple instantiation
		CFactory();

	public:
		// static singleton retrieval method
		static CFactory& Instance() {
			static CFactory gInstance;
			return gInstance;
		}

		// registers a factory for a given object type (built-in types)
		template<typename T>
		void Register_Factory(const std::string& name) {
			mFactories[name] = []() -> std::unique_ptr<CScene_Entity> {
				return std::make_unique<T>();
			};
		}

		// registers a prototype template
		void Register_Prototype(const std::string& name, std::unique_ptr<CScene_Entity>&& prototype);
		// creates an entity based on given name
		std::unique_ptr<CScene_Entity> Create(const std::string& name);
};

#define sFactory CFactory::Instance()
