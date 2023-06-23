#pragma once

#include <memory>
#include <optional>
#include <blend2d.h>
#include <stdexcept>
#include <set>

#include "../consts.h"
#include "../parser_entities.h"

class CScene;

/*
 * Type of scene entity
 */
enum class NEntity_Type {
	None,

	Object,		// drawable object
	Wait,		// wait synchronization entity
	Animate,	// animation entity

	count
};

// convenience macro
constexpr size_t Entity_Type_Count = static_cast<size_t>(NEntity_Type::count);

/*
 * Type of drawable object - this is present mostly for debugging reasons
 */
enum class NObject_Type {
	None,

	Rectangle,
	Circle,
	Composite,

	count
};

// convenience macro
constexpr size_t Object_Type_Count = static_cast<size_t>(NObject_Type::count);

/*
 * Result of entity execution
 */
enum class NExecution_Result {
	Pass,			// act like nothing synchronization-relevant happened
	Suspend,		// suspend the command processing, until this command returns Pass
};

/*
 * Exception class to indicate invalid parameter type
 */
class CInvalid_Parameter_Type : public std::exception {
	private:
		// name of parameter that caused the exception
		std::string mParam_Name;

	public:
		CInvalid_Parameter_Type(const std::string& paramName) : mParam_Name(paramName) {
			//
		}

		const std::string& Get_Param_Name() const {
			return mParam_Name;
		}

		char const* what() const override {
			return "Requested parameter has a different than requested data type";
		}
};

/*
 * Container class representing canvas transformation
 */
class CTransform {
	private:
		// offset
		double mX = 0, mY = 0;
		// rotation
		double mRotate = 0;
		// scale
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

		// applies transformation to given context
		void Apply(BLContext& ctx) const {
			ctx.translate(mX, mY);
			ctx.rotate(mRotate);
			ctx.scale(mScale);
		}

		// reverses the transformation on a given context
		void Unapply(BLContext& ctx) const {
			ctx.scale(1.0 / mScale);
			ctx.rotate(-mRotate);
			ctx.translate(-mX, -mY);
		}

		// generates an identity transformation
		static CTransform Identity() {
			return CTransform(0, 0, 0, 1.0);
		}
};

/*
 * Transform RAII guard to apply transformation upon construction and unapply during destruction
 */
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

/*
 * Value store to serve as parameter/attribute resolver
 */
class CValue_Store {
	private:
		// stored values (attribute values)
		std::map<std::string, TValue_Spec> mValues;

	public:
		// adds a new value to store
		void Add(const std::string& key, const TValue_Spec& value) {
			mValues[key] = value;
		}

		// retrieves a value from store
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

		// merges two value stores into this instance
		void Merge_With(const CValue_Store& other) {
			for (auto& v : other.mValues) {
				if (mValues.find(v.first) == mValues.end())
					mValues[v.first] = v.second;
			}
		}
};

/*
 * Base class for all parameter wrappers
 */
class CGeneric_Param_Wrapper {
	public:
		// retrieves value from the container
		virtual TValue_Spec Get_Value() const = 0;
		// sets the value to the container
		virtual void Set_Value(const TValue_Spec& src) = 0;
};

/*
 * A parameter wrapper class so we can resolve parameters at runtime
 */
template<typename T>
class CParam_Wrapper : public CGeneric_Param_Wrapper {
	private:
		// an actual value; mutability is needed for lazyloading of actual values during runtime
		mutable std::optional<T> mValue;
		// attribute name to be used for value resolution
		std::optional<std::string> mAttribute_Name;

	public:
		CParam_Wrapper() {};
		CParam_Wrapper(T value) : mValue(value) {};

		CParam_Wrapper& operator=(const T& value) {
			mValue = value;
			return *this;
		}

		// set attribute name to be used for resolution
		void Set_Resolve_Key(const std::string& key) {
			mAttribute_Name = key;
		}

		// retrieves an actual value of the parameter
		TValue_Spec Get_Value() const override {
			return TValue_Spec{ NValue_Type::Identifier, mValue.value() };
		}

		// sets the value to the parameter
		void Set_Value(const TValue_Spec& src) override {
			// this is a rather tricky construction, that ensured the correct type of target variant
			std::visit([this](auto&& val) {
				if constexpr (std::is_same_v<std::remove_cvref_t<decltype(val)>, T>)
					mValue = val;
			}, src.value);
		}

