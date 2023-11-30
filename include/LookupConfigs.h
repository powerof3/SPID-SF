#pragma once

namespace RECORD
{
	enum TYPE
	{
		kSpell = 0,
		kPerk,
		kItem,
		kOutfit,
		kFaction,
		kKeyword,
		kDeathItem,

		kTotal
	};

	inline constexpr std::array add{ "Spell"sv, "Perk"sv, "Item"sv, "Outfit"sv, "Faction"sv, "Keyword"sv, "DeathItem"sv };
}

namespace INI
{
	struct Section
	{
		std::string                        section;
		std::map<std::string, std::string> key_map{};
	};

	struct Data
	{
		enum TYPE : std::uint32_t
		{
			kFormIDPair = 0,
			kFormID = kFormIDPair,
			kStrings,
			kESP = kStrings,
			kFilterIDs,
			kLevel,
			kTraits,
			kIdxOrCount,
			kChance
		};

		Data(const std::string& a_rawForm, const std::map<std::string, std::string>& a_filters, std::string a_path);

		FormOrEditorID          rawForm{};
		StringFilters           stringFilters{};
		Filters<FormOrEditorID> rawFormFilters{};
		Range<std::uint16_t>    actorLevel{};
		std::vector<ActorValue> actorValues{};
		Traits                  traits{};
		IdxOrCount              idxOrCount{ 1 };
		Chance                  chance{ 100 };
		std::string             path{};
	};
	using DataVec = std::vector<Data>;

	inline StringMap<DataVec> configs{};

	std::pair<bool, bool> ReadConfigs();
	void                  ReadConfig(const std::string& a_path);
}
