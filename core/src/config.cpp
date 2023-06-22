#include "config.h"

CConfig::CConfig() {
	//
}

bool CConfig::Build(CBlock* configBlock) {
	auto* params = configBlock->Get_Parameters();

	auto mp = params->Get_Parameters();

	auto getParam = [&mp]<typename T>(const std::string& key, T&& default_val) -> T {
		auto itr = mp.find(key);
		if (itr == mp.end())
			return default_val;
		return std::get<T>(itr->second.value);
	};

	mWidth = static_cast<size_t>(getParam("width", (double)mWidth));
	mHeight = static_cast<size_t>(getParam("height", (double)mHeight));
	mFPS = static_cast<size_t>(getParam("fps", (double)mFPS));
	mDefault_Background = getParam("defaultbackground", (rgb_t)mDefault_Background);

	mInitialized = true;

	return true;
}
