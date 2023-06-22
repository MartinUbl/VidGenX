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

class CAnimation {
	public:
		// TODO
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

template<typename T>
class CParam_Wrapper {
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

class CScene_Entity {
	private:
		NEntity_Type mType;

	protected:
		CValue_Store mDefault_Value_Store;
		std::set<std::string> mResolvable_Attributes;

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

		virtual std::unique_ptr<CScene_Entity> Clone() const { return nullptr; }
		virtual void Apply_Body(CCommand* command) { }
		virtual void Apply_Parameters(const CParams* params) = 0;
		virtual void Execute(CScene& scene) = 0;
};

class CEntity_Wait : public CScene_Entity {
	private:
		CParam_Wrapper<int> mWait_Duration = 0;

	public:
		CEntity_Wait() : CScene_Entity(NEntity_Type::Wait) {}

		std::unique_ptr<CScene_Entity> Clone() const override {
			auto ptr = std::make_unique<CEntity_Wait>(*this);
			return ptr;
		}

		void Apply_Parameters(const CParams* params) override;
		void Execute(CScene& scene) override;
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

		void Execute(CScene& scene) override { }

		virtual void Apply_Parameters(const CParams* params) override;
		virtual std::unique_ptr<CAnimation> Animate(CParams* targetValues);
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
		std::unique_ptr<CAnimation> Animate(CParams* targetValues) override;
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
		std::unique_ptr<CAnimation> Animate(CParams* targetValues) override;
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
		std::unique_ptr<CAnimation> Animate(CParams* targetValues) override;
		bool Render(BLContext& context, const CTransform& transform) const override;
};
