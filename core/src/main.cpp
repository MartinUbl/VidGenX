#include <iostream>
#include <stdexcept>
#include <fstream>
#include <list>
#include <algorithm>
#include <memory>
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

void draw_loop() {

	/*
	BLImage img(480, 480, BL_FORMAT_PRGB32);

	// Attach a rendering context into `img`.
	BLContext ctx(img);

	// Clear the image.
	ctx.setCompOp(BL_COMP_OP_SRC_COPY);
	ctx.fillAll();

	// Fill some path.
	BLPath path;
	path.moveTo(26, 31);
	path.cubicTo(642, 132, 587, -136, 25, 464);
	path.cubicTo(882, 404, 144, 267, 27, 31);

	ctx.setCompOp(BL_COMP_OP_SRC_OVER);
	ctx.setFillStyle(BLRgba32(0xFFFFFFFF));
	ctx.fillPath(path);

	// Detach the rendering context from `img`.
	ctx.end();

	// Let's use some built-in codecs provided by Blend2D.
	img.writeToFile("C:\\Data\\Dev\\REPO\\VidGenX\\testout\\bl_sample_1.png");
	*/

}

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


	{
		BLImage img(static_cast<int>(sConfig.Get_Width()), static_cast<int>(sConfig.Get_Height()), BL_FORMAT_PRGB32);

		BLContext ctx(img);

		ctx.setCompOp(BL_COMP_OP_SRC_COPY);
		ctx.fillAll();

		scenes[0]->Begin();
		scenes[0]->Render_Frame(ctx);

		ctx.end();

		img.writeToFile("C:\\Data\\Dev\\REPO\\VidGenX\\testout\\bl_sample_1.png");
	}

	draw_loop();

	return 0;
}
