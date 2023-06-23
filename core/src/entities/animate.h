#pragma once

#include "shared.h"

/*
 * Animation entity
 */
class CEntity_Animate : public CScene_Entity {
	private:
		// animation duration
		CParam_Wrapper<int> mDuration = 0;

		// animated parameter structure
		struct TAnimate_Param {
			std::string paramName;					// name of the parameter
			std::optional<TValue_Spec> initial;		// initial value of the parameter
			TValue_Spec target;						// target value of the parameter
		};

		// vector of all animater parameters
		std::vector<TAnimate_Param> mAnimation_Params;

	protected:
		// start frame of animation
		std::optional<size_t> mStart_Frame;

	public:
		CEntity_Animate() : CScene_Entity(NEntity_Type::Animate) {}

		std::unique_ptr<CScene_Entity> Clone() const override {
			auto ptr = std::make_unique<CEntity_Animate>(*this);
			return ptr;
		}

		void Apply_Parameters(const CParams* params) override;
		NExecution_Result Execute(CScene& scene) override;
};