		// resolves a value of this parameter
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

/*
 * A base class for all scene entities
 */
class CScene_Entity {
	private:
		// type of entity
		NEntity_Type mType;

	protected:
		// value store used for parameter resolution
		CValue_Store mDefault_Value_Store;
		// set of resolvable attributes for runtime resolution
		std::set<std::string> mResolvable_Attributes;
		// reference to object, upon which the entity is invoked (e.g., animation)
		std::optional<std::string> mObject_Reference;
		// map of references to parameters; this is needed for e.g., animations
		std::map<std::string, CGeneric_Param_Wrapper*> mParam_Reference;

		// register entity parameter and resolve its value, if possible
		template<typename T, typename TTarget>
		void Assign_Helper(const std::string& key, const std::map<std::string, TValue_Spec>& paramMap, TTarget& target) {
			auto itr = paramMap.find(key);
			if (itr != paramMap.end()) {
				// store the parameter reference
				mParam_Reference[key] = &target;
				// if it is an identifier, postpone the resolution for later
				if (itr->second.type == NValue_Type::Identifier) {
					target.Set_Resolve_Key(std::get<std::string>(itr->second.value));
				}
				// otherwise try to resolve it immediatelly
				else {
					try {
						target = std::get<T>(itr->second.value);
					}
					catch (std::bad_variant_access&) {
						throw CInvalid_Parameter_Type(key);
					}
				}
			}
		}

	public:
		explicit CScene_Entity(NEntity_Type type) : mType(type) {}
		virtual ~CScene_Entity() = default;

		// retrieves entity type
		NEntity_Type Get_Type() const {
			return mType;
		}

		// applies attribute block to the entity - so the resolver can determine, which identifiers are attributes and which are parameters
		void Apply_Attribute_Block(const CAttributes* attrs) {
			if (!attrs) {
				return;
			}

			auto ats = attrs->Get_Attribute_List();
			for (auto& at : ats) {
				mResolvable_Attributes.insert(at);
			}
		}

		// retrieves default value store
		CValue_Store& Get_Value_Store() {
			return mDefault_Value_Store;
		}

		// sets object reference upon which the entity is executed
		void Set_Object_Reference(const std::string& objRef) {
			mObject_Reference = objRef;
		}

		// retrieves a reference to parameter wrapper
		CGeneric_Param_Wrapper* Get_Param_Ref(const std::string& refName) {
			auto itr = mParam_Reference.find(refName);
			if (itr == mParam_Reference.end())
				return nullptr;
			return itr->second;
		}

		// clones the entity (preferably deep clone)
		virtual std::unique_ptr<CScene_Entity> Clone() const { return nullptr; }
		// applies a body to the entity; this is needed for e.g., composite entities
		virtual void Apply_Body(CCommand* command) { }
		// applies parameters to the entity; every child should call the parent method, if possible, as the child extends parent's parameter block as well
		virtual void Apply_Parameters(const CParams* params) = 0;
		// executes the body of the entity to perform some action
		virtual NExecution_Result Execute(CScene& scene) = 0;
};

/*
 * A base class for all renderable objects
 */
class CScene_Object : public CScene_Entity {
	protected:
		// position
		CParam_Wrapper<double> mX = 0.0, mY = 0.0;
		// rotation
		CParam_Wrapper<double> mRotate = 0.0;
		// scale
		CParam_Wrapper<double> mScale = 1.0;
		// type of the object
		NObject_Type mObject_Type = NObject_Type::None;

	public:
		explicit CScene_Object(NObject_Type type);
		virtual ~CScene_Object();

		// retrieves the X coordinate
		double Get_X() const;
		// retrieves the Y coordinate
		double Get_Y() const;
		// retrieves the object rotation
		double Get_Rotate() const;
		// retrieves the object scale
		double Get_Scale() const;

		// default execution policy is to pass to next objects in the scene
		NExecution_Result Execute(CScene& scene) override { return NExecution_Result::Pass; }

		virtual void Apply_Parameters(const CParams* params) override;
		virtual bool Render(BLContext& context, const CTransform& transform) const = 0;
};

/*
 * Base class for all clonable objects
 */
template<typename T>
class CBasic_Clonable_Scene_Object : public CScene_Object {
	public:
		using CScene_Object::CScene_Object;

		std::unique_ptr<CScene_Entity> Clone() const override {
			// clone is performed via copy constructor by default
			auto ptr = std::make_unique<T>(*static_cast<const T*>(this));
			return ptr;
		}
};
