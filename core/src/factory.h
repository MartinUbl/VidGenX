#pragma once

#include "parser_entities.h"
#include "objects.h"

#include <thread>
#include <memory>
#include <map>
#include <vector>
#include <future>

class CFactory {
	private:
		CFactory();

		std::map<std::string, std::function<std::unique_ptr<CScene_Entity>()>> mFactories;
		std::map<std::string, std::unique_ptr<CScene_Entity>> mPrototypes;

	public:
		static CFactory& Instance() {
			static CFactory gInstance;
			return gInstance;
		}

		template<typename T>
		void Register_Factory(const std::string& name) {
			mFactories[name] = []() -> std::unique_ptr<CScene_Entity> {
				return std::make_unique<T>();
			};
		}

		void Register_Prototype(const std::string& name, std::unique_ptr<CScene_Entity>&& prototype) {
			mPrototypes[name] = std::move(prototype);
		}

		std::unique_ptr<CScene_Entity> Create(const std::string& name);
};

#define sFactory CFactory::Instance()
