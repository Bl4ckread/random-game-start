#include "RE/B/BaseFormComponent.h"
#include "RE/R/RaceSexMenu.h"
#include "RE/T/TESForm.h"
#include "REL/THook.h"
#include "REX/FTomlSettingStore.h"
#include "REX/REX.h"
#include "SKSE/Interfaces.h"

namespace RNSD
{

namespace CONFIG
{

inline REX::TOML::Bool randomise_time{"General", "bRandomiseTime", true};
inline REX::TOML::Bool randomise_month{"General", "bRandomiseMonth", true};
inline REX::TOML::Bool randomise_day{"General", "bRandomiseDay", true};

inline void LoadConfig()
{

    auto toml = REX::TSingleton<REX::FTomlSettingStore>::GetSingleton();
    toml->Init("Data/SKSE/Plugins/time-randomiser.toml", "Data/SKSE/Plugins/time-randomiser_custom.toml");
    toml->Load();
}
} // namespace CONFIG

bool wasChanged              = false;
bool started_fresh           = false;
static int cached_start_time = -1;

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
    auto cal = RE::Calendar::GetSingleton();

    if (!cal)
    {
        REX::FAIL("Critical Error: Calendar singleton unavailable");
    }

    int orig_month = static_cast<int>(cal->gameMonth->value);
    int orig_day   = static_cast<int>(cal->gameDay->value);

    if (CONFIG::randomise_month.GetValue())
    {
        RandomiseGlobal(cal->gameMonth, RE::Calendar::Months::kMorningStar, RE::Calendar::Months::kEveningStar);
    }

    int mo      = static_cast<int>(cal->gameMonth->value);
    int max_day = RE::Calendar::DAYS_IN_MONTH[mo];

    if (CONFIG::randomise_day.GetValue())
    {
        RandomiseGlobal(cal->gameDay, 1, max_day);
    }

    if (CONFIG::randomise_month.GetValue() || CONFIG::randomise_day.GetValue())
    {
        int new_month = static_cast<int>(cal->gameMonth->value);
        int new_day   = static_cast<int>(cal->gameDay->value);
        int delta = CalendarUtil::GetDayOfYear(new_month, new_day) - CalendarUtil::GetDayOfYear(orig_month, orig_day);

        if (delta != 0)
        {
            cal->rawDaysPassed += static_cast<float>(delta);
        }
    }

    if (CONFIG::randomise_time.GetValue())
    {
        RandomiseGlobal(cal->gameHour, 0, 23);
        cached_start_time = static_cast<int>(cal->gameHour->value);
    }

    REX::INFO("Start set to {} {} ({}), hour {}", cal->GetMonthName(), cal->GetDay(), cal->GetDayName(),
              cal->GetHour());
}

struct RaceMenuHook
{
    static inline RE::UI_MESSAGE_RESULTS Call(RE::RaceSexMenu* a_this, RE::UIMessage& a_message)
    {
        auto* calendar = RE::Calendar::GetSingleton();

        if (a_message.type == RE::UI_MESSAGE_TYPE::kShow)
        {
            wasChanged = (static_cast<int>(calendar->gameHour->value) == cached_start_time);
        }

        if (a_message.type == RE::UI_MESSAGE_TYPE::kHide)
        {

            if (!wasChanged && started_fresh && CONFIG::randomise_time.GetValue())
            {
                RandomiseGlobal(calendar->gameHour, 0, 23);
            }
            started_fresh = false;
        }
        return func(a_this, a_message);
    }
    static inline REL::THookVFT func{RE::RaceSexMenu::VTABLE[0], 0x4, Call};
};

} // namespace RNSD

void SetGlobalsToStart()
{


    auto time  = RE::TESForm::LookupByEditorID<RE::TESGlobal>("GameHour");
    auto month = RE::TESForm::LookupByEditorID<RE::TESGlobal>("GameMonth");
    auto day   = RE::TESForm::LookupByEditorID<RE::TESGlobal>("GameDay");

    if (day)
        day->value = 0;
    if (time)
        time->value = 0;
    if (month)
        month->value = 0;
}

void Listener(SKSE::MessagingInterface::Message* a_msg)
{

    switch (a_msg->type)
    {

        case SKSE::MessagingInterface::kDataLoaded:
            SetGlobalsToStart();
            break;
        case SKSE::MessagingInterface::kNewGame:
            SetGlobalsToStart();
            RNSD::wasChanged    = false;
            RNSD::started_fresh = true;
            RNSD::SetRandomStart();
            break;
        case SKSE::MessagingInterface::kPostLoadGame:
            RNSD::started_fresh = false;
            break;
    }
}

SKSE_PLUGIN_LOAD(const SKSE::LoadInterface* skse)
{
    Init(skse);
    RNSD::CONFIG::LoadConfig();
    if (!SKSE::GetMessagingInterface()->RegisterListener(Listener))
    {
        return false;
    }
    return true;
}