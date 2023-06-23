#pragma once

#include "parser_entities.h"

/*
 * Global config for generated videos
 */
class CConfig
{
	private:
		// canvas width
		size_t mWidth = 100;
		// canvas height
		size_t mHeight = 100;
		// framerate of the video
		size_t mFPS = 30;
		// default background to use
		uint32_t mDefault_Background = 0;

		// is the config initialized?
		bool mInitialized = false;

		// private constructor to avoid multiple instantiation
		CConfig();

	public:
		// static singleton retrieving method
		static CConfig& Instance() {
			static CConfig gInstance;
			return gInstance;
		}

		// builds the config from given config block
		bool Build(CBlock* configBlock);

		// is the config initialized properly?
		bool Is_Initialized() const;
		// retrieves cavnas width
		size_t Get_Width() const;
		// retrieves canvas height
		size_t Get_Height() const;
		// retrieves video framerate
		size_t Get_FPS() const;
		// retrieves default video background
		uint32_t Get_Default_Background() const;
};

#define sConfig CConfig::Instance()
