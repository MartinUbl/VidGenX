#include "wait.h"

#include "../scene.h"
#include <spdlog/spdlog.h>

void CEntity_Wait::Apply_Parameters(const CParams* params) {
	auto pars = params->Get_Parameters();

	try {
		Assign_Helper<int>("duration", pars, mWait_Duration);
	}
	catch (CInvalid_Parameter_Type& ex) {
		spdlog::error("The parameter {} has a different type, than expected", ex.Get_Param_Name());
	}
}

NExecution_Result CEntity_Wait::Execute(CScene& scene) {

	if (!mSuspend_Frame.has_value()) {
		mSuspend_Frame = scene.Get_Current_Frame();
	}

	auto frameDiff = (scene.Get_Current_Frame() - mSuspend_Frame.value());

	if (frameDiff > (mWait_Duration.Get_Value(mDefault_Value_Store) / 1000) * sConfig.Get_FPS())
		return NExecution_Result::Pass;

	return NExecution_Result::Suspend;
}
