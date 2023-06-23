#pragma once

#include "shared.h"

/*
 * Rectangle entity
 */
class CRectangle : public CBasic_Clonable_Scene_Object<CRectangle> {
	private:
		// rectangle width
		CParam_Wrapper<double> mWidth = 0;
		// rectangle height
		CParam_Wrapper<double> mHeight = 0;
		// rectangle fill color
		CParam_Wrapper<rgb_t> mFill_Color = 0;
		// stroke color
		CParam_Wrapper<rgb_t> mStroke_Color = 0;
		// stroke width
		CParam_Wrapper<double> mStroke_Width = 0;

	public:
		CRectangle() : CBasic_Clonable_Scene_Object(NObject_Type::Rectangle) {}

		void Apply_Parameters(const CParams* params) override;
		bool Render(BLContext& context, const CTransform& transform) const override;
};
