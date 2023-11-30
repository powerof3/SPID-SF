#include "DistributeManager.h"
#include "LookupConfigs.h"
#include "LookupForms.h"

bool shouldLookupForms{ false };
bool shouldLogErrors{ false };
bool shouldDistribute{ false };

void MessageHandler(SFSE::MessagingInterface::Message* a_message)
{
	switch (a_message->type) {
	case SFSE::MessagingInterface::kPostLoad:
		{
			logger::info("{:*^50}", "DEPENDENCIES");

			const auto tweaks = SFSE::WinAPI::GetModuleHandle(L"po3_Tweaks");
			logger::info("powerofthree's Tweaks (po3_tweaks) detected : {}", tweaks != nullptr);

			if (std::tie(shouldLookupForms, shouldLogErrors) = INI::ReadConfigs(); shouldLookupForms) {
				logger::info("{:*^50}", "HOOKS");
				Distribute::LeveledActor::Install();
			}
		}
		break;
	case SFSE::MessagingInterface::kPostDataLoad:
		{
			if (shouldLookupForms && Lookup::LookupSuccess) {
				Distribute::ApplyToNPCs();
				Distribute::Manager::OnDataLoaded();
			}

			if (shouldLogErrors) {
				const auto error = std::format("[SPID] Errors found when reading configs. Check {}.log in {} for more info\n", Version::PROJECT, SFSE::log::log_directory()->string());
				RE::ConsoleLog::GetSingleton()->Print(error.c_str());
			}
		}
		break;
	default:
		break;
	}
}

DLLEXPORT constinit auto SFSEPlugin_Version = []() noexcept {
	SFSE::PluginVersionData data{};

	data.PluginVersion(Version::MAJOR);
	data.PluginName(Version::PROJECT);
	data.AuthorName("powerofthree");
	data.UsesSigScanning(false);
	data.UsesAddressLibrary(true);
	data.HasNoStructUse(false);
	data.IsLayoutDependent(true);
	data.CompatibleVersions({ SFSE::RUNTIME_LATEST });

	return data;
}();

DLLEXPORT bool SFSEAPI SFSEPlugin_Load(const SFSE::LoadInterface* a_sfse)
{
	SFSE::Init(a_sfse);

	logger::info("Game version : {}", a_sfse->RuntimeVersion().string());
	logger::info("Plugin version : {}", Version::NAME);

	const auto messaging = SFSE::GetMessagingInterface();
	messaging->RegisterListener(MessageHandler);

	return true;
}
