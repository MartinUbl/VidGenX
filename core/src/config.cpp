#include "config.h"

#include <spdlog/spdlog.h>

CConfig::CConfig() {
	//
}

bool CConfig::Build(CBlock* configBlock) {
	auto* params = configBlock->Get_Parameters();
	if (!params) {
		spdlog::error("No parameters found for given config block");
		return false;
	}

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

	if (mWidth == 0) {
		spdlog::error("Config width cannot be null");
		return false;
	}
	if (mHeight == 0) {
		spdlog::error("Config height cannot be null");
		return false;
	}
	if (mFPS == 0) {
		spdlog::error("Config framerate (FPS) cannot be null");
		return false;
	}

	return true;
}

bool CConfig::Is_Initialized() const {
	return mInitialized;
}

size_t CConfig::Get_Width() const {
	return mWidth;
}

size_t CConfig::Get_Height() const {
	return mHeight;
}

size_t CConfig::Get_FPS() const {
	return mFPS;
}

uint32_t CConfig::Get_Default_Background() const {
	return mDefault_Background;
}
