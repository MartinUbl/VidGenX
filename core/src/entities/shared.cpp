#include "shared.h"

#include "../scene.h"
#include <numbers>

#include <spdlog/spdlog.h>

CScene_Object::CScene_Object(NObject_Type type) : CScene_Entity(NEntity_Type::Object), mObject_Type(type) {
	//
}

CScene_Object::~CScene_Object() {
	//
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
