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

#include <blend2d.h>

extern std::vector<CBlock*> _Blocks;

class CAnalyzer_State {
	public:
		CAnalyzer_State(const std::string& input) {
			if (!(m_state = yy_scan_bytes(input.c_str(), static_cast<int>(input.length())))) {
				throw std::invalid_argument{ "Cannot parse the input string" };
			}
		}

		virtual ~CAnalyzer_State() {
			yy_delete_buffer(m_state);
		}

		void Parse() {
			auto result = yyparse();

			if (result != 0) {
				throw std::runtime_error{ "The input cannot be parsed due to syntax errors" };
			}
		}

	private:
		YY_BUFFER_STATE m_state;
};

int main(int argc, char** argv)
{
	try {
		std::ifstream ifs(argv[1]);

		std::string str((std::istreambuf_iterator<char>(ifs)),
			std::istreambuf_iterator<char>());

		CAnalyzer_State state(str);

		state.Parse();
	}
	catch (std::exception& ex) {
		std::cerr << "Cannot parse: " << ex.what() << std::endl;
		return 4;
	}

	std::vector<std::unique_ptr<CScene>> scenes;

	// sort by type to preserve correct loading order: config, constants, prototypes, scenes
	std::sort(_Blocks.begin(), _Blocks.end(), [](const CBlock* a, const CBlock* b) {
		return static_cast<int>(a->Get_Type()) < static_cast<int>(b->Get_Type());
	});

	for (auto& bl : _Blocks) {

		switch (bl->Get_Type()) {
			case NBlock_Type::Config:
			{
				if (sConfig.Is_Initialized()) {
					std::cerr << "Multiple configs found, cannot proceed" << std::endl;
					return 1;
				}
				sConfig.Build(bl);
				break;
			}
			case NBlock_Type::Consts:
			{
				if (sConsts.Is_Initialized()) {
					std::cerr << "Multiple constants block found, cannot proceed" << std::endl;
					return 1;
				}
				sConsts.Build(bl);
				break;
			}
			case NBlock_Type::Prototypes:
			{
				if (sPrototypes.Is_Initialized()) {
					std::cerr << "Multiple prototypes block found, cannot proceed" << std::endl;
					return 1;
				}
				sPrototypes.Build(bl);
				break;
			}
			case NBlock_Type::Scene:
			{
				auto sc = CScene::Build_From(bl);
				if (sc) {
					scenes.push_back(std::move(sc));
				}
				else {
					std::cerr << "Cannot build one or more scenes, cannot proceed" << std::endl;
					return 2;
				}
				break;
			}
		}
	}

	std::filesystem::path baseOutDir = "C:\\Data\\Dev\\REPO\\VidGenX\\testout\\";
	std::filesystem::path ffmpegPath = "C:\\Data\\Dev\\REPO\\_dep\\ffmpeg\\bin\\ffmpeg.exe";

	size_t frameStart = 0;

	for (size_t scIdx = 0; scIdx < scenes.size(); scIdx++)
	{
		scenes[scIdx]->Begin();
		do {

			std::cout << "Scene " << scIdx <<" frame " << scenes[scIdx]->Get_Current_Frame() << std::endl;

			BLImage img(static_cast<int>(sConfig.Get_Width()), static_cast<int>(sConfig.Get_Height()), BL_FORMAT_PRGB32);
			BLContext ctx(img);

			ctx.setCompOp(BL_COMP_OP_SRC_COPY);
			ctx.fillAll();

			scenes[scIdx]->Render_Frame(ctx);

			ctx.end();

			std::string filename = std::format("frame_{:06}.png", (frameStart + scenes[scIdx]->Get_Current_Frame()));

			img.writeToFile((baseOutDir / filename).string().c_str());
		} while (scenes[scIdx]->Next_Frame());

		frameStart += scenes[scIdx]->Get_Current_Frame();
	}

	std::string command = ffmpegPath.string() + " -framerate " + std::to_string(sConfig.Get_FPS()) + " -pattern_type sequence -i \"" + baseOutDir.string() + "\\frame_%06d.png\" -y -c:v copy -pix_fmt yuv420p " + baseOutDir.string() + "\\out.avi";

	std::system(command.c_str());

	return 0;
}
