#pragma once

namespace NPC
{
	struct Data
	{
		Data(RE::TESNPC* a_npc);
		Data(RE::Actor* a_actor);

		[[nodiscard]] RE::TESNPC* GetNPC() const;

		[[nodiscard]] bool HasStringFilter(const StringVec& a_strings, bool a_all = false) const;
		[[nodiscard]] bool ContainsStringFilter(const StringVec& a_strings) const;
		bool               InsertKeyword(const char* a_keyword);
		[[nodiscard]] bool HasFormFilter(const FormVec& a_forms, bool all = false) const;

		[[nodiscard]] std::uint16_t GetLevel() const;
		[[nodiscard]] RE::SEX       GetSex() const;
		[[nodiscard]] bool          IsUnique() const;
		[[nodiscard]] bool          IsChild() const;
		[[nodiscard]] bool          IsLeveled() const;

		[[nodiscard]] RE::TESRace* GetRace() const;

	private:
		struct ID
		{
			ID() = default;
			explicit ID(const RE::TESNPC* a_npc);

			[[nodiscard]] bool contains(const std::string& a_str) const;

			bool operator==(const RE::TESFile* a_mod) const;
			bool operator==(const std::string& a_str) const;
			bool operator==(RE::FormID a_formID) const;

			RE::FormID  formID{ 0 };
			std::string editorID{};
		};

		[[nodiscard]] bool has_keyword_string(const std::string& a_string) const;
		[[nodiscard]] bool has_form(RE::TESForm* a_form) const;

		RE::TESNPC*   npc;
		RE::TESRace*  race;
		ID            IDs;
		StringSet     keywords{};
		std::uint16_t level;
		RE::SEX       sex;
		bool          unique;
		bool          child;
		bool          leveled;
	};
}

using NPCData = NPC::Data;
