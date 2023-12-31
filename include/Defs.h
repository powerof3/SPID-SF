#pragma once

// for visting variants
template <class... Ts>
struct overload : Ts...
{
	using Ts::operator()...;
};

template <class K, class D>
using Map = ankerl::unordered_dense::segmented_map<K, D>;
template <class K>
using Set = ankerl::unordered_dense::segmented_set<K>;

struct string_hash
{
	using is_transparent = void;  // enable heterogeneous overloads
	using is_avalanching = void;  // mark class as high quality avalanching hash

	[[nodiscard]] std::uint64_t operator()(std::string_view str) const noexcept
	{
		return ankerl::unordered_dense::hash<std::string_view>{}(str);
	}
};

template <class D>
using StringMap = ankerl::unordered_dense::segmented_map<std::string, D, string_hash, std::equal_to<>>;
using StringSet = ankerl::unordered_dense::segmented_set<std::string, string_hash, std::equal_to<>>;

// Record = FormOrEditorID|StringFilters|RawFormFilters|LevelFilters|Traits|IdxOrCount|Chance

using FormModPair = std::pair<
	std::optional<RE::FormID>,    // formID
	std::optional<std::string>>;  // modName

using FormOrEditorID = std::variant<
	FormModPair,   // formID~modName
	std::string>;  // editorID

using FormOrMod = std::variant<RE::TESForm*,  // form
	const RE::TESFile*>;                      // mod

using Form = std::variant<FormOrEditorID, FormModPair>;

template <class T>
struct Filters
{
	std::vector<T> ALL{};
	std::vector<T> NOT{};
	std::vector<T> MATCH{};
};

using StringVec = std::vector<std::string>;
struct StringFilters : Filters<std::string>
{
	StringVec ANY{};
};

using RawFormVec = std::vector<FormOrEditorID>;
using RawFormFilters = Filters<FormOrEditorID>;

using FormVec = std::vector<FormOrMod>;
using FormFilters = Filters<FormOrMod>;

template <typename T>
struct Range
{
	Range() = default;

	explicit Range(T a_min) :
		min(a_min)
	{}
	Range(T a_min, T a_max) :
		min(a_min),
		max(a_max)
	{}

	[[nodiscard]] bool IsValid() const
	{
		return min > std::numeric_limits<T>::min();  // min must always be valid, max is optional
	}
	[[nodiscard]] bool IsInRange(T value) const
	{
		return value >= min && value <= max;
	}

	// members
	T min{ std::numeric_limits<T>::min() };
	T max{ std::numeric_limits<T>::max() };
};

struct ActorValue
{
	std::variant<FormOrEditorID, RE::ActorValueInfo*> type;
	Range<float>                                      range;
};

struct Traits
{
	std::optional<RE::SEX> sex{};
	std::optional<bool>    unique{};
	std::optional<bool>    child{};
	std::optional<bool>    leveled{};
};

using IdxOrCount = std::int32_t;
using Chance = std::uint32_t;

/// A standardized way of converting any object to string.
///
///	<p>
///	Overload `operator<<` to provide custom formatting for your value.
///	Alternatively, specialize this method and provide your own implementation.
///	</p>
template <typename Value>
std::string describe(Value value)
{
	std::ostringstream os;
	os << value;
	return os.str();
}

inline std::ostream& operator<<(std::ostream& os, RE::TESFile* file)
{
	os << file->fileName;
	return os;
}

inline std::ostream& operator<<(std::ostream& os, RE::TESForm* form)
{
	if (const std::string edid = form->GetFormEditorID(); !edid.empty()) {
		os << edid << " ";
	}
	os << "["
	   << std::to_string(form->GetFormType())
	   << ":"
	   << std::setfill('0')
	   << std::setw(sizeof(RE::FormID) * 2)
	   << std::uppercase
	   << std::hex
	   << form->GetFormID()
	   << "]";

	return os;
}
