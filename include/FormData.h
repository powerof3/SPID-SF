#pragma once

#include "Cache.h"
#include "LookupConfigs.h"
#include "LookupFilters.h"

namespace Forms
{
	namespace detail
	{
		inline bool formID_to_form(RE::TESDataHandler* a_dataHandler, RawFormVec& a_rawFormVec, FormVec& a_formVec, const std::string& a_path, bool a_all = false)
		{
			if (a_rawFormVec.empty()) {
				return true;
			}

			for (auto& formOrEditorID : a_rawFormVec) {
				std::visit(
					overload{
						[&](FormModPair& formMod) {
							if (auto& [formID, modName] = formMod; modName && !formID) {
								if (const RE::TESFile* filterMod = a_dataHandler->LookupModByName(*modName); filterMod) {
									a_formVec.emplace_back(filterMod);
								} else {
									buffered_logger::error("\t\t[{}] Filter ({}) SKIP - mod cannot be found", a_path, *modName);
								}
							} else if (formID) {
								if (auto filterForm = modName ?
						                                  a_dataHandler->LookupForm(*formID, *modName) :
						                                  RE::TESForm::LookupByID(*formID)) {
									const auto formType = filterForm->GetFormType();
									if (Cache::FormType::GetWhitelisted(formType)) {
										a_formVec.emplace_back(filterForm);
									} else {
										buffered_logger::error("\t\t[{}] Filter [0x{:X}] ({}) SKIP - invalid formtype ({})", a_path, *formID, modName.value_or(""), formType);
									}
								} else {
									buffered_logger::error("\t\t[{}] Filter [0x{:X}] ({}) SKIP - form doesn't exist", a_path, *formID, modName.value_or(""));
								}
							}
						},
						[&](std::string& editorID) {
							if (auto filterForm = RE::TESForm::LookupByEditorID(editorID); filterForm) {
								const auto formType = filterForm->GetFormType();
								if (Cache::FormType::GetWhitelisted(formType)) {
									a_formVec.emplace_back(filterForm);
								} else {
									buffered_logger::error("\t\t[{}] Filter ({}) SKIP - invalid formtype ({})", a_path, editorID, formType);
								}
							} else {
								buffered_logger::error("\t\t[{}] Filter ({}) SKIP - form doesn't exist", a_path, editorID);
							}
						} },
					formOrEditorID);
			}
			return !a_all && !a_formVec.empty() || a_formVec.size() == a_rawFormVec.size();
		}
	}

	template <class Form>
	struct Data
	{
		Form*       form{ nullptr };
		IdxOrCount  idxOrCount{ 1 };
		FilterData  filters{};
		std::string path{};

		bool operator==(const Data& a_rhs) const;
	};

	template <class Form>
	using DataVec = std::vector<Data<Form>>;

	template <class Form>
	struct Distributables
	{
		Distributables(RECORD::TYPE a_type) :
			type(a_type)
		{}

		explicit operator bool();

		std::size_t    GetSize() const;
		RECORD::TYPE   GetType() const;
		DataVec<Form>& GetForms();

		void LookupForms(RE::TESDataHandler* a_dataHandler, std::string_view a_type, INI::DataVec& a_INIDataVec);
		void FinishLookupForms();

		void IncrementDistributionCount(Form* a_form);
		void LogNumDistributed(const std::size_t a_totalNPCCount) const;

	private:
		RECORD::TYPE              type;
		DataVec<Form>             forms{};
		Map<Form*, std::uint32_t> countMap{};
	};

	inline Distributables<RE::SpellItem>      spells{ RECORD::kSpell };
	inline Distributables<RE::BGSPerk>        perks{ RECORD::kPerk };
	inline Distributables<RE::TESBoundObject> items{ RECORD::kItem };
	inline Distributables<RE::BGSOutfit>      outfits{ RECORD::kOutfit };
	inline Distributables<RE::BGSKeyword>     keywords{ RECORD::kKeyword };
	inline Distributables<RE::TESBoundObject> deathItems{ RECORD::kDeathItem };
	inline Distributables<RE::TESFaction>     factions{ RECORD::kFaction };

	std::size_t GetTotalEntries();
	std::size_t GetTotalLeveledEntries();

	template <typename Func, typename... Args>
	void ForEachDistributable(Func&& a_func, Args&&... args)
	{
		a_func(keywords, std::forward<Args>(args)...);
		a_func(spells, std::forward<Args>(args)...);
		a_func(perks, std::forward<Args>(args)...);
		a_func(items, std::forward<Args>(args)...);
		a_func(deathItems, std::forward<Args>(args)...);
		a_func(outfits, std::forward<Args>(args)...);
		a_func(factions, std::forward<Args>(args)...);
	}
}

template <class Form>
bool Forms::Data<Form>::operator==(const Data& a_rhs) const
{
	if (!form || !a_rhs.form) {
		return false;
	}
	return form->GetFormID() == a_rhs.form->GetFormID();
}

template <class Form>
Forms::Distributables<Form>::operator bool()
{
	return !forms.empty();
}

template <class Form>
std::size_t Forms::Distributables<Form>::GetSize() const
{
	return forms.size();
}

template <class Form>
RECORD::TYPE Forms::Distributables<Form>::GetType() const
{
	return type;
}

template <class Form>
Forms::DataVec<Form>& Forms::Distributables<Form>::GetForms()
{
	return forms;
}

