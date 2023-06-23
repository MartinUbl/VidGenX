#include "objects.h"
#include "factory.h"
#include "scene.h"
#include "consts.h"
#include <functional>
#include <numbers>
#include <iostream>

#include <spdlog/spdlog.h>

CScene_Object::CScene_Object(NObject_Type type) : CScene_Entity(NEntity_Type::Object), mObject_Type(type) {
	//
}

CScene_Object::~CScene_Object() {
	//
}

void CScene_Object::Set_X(double x) {
	mX = x;
}

void CScene_Object::Set_Y(double y) {
	mY = y;
}

void CScene_Object::Set_Rotate(double angle) {
	mRotate = angle;
}

void CScene_Object::Set_Scale(double scale) {
	mScale = scale;
}

double CScene_Object::Get_X() const {
	return mX.Get_Value(mDefault_Value_Store);
}

double CScene_Object::Get_Y() const {
	return mY.Get_Value(mDefault_Value_Store);
}

double CScene_Object::Get_Rotate() const {
	return mRotate.Get_Value(mDefault_Value_Store) * std::numbers::pi / 180.0;
}

double CScene_Object::Get_Scale() const {
	return mScale.Get_Value(mDefault_Value_Store);
}

void CScene_Object::Set_Position(double x, double y) {
	Set_X(x);
	Set_Y(y);
}

void CScene_Object::Apply_Parameters(const CParams* params) {
	auto pars = params->Get_Parameters();

	try {
		Assign_Helper<double>("x", pars, mX);
		Assign_Helper<double>("y", pars, mY);
		Assign_Helper<double>("rotate", pars, mRotate);
		Assign_Helper<double>("scale", pars, mScale);
	}
	catch (CInvalid_Parameter_Type& ex) {
		spdlog::error("The parameter {} has a different type, than expected", ex.Get_Param_Name());
	}
}

/**************************************************/

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

/**************************************************/

void CRectangle::Apply_Parameters(const CParams* params) {
	CScene_Object::Apply_Parameters(params);

	auto pars = params->Get_Parameters();

	try {
		Assign_Helper<double>("width", pars, mWidth);
		Assign_Helper<double>("height", pars, mHeight);
		Assign_Helper<rgb_t>("fill", pars, mFill_Color);
		Assign_Helper<rgb_t>("stroke", pars, mStroke_Color);
		Assign_Helper<int>("strokewidth", pars, mStroke_Width);
	}
	catch (CInvalid_Parameter_Type& ex) {
		spdlog::error("The parameter {} has a different type, than expected", ex.Get_Param_Name());
	}
}

bool CRectangle::Render(BLContext& context, const CTransform& transform) const {

	CTransform_Guard _(transform, context);
	{
		CTransform tr(Get_X(), Get_Y(), Get_Rotate(), Get_Scale());
		CTransform_Guard _(tr, context);

		context.setCompOp(BL_COMP_OP_SRC_OVER);

		context.setFillStyle(BLRgba32(mFill_Color.Get_Value(mDefault_Value_Store)));
		context.setStrokeStyle(BLRgba32(mStroke_Color.Get_Value(mDefault_Value_Store)));
		context.setStrokeWidth(mStroke_Width.Get_Value(mDefault_Value_Store));

		BLRect rect(0, 0, mWidth.Get_Value(mDefault_Value_Store), mHeight.Get_Value(mDefault_Value_Store));

		context.strokeRect(rect);
		context.fillRect(rect);
	}

	return true;
}

/**************************************************/

void CCircle::Apply_Parameters(const CParams* params) {
	CScene_Object::Apply_Parameters(params);

	auto pars = params->Get_Parameters();

	try {
		Assign_Helper<double>("r", pars, mRadius);
		Assign_Helper<rgb_t>("fill", pars, mFill_Color);
		Assign_Helper<rgb_t>("stroke", pars, mStroke_Color);
		Assign_Helper<double>("strokewidth", pars, mStroke_Width);
	}
	catch (CInvalid_Parameter_Type& ex) {
		spdlog::error("The parameter {} has a different type, than expected", ex.Get_Param_Name());
	}
}

bool CCircle::Render(BLContext& context, const CTransform& transform) const {

	CTransform_Guard _(transform, context);
	{
		CTransform tr(Get_X(), Get_Y(), Get_Rotate(), Get_Scale());
		CTransform_Guard _(tr, context);
		{
			context.setCompOp(BL_COMP_OP_SRC_OVER);

			context.setFillStyle(BLRgba32(mFill_Color.Get_Value(mDefault_Value_Store)));
			context.setStrokeStyle(BLRgba32(mStroke_Color.Get_Value(mDefault_Value_Store)));
			context.setStrokeWidth(mStroke_Width.Get_Value(mDefault_Value_Store));

			BLCircle circle(0, 0, mRadius.Get_Value(mDefault_Value_Store));

			context.strokeCircle(circle);
			context.fillCircle(circle);
		}
	}

	return true;
}

/**************************************************/

void CComposite::Apply_Parameters(const CParams* params) {
	CScene_Object::Apply_Parameters(params);

	CParams p;
	params->Merge_To(&p);
	p.Remove_Parameter("x");
	p.Remove_Parameter("y");

	auto& rp = p.Get_Parameters();
	for (auto& rs : mResolvable_Attributes) {
		if (rp.find(rs) != rp.end())
			mDefault_Value_Store.Add(rs, rp.at(rs));
	}

	for (auto& obj : mObjects)
		obj->Apply_Parameters(&p);
}

void CComposite::Apply_Body(CCommand* command) {

	for (auto sct : command->Get_Subcommands()) {

		for (auto sc : sct->Get_Subcommands()) {

			std::string namecopy(sc->Get_Entity_Name());
			std::transform(namecopy.begin(), namecopy.end(), namecopy.begin(), [](char c) { return std::tolower(c); });

			auto obj = sFactory.Create(namecopy);

			if (!obj)
				continue;

			if (!sc->Get_Subcommands().empty()) {
				obj->Apply_Body(sc);
			}

			if (sc->Get_Params()) {
				obj->Apply_Parameters(sc->Get_Params());
			}

			if (sc->Get_Attributes()) {
				obj->Apply_Attribute_Block(sc->Get_Attributes());
			}

			if (!sc->Get_Object_Reference().empty()) {
				obj->Set_Object_Reference(sc->Get_Object_Reference());
			}

			mObjects.push_back(std::move(obj));
		}
	}

}

bool CComposite::Render(BLContext& context, const CTransform& transform) const {

	CTransform_Guard _(transform, context);
	{
		CTransform tr(Get_X(), Get_Y(), Get_Rotate(), Get_Scale());

		for (auto& obj : mObjects) {

			obj->Get_Value_Store().Merge_With(mDefault_Value_Store);

			auto* object = dynamic_cast<CScene_Object*>(obj.get());
			if (object)
				object->Render(context, tr);
		}
	}

	return true;
}

/**************************************************/

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

template<typename T>
T Animate_Linear(T source, T target, double progress) {
	if constexpr (std::is_integral_v<std::remove_cvref_t<T>> || std::is_floating_point_v<std::remove_cvref_t<T>>) {
		return static_cast<T>(static_cast<const double>(source) + static_cast<const double>(target - source) * progress);
	}

	return source;
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
