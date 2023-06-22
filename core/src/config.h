#pragma once

#include "parser_entities.h"

class CConfig
{
	private:
		size_t mWidth = 100;
		size_t mHeight = 100;
		size_t mFPS = 30;
		uint32_t mDefault_Background = 0;

		bool mInitialized = false;

		CConfig();

	public:
		static CConfig& Instance() {
			static CConfig gInstance;
			return gInstance;
		}

		bool Build(CBlock* configBlock);

		bool Is_Initialized() const {
			return mInitialized;
		}

		size_t Get_Width() const {
			return mWidth;
		}

		size_t Get_Height() const {
			return mHeight;
		}

		size_t Get_FPS() const {
			return mFPS;
		}

		uint32_t Get_Default_Background() const {
			return mDefault_Background;
		}
};

#define sConfig CConfig::Instance()
