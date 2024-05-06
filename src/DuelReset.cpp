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

#include "DuelReset.h"
#include "GameTime.h"

DuelReset* DuelReset::instance()
{
    static DuelReset instance;
    return &instance;
}

void DuelReset::ResetSpellCooldowns(Player* player)
{
    uint32 infTime = GameTime::GetGameTimeMS().count() + infinityCooldownDelayCheck;
    SpellCooldowns::iterator itr, next;

    for (itr = player->GetSpellCooldownMap().begin(); itr != player->GetSpellCooldownMap().end(); itr = next)
    {
        next = itr;
        ++next;
        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(itr->first);
        if (!spellInfo)
            continue;

        // Get correct spell cooldown times
        uint32 remainingCooldown = player->GetSpellCooldownDelay(spellInfo->Id);
        int32 totalCooldown = spellInfo->RecoveryTime;
        int32 categoryCooldown = spellInfo->CategoryRecoveryTime;
        player->ApplySpellMod(spellInfo->Id, SPELLMOD_COOLDOWN, totalCooldown, nullptr);
        if (int32 cooldownMod = player->GetTotalAuraModifier(SPELL_AURA_MOD_COOLDOWN))
            totalCooldown += cooldownMod * IN_MILLISECONDS;

        if (!spellInfo->HasAttribute(SPELL_ATTR6_NO_CATEGORY_COOLDOWN_MODS))
            player->ApplySpellMod(spellInfo->Id, SPELLMOD_COOLDOWN, categoryCooldown, nullptr);

        // Clear cooldown if < 10min & (passed time > 30sec or onDuelEnd)
        if (remainingCooldown > 0
            && itr->second.end < infTime
            )
            player->RemoveSpellCooldown(itr->first, true);
    }

    if (Pet* pet = player->GetPet())
    {
        for (CreatureSpellCooldowns::const_iterator itr2 = pet->m_CreatureSpellCooldowns.begin(); itr2 != pet->m_CreatureSpellCooldowns.end(); ++itr2)
            player->SendClearCooldown(itr2->first, pet);

        // actually clear cooldowns
        pet->m_CreatureSpellCooldowns.clear();
    }
}

void DuelReset::LoadConfig(bool /*reload*/)
{
    m_enableCooldowns = sConfigMgr->GetOption<bool>("DuelReset.Cooldowns", true);
    m_enableHealth = sConfigMgr->GetOption<bool>("DuelReset.HealthMana", true);
    m_cooldownAge = sConfigMgr->GetOption<uint32>("DuelReset.CooldownAge", 30);

    FillWhitelist(sConfigMgr->GetOption<std::string>("DuelReset.Zones", "0"), m_zoneWhitelist);
    FillWhitelist(sConfigMgr->GetOption<std::string>("DuelReset.Areas", "12;14;809"), m_areaWhitelist);
}

void DuelReset::FillWhitelist(std::string zonesAreas, std::vector<uint32> &whitelist)
{
    whitelist.clear();

    if (zonesAreas.empty())
        return;

    std::string zone;
    std::istringstream zoneStream(zonesAreas);
    while (std::getline(zoneStream, zone, ';'))
    {
        whitelist.push_back(stoi(zone));
    }
}

bool DuelReset::IsAllowedInArea(Player* player) const
{
    return (std::find(m_zoneWhitelist.begin(), m_zoneWhitelist.end(), player->GetZoneId()) != m_zoneWhitelist.end())
        || (std::find(m_areaWhitelist.begin(), m_areaWhitelist.end(), player->GetAreaId()) != m_areaWhitelist.end())
        || m_zoneWhitelist.empty();
}

bool DuelReset::GetResetCooldownsEnabled() const
{
    return m_enableCooldowns;
}

bool DuelReset::GetResetHealthEnabled() const
{
    return m_enableHealth;
}

uint32 DuelReset::GetCooldownAge() const
{
    return m_cooldownAge;
}

std::vector<uint32> DuelReset::GetZoneWhitelist() const
{
    return m_zoneWhitelist;
}

std::vector<uint32> DuelReset::GetAreaWhitelist() const
{
    return m_areaWhitelist;
}
