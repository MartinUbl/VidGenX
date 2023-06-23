#pragma once

#include <memory>
#include <optional>
#include <blend2d.h>
#include <stdexcept>
#include <set>
#include "consts.h"
#include "parser_entities.h"

class CScene;

enum class NEntity_Type {
	None,

	Object,
	Wait,
	Animate,

	count
};

// convenience macro
constexpr size_t Entity_Type_Count = static_cast<size_t>(NEntity_Type::count);

enum class NObject_Type {
	None,

	Rectangle,
	Circle,
	Ellipse,
	Round_Rectangle,
	Text,
	Composite,

	count
};

// convenience macro
constexpr size_t Object_Type_Count = static_cast<size_t>(NObject_Type::count);

enum class NExecution_Result {
	Pass,			// act like nothing synchronization-relevant happened
	Suspend,		// suspend the command processing, until this command returns Pass
};

class CTransform {
	private:
		double mX = 0, mY = 0;
		double mRotate = 0;
		double mScale = 1.0;

	public:
		CTransform() {};
		CTransform(double x, double y, double rotate = 0, double scale = 1.0) : mX(x), mY(y), mRotate(rotate), mScale(scale) {}

		CTransform(const CTransform& other) : mX(other.mX), mY(other.mY), mRotate(other.mRotate), mScale(other.mScale) {}
		CTransform(CTransform&& other) noexcept : mX(other.mX), mY(other.mY), mRotate(other.mRotate), mScale(other.mScale) {}

		CTransform& operator=(const CTransform& other) {
			mX = other.mX; mY = other.mY; mRotate = other.mRotate; mScale = other.mScale;
		}
		CTransform& operator=(CTransform&& other) noexcept {
			mX = other.mX; mY = other.mY; mRotate = other.mRotate; mScale = other.mScale;
		}

		double Get_X() const {
			return mX;
		}

		double Get_Y() const {
			return mY;
		}

		double Get_Rotate() const {
			return mRotate;
		}

		double Get_Scale() const {
			return mScale;
		}

		void Apply(BLContext& ctx) const {
			ctx.translate(mX, mY);
			ctx.rotate(mRotate);
			ctx.scale(mScale);
		}

		void Unapply(BLContext& ctx) const {
			ctx.scale(1.0 / mScale);
			ctx.rotate(-mRotate);
			ctx.translate(-mX, -mY);
		}

		static CTransform Identity() {
			return CTransform(0, 0, 0, 1.0);
		}
};

class CTransform_Guard {
	private:
		const CTransform& mTransform;
		BLContext& mContext;

	public:
		CTransform_Guard(const CTransform& transform, BLContext& context) : mTransform(transform), mContext(context) {
			mTransform.Apply(mContext);
		}

		~CTransform_Guard() {
			mTransform.Unapply(mContext);
		}
};

class CValue_Store {
	private:
		std::map<std::string, TValue_Spec> mValues;

	public:
		void Add(const std::string& key, const TValue_Spec& value) {
			mValues[key] = value;
		}

		template<typename T>
		T Get_Value(const std::string& key) const {
			try {
				auto& val = mValues.at(key);
				if (val.type == NValue_Type::Identifier) {
					return std::get<T>(sConsts.Get_Constant(std::get<std::string>(val.value)).value);
				}
				return std::get<T>(mValues.at(key).value);
			}
			catch (...) {
				return std::get<T>(sConsts.Get_Constant(key).value);
			}
		}

		void Merge_With(const CValue_Store& other) {
			for (auto& v : other.mValues) {
				if (mValues.find(v.first) == mValues.end())
					mValues[v.first] = v.second;
			}
		}
};

class CGeneric_Param_Wrapper {
	public:
		virtual TValue_Spec Get_Value() const = 0;
		virtual void Set_Value(const TValue_Spec& src) = 0;
};

template<typename T>
class CParam_Wrapper : public CGeneric_Param_Wrapper {
	private:
		mutable std::optional<T> mValue;
		std::optional<std::string> mAttribute_Name;

	public:
		CParam_Wrapper() {};
		CParam_Wrapper(T value) : mValue(value) {};

		CParam_Wrapper& operator=(const T& value) {
			mValue = value;
			return *this;
		}

		void Set_Resolve_Key(const std::string& key) {
			mAttribute_Name = key;
		}

		TValue_Spec Get_Value() const override {
			return TValue_Spec{ NValue_Type::Identifier, mValue.value() };
		}

		void Set_Value(const TValue_Spec& src) override {
			std::visit([this](auto&& val) {
				if constexpr (std::is_same_v<std::remove_cvref_t<decltype(val)>, T>)
					mValue = val;
			}, src.value);
		}

		template<typename TStore>
		T Get_Value(const TStore& store) const {

			if (mAttribute_Name.has_value()) {
				mValue = store.Get_Value<T>(mAttribute_Name.value());
				if (mValue.has_value())
					return mValue.value();
			}

			if (mValue.has_value()) {
				return mValue.value();
			}

			throw std::runtime_error{ "Cannot resolve runtime identifier" };
		}
};

class CEntity_Animate;

class CScene_Entity {
	friend class CEntity_Animate;

	private:
		NEntity_Type mType;

	protected:
		CValue_Store mDefault_Value_Store;
		std::set<std::string> mResolvable_Attributes;
		std::optional<std::string> mObject_Reference;
		std::map<std::string, CGeneric_Param_Wrapper*> mParam_Reference;

