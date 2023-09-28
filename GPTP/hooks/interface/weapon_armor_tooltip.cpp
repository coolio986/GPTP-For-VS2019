#include "weapon_armor_tooltip.h"
#include <SCBW/api.h>
#include <SCBW/enumerations/WeaponId.h>
#include <cstdio>

#include <SCBW/enumerations.h>

char buffer[128];

//Returns the special damage multiplier factor for units that don't use the
//"Damage Factor" property in weapons.dat.
u8 getDamageFactorForTooltip(u8 weaponId, const CUnit *unit) {
  u8 maxHits = 0;
  if (units_dat::GroundWeapon[unit->id] == weaponId)
    maxHits = units_dat::MaxGroundHits[unit->id];
  else if (units_dat::AirWeapon[unit->id] == weaponId)
    maxHits = units_dat::MaxAirHits[unit->id];

  if (maxHits > 1)
    return maxHits * weapons_dat::DamageFactor[weaponId];

	if (unit->id == UnitId::valkyrie)
		return 1;

	return weapons_dat::DamageFactor[weaponId];
}

//Returns the C-string for the tooltip text of the unit's weapon icon.
//This function is used for weapon icons and special icons.
//Precondition: @p entryStrIndex is a stat_txt.tbl string index.

/*const char* getDamageTooltipString(u8 weaponId, const CUnit *unit, u16 entryStrIndex) {
  const char *entryName = statTxtTbl->getString(entryStrIndex);
  const char *damageStr = statTxtTbl->getString(777);           //"Damage:"

  const u8 damageFactor = getDamageFactorForTooltip(weaponId, unit);
  const u8 upgradeLevel = scbw::getUpgradeLevel(unit->playerId, weapons_dat::DamageUpgrade[weaponId]);
  const u16 baseDamage = weapons_dat::DamageAmount[weaponId] * damageFactor;
  const u16 bonusDamage = weapons_dat::DamageBonus[weaponId] * damageFactor * upgradeLevel;

  //Set damage color
  char damageColor;
  switch (weapons_dat::DamageType[weaponId]) {
    case DamageType::Independent:
      damageColor = GameTextColor::White;
      break;
    case DamageType::Explosive:
      damageColor = GameTextColor::Red;
      break;
    case DamageType::Concussive:
      damageColor = GameTextColor::Yellow;
      break;
    default: //Normal
      damageColor = GameTextColor::RiverBlue;
      break;
    case DamageType::IgnoreArmor:
      damageColor = GameTextColor::Orange;
      break;
  }

  if (bonusDamage > 0)
    sprintf_s(buffer, sizeof(buffer), "%c%s\x01\n%s %d+%d",
              damageColor, entryName, damageStr, baseDamage, bonusDamage);
  else
    sprintf_s(buffer, sizeof(buffer), "%c%s\x01\n%s %d",
              damageColor, entryName, damageStr, baseDamage);

  return buffer;
}
*/
const char* getDamageTooltipString(u8 weaponId, CUnit* unit, u16 entryStrIndex) {
	//Default StarCraft behavior

	const char* entryName = statTxtTbl->getString(entryStrIndex);
	const char* damageStr = statTxtTbl->getString(777);					 //"Damage:"
	const char* perRocketStr = statTxtTbl->getString(1301);	 //"per rocket"

	const u8 damageFactor = getDamageFactorForTooltip(weaponId, unit);
	const u8 upgradeLevel = scbw::getUpgradeLevel(unit->playerId, weapons_dat::DamageUpgrade[weaponId]);
	const u16 baseDamage = weapons_dat::DamageAmount[weaponId] * damageFactor;
	const u16 bonusDamage = weapons_dat::DamageBonus[weaponId] * damageFactor * upgradeLevel;

	if (weaponId == WeaponId::HaloRockets) {

		if (bonusDamage > 0)
			sprintf_s(buffer, sizeof(buffer), "%s\n%s %d+%d %s",
								entryName, damageStr, baseDamage, bonusDamage, perRocketStr);
		else
			sprintf_s(buffer, sizeof(buffer), "%s\n%s %d %s",
								entryName, damageStr, baseDamage, perRocketStr);
	}
	else {
		if (bonusDamage > 0)
			sprintf_s(buffer, sizeof(buffer), "%s\n%s %d+%d",
								entryName, damageStr, baseDamage, bonusDamage);
		else
			sprintf_s(buffer, sizeof(buffer), "%s\n%s %d",
								entryName, damageStr, baseDamage);
	}

	return buffer;
}

