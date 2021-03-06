#pragma once

#include "Template.h"

#include "INIParser.h"
#include "Enums.h"
#include "Constructs.h"
#include "../Misc/SavegameDef.h"

#include <InfantryTypeClass.h>
#include <AircraftTypeClass.h>
#include <UnitTypeClass.h>
#include <BuildingTypeClass.h>
#include <FootClass.h>
#include <VocClass.h>
#include <VoxClass.h>

namespace detail {
	template <typename T>
	inline bool read(T& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate = false) {
		if(parser.ReadString(pSection, pKey)) {
			using base_type = std::remove_pointer_t<T>;

			auto const pValue = parser.value();
			auto const parsed = (allocate ? base_type::FindOrAllocate : base_type::Find)(pValue);
			if(parsed || INIClass::IsBlank(pValue)) {
				value = parsed;
				return true;
			} else {
				Debug::INIParseFailed(pSection, pKey, pValue);
			}
		}
		return false;
	}

	template <>
	inline bool read<bool>(bool& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate) {
		bool buffer;
		if(parser.ReadBool(pSection, pKey, &buffer)) {
			value = buffer;
			return true;
		} else if(!parser.empty()) {
			Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a valid boolean value [1, true, yes, 0, false, no]");
		}
		return false;
	}

	template <>
	inline bool read<int>(int& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate) {
		int buffer;
		if(parser.ReadInteger(pSection, pKey, &buffer)) {
			value = buffer;
			return true;
		} else if(!parser.empty()) {
			Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a valid number");
		}
		return false;
	}

	template <>
	inline bool read<BYTE>(BYTE& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate) {
		int buffer;
		if(parser.ReadInteger(pSection, pKey, &buffer)) {
			if(buffer <= 255 && buffer >= 0) {
				value = static_cast<BYTE>(buffer); // shut up shut up shut up C4244
				return true;
			} else {
				Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a valid number between 0 and 255 inclusive.");
			}
		} else if(!parser.empty()) {
			Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a valid number");
		}
		return false;
	}

	template <>
	inline bool read<float>(float& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate) {
		double buffer;
		if(parser.ReadDouble(pSection, pKey, &buffer)) {
			value = static_cast<float>(buffer);
			return true;
		} else if(!parser.empty()) {
			Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a valid floating point number");
		}
		return false;
	}

	template <>
	inline bool read<double>(double& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate) {
		double buffer;
		if(parser.ReadDouble(pSection, pKey, &buffer)) {
			value = buffer;
			return true;
		} else if(!parser.empty()) {
			Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a valid floating point number");
		}
		return false;
	}

	template <>
	inline bool read<ColorStruct>(ColorStruct& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate) {
		ColorStruct buffer;
		if(parser.Read3Bytes(pSection, pKey, reinterpret_cast<byte*>(&buffer))) {
			value = buffer;
			return true;
		} else if(!parser.empty()) {
			Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a valid R,G,B color");
		}
		return false;
	}

	template <>
	inline bool read<CSFText>(CSFText& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate) {
		if(parser.ReadString(pSection, pKey)) {
			value = parser.value();
			return true;
		}
		return false;
	}

	template <>
	inline bool read<SHPStruct*>(SHPStruct*& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate) {
		if(parser.ReadString(pSection, pKey)) {
			char flag[256];
			auto const pValue = parser.value();
			_snprintf_s(flag, 255, "%s.shp", pValue);
			if(auto const pImage = FileSystem::LoadSHPFile(flag)) {
				value = pImage;
				return true;
			} else {
				Debug::Log(Debug::Severity::Warning, "Failed to find file %s referenced by [%s]%s=%s\n", flag, pSection, pKey, pValue);
				Debug::RegisterParserError();
			}
		}
		return false;
	}

