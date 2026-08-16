
#include "RE/R/RaceSexMenu.h"
#include "RE/T/TESDataHandler.h"
#include "RE/T/TESForm.h"
#include "RE/U/UIMessage.h"
#include "REL/Relocation.h"
#include "SKSE/API.h"
#include "SKSE/Impl/PCH.h"
#include "SKSE/Interfaces.h"
#include "st-hooks.h"
#include "st-random.h"

bool wasChanged              = false;
static int cached_start_time = -1;

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
    RandomiseGlobal(game_month, 1, 12);

    int mo      = static_cast<int>(game_month->value);
    int max_day = 28;
    switch (mo)
    {

        case 1:
        case 3:
        case 5:
        case 7:
        case 8:
        case 10:
        case 12:
            max_day = 31;
            break;
        case 2:
            max_day = 28;
            break;
        default:
            max_day = 30;
            break;
    }


    RandomiseGlobal(game_day, 1, max_day);
    RandomiseGlobal(game_time, 0, 23);

    cached_start_time = game_time->value;

    logs::info("{} set to {}, {} set to {}, {} set to {}", game_month->GetFormEditorID(), game_month->value,
               game_day->GetFormEditorID(), game_day->value, game_time->GetFormEditorID(), game_time->value);
}


namespace RNSD
{
struct RaceMenuHook
{

    static inline void InstallHook() { HookUtils::WriteVFunc<RE::RaceSexMenu, 0, 0x4, RaceMenuHook>(); }

    static inline RE::UI_MESSAGE_RESULTS Call(RE::RaceSexMenu* a_this, RE::UIMessage& a_message)
    {

        if (a_message.type == RE::UI_MESSAGE_TYPE::kShow)
        {
            if (static_cast<int>(RNSD::FORMS::game_time->value) != cached_start_time)
            {
                wasChanged = false;
            }
            else
            {
                wasChanged = true;
            }
        }

        if (a_message.type == RE::UI_MESSAGE_TYPE::kHide && !wasChanged)
        {
            wasChanged = true;
            RandomiseGlobal(RNSD::FORMS::game_time, 0, 23);
        }
        return func(a_this, a_message);
    }
    static inline REL::Relocation<decltype(Call)> func;
};
} // namespace RNSD


void Listener(SKSE::MessagingInterface::Message* a_msg)
{

    switch (a_msg->type)
    {
        case SKSE::MessagingInterface::kDataLoaded:
            RNSD::FORMS::LoadForms();
            break;
        case SKSE::MessagingInterface::kNewGame:
            wasChanged = false;
            SetRandomStart();
            break;
    }
}

SKSEPluginLoad(const SKSE::LoadInterface* skse)
{
    Init(skse);
    RNSD::RaceMenuHook::InstallHook();
    if (!SKSE::GetMessagingInterface()->RegisterListener(Listener))
    {
        return false;
    }
    return true;
}