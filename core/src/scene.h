#pragma once

#include "config.h"
#include "parser_entities.h"
#include "objects.h"

#include <memory>
#include <map>
#include <blend2d.h>

/*
 * Scene instance class
 */
class CScene {
	private:
		// current working entity index
		size_t mCurrent_Entity = 0;
		// current frame being rendered
		size_t mFrame_Counter = 0;
		// a counter for object labeling, if they don't have their own identifier assigned
		size_t mObject_Counter = 1;
		// maximum frame to which the scene should be rendered
		size_t mMax_Frame = 0;

		// scene entities (loaded and instantiated from the beginning)
		std::vector<std::unique_ptr<CScene_Entity>> mEntities;
		// scene objects reference - references the index in mEntities
		std::map<std::string, size_t> mScene_Objects;
		// entities currently present on the screen
		std::vector<size_t> mWorking_Entites;

	public:
		CScene();

		// builds an instance of scene from given scene block
		static std::unique_ptr<CScene> Build_From(CBlock* block);

		// retrieves an object pointer by its name
		const std::unique_ptr<CScene_Entity>& Get_Object_By_Name(const std::string& name);

		// begins the scene rendering
		void Begin();
		// updates scene according to current frame
		void Update_Scene();
		// moves to next frame, updates the scene accordingly
		bool Next_Frame();
		// renders the current frame to given context
		void Render_Frame(BLContext& context);
		// retrieves current frame index
		size_t Get_Current_Frame() const;
};
