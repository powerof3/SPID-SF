#include "DistributeManager.h"
#include "Distribute.h"
#include "FormData.h"

namespace Distribute
{
	namespace LeveledActor
	{
		struct CopyFromTemplateForms
		{
			static void thunk(RE::TESActorBaseData* a_this, RE::TESActorBase** a_templateForms)
			{
				func(a_this, a_templateForms);

				if (!a_this->GetUsesLeveledTemplate() || !a_this->templateForms || !a_templateForms) {
					return;
				}

				if (const auto npc = stl::adjust_pointer<RE::TESNPC>(a_this, -0x0E0); npc) {
					auto npcData = NPCData(npc);
					Distribute(npcData);
				}
			}
			static inline REL::Relocation<decltype(thunk)> func;
			static inline size_t                           idx{ 0xB };
		};

		void Install()
		{
			stl::write_vfunc<RE::TESNPC, CopyFromTemplateForms>(10);
			logger::info("\tHooked leveled actor init");
		}
	}

	void Manager::OnDataLoaded()
	{
		if (Forms::deathItems) {
			logger::info("{:*^50}", "EVENTS");

			RE::TESDeathEvent::GetEventSource()->RegisterSink(GetSingleton());
			logger::info("Registered for {}", typeid(RE::TESDeathEvent).name());
		}

		// Clear logger's buffer to free some memory :)
		buffered_logger::clear();
	}

	RE::BSEventNotifyControl Manager::ProcessEvent(const RE::TESDeathEvent& a_event, RE::BSTEventSource<RE::TESDeathEvent>*)
	{
		constexpr auto is_NPC = [](auto&& a_ref) {
			return a_ref && !a_ref->IsPlayerRef();
		};

		if (a_event.dead && is_NPC(a_event.actorDying)) {
			if (const auto actor = a_event.actorDying->As<RE::Actor>()) {
				const auto npcData = NPCData(actor->GetActorBase());
				for_each_form<RE::TESBoundObject>(npcData, Forms::deathItems, [&](auto* a_deathItem, IdxOrCount a_count) {
					actor->AddObjectToContainer(a_deathItem, nullptr, a_count, nullptr, RE::ITEM_TRANSFER_REASON::kNone);
					return true;
				});
			}
		}

		return RE::BSEventNotifyControl::kContinue;
	}

	void ApplyToNPCs()
	{
		if (const auto dataHandler = RE::TESDataHandler::GetSingleton(); dataHandler) {
			dataHandler->ForEachFormOfType<RE::TESNPC>([](auto* a_npc) {
				if (a_npc && !a_npc->IsPlayer() && !(a_npc->GetUsesLeveledTemplate() || a_npc->IsUnique())) {
					auto npcData = NPCData(a_npc);
				    Distribute(npcData);
				}
				return RE::BSContainer::ForEachResult::kContinue;
			});
		}
	}

}
