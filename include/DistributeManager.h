#pragma once

namespace Distribute
{
	inline std::once_flag  initDist;

	namespace LeveledActor
	{
		void Install();
	}

	class Manager :
		public ISingleton<Manager>,
		public RE::BSTEventSink<RE::TESDeathEvent>
	{
	public:
		static void OnDataLoaded();

	private:
		RE::BSEventNotifyControl ProcessEvent(const RE::TESDeathEvent& a_event, RE::BSTEventSource<RE::TESDeathEvent>*) override;
	};

	void ApplyToNPCs();
}
