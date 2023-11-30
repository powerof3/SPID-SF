#pragma once

#include "FormData.h"
#include "LookupNPC.h"

namespace Distribute
{
	inline std::once_flag onInit;
	inline std::size_t    totalNPCs{ 0 };

	namespace detail
	{
		template <class Form>
		bool passed_filters(
			const NPCData&           a_npcData,
			const Forms::Data<Form>& a_formData)
		{
			return a_formData.filters.PassedFilters(a_npcData) == Filter::Result::kPass;
		}

		template <class Form>
		bool has_form(RE::TESNPC* a_npc, Form* a_form)
		{
			bool result = false;
			if constexpr (std::is_same_v<RE::TESFaction, Form>) {
				result = a_npc->IsInFaction(a_form);
			} else if constexpr (std::is_same_v<RE::BGSPerk, Form>) {
				result = std::ranges::find_if(a_npc->perks, [&](const auto& perkRank) { return perkRank.perk == a_form; });
			} else if constexpr (std::is_same_v<RE::SpellItem, Form>) {
				result = std::ranges::contains(a_npc->spells, a_form);
			}
			return result;
		}

		inline bool can_equip_outfit(RE::TESNPC* a_npc, RE::TESRace* a_race, RE::BGSOutfit* a_outfit)
		{
			if (a_npc->defaultOutfit == a_outfit) {
				return false;
			}

			for (const auto& item: a_outfit->outfitItems) {
				if (const auto armor = item->As<RE::TESObjectARMO>(); armor && !armor->CanUseArmor(a_race)) {
					return false;
				}
			}

			return true;
		}
	}

	// old method (distributing one by one)
	// for now, only death items use this
	template <class Form>
	void for_each_form(
		const NPCData&                         a_npcData,
		Forms::Distributables<Form>&           a_distributables,
		std::function<bool(Form*, IdxOrCount)> a_callback)
	{
		const auto& vec = a_distributables.GetForms();

		for (auto& formData : vec) {
			if (detail::passed_filters(a_npcData, formData)) {
				a_callback(formData.form, formData.idxOrCount);
				a_distributables.IncrementDistributionCount(formData.form);
			}
		}
	}

	// outfits/sleep outfits
	// overridable forms
	template <class Form>
	void for_each_form(
		const NPCData&               a_npcData,
		Forms::Distributables<Form>& a_distributables,
		std::function<bool(Form*)>   a_callback)
	{
		const auto& vec = a_distributables.GetForms();

		for (auto& formData : vec) {  // Vector is reversed in FinishLookupForms
			if (detail::passed_filters(a_npcData, formData)) {
				auto form = formData.form;
				if (a_callback(form)) {
					a_distributables.IncrementDistributionCount(form);
					break;
				}
			}
		}
	}

	// items
	template <class Form>
	void for_each_form(
		const NPCData&                                    a_npcData,
		Forms::Distributables<Form>&                      a_distributables,
		std::function<void(std::map<Form*, IdxOrCount>&)> a_callback)
	{
		const auto& vec = a_distributables.GetForms();

		if (vec.empty()) {
			return;
		}

		std::map<Form*, IdxOrCount> collectedForms{};

		for (auto& formData : vec) {
			if (detail::passed_filters(a_npcData, formData)) {
				collectedForms.emplace(formData.form, formData.idxOrCount);
				a_distributables.IncrementDistributionCount(formData.form);
			}
		}

		if (!collectedForms.empty()) {
			a_callback(collectedForms);
		}
	}

	// spells, perks, shouts, keywords
	// forms that can be added to
	template <class Form>
	void for_each_form(
		NPCData&                                       a_npcData,
		Forms::Distributables<Form>&                   a_distributables,
		std::function<void(const std::vector<Form*>&)> a_callback)
	{
		const auto& vec = a_distributables.GetForms();

		if (vec.empty()) {
			return;
		}

		const auto npc = a_npcData.GetNPC();

		std::vector<Form*> collectedForms{};
		Set<RE::FormID>    collectedFormIDs{};

		collectedForms.reserve(vec.size());
		collectedFormIDs.reserve(vec.size());

		for (auto& formData : vec) {
			auto form = formData.form;
			auto formID = form->GetFormID();
			if (collectedFormIDs.contains(formID)) {
				continue;
			}
			if constexpr (std::is_same_v<RE::BGSKeyword, Form>) {
				if (detail::passed_filters(a_npcData, formData) && a_npcData.InsertKeyword(form->GetFormEditorID())) {
					collectedForms.emplace_back(form);
					collectedFormIDs.emplace(formID);

					a_distributables.IncrementDistributionCount(form);
				}
			} else {
				if (detail::passed_filters(a_npcData, formData) && !detail::has_form(npc, form) && collectedFormIDs.emplace(formID).second) {
					collectedForms.emplace_back(form);
					a_distributables.IncrementDistributionCount(form);
				}
			}
		}

		if (!collectedForms.empty()) {
			a_callback(collectedForms);
		}
	}

	void Distribute(NPCData& a_npcData);
	void DistributeInventory(const NPCData& a_npcData);
}
