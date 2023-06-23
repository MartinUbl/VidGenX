#pragma once

#include "shared.h"

/*
 * Wait synchronization entity
 */
class CEntity_Wait : public CScene_Entity {
	private:
		// wait duration in milliseconds
		CParam_Wrapper<int> mWait_Duration = 0;

	protected:
		// frame that is the first frame involved in waiting
		std::optional<size_t> mSuspend_Frame;

	public:
		CEntity_Wait() : CScene_Entity(NEntity_Type::Wait) {}

		std::unique_ptr<CScene_Entity> Clone() const override {
			auto ptr = std::make_unique<CEntity_Wait>(*this);
			return ptr;
		}

		void Apply_Parameters(const CParams* params) override;
		NExecution_Result Execute(CScene& scene) override;
};