	template <>
	inline bool read<MouseCursor>(MouseCursor& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate) {
		auto ret = false;

		// compact way to define the cursor in one go
		if(parser.ReadString(pSection, pKey)) {
			auto const buffer = parser.value();
			char* context = nullptr;
			if(auto const pFrame = strtok_s(buffer, Ares::readDelims, &context)) {
				Parser<int>::Parse(pFrame, &value.Frame);
			}
			if(auto const pCount = strtok_s(nullptr, Ares::readDelims, &context)) {
				Parser<int>::Parse(pCount, &value.Count);
			}
			if(auto const pInterval = strtok_s(nullptr, Ares::readDelims, &context)) {
				Parser<int>::Parse(pInterval, &value.Interval);
			}
			if(auto const pFrame = strtok_s(nullptr, Ares::readDelims, &context)) {
				Parser<int>::Parse(pFrame, &value.MiniFrame);
			}
			if(auto const pCount = strtok_s(nullptr, Ares::readDelims, &context)) {
				Parser<int>::Parse(pCount, &value.MiniCount);
			}
			if(auto const pHotX = strtok_s(nullptr, Ares::readDelims, &context)) {
				MouseCursorHotSpotX::Parse(pHotX, &value.HotX);
			}
			if(auto const pHotY = strtok_s(nullptr, Ares::readDelims, &context)) {
				MouseCursorHotSpotY::Parse(pHotY, &value.HotY);
			}

			ret = true;
		}

		char pFlagName[32];
		_snprintf_s(pFlagName, 31, "%s.Frame", pKey);
		ret |= read(value.Frame, parser, pSection, pFlagName);

		_snprintf_s(pFlagName, 31, "%s.Count", pKey);
		ret |= read(value.Count, parser, pSection, pFlagName);

		_snprintf_s(pFlagName, 31, "%s.Interval", pKey);
		ret |= read(value.Interval, parser, pSection, pFlagName);

		_snprintf_s(pFlagName, 31, "%s.MiniFrame", pKey);
		ret |= read(value.MiniFrame, parser, pSection, pFlagName);

		_snprintf_s(pFlagName, 31, "%s.MiniCount", pKey);
		ret |= read(value.MiniCount, parser, pSection, pFlagName);

		_snprintf_s(pFlagName, 31, "%s.HotSpot", pKey);
		if(parser.ReadString(pSection, pFlagName)) {
			auto const pValue = parser.value();
			char* context = nullptr;
			auto const pHotX = strtok_s(pValue, ",", &context);
			MouseCursorHotSpotX::Parse(pHotX, &value.HotX);

			if(auto const pHotY = strtok_s(nullptr, ",", &context)) {
				MouseCursorHotSpotY::Parse(pHotY, &value.HotY);
			}

			ret = true;
		}

		return ret;
	}

	template <>
	inline bool read<RocketStruct>(RocketStruct& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate) {
		auto ret = false;

		char pFlagName[0x40];
		_snprintf_s(pFlagName, 0x3F, "%s.PauseFrames", pKey);
		ret |= read(value.PauseFrames, parser, pSection, pFlagName);

		_snprintf_s(pFlagName, 0x3F, "%s.TiltFrames", pKey);
		ret |= read(value.TiltFrames, parser, pSection, pFlagName);

		_snprintf_s(pFlagName, 0x3F, "%s.PitchInitial", pKey);
		ret |= read(value.PitchInitial, parser, pSection, pFlagName);

		_snprintf_s(pFlagName, 0x3F, "%s.PitchFinal", pKey);
		ret |= read(value.PitchFinal, parser, pSection, pFlagName);

		_snprintf_s(pFlagName, 0x3F, "%s.TurnRate", pKey);
		ret |= read(value.TurnRate, parser, pSection, pFlagName);

		// sic! integer read like a float.
		_snprintf_s(pFlagName, 0x3F, "%s.RaiseRate", pKey);
		float buffer;
		if(read(buffer, parser, pSection, pFlagName)) {
			value.RaiseRate = Game::F2I(buffer);
			ret = true;
		}	

		_snprintf_s(pFlagName, 0x3F, "%s.Acceleration", pKey);
		ret |= read(value.Acceleration, parser, pSection, pFlagName);

		_snprintf_s(pFlagName, 0x3F, "%s.Altitude", pKey);
		ret |= read(value.Altitude, parser, pSection, pFlagName);

		_snprintf_s(pFlagName, 0x3F, "%s.Damage", pKey);
		ret |= read(value.Damage, parser, pSection, pFlagName);

		_snprintf_s(pFlagName, 0x3F, "%s.EliteDamage", pKey);
		ret |= read(value.EliteDamage, parser, pSection, pFlagName);

		_snprintf_s(pFlagName, 0x3F, "%s.BodyLength", pKey);
		ret |= read(value.BodyLength, parser, pSection, pFlagName);

		_snprintf_s(pFlagName, 0x3F, "%s.LazyCurve", pKey);
		ret |= read(value.LazyCurve, parser, pSection, pFlagName);

		_snprintf_s(pFlagName, 0x3F, "%s.Type", pKey);
		ret |= read(value.Type, parser, pSection, pFlagName);

		return ret;
	}

	template <>
	inline bool read<Leptons>(Leptons& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate) {
		double buffer;
		if(parser.ReadDouble(pSection, pKey, &buffer)) {
			value = Leptons(Game::F2I(buffer * 256.0));
			return true;
		} else if(!parser.empty()) {
			Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a valid floating point number");
		}
		return false;
	}

