#include "LookupNPC.h"

namespace NPC
{
	Data::ID::ID(const RE::TESNPC* a_npc) :
		formID(a_npc->GetFormID()),
		editorID(a_npc->GetFormEditorID())
	{}

	bool Data::ID::contains(const std::string& a_str) const
	{
		return string::icontains(editorID, a_str);
	}

	bool Data::ID::operator==(const RE::TESFile* a_mod) const
	{
		return a_mod->IsFormInMod(formID);
	}

	bool Data::ID::operator==(const std::string& a_str) const
	{
		return string::iequals(editorID, a_str);
	}

	bool Data::ID::operator==(RE::FormID a_formID) const
	{
		return formID == a_formID;
	}

	Data::Data(RE::TESNPC* a_npc) :
		npc(a_npc),
		race(a_npc->GetRace()),
		IDs(a_npc),
		level(a_npc->GetLevel()),
		sex(a_npc->GetSex()),
		unique(a_npc->IsUnique()),
		child(race->formEditorID.contains("ChildRace")),
		leveled(a_npc->GetUsesLeveledTemplate())
	{
		a_npc->ForEachKeyword([&](const RE::BGSKeyword* a_keyword) {
			keywords.emplace(a_keyword->GetFormEditorID());
			return RE::BSContainer::ForEachResult::kContinue;
		});
		a_npc->ForEachKeyword([&](const RE::BGSKeyword* a_keyword) {
			keywords.emplace(a_keyword->GetFormEditorID());
			return RE::BSContainer::ForEachResult::kContinue;
		});
	}

	RE::TESNPC* Data::GetNPC() const
	{
		return npc;
	}

	bool Data::has_keyword_string(const std::string& a_string) const
	{
		return std::any_of(keywords.begin(), keywords.end(), [&](const auto& keyword) {
			return string::iequals(keyword, a_string);
		});
	}

	bool Data::HasStringFilter(const StringVec& a_strings, bool a_all) const
	{
		if (a_all) {
			return std::ranges::all_of(a_strings, [&](const auto& str) {
				return has_keyword_string(str) || IDs == str;
			});
		} else {
			return std::ranges::any_of(a_strings, [&](const auto& str) {
				return has_keyword_string(str) || IDs == str;
			});
		}
	}

	bool Data::ContainsStringFilter(const StringVec& a_strings) const
	{
		return std::ranges::any_of(a_strings, [&](const auto& str) {
			return IDs.contains(str) ||
			       std::any_of(keywords.begin(), keywords.end(), [&](const auto& keyword) {
					   return string::icontains(keyword, str);
				   });
		});
	}

	bool Data::InsertKeyword(const char* a_keyword)
	{
		return keywords.emplace(a_keyword).second;
	}

	bool Data::has_form(RE::TESForm* a_form) const
	{
		switch (a_form->GetFormType()) {
		case RE::FormType::kCSTY:
			return npc->GetCombatStyle() == a_form;
		case RE::FormType::kCLAS:
			return npc->npcClass == a_form;
		case RE::FormType::kFACT:
			return npc->IsInFaction(a_form->As<RE::TESFaction>());
		case RE::FormType::kRACE:
			return GetRace() == a_form;
		case RE::FormType::kOTFT:
			return npc->defaultOutfit == a_form;
		case RE::FormType::kNPC_:
			{
				const auto filterFormID = a_form->GetFormID();
				return npc == a_form || IDs == filterFormID;
			}
		case RE::FormType::kVTYP:
			return npc->GetObjectVoiceType() == a_form;
		case RE::FormType::kFLST:
			{
				bool result = false;

				const auto list = a_form->As<RE::BGSListForm>();
				list->ForEachForm([&](RE::TESForm* a_formInList) {
					if (result = has_form(a_formInList); result) {
						return RE::BSContainer::ForEachResult::kStop;
					}
					return RE::BSContainer::ForEachResult::kContinue;
				});

				return result;
			}
		default:
			return false;
		}
	}

	bool Data::HasFormFilter(const FormVec& a_forms, bool all) const
	{
		const auto has_form_or_file = [&](const std::variant<RE::TESForm*, const RE::TESFile*>& a_formFile) {
			if (std::holds_alternative<RE::TESForm*>(a_formFile)) {
				const auto form = std::get<RE::TESForm*>(a_formFile);
				return form && has_form(form);
			}
			if (std::holds_alternative<const RE::TESFile*>(a_formFile)) {
				const auto file = std::get<const RE::TESFile*>(a_formFile);
				return file && IDs == file;
			}
			return false;
		};

		if (all) {
			return std::ranges::all_of(a_forms, has_form_or_file);
		} else {
			return std::ranges::any_of(a_forms, has_form_or_file);
		}
	}

	std::uint16_t Data::GetLevel() const
	{
		return level;
	}

	RE::SEX Data::GetSex() const
	{
		return sex;
	}

	bool Data::IsUnique() const
	{
		return unique;
	}

	bool Data::IsChild() const
	{
		return child;
	}

	bool Data::IsLeveled() const
	{
		return leveled;
	}

	RE::TESRace* Data::GetRace() const
	{
		return race;
	}
}
