#include "Distribute.h"
#include "LookupForms.h"

namespace Distribute
{
	void Distribute(NPCData& a_npcData)
	{
		std::call_once(onInit, [] { Lookup::DoFormLookup(); });

		const auto npc = a_npcData.GetNPC();

		for_each_form<RE::BGSKeyword>(a_npcData, Forms::keywords, [&](const std::vector<RE::BGSKeyword*>& a_keywords) {
			npc->keywords.reserve(static_cast<std::uint32_t>(a_keywords.size()));
			for (auto& keyword : a_keywords) {
				npc->keywords.emplace_back(keyword);
			}
		});

		for_each_form<RE::TESFaction>(a_npcData, Forms::factions, [&](const std::vector<RE::TESFaction*>& a_factions) {
			npc->factions.reserve(static_cast<std::uint32_t>(a_factions.size()));
			for (auto& faction : a_factions) {
				RE::FACTION_RANK faction_rank{ faction, 1 };
				npc->factions.emplace_back(faction_rank);
			}
		});

		for_each_form<RE::SpellItem>(a_npcData, Forms::spells, [&](const std::vector<RE::SpellItem*>& a_spells) {
			npc->spells.reserve(static_cast<std::uint32_t>(a_spells.size()));
			for (auto& spell : a_spells) {
				npc->spells.emplace_back(spell);
			}
		});

		for_each_form<RE::BGSPerk>(a_npcData, Forms::perks, [&](const std::vector<RE::BGSPerk*>& a_perks) {
			npc->perks.reserve(static_cast<std::uint32_t>(a_perks.size()));
			for (auto& perk : a_perks) {
				npc->perks.emplace_back(RE::PerkRankData{ perk, 1 });
			}
		});

		for_each_form<RE::TESBoundObject>(a_npcData, Forms::items, [&]([[maybe_unused]] std::map<RE::TESBoundObject*, IdxOrCount>& a_objects) {
			for (auto& [object, count] : a_objects) {
				npc->AddObjectToContainer(npc, object, count, nullptr);
			}
		});

		for_each_form<RE::BGSOutfit>(a_npcData, Forms::outfits, [&](auto* a_outfit) {
			if (detail::can_equip_outfit(npc, npc->GetRace(), a_outfit)) {
				npc->defaultOutfit = a_outfit;
				return true;
			}
			return false;
		});

		++totalNPCs;
	}
}