namespace hooks {

//Returns the C-string for the tooltip text of the unit's weapon icon.
/*
//Added: Range parameter
const char* getWeaponTooltipString(u8 weaponId, const CUnit *unit) {
  static char buffer2[200];

  u32 baseRange = (weapons_dat::MaxRange[weaponId] + 16) / 32;
  u32 modifiedRange = (unit->getMaxWeaponRange(weaponId) + 16) / 32;

  //Display activation range when Spider Mines are selected
  if (weaponId == WeaponId::SpiderMines) {
    baseRange = units_dat::SeekRange[unit->id];
    modifiedRange = unit->getSeekRange();
  }

  const char* baseTooltipStr = getDamageTooltipString(weaponId, unit, weapons_dat::Label[weaponId]);
  if (baseRange == modifiedRange) {
    if (baseRange != 0) {
      sprintf_s(buffer2, sizeof(buffer2), "%s\nRange: %d",
                baseTooltipStr, baseRange);
    }
    else {
      sprintf_s(buffer2, sizeof(buffer2), "%s\nRange: Melee",
                baseTooltipStr);      
    }
  }
  else {
    sprintf_s(buffer2, sizeof(buffer2), "%s\nRange: %d+%d",
              baseTooltipStr, baseRange, modifiedRange - baseRange);
  }

  return buffer2;
*/

const char* getWeaponTooltipString(u8 weaponId, CUnit* unit) {
	return getDamageTooltipString(weaponId, unit, weapons_dat::Label[weaponId]);
}

//Returns the C-string for the tooltip text of the unit's armor icon.
const char* getArmorTooltipString(CUnit* unit) {
	//Default StarCraft behavior
	
	const u16 labelId = upgrades_dat::Label[units_dat::ArmorUpgrade[unit->id]];
	const char *armorUpgradeName = statTxtTbl->getString(labelId);
	const char *armorStr = statTxtTbl->getString(778);						//"Armor:"

	const u8 baseArmor = units_dat::ArmorAmount[unit->id];
	const u8 bonusArmor = unit->getArmorBonus();

	if (bonusArmor > 0)
		sprintf_s(buffer, sizeof(buffer), "%s\n%s %d+%d",
							armorUpgradeName, armorStr, baseArmor, bonusArmor);
	else
		sprintf_s(buffer, sizeof(buffer), "%s\n%s %d",
							armorUpgradeName, armorStr, baseArmor);

	return buffer;
}


//Returns the C-string for the tooltip text of the plasma shield icon.
const char* getShieldTooltipString(CUnit* unit) {
	//Default StarCraft behavior

	const u16 labelId = upgrades_dat::Label[UpgradeId::ProtossPlasmaShields];
	const char *shieldUpgradeName = statTxtTbl->getString(labelId);
	const char *shieldStr = statTxtTbl->getString(779);					 //"Shields:"

	const u8 shieldUpgradeLevel = scbw::getUpgradeLevel(unit->playerId, UpgradeId::ProtossPlasmaShields);

	if (shieldUpgradeLevel > 0)
		sprintf_s(buffer, sizeof(buffer), "%s\n%s %d+%d",
							shieldUpgradeName, shieldStr, 0, shieldUpgradeLevel);
	else
		sprintf_s(buffer, sizeof(buffer), "%s\n%s %d",
							shieldUpgradeName, shieldStr, 0);

	return buffer;
}

//Returns the C-string for the tooltip text of the Interceptor icon (Carriers),
//Scarab icon (Reavers), Nuclear Missile icon (Nuclear Silos), and Spider Mine
//icon (Vultures).
/*
const char* getSpecialTooltipString(u16 iconUnitId, const CUnit *unit) {
  static char buffer2[200];

  //Add range parameter for Carriers and Reavers
  if (iconUnitId == UnitId::interceptor || iconUnitId == UnitId::scarab) {
    const char *damageTooltipStr;
    if (iconUnitId == UnitId::interceptor)
      damageTooltipStr = getDamageTooltipString(WeaponId::PulseCannon, unit, 791);  //"Interceptors"
    else
      damageTooltipStr = getDamageTooltipString(WeaponId::Scarab, unit, 792);       //"Scarabs"
    
    //Carriers and Reavers use the Target Seek Range
    const u32 baseRange = units_dat::SeekRange[unit->id];
    const u32 modifiedRange = unit->getSeekRange();

    if (baseRange == modifiedRange) {
      sprintf_s(buffer2, sizeof(buffer2), "%s\nRange: %d",
                damageTooltipStr, baseRange);
    }
    else {
      sprintf_s(buffer2, sizeof(buffer2), "%s\nRange: %d+%d",
                damageTooltipStr, baseRange, modifiedRange - baseRange);
    }

    return buffer2;
  }
*/
const char* getSpecialTooltipString(u16 iconUnitId, CUnit* unit) {
	//Default StarCraft behavior

	if (iconUnitId == UnitId::interceptor)
		return getDamageTooltipString(WeaponId::PulseCannon, unit, 791);		//"Interceptors"

	if (iconUnitId == UnitId::scarab)
		return getDamageTooltipString(WeaponId::Scarab, unit, 792);				 //"Scarabs"

	if (iconUnitId == UnitId::nuclear_missile)
		return statTxtTbl->getString(793);																	//"Nukes"

	if (iconUnitId == UnitId::spider_mine)
		return getDamageTooltipString(WeaponId::SpiderMines, unit, 794);		//"Spider Mines"

	//Should never reach here
	return "";

}

} //hooks
