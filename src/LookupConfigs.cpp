#include "LookupConfigs.h"

namespace INI
{
	Data::Data(const std::string& a_rawForm, const std::map<std::string, std::string>& a_filters, std::string a_path) :
		rawForm(distribution::get_record(a_rawForm)),
		path(std::move(a_path))
	{
		const auto for_each_filter = [&](const std::string& a_filter, std::function<void(std::string&)> a_callback) {
			if (const auto it = a_filters.find(a_filter); it != a_filters.end()) {
				for (auto& str : distribution::split_entry(it->second)) {
					a_callback(str);
				}
			}
		};

		for_each_filter("StringFilters", [&](auto& str) {
			if (str.contains("+"sv)) {
				auto strings = distribution::split_entry(str, "+");
				stringFilters.ALL.insert(stringFilters.ALL.end(), strings.begin(), strings.end());

			} else if (str.at(0) == '-') {
				str.erase(0, 1);
				stringFilters.NOT.emplace_back(str);

			} else if (str.at(0) == '*') {
				str.erase(0, 1);
				stringFilters.ANY.emplace_back(str);

			} else {
				stringFilters.MATCH.emplace_back(str);
			}
		});

		for_each_filter("FormFilters", [&](auto& IDs) {
			if (IDs.contains("+"sv)) {
				auto splitIDs_ALL = distribution::split_entry(IDs, "+");
				for (auto& IDs_ALL : splitIDs_ALL) {
					rawFormFilters.ALL.push_back(distribution::get_record(IDs_ALL));
				}
			} else if (IDs.at(0) == '-') {
				IDs.erase(0, 1);
				rawFormFilters.NOT.push_back(distribution::get_record(IDs));

			} else {
				rawFormFilters.MATCH.push_back(distribution::get_record(IDs));
			}
		});

		for_each_filter("ActorLevel", [&](auto& str) {
			if (auto actor_level = string::split(str, "/"); actor_level.size() > 1) {
				auto minLevel = string::to_num<std::uint16_t>(actor_level[0]);
				auto maxLevel = string::to_num<std::uint16_t>(actor_level[1]);

				actorLevel = Range(minLevel, maxLevel);
			} else {
				actorLevel = Range(string::to_num<std::uint16_t>(str));
			}
		});

		for_each_filter("ActorValues", [&](auto& str) {
			//av(min/max) -> av min max
			std::ranges::replace_if(
				str, [](unsigned char ch) { return ch == '(' || ch == ')' || ch == '/'; }, ' ');
			if (auto avs = string::split(str, " "); !avs.empty()) {
				auto type = distribution::get_record(avs[0]);
				auto minLevel = string::to_num<float>(avs[1]);
				if (avs.size() > 2) {
					auto maxLevel = string::to_num<float>(avs[2]);
					actorValues.push_back({ type, Range(minLevel, maxLevel) });
				} else {
					actorValues.push_back({ type, Range(minLevel) });
				}
			}
		});

		for_each_filter("Traits", [&](auto& str) {
			switch (string::const_hash(str)) {
			case "M"_h:
			case "-F"_h:
				traits.sex = RE::SEX::kMale;
				break;
			case "F"_h:
			case "-M"_h:
				traits.sex = RE::SEX::kFemale;
				break;
			case "U"_h:
				traits.unique = true;
				break;
			case "-U"_h:
				traits.unique = false;
				break;
			case "C"_h:
				traits.child = true;
				break;
			case "-C"_h:
				traits.child = false;
				break;
			case "L"_h:
				traits.leveled = true;
				break;
			case "-L"_h:
				traits.leveled = false;
				break;
			default:
				break;
			}
		});

		if (const auto it = a_filters.find("ItemCount"); it != a_filters.end() && distribution::is_valid_entry(it->second)) {
			idxOrCount = string::to_num<std::int32_t>(it->second);
		}

		if (const auto it = a_filters.find("Chance"); it != a_filters.end() && distribution::is_valid_entry(it->second)) {
			chance = string::to_num<Chance>(it->second);
		}
	}

	std::pair<bool, bool> ReadConfigs()
	{
		logger::info("{:*^50}", "INI");

		const std::filesystem::path spidFolder{ R"(Data\SPID)" };
		if (!exists(spidFolder)) {
			logger::warn("SPID folder not found...");
			return { false, false };
		}

		const std::vector<std::string> files = distribution::get_configs(R"(Data\SPID)", "");

		if (files.empty()) {
			logger::warn("No .ini files were found within the Data/SPID folder...");
			return { false, false };
		}

		logger::info("{} matching inis found", files.size());

		bool shouldLogErrors{ false };

		for (const auto& path : files) {
			ReadConfig(path);
		}

		return { true, shouldLogErrors };
	}

	void ReadConfig(const std::string& a_path)
	{
		logger::info("\tINI : {}", a_path);

		std::ifstream infile{ a_path };

		if (!infile.good()) {
			logger::error("\t\tcouldn't read INI");
			return;
		}

		auto truncPath = a_path.substr(10);

		std::vector<Section> ini;

		std::uint32_t index{ 0 };
		std::string   line;

		while (std::getline(infile, line)) {
			// Ignore comments and empty lines
			if (line.empty() || line[0] == ';' || line[0] == '#') {
				continue;
			}
			// trim whitespace
			string::trim(line);
			// read section
			if (line[0] == '[' && line[line.size() - 1] == ']') {
				ini.emplace_back(Section{ line.substr(1, line.size() - 2) });
				index++;
			} else if (index > 0) {
				if (const std::size_t delimiterPos = line.find('='); delimiterPos != std::string::npos) {
					auto key = string::trim_copy(line.substr(0, delimiterPos));
					auto value = string::trim_copy(line.substr(delimiterPos + 1));

					ini[index - 1].key_map[key] = value;
				}
			}
		}

		for (auto& [section, values] : ini) {
		    auto pos = section.find('|');
			if (pos == std::string::npos) {
				continue;
			}

			auto type = section.substr(0, pos);
			auto rawForm = section.substr(pos + 1);

			configs[type].emplace_back(rawForm, values, truncPath);
		}
	}
}
