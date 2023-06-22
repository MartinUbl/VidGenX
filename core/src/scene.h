#pragma once

#include "config.h"
#include "parser_entities.h"
#include "objects.h"

#include <memory>
#include <map>
#include <blend2d.h>

class CScene {
	private:
		size_t mCurrent_Entity = 0;
		size_t mFrame_Counter = 0;
		size_t mObject_Counter = 1;
		size_t mMax_Frame = 0;

		std::vector<std::unique_ptr<CScene_Entity>> mEntities;

		std::map<std::string, size_t> mScene_Objects;

		std::vector<size_t> mWorking_Entites;

	public:
		CScene();

		static std::unique_ptr<CScene> Build_From(CBlock* block);

		const std::unique_ptr<CScene_Entity>& Get_Object_By_Name(const std::string& name);

		void Begin();
		void Update_Scene();
		bool Next_Frame();
		void Render_Frame(BLContext& context);

		size_t Get_Current_Frame() const {
			return mFrame_Counter;
		}
};
