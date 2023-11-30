#pragma once

namespace NPC
{
	struct Data;
}

namespace Filter
{
	enum class Result
	{
		kFail = 0,
		kPass
	};

	struct Data
	{
		Data(StringFilters a_strings, FormFilters a_formFilters, Range<std::uint16_t> a_level, std::vector<ActorValue> a_actorValues, Traits a_traits, Chance a_chance);

		[[nodiscard]] Result PassedFilters(const NPC::Data& a_npcData) const;

		// members
		StringFilters           strings{};
		FormFilters             forms{};
		Range<std::uint16_t>    actorLevel{};
		std::vector<ActorValue> actorValues{};
		Traits                  traits{};
		Chance                  chance{ 100 };

	private:
		[[nodiscard]] Result passed_string_filters(const NPC::Data& a_npcData) const;
		[[nodiscard]] Result passed_form_filters(const NPC::Data& a_npcData) const;
		[[nodiscard]] Result passed_level_filters(const NPC::Data& a_npcData) const;
		[[nodiscard]] Result passed_trait_filters(const NPC::Data& a_npcData) const;
	};
}

using FilterData = Filter::Data;
