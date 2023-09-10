#include "spammers.h"

void spammers::clan_tag()
{
    auto apply = [](const char* tag) -> void
    {
        using Fn = int(__fastcall*)(const char*, const char*);
        static auto fn = reinterpret_cast<Fn>(util::FindSignature(crypt_str("engine.dll"), crypt_str("53 56 57 8B DA 8B F9 FF 15")));

        fn(tag, tag);
    };

    static auto removed = false;

    if (!c_config::get()->b["misc_clantag"] && !removed)
    {
        removed = true;
        apply(crypt_str(""));
        return;
    }

    auto nci = m_engine()->GetNetChannelInfo();

    if (!nci)
        return;

    static auto time = -1;

    auto ticks = TIME_TO_TICKS(nci->GetAvgLatency(FLOW_OUTGOING)) + (float)m_globals()->m_tickcount; //-V807
    auto intervals = 0.35f / m_globals()->m_intervalpertick;

    if (c_config::get()->b["misc_clantag"])
    {
        auto main_time = (int)(ticks / intervals) % 30;

        if (main_time != time && !m_clientstate()->iChokedCommands)
        {
            auto tag = crypt_str("");
            switch (main_time)
            {
            case 0: tag = crypt_str("sense          "); break;
            case 1: tag = crypt_str("ense           "); break;
            case 2: tag = crypt_str("nse            "); break;
            case 3: tag = crypt_str("se             "); break;
            case 4: tag = crypt_str("e              "); break;
            case 5: tag = crypt_str("               "); break;
            case 6: tag = crypt_str("             ga"); break;
            case 7: tag = crypt_str("            gam"); break;
            case 8: tag = crypt_str("           game"); break;
            case 9: tag = crypt_str("          games"); break;
            case 10:tag = crypt_str("         gamese"); break;
            case 11:tag = crypt_str("        gamesen"); break;
            case 12:tag = crypt_str("       gamesens"); break;
            case 13:tag = crypt_str("      gamesense"); break;
            case 14:tag = crypt_str("      gamesense"); break;
            case 15:tag = crypt_str("     gamesense "); break;
            case 16:tag = crypt_str("     gamesense "); break;
            case 17:tag = crypt_str("    gamesense  "); break;
            case 18:tag = crypt_str("    gamesense  "); break;
            case 19:tag = crypt_str("   gamesense   "); break;
            case 20:tag = crypt_str("   gamesense   "); break;
            case 21:tag = crypt_str("  gamesense    "); break;
            case 22:tag = crypt_str("  gamesense    "); break;
            case 23:tag = crypt_str(" gamesense     "); break;
            case 24:tag = crypt_str(" gamesense     "); break;
            case 25:tag = crypt_str("gamesense      "); break;
            case 26:tag = crypt_str("gamesense      "); break;
            case 27:tag = crypt_str("amesense       "); break;
            case 28:tag = crypt_str("mesense        "); break;
            case 29:tag = crypt_str("esense         "); break;
            }

            apply(tag);
            time = main_time;
        }
        removed = false;
    }
}