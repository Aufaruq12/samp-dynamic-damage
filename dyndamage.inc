/*
    ==================================================
    [INCLUDE] Super Dynamic Distance Damage System
    Author: Faruqq12
    Version: 0.1 Beta
    Description: Damage dinamis berdasarkan jarak & jenis senjata
    ==================================================
*/

#if defined _dyndamage_included
    #endinput
#endif
#define _dyndamage_included

#include <a_samp>

#define RANGE_POINT_BLANK   5.0
#define RANGE_CLOSE         15.0
#define RANGE_MEDIUM        40.0
#define RANGE_LONG          80.0

stock Float:GetWeaponDistanceMultiplier(weaponid, Float:distance)
{
    new Float:multiplier = 1.0;

    switch(weaponid)
    {
        // 1. SHOTGUNS (Pump, Sawn-off, Combat)
        case 25, 26, 27:
        {
            if(distance <= RANGE_POINT_BLANK) multiplier = 1.6;      // +60% Damage (Insta-kill range)
            else if(distance <= RANGE_CLOSE) multiplier = 1.2;       // +20% Damage
            else if(distance <= RANGE_MEDIUM) multiplier = 0.5;      // -50% Damage
            else multiplier = 0.1;                                   // -90% Damage (Peluru menyebar)
        }
        
        // 2. SNIPER & RIFLE (Country Rifle, Sniper)
        case 33, 34:
        {
            if(distance <= RANGE_CLOSE) multiplier = 0.7;            // -30% Damage (Susah no-scope jarak dekat)
            else if(distance <= RANGE_MEDIUM) multiplier = 1.0;      // Damage Normal
            else if(distance <= RANGE_LONG) multiplier = 1.3;        // +30% Damage
            else multiplier = 1.5;                                   // +50% Damage (Makin jauh makin sakit)
        }
        
        // 3. ASSAULT RIFLES (M4, AK-47)
        case 30, 31:
        {
            if(distance <= RANGE_CLOSE) multiplier = 1.1;            // +10% Damage
            else if(distance <= RANGE_MEDIUM) multiplier = 1.0;      // Damage Normal
            else if(distance <= RANGE_LONG) multiplier = 0.8;        // -20% Damage
            else multiplier = 0.6;                                   // -40% Damage
        }
        
        // 4. SMGs & PISTOLS (Deagle, 9mm, Uzi, MP5, dll)
        case 22..24, 28, 29, 32:
        {
            if(distance <= RANGE_CLOSE) multiplier = 1.15;           // +15% Damage
            else if(distance <= RANGE_MEDIUM) multiplier = 0.85;     // -15% Damage
            else multiplier = 0.4;                                   // -60% Damage
        }
        
        // 5. MELEE WEAPONS (Pisau, Katana, Bat, dll)
        case 0..15:
        {
            multiplier = 1.0; // Melee selalu normal karena jaraknya fix
        }
        
        // 6. DEFAULT (Minigun, Flamethrower, dll)
        default:
        {
            if(distance <= RANGE_CLOSE) multiplier = 1.1;
            else if(distance <= RANGE_MEDIUM) multiplier = 1.0;
            else multiplier = 0.8;
        }
    }
    return multiplier;
}

// --- Logika Eksekusi Damage (ALS Hook OnPlayerTakeDamage) ---
public OnPlayerTakeDamage(playerid, issuerid, Float:amount, weaponid, bodypart)
{
    // Pastikan damage berasal dari player lain (bukan jatuh/ledakan mobil)
    if(issuerid != INVALID_PLAYER_ID)
    {
        new Float:ix, Float:iy, Float:iz;
        GetPlayerPos(issuerid, ix, iy, iz);
        new Float:distance = GetPlayerDistanceFromPoint(playerid, ix, iy, iz);

        // Ambil multiplier berdasarkan jarak dan senjata
        new Float:dmg_multiplier = GetWeaponDistanceMultiplier(weaponid, distance);

        // SA-MP secara bawaan sudah mengurangi HP korban sebesar 'amount'.
        // Kita hitung selisih untuk menambahkan atau mengurangi sisa damage.
        new Float:actual_damage = amount * dmg_multiplier;
        new Float:damage_difference = actual_damage - amount;

        if(damage_difference != 0.0)
        {
            ApplyDynamicDamage(playerid, damage_difference);
        }
    }

    // Call next hook
    #if defined DYNDMG_OnPlayerTakeDamage
        return DYNDMG_OnPlayerTakeDamage(playerid, issuerid, amount, weaponid, bodypart);
    #else
        return 1;
    #endif
}

// ALS Hooking Macro
#if defined _ALS_OnPlayerTakeDamage
    #undef OnPlayerTakeDamage
#else
    #define _ALS_OnPlayerTakeDamage
#endif
#define OnPlayerTakeDamage DYNDMG_OnPlayerTakeDamage
#if defined DYNDMG_OnPlayerTakeDamage
    forward DYNDMG_OnPlayerTakeDamage(playerid, issuerid, Float:amount, weaponid, bodypart);
#endif

stock ApplyDynamicDamage(playerid, Float:difference)
{
    new Float:hp, Float:armour;
    GetPlayerHealth(playerid, hp);
    GetPlayerArmour(playerid, armour);

    if(difference > 0.0)
    {
        if(armour > 0.0)
        {
            if(armour >= difference) 
            {
                SetPlayerArmour(playerid, armour - difference);
            }
            else 
            {
                SetPlayerArmour(playerid, 0.0);
                SetPlayerHealth(playerid, hp - (difference - armour));
            }
        }
        else 
        {
            SetPlayerHealth(playerid, hp - difference);
        }
    }
    else if(difference < 0.0)
    {
        new Float:heal_amount = -difference;
        new Float:max_hp = 100.0; // Ganti jika servermu punya sistem max HP > 100

        if(hp + heal_amount > max_hp)
        {
            new Float:overflow = (hp + heal_amount) - max_hp;
            SetPlayerHealth(playerid, max_hp);
            // Sisa heal dikembalikan ke armor agar tidak hangus
            SetPlayerArmour(playerid, armour + overflow); 
        }
        else 
        {
            SetPlayerHealth(playerid, hp + heal_amount);
        }
    }
}
