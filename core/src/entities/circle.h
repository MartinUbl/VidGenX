#pragma once

#include "shared.h"

/*
 * Circle entity
 */
class CCircle : public CBasic_Clonable_Scene_Object<CCircle> {
	private:
		// circle radius
		CParam_Wrapper<double> mRadius = 0;
		// fill color
		CParam_Wrapper<rgb_t> mFill_Color = 0;
		// stroke color
		CParam_Wrapper<rgb_t> mStroke_Color = 0;
		// stroke width
		CParam_Wrapper<double> mStroke_Width = 0;

	public:
		CCircle() : CBasic_Clonable_Scene_Object(NObject_Type::Circle) {}

		void Apply_Parameters(const CParams* params) override;
		bool Render(BLContext& context, const CTransform& transform) const override;
};
