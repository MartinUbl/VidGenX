#include <iostream>
#include <stdexcept>
#include <fstream>
#include <list>
#include <algorithm>
#include <memory>
#include <format>
#include <filesystem>
#include "parser_entities.h"
#include "vdlang_lex.h"
#include "vdlang_parser.h"

#include "config.h"
#include "consts.h"
#include "prototypes.h"
#include "scene.h"

#include "controller.h"

#include <blend2d.h>
#include <spdlog/spdlog.h>
#include <SimpleIni.h>

// blocks defined by the parser file
extern std::vector<CBlock*> _Blocks;

/*
 * Helper RAII class for wrapping the bison parser
 */
class CAnalyzer_State {
	private:
		// bison buffer state
		YY_BUFFER_STATE m_state;

	public:
		// constructs the analyzer helper using input string to be parsed
		CAnalyzer_State(const std::string& input) {
			if (!(m_state = yy_scan_bytes(input.c_str(), static_cast<int>(input.length())))) {
				throw std::invalid_argument{ "Cannot parse the input string" };
			}
		}

		virtual ~CAnalyzer_State() {
			yy_delete_buffer(m_state);
		}

		// parses the input, throws an exception on error
		void Parse() {
			auto result = yyparse();

			if (result != 0) {
				throw std::runtime_error{ "The input cannot be parsed due to syntax errors" };
			}
		}
};

CController::CController() {
	//
}

int CController::Initialize(const std::vector<std::string>& argv) {

	// load cli parameters

	mSource_File = argv[1];
	mOutput_Directory = argv[2];

	// load config file

	CSimpleIniA appConfig;
	SI_Error rc = appConfig.LoadFile("vidgenx.ini");
	if (rc != SI_OK) {
		spdlog::warn("Cannot find vidgenx.ini file, using defaults");
	}

	mFFMPEG_Binary = appConfig.GetValue("general", "ffmpeg_binary", "ffmpeg");
	if (mFFMPEG_Binary.empty())
		mFFMPEG_Binary = "ffmpeg";

	return 0;
}

bool CController::Parse_Input_Files() {
	spdlog::info("Parsing the input file");

	try {
		std::ifstream ifs(mSource_File);

		std::string str((std::istreambuf_iterator<char>(ifs)),
			std::istreambuf_iterator<char>());

		CAnalyzer_State state(str);

		state.Parse();
	}
	catch (std::exception& ex) {
		spdlog::error("Cannot parse the input file, error: {}", ex.what());
		return false;
	}

	return true;
}

bool CController::Parse_Blocks() {

	// sort by block index (primary sort)
	std::sort(_Blocks.begin(), _Blocks.end(), [](const CBlock* a, const CBlock* b) {
		return static_cast<int>(a->Get_Block_Index()) < static_cast<int>(b->Get_Block_Index());
	});

	// sort by type to preserve correct loading order: config, constants, prototypes, scenes
	std::stable_sort(_Blocks.begin(), _Blocks.end(), [](const CBlock* a, const CBlock* b) {
		return static_cast<int>(a->Get_Type()) < static_cast<int>(b->Get_Type());
	});

	for (auto& bl : _Blocks) {

		switch (bl->Get_Type()) {
			case NBlock_Type::Config:
			{
				if (sConfig.Is_Initialized()) {
					spdlog::error("Multiple config blocks found, cannot proceed");
					return false;
				}
				sConfig.Build(bl);
				break;
			}
			case NBlock_Type::Consts:
			{
				if (sConsts.Is_Initialized()) {
					spdlog::error("Multiple constants blocks found, cannot proceed");
					return false;
				}
				sConsts.Build(bl);
				break;
			}
			case NBlock_Type::Prototypes:
			{
				if (sPrototypes.Is_Initialized()) {
					spdlog::error("Multiple prototypes blocks found, cannot proceed");
					return false;
				}
				sPrototypes.Build(bl);
				break;
			}
			case NBlock_Type::Scene:
			{
				auto sc = CScene::Build_From(bl);
				if (sc) {
					mScenes.push_back(std::move(sc));
				}
				else {
					spdlog::error("Cannot build all scenes, cannot proceed");
					return false;
				}
				break;
			}
		}
	}

	return true;
}

bool CController::Render_Scenes() {
	size_t frameStart = 0;

	for (size_t scIdx = 0; scIdx < mScenes.size(); scIdx++)
	{
		mScenes[scIdx]->Begin();
		do {
			spdlog::info("Rendering scene {}, frame {}", scIdx, mScenes[scIdx]->Get_Current_Frame());

			BLImage img(static_cast<int>(sConfig.Get_Width()), static_cast<int>(sConfig.Get_Height()), BL_FORMAT_PRGB32);
			BLContext ctx(img);

			ctx.setCompOp(BL_COMP_OP_SRC_COPY);
			ctx.fillAll();

			mScenes[scIdx]->Render_Frame(ctx);

			ctx.end();

			std::string filename = std::format("frame_{:06}.png", (frameStart + mScenes[scIdx]->Get_Current_Frame()));

			img.writeToFile((mOutput_Directory / filename).string().c_str());
		} while (mScenes[scIdx]->Next_Frame());

		frameStart += mScenes[scIdx]->Get_Current_Frame();
	}

	return true;
}

bool CController::Stitch_Video() {
	spdlog::info("Stitching frames to a video...");

	std::string command = mFFMPEG_Binary.string() + " -framerate " + std::to_string(sConfig.Get_FPS()) + " -pattern_type sequence -i \"" + mOutput_Directory.string() + "\\frame_%06d.png\" -y -c:v copy -pix_fmt yuv420p " + mOutput_Directory.string() + "\\out.avi >NUL 2>&1";

	std::system(command.c_str());

	return true;
}

int CController::Run() {

	if (!Parse_Input_Files()) {
		return 1;
	}

	if (!Parse_Blocks()) {
		return 2;
	}

	if (!Render_Scenes()) {
		return 3;
	}

	if (!Stitch_Video()) {
		return 4;
	}

	spdlog::info("Completed");

	return 0;
}
