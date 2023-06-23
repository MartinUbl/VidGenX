#include "animate.h"

#include "../scene.h"
#include <spdlog/spdlog.h>

#include "builtin_animatons.h"

void CEntity_Animate::Apply_Parameters(const CParams* params) {
	auto pars = params->Get_Parameters();

	try {
		Assign_Helper<int>("duration", pars, mDuration);
	}
	catch (CInvalid_Parameter_Type& ex) {
		spdlog::error("The parameter {} has a different type, than expected", ex.Get_Param_Name());
	}

	for (auto& p : pars) {
		if (p.first == "duration")
			continue;

		mAnimation_Params.push_back({
			p.first,
			std::nullopt,
			p.second
			});
	}
}

NExecution_Result CEntity_Animate::Execute(CScene& scene) {

	auto& obj = scene.Get_Object_By_Name(mObject_Reference.value());
	if (obj) {
		for (auto& ap : mAnimation_Params) {
			auto* ref = obj->Get_Param_Ref(ap.paramName);
			if (!ref) {
				continue;
			}

			if (!ap.initial.has_value()) {
				ap.initial = ref->Get_Value();
				mStart_Frame = scene.Get_Current_Frame();
			}

			auto frameDiff = (scene.Get_Current_Frame() - mStart_Frame.value());

			auto numFrames = (mDuration.Get_Value(mDefault_Value_Store) / 1000) * sConfig.Get_FPS();

			double progress = 0;
			if (numFrames < frameDiff)
				progress = 1;
			else if (frameDiff < 0)
				progress = 0;
			else
				progress = static_cast<double>(frameDiff) / static_cast<double>(numFrames);

			std::visit([this, ap, ref, progress](auto&& sval) {
				std::visit([this, ap, ref, sval, progress](auto&& tval) {

					if constexpr (std::is_same_v<std::remove_cvref_t<decltype(sval)>, std::remove_cvref_t<decltype(tval)>>) {

						using TVal = std::remove_cvref_t<decltype(sval)>;

						auto res = Animate_Linear(sval, tval, progress);

						ref->Set_Value(TValue_Spec{ ap.initial.value().type, res });
					}

					}, ap.target.value);
				}, ap.initial.value().value);
		}
	}

	return NExecution_Result::Pass;
}