		template<typename T, typename TTarget>
		void Assign_Helper(const std::string& key, const std::map<std::string, TValue_Spec>& paramMap, TTarget& target) {
			auto itr = paramMap.find(key);
			if (itr != paramMap.end()) {
				mParam_Reference[key] = &target;
				if (itr->second.type == NValue_Type::Identifier) {
					target.Set_Resolve_Key(std::get<std::string>(itr->second.value));
				}
				else {
					target = std::get<T>(itr->second.value);
				}
			}
		}

	public:
		explicit CScene_Entity(NEntity_Type type) : mType(type) {}
		virtual ~CScene_Entity() = default;

		NEntity_Type Get_Type() const {
			return mType;
		}

		void Apply_Attribute_Block(const CAttributes* attrs) {
			if (!attrs) {
				return;
			}

			auto ats = attrs->Get_Attribute_List();
			for (auto& at : ats) {
				mResolvable_Attributes.insert(at);
			}
		}

		CValue_Store& Get_Value_Store() {
			return mDefault_Value_Store;
		}

		void Set_Object_Reference(const std::string& objRef) {
			mObject_Reference = objRef;
		}

		CGeneric_Param_Wrapper* Get_Param_Ref(const std::string& refName) {
			auto itr = mParam_Reference.find(refName);
			if (itr == mParam_Reference.end())
				return nullptr;
			return itr->second;
		}

		virtual std::unique_ptr<CScene_Entity> Clone() const { return nullptr; }
		virtual void Apply_Body(CCommand* command) { }
		virtual void Apply_Parameters(const CParams* params) = 0;
		virtual NExecution_Result Execute(CScene& scene) = 0;
};

class CEntity_Wait : public CScene_Entity {
	private:
		CParam_Wrapper<int> mWait_Duration = 0;

	protected:
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

class CEntity_Animate : public CScene_Entity {
	private:
		CParam_Wrapper<int> mDuration = 0;

		struct TAnimate_Param {
			std::string paramName;
			std::optional<TValue_Spec> initial;
			TValue_Spec target;
		};

		std::vector<TAnimate_Param> mAnimation_Params;

	protected:
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

class CScene_Object : public CScene_Entity {
	protected:
		CParam_Wrapper<double> mX = 0.0, mY = 0.0;
		CParam_Wrapper<double> mRotate = 0.0;
		CParam_Wrapper<double> mScale = 1.0;
		NObject_Type mObject_Type = NObject_Type::None;

	public:
		explicit CScene_Object(NObject_Type type);
		virtual ~CScene_Object();

		void Set_X(double x);
		void Set_Y(double y);
		void Set_Position(double x, double y);
		void Set_Rotate(double angle);
		void Set_Scale(double scale);

		double Get_X() const;
		double Get_Y() const;
		double Get_Rotate() const;
		double Get_Scale() const;

		NExecution_Result Execute(CScene& scene) override { return NExecution_Result::Pass; }

		virtual void Apply_Parameters(const CParams* params) override;
		virtual bool Render(BLContext& context, const CTransform& transform) const = 0;
};

template<typename T>
class CBasic_Clonable_Scene_Object : public CScene_Object {
	public:
		using CScene_Object::CScene_Object;

		std::unique_ptr<CScene_Entity> Clone() const override {
			auto ptr = std::make_unique<T>(*static_cast<const T*>(this));
			return ptr;
		}
};

class CRectangle : public CBasic_Clonable_Scene_Object<CRectangle> {
	private:
		CParam_Wrapper<double> mWidth = 0;
		CParam_Wrapper<double> mHeight = 0;
		CParam_Wrapper<rgb_t> mFill_Color = 0;
		CParam_Wrapper<rgb_t> mStroke_Color = 0;
		CParam_Wrapper<double> mStroke_Width = 0;

	public:
		CRectangle() : CBasic_Clonable_Scene_Object(NObject_Type::Rectangle) {}

		void Apply_Parameters(const CParams* params) override;
		bool Render(BLContext& context, const CTransform& transform) const override;
};

class CCircle : public CBasic_Clonable_Scene_Object<CCircle> {
	private:
		CParam_Wrapper<double> mRadius = 0;
		CParam_Wrapper<rgb_t> mFill_Color = 0;
		CParam_Wrapper<rgb_t> mStroke_Color = 0;
		CParam_Wrapper<double> mStroke_Width = 0;

	public:
		CCircle() : CBasic_Clonable_Scene_Object(NObject_Type::Circle) {}

		void Apply_Parameters(const CParams* params) override;
		bool Render(BLContext& context, const CTransform& transform) const override;
};

class CComposite : public CBasic_Clonable_Scene_Object<CComposite> {
	private:
		std::list<std::unique_ptr<CScene_Entity>> mObjects;

	public:
		CComposite() : CBasic_Clonable_Scene_Object(NObject_Type::Composite) {}

		CComposite(const CComposite& other) : CBasic_Clonable_Scene_Object(other) {
			for (auto& obj : other.mObjects)
				mObjects.push_back(obj->Clone());
		}

		void Apply_Body(CCommand* command) override;
		void Apply_Parameters(const CParams* params) override;
		bool Render(BLContext& context, const CTransform& transform) const override;
};
