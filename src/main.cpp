
#include "RE/T/TESDataHandler.h"
#include "RE/T/TESForm.h"
#include "SKSE/API.h"
#include "SKSE/Impl/PCH.h"
#include "SKSE/Interfaces.h"
#include "st-random.h"
namespace RNSD::FORMS
{
inline RE::TESGlobal* game_month{};
inline RE::TESGlobal* game_day{};
inline RE::TESGlobal* game_time{};

inline void LoadForms()
{

    game_month = RE::TESForm::LookupByEditorID<RE::TESGlobal>("GameMonth");
    game_day   = RE::TESForm::LookupByEditorID<RE::TESGlobal>("GameDay");
    game_time  = RE::TESForm::LookupByEditorID<RE::TESGlobal>("GameHour");

    if (!game_month || !game_day || !game_time)
    {
        SKSE::stl::report_and_fail("Critical Error: Can NOT fine global variables");
    }
}

} // namespace RNSD::FORMS

void RandomiseGlobal(RE::TESGlobal* a_glob, int a_min, int a_max)
{

    if (!a_glob)
    {
        return;
    }
    a_glob->value = RandomiserUtil::GetRandomInt(a_min, a_max);
}

void SetRandomStart()
{
    using namespace RNSD::FORMS;
    RandomiseGlobal(game_month, 0, 12);
    RandomiseGlobal(game_day, 1, 28);
    RandomiseGlobal(game_time, 0, 23);

    logs::info("{} set to {}, {} set to {}, {} set to {}", game_month->GetFormEditorID(), game_month->value,
               game_day->GetFormEditorID(), game_day->value, game_time->GetFormEditorID(), game_time->value);
}


void Listener(SKSE::MessagingInterface::Message* a_msg)
{

    switch (a_msg->type)
    {
        case SKSE::MessagingInterface::kDataLoaded:
            RNSD::FORMS::LoadForms();
            break;
        case SKSE::MessagingInterface::kNewGame:
            SetRandomStart();
            break;
    }
}

SKSEPluginLoad(const SKSE::LoadInterface* skse)
{
    Init(skse);
    if(!SKSE::GetMessagingInterface()->RegisterListener(Listener){
        return false;
    }
    return true;
}