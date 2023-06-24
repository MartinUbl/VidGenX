#pragma once

#include <map>
#include <vector>
#include <string>
#include <variant>
#include <optional>
#include <list>

enum class NBlock_Type {
	Config,
	Consts,
	Prototypes,
	Scene,
};

enum class NValue_Type {
	Float,
	String,
	Identifier,			// always a string
	RGB,				// always converted to integer (ARGB)
	Timespec,			// always converted to milliseconds
};

using rgb_t = uint32_t;

struct TValue_Spec {
	NValue_Type type = NValue_Type::Float;
	std::variant<int, double, rgb_t, std::string> value;
};

struct TParam_Entry {
	std::string key;
	TValue_Spec value;
};

class CParams {
	private:
		std::map<std::string, TValue_Spec> mParameters;

	public:
		template<typename T>
		void Add_Parameter(NValue_Type type, const std::string& key, T&& value) {
			mParameters[key] = {
				type,
				value
			};
		}

		void Add_Parameter(TParam_Entry* entry) {
			mParameters[entry->key] = entry->value;
		}

		void Merge_To(CParams* target) const {
			for (auto& p : mParameters) {
				target->mParameters[p.first] = p.second;
			}
		}

		void Remove_Parameter(const std::string& key) {
			mParameters.erase(key);
		}

		const std::map<std::string, TValue_Spec>& Get_Parameters() const {
			return mParameters;
		}
};

class CAttributes {
	private:
		std::list<std::string> mAttributes;

	public:
		void Add_Attribute(const std::string& param) {
			mAttributes.push_back(param);
		}

		void Merge_To(CAttributes* target) {
			for (auto& p : mAttributes) {
				target->mAttributes.push_front(p);
			}
		}

		const std::list<std::string>& Get_Attribute_List() const {
			return mAttributes;
		}
};

class CCommand {
	private:
		std::string mIdentifier;
		std::string mEntity_Name;
		std::string mObject_Reference;
		CParams* mParams = nullptr;
		CAttributes* mAttributes = nullptr;
		std::optional<TValue_Spec> mValue;

		std::list<CCommand*> mSubcommands;

	public:
		CCommand() {
			//
		}

		~CCommand() {
			if (mParams)
				delete mParams;

			if (mAttributes)
				delete mAttributes;
		}

		void Set_Identifier(const std::string& identifier) {
			mIdentifier = identifier;
		}

		const std::string& Get_Identifier() const {
			return mIdentifier;
		}

		void Set_Object_Reference(const std::string& objRef) {
			mObject_Reference = objRef;
		}

		const std::string& Get_Object_Reference() const {
			return mObject_Reference;
		}

		void Set_Entity_Name(const std::string& entityName) {
			mEntity_Name = entityName;
		}

		const std::string& Get_Entity_Name() const {
			return mEntity_Name;
		}

		void Set_Value(std::optional<TValue_Spec> value) {
			mValue = value;
		}

		const std::optional<TValue_Spec>& Get_Value() const {
			return mValue;
		}

		void Set_Params(CParams* params) {
			if (mParams)
				delete mParams;

			mParams = params;
		}

		const CParams* Get_Params() const {
			return mParams;
		}

		void Set_Attributes(CAttributes* attrs) {
			if (mAttributes)
				delete mAttributes;

			mAttributes = attrs;
		}

		const CAttributes* Get_Attributes() const {
			return mAttributes;
		}

		void Add_Command(CCommand* command) {
			mSubcommands.push_front(command);
		}

		const std::list<CCommand*>& Get_Subcommands() const {
			return mSubcommands;
		}
};

class CBlock {
	private:
		CParams* mParams = nullptr;
		NBlock_Type mType;
		CCommand* mContent = nullptr;
		size_t mBlock_Index = 0;

	public:
		CBlock(NBlock_Type type) : mParams{ nullptr }, mType(type), mContent{ nullptr } {
			//
		}

		~CBlock() {
			if (mParams)
				delete mParams;
			if (mContent)
				delete mContent;
		}

		void Set_Params(CParams* params) {
			if (mParams)
				delete mParams;

			mParams = params;
		}

		void Set_Command(CCommand* cmd) {
			if (mContent)
				delete mContent;
			mContent = cmd;
		}

		NBlock_Type Get_Type() const {
			return mType;
		}

		const CParams* Get_Parameters() const {
			return mParams;
		}

		const CCommand* Get_Content() const {
			return mContent;
		}

		void Set_Block_Index(size_t idx) {
			mBlock_Index = idx;
		}

		size_t Get_Block_Index() const {
			return mBlock_Index;
		}
};
