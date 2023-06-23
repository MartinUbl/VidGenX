#include "rectangle.h"

#include "../scene.h"
#include <spdlog/spdlog.h>

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