	template <>
	inline bool read<OwnerHouseKind>(OwnerHouseKind& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate) {
		if(parser.ReadString(pSection, pKey)) {
			if(_strcmpi(parser.value(), "default") == 0) {
				value = OwnerHouseKind::Default;
			} else if(_strcmpi(parser.value(), "invoker") == 0) {
				value = OwnerHouseKind::Invoker;
			} else if(_strcmpi(parser.value(), "killer") == 0) {
				value = OwnerHouseKind::Killer;
			} else if(_strcmpi(parser.value(), "victim") == 0) {
				value = OwnerHouseKind::Victim;
			} else if(_strcmpi(parser.value(), "civilian") == 0) {
				value = OwnerHouseKind::Civilian;
			} else if(_strcmpi(parser.value(), "special") == 0) {
				value = OwnerHouseKind::Special;
			} else if(_strcmpi(parser.value(), "neutral") == 0) {
				value = OwnerHouseKind::Neutral;
			} else if(_strcmpi(parser.value(), "random") == 0) {
				value = OwnerHouseKind::Random;
			} else {
				Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a owner house kind");
				return false;
			}
			return true;
		}
		return false;
	}

	template <>
	inline bool read<Mission>(Mission& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate) {
		if(parser.ReadString(pSection, pKey)) {
			auto const mission = MissionControlClass::FindIndex(parser.value());
			if(mission != Mission::None) {
				value = mission;
				return true;
			} else if(!parser.empty()) {
				Debug::INIParseFailed(pSection, pKey, parser.value(), "Invalid Mission name");
			}
		}
		return false;
	}

	template <>
	inline bool read<SuperWeaponAITargetingMode>(SuperWeaponAITargetingMode& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate) {
		if(parser.ReadString(pSection, pKey)) {
			static const auto Modes = {
				"none", "nuke", "lightningstorm", "psychicdominator", "paradrop",
				"geneticmutator", "forceshield", "notarget", "offensive", "stealth",
				"self", "base", "multimissile", "hunterseeker", "enemybase" };

			auto it = Modes.begin();
			for(auto i = 0u; i < Modes.size(); ++i) {
				if(_strcmpi(parser.value(), *it++) == 0) {
					value = static_cast<SuperWeaponAITargetingMode>(i);
					return true;
				}
			}

			Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a targeting mode");
		}
		return false;
	}

	template <>
	inline bool read<SuperWeaponTarget>(SuperWeaponTarget& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate) {
		if(parser.ReadString(pSection, pKey)) {
			auto parsed = SuperWeaponTarget::None;

			auto str = parser.value();
			char* context = nullptr;
			for(auto cur = strtok_s(str, Ares::readDelims, &context); cur; cur = strtok_s(nullptr, Ares::readDelims, &context)) {
				if(!_strcmpi(cur, "land")) {
					parsed |= SuperWeaponTarget::Land;
				} else if(!_strcmpi(cur, "water")) {
					parsed |= SuperWeaponTarget::Water;
				} else if(!_strcmpi(cur, "empty")) {
					parsed |= SuperWeaponTarget::NoContent;
				} else if(!_strcmpi(cur, "infantry")) {
					parsed |= SuperWeaponTarget::Infantry;
				} else if(!_strcmpi(cur, "units")) {
					parsed |= SuperWeaponTarget::Unit;
				} else if(!_strcmpi(cur, "buildings")) {
					parsed |= SuperWeaponTarget::Building;
				} else if(!_strcmpi(cur, "all")) {
					parsed |= SuperWeaponTarget::All;
				} else if(_strcmpi(cur, "none")) {
					Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a super weapon target");
					return false;
				}
			}
			value = parsed;
			return true;
		}
		return false;
	}

	template <>
	inline bool read<SuperWeaponAffectedHouse>(SuperWeaponAffectedHouse& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate) {
		if(parser.ReadString(pSection, pKey)) {
			auto parsed = SuperWeaponAffectedHouse::None;

			auto str = parser.value();
			char* context = nullptr;
			for(auto cur = strtok_s(str, Ares::readDelims, &context); cur; cur = strtok_s(nullptr, Ares::readDelims, &context)) {
				if(!_strcmpi(cur, "owner")) {
					parsed |= SuperWeaponAffectedHouse::Owner;
				} else if(!_strcmpi(cur, "allies")) {
					parsed |= SuperWeaponAffectedHouse::Allies;
				} else if(!_strcmpi(cur, "enemies")) {
					parsed |= SuperWeaponAffectedHouse::Enemies;
				} else if(!_strcmpi(cur, "team")) {
					parsed |= SuperWeaponAffectedHouse::Team;
				} else if(!_strcmpi(cur, "others")) {
					parsed |= SuperWeaponAffectedHouse::NotOwner;
				} else if(!_strcmpi(cur, "all")) {
					parsed |= SuperWeaponAffectedHouse::All;
				} else if(_strcmpi(cur, "none")) {
					Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a super weapon affected house");
					return false;
				}
			}
			value = parsed;
			return true;
		}
		return false;
	}

