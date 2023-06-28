#pragma once

#include <string>
#include <vector>
#include <filesystem>

#include "scene.h"

/*
 * Application main controller - controls the flow of video rendering
 */
class CController {
	private:
		// source .vdef file
		std::filesystem::path mSource_File;
		// output directory - for images and for video as well
		std::filesystem::path mOutput_Directory;

		// path to FFMPEG binary (ffmpeg.exe on Windows or ffmpeg on Linux/macOS)
		std::filesystem::path mFFMPEG_Binary;

		// vector of scenes to be rendered
		std::vector<std::unique_ptr<CScene>> mScenes;

		// total number of frames to be stitched
		size_t mTotal_Frames = 0;

	protected:
		// parses input files into a internal representation
		bool Parse_Input_Files();
		// parses blocks from internal representation
		bool Parse_Blocks();
		// renders scenes (all frames) based on parsed blocks
		bool Render_Scenes();
		// stitches video together using rendered images
		bool Stitch_Video();

	public:
		CController();

		// initialize the controller based on user inputs
		int Initialize(const std::vector<std::string>& argv);
		// run the video generation
		int Run();
};