template <class Form>
void Forms::Distributables<Form>::LookupForms(RE::TESDataHandler* a_dataHandler, std::string_view a_type, INI::DataVec& a_INIDataVec)
{
	if (a_INIDataVec.empty()) {
		return;
	}

	logger::info("{}", a_type);

	forms.reserve(a_INIDataVec.size());

	for (auto& [formOrEditorID, strings, filterIDs, actorLevel, actorValues, traits, idxOrCount, chance, path] : a_INIDataVec) {
		Form* form = nullptr;

		std::visit(
			overload{
				[&](FormModPair& formMod) {
					auto& [formID, modName] = formMod;
					if (modName) {
						if constexpr (std::is_same_v<Form, RE::TESBoundObject>) {
							const auto base = a_dataHandler->LookupForm(*formID, *modName);
							form = base ? static_cast<Form*>(base) : nullptr;
						} else {
							form = a_dataHandler->LookupForm<Form>(*formID, *modName);
						}
					} else {
						if constexpr (std::is_same_v<Form, RE::TESBoundObject>) {
							const auto base = RE::TESForm::LookupByID(*formID);
							form = base ? static_cast<Form*>(base) : nullptr;
						} else {
							form = RE::TESForm::LookupByID<Form>(*formID);
						}
					}
					if (!form) {
						buffered_logger::error("\t[{}] [0x{:X}] ({}) FAIL - formID doesn't exist", path, *formID, modName.value_or(""));
					} else if constexpr (std::is_same_v<Form, RE::BGSKeyword>) {
						if (string::is_empty(form->GetFormEditorID())) {
							form = nullptr;
							buffered_logger::error("\t[{}] [0x{:X}] ({}) FAIL - keyword does not have a valid editorID", path, *formID, modName.value_or(""));
						}
					}
				},
				[&](const std::string& editorID) {
					if constexpr (std::is_same_v<Form, RE::BGSKeyword>) {
						RE::BGSKeyword* foundKeyword = nullptr;

						a_dataHandler->ForEachFormOfType<RE::BGSKeyword>([&](auto* keyword) {
							if (keyword->formEditorID == editorID) {
								foundKeyword = keyword;
								return RE::BSContainer::ForEachResult::kStop;
							}
							return RE::BSContainer::ForEachResult::kContinue;
						});

						if (foundKeyword) {
							form = foundKeyword;
						} else {
							buffered_logger::critical("\t[{}] {} INFO - creating keyword", path, editorID);
						    if (auto keyword = a_dataHandler->CreateFormOfType<RE::BGSKeyword>(0); keyword != nullptr) {
								keyword->SetFormEditorID(editorID.c_str());
								a_dataHandler->AddFormToDataHandler(keyword);

								form = keyword;
							} else {
								buffered_logger::critical("\t[{}] {} FAIL - couldn't create keyword", path, editorID);
							}
						}
					} else {
						form = RE::TESForm::LookupByEditorID<Form>(editorID);
						if constexpr (std::is_same_v<Form, RE::TESBoundObject>) {
							if (!form) {
								const auto base = RE::TESForm::LookupByEditorID(editorID);
								form = base ? static_cast<Form*>(base) : nullptr;
							}
						}
						if (!form) {
							buffered_logger::error("\t[{}] {} FAIL - editorID doesn't exist", path, editorID);
						}
					}
				} },
			formOrEditorID);

		if (!form) {
			continue;
		}

		FormFilters filterForms{};

		bool validEntry = detail::formID_to_form(a_dataHandler, filterIDs.ALL, filterForms.ALL, path, true);
		if (validEntry) {
			validEntry = detail::formID_to_form(a_dataHandler, filterIDs.NOT, filterForms.NOT, path);
		}
		if (validEntry) {
			validEntry = detail::formID_to_form(a_dataHandler, filterIDs.MATCH, filterForms.MATCH, path);
		}

		if (!validEntry) {
			continue;
		}

		for (auto& [av, range] : actorValues) {
			if (auto* avID = std::get_if<FormOrEditorID>(&av)) {
				std::visit(
					overload{
						[&](FormModPair& formMod) {
							auto& [formID, modName] = formMod;
							if (modName) {
								av = a_dataHandler->LookupForm<RE::ActorValueInfo>(*formID, *modName);
							} else {
								av = RE::TESForm::LookupByID<RE::ActorValueInfo>(*formID);
							}
						},
						[&](const std::string& editorID) {
							av = RE::TESForm::LookupByEditorID<RE::ActorValueInfo>(editorID);
						} },
					*avID);
			}
		}

		forms.emplace_back(form, idxOrCount, FilterData(strings, filterForms, actorLevel, actorValues, traits, chance), path);
		countMap.emplace(form, 0);
	}
}

template <class Form>
void Forms::Distributables<Form>::FinishLookupForms()
{
	if (forms.empty()) {
		return;
	}

	// reverse overridable form vectors, winning configs first (Zzzz -> Aaaa)
	// entry order within config is preserved
	if constexpr (std::is_same_v<RE::BGSOutfit, Form> || std::is_same_v<RE::TESObjectARMO, Form>) {
		std::stable_sort(forms.begin(), forms.end(), [](const auto& a_form1, const auto& a_form2) {
			return a_form1.path > a_form2.path;  // Compare paths in reverse order
		});
	}
}

template <class Form>
void Forms::Distributables<Form>::IncrementDistributionCount(Form* a_form)
{
	++countMap[a_form];
}

template <class Form>
void Forms::Distributables<Form>::LogNumDistributed(const std::size_t a_totalNPCCount) const
{
	if (forms.empty()) {
		return;
	}

    logger::info("{}", RECORD::add[GetType()]);

	for (auto& [form, count] : countMap) {
		logger::info("\t{} [0x{:X}] added to {}/{} NPCs", form->GetFormEditorID(), form->GetFormID(), count, a_totalNPCCount);
	}
}
