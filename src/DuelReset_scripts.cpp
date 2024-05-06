/*
 *  Originally written  for TrinityCore by ShinDarth and GigaDev90 (www.trinitycore.org)
 *  Converted as module for AzerothCore by ShinDarth and Yehonal   (www.azerothcore.org)
 *  Reworked by Gozzim
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as published
 *  by the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "ScriptMgr.h"
#include "Player.h"
#include "Pet.h"
#include "SpellInfo.h"
#include "DuelReset.h"

class DuelResetAfterConfigLoad : public WorldScript
{
public:
    DuelResetAfterConfigLoad() : WorldScript("DuelResetAfterConfigLoad") { }

    void OnAfterConfigLoad(bool reload) override
    {
        sDuelReset->LoadConfig(reload);
    }

    void OnStartup() override
    {
        sDuelReset->LoadConfig(false);
    }
};

class DuelResetScript : public PlayerScript {
public:
    DuelResetScript() : PlayerScript("DuelResetScript") {}

    // Called when a duel starts (after 3s countdown)
    void OnDuelStart(Player *player1, Player *player2) override {
        // Cooldowns reset
        if (sDuelReset->GetResetCooldownsEnabled())
        {
            sDuelReset->ResetSpellCooldowns(player1);
            sDuelReset->ResetSpellCooldowns(player2);
        }

        // Health and mana reset
        if (sDuelReset->GetResetHealthEnabled())
        {
            player1->ResetAllPowers();
            player2->ResetAllPowers();
        }
    }

    // Called when a duel ends
    void OnDuelEnd(Player *winner, Player *loser, DuelCompleteType type) override
    {
        if (sDuelReset->GetResetHealthEnabled())
        {
            winner->ResetAllPowers();
            loser->ResetAllPowers();
        }
        if (sDuelReset->GetResetCooldownsEnabled())
        {
            sDuelReset->ResetSpellCooldowns(winner);
            sDuelReset->ResetSpellCooldowns(loser);
        }
    }
};

void AddSC_DuelReset()
{
    new DuelResetAfterConfigLoad();
    new DuelResetScript();
}