	template <typename T>
	void parse_values(std::vector<T>& vector, INI_EX &parser, const char* pSection, const char* pKey) {
		char* context = nullptr;
		for(auto pCur = strtok_s(parser.value(), Ares::readDelims, &context); pCur; pCur = strtok_s(nullptr, Ares::readDelims, &context)) {
			T buffer = T();
			if(Parser<T>::Parse(pCur, &buffer)) {
				vector.push_back(buffer);
			} else if(!INIClass::IsBlank(pCur)) {
				Debug::INIParseFailed(pSection, pKey, pCur);
			}
		}
	}

	template <typename Lookuper, typename T>
	void parse_indexes(std::vector<T>& vector, INI_EX &parser, const char* pSection, const char* pKey) {
		char* context = nullptr;
		for(auto pCur = strtok_s(parser.value(), Ares::readDelims, &context); pCur; pCur = strtok_s(nullptr, Ares::readDelims, &context)) {
			int idx = Lookuper::FindIndex(pCur);
			if(idx != -1) {
				vector.push_back(idx);
			} else if(!INIClass::IsBlank(pCur)) {
				Debug::INIParseFailed(pSection, pKey, pCur);
			}
		}
	}
}


// Valueable

template <typename T>
void __declspec(noinline) Valueable<T>::Read(INI_EX &parser, const char* pSection, const char* pKey, bool Allocate) {
	detail::read(this->Value, parser, pSection, pKey, Allocate);
}

template <typename T>
bool Valueable<T>::Load(AresStreamReader &Stm, bool RegisterForChange) {
	return Savegame::ReadAresStream(Stm, this->Value, RegisterForChange);
}

template <typename T>
bool Valueable<T>::Save(AresStreamWriter &Stm) const {
	return Savegame::WriteAresStream(Stm, this->Value);
}


// ValueableIdx

template <typename Lookuper>
void __declspec(noinline) ValueableIdx<Lookuper>::Read(INI_EX &parser, const char* pSection, const char* pKey) {
	if(parser.ReadString(pSection, pKey)) {
		const char * val = parser.value();
		int idx = Lookuper::FindIndex(val);
		if(idx != -1 || INIClass::IsBlank(val)) {
			this->Value = idx;
		} else {
			Debug::INIParseFailed(pSection, pKey, val);
		}
	}
}


// Nullable

template <typename T>
void __declspec(noinline) Nullable<T>::Read(INI_EX &parser, const char* pSection, const char* pKey, bool Allocate) {
	if(detail::read(this->Value, parser, pSection, pKey, Allocate)) {
		this->HasValue = true;
	}
}

template <typename T>
bool Nullable<T>::Load(AresStreamReader &Stm, bool RegisterForChange) {
	this->Reset();
	auto ret = Savegame::ReadAresStream(Stm, this->HasValue);
	if(ret && this->HasValue) {
		ret = Savegame::ReadAresStream(Stm, this->Value, RegisterForChange);
	}
	return ret;
}

template <typename T>
bool Nullable<T>::Save(AresStreamWriter &Stm) const {
	auto ret = Savegame::WriteAresStream(Stm, this->HasValue);
	if(this->HasValue) {
		ret = Savegame::WriteAresStream(Stm, this->Value);
	}
	return ret;
}


// NullableIdx

template <typename Lookuper>
void __declspec(noinline) NullableIdx<Lookuper>::Read(INI_EX &parser, const char* pSection, const char* pKey) {
	if(parser.ReadString(pSection, pKey)) {
		const char * val = parser.value();
		int idx = Lookuper::FindIndex(val);
		if(idx != -1 || INIClass::IsBlank(val)) {
			this->Value = idx;
			this->HasValue = true;
		} else {
			Debug::INIParseFailed(pSection, pKey, val);
		}
	}
}


// Promotable

template <typename T>
void __declspec(noinline) Promotable<T>::Read(INI_EX &parser, const char* const pSection, const char* const pBaseFlag, const char* const pSingleFlag) {

	// read the common flag, with the trailing dot being stripped
	char flagName[0x40];
	auto const pSingleFormat = pSingleFlag ? pSingleFlag : pBaseFlag;
	auto res = _snprintf_s(flagName, _TRUNCATE, pSingleFormat, "");
	if(res > 0 && flagName[res - 1] == '.') {
		flagName[res - 1] = '\0';
	}

	T placeholder;
	if(detail::read(placeholder, parser, pSection, flagName)) {
		this->SetAll(placeholder);
	}

	// read specific flags
	_snprintf_s(flagName, _TRUNCATE, pBaseFlag, "Rookie");
	detail::read(this->Rookie, parser, pSection, flagName);

	_snprintf_s(flagName, _TRUNCATE, pBaseFlag, "Veteran");
	detail::read(this->Veteran, parser, pSection, flagName);

	_snprintf_s(flagName, _TRUNCATE, pBaseFlag, "Elite");
	detail::read(this->Elite, parser, pSection, flagName);
};

template <typename T>
bool Promotable<T>::Load(AresStreamReader &Stm, bool RegisterForChange) {
	return Savegame::ReadAresStream(Stm, this->Rookie, RegisterForChange)
		&& Savegame::ReadAresStream(Stm, this->Veteran, RegisterForChange)
		&& Savegame::ReadAresStream(Stm, this->Elite, RegisterForChange);
}

template <typename T>
bool Promotable<T>::Save(AresStreamWriter &Stm) const {
	return Savegame::WriteAresStream(Stm, this->Rookie)
		&& Savegame::WriteAresStream(Stm, this->Veteran)
		&& Savegame::WriteAresStream(Stm, this->Elite);
}


// ValueableVector

template <typename T>
void __declspec(noinline) ValueableVector<T>::Read(INI_EX &parser, const char* pSection, const char* pKey) {
	if(parser.ReadString(pSection, pKey)) {
		this->clear();
		detail::parse_values<T>(*this, parser, pSection, pKey);
	}
}

template <typename T>
bool ValueableVector<T>::Load(AresStreamReader &Stm, bool RegisterForChange) {
	size_t size = 0;
	if(Savegame::ReadAresStream(Stm, size, RegisterForChange)) {
		this->clear();
		this->reserve(size);

		for(size_t i = 0; i < size; ++i) {
			value_type buffer = value_type();
			Savegame::ReadAresStream(Stm, buffer, false);
			this->push_back(std::move(buffer));

			if(RegisterForChange) {
				Swizzle swizzle(this->back());
			}
		}
		return true;
	}
	return false;
}

template <typename T>
bool ValueableVector<T>::Save(AresStreamWriter &Stm) const {
	auto size = this->size();
	if(Savegame::WriteAresStream(Stm, size)) {
		for(auto const& item : *this) {
			if(!Savegame::WriteAresStream(Stm, item)) {
				return false;
			}
		}
		return true;
	}
	return false;
}


// NullableVector

template <typename T>
void __declspec(noinline) NullableVector<T>::Read(INI_EX &parser, const char* pSection, const char* pKey) {
	if(parser.ReadString(pSection, pKey)) {
		this->clear();

		auto const non_default = _strcmpi(parser.value(), "<default>") != 0;
		this->hasValue = non_default;

		if(non_default) {
			detail::parse_values<T>(*this, parser, pSection, pKey);
		}
	}
}

template <typename T>
bool NullableVector<T>::Load(AresStreamReader &Stm, bool RegisterForChange) {
	this->clear();
	if(Savegame::ReadAresStream(Stm, this->hasValue, RegisterForChange)) {
		return !this->hasValue || ValueableVector<T>::Load(Stm, RegisterForChange);
	}
	return false;
}

template <typename T>
bool NullableVector<T>::Save(AresStreamWriter &Stm) const {
	if(Savegame::WriteAresStream(Stm, this->hasValue)) {
		return !this->hasValue || ValueableVector<T>::Save(Stm);
	}
	return false;
}


// ValueableIdxVector

template <typename Lookuper>
void __declspec(noinline) ValueableIdxVector<Lookuper>::Read(INI_EX &parser, const char* pSection, const char* pKey) {
	if(parser.ReadString(pSection, pKey)) {
		this->clear();
		detail::parse_indexes<Lookuper>(*this, parser, pSection, pKey);
	}
}


// NullableIdxVector

template <typename Lookuper>
void __declspec(noinline) NullableIdxVector<Lookuper>::Read(INI_EX &parser, const char* pSection, const char* pKey) {
	if(parser.ReadString(pSection, pKey)) {
		this->clear();

		auto const non_default = _strcmpi(parser.value(), "<default>") != 0;
		this->hasValue = non_default;

		if(non_default) {
			detail::parse_indexes<Lookuper>(*this, parser, pSection, pKey);
		}
	}
}
