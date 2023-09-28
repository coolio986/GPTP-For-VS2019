#include "weapon_cooldown.h"
#include "../../SCBW/scbwdata.h"
#include "../../SCBW/enumerations.h"

//V241 for VS2008

//KYSXD helpers
static const u32 terranStimModifier_cooldown = 5;
//acidSpore, ensnare, stim, upgrade modifiers.
static const u32 acidSporeModifier_weapon(const CUnit *unit, u8 weaponId) {
	u32 acidSporeIncreaseModifier = 0;
	if (unit->acidSporeCount) {
		u32 cooldown = weapons_dat::Cooldown[weaponId];
		acidSporeIncreaseModifier = (cooldown >> 3) * unit->acidSporeCount;
	}
	return acidSporeIncreaseModifier;
}

const u32 stimModifier_weapon(const CUnit *unit) {
	u32 finalStimModifier = 10;
	if(unit->stimTimer) {
		switch(unit->id) {
			//Normal stim for regular units (terran units):
			//decrease cd by 50 per cent
			case UnitId::TerranMarine:
			case UnitId::TerranFirebat:
			case UnitId::Hero_JimRaynorMarine:
			case UnitId::Hero_GuiMontag:
				finalStimModifier = terranStimModifier_cooldown;
				break;
			default:
				break;
		}
	}
	return finalStimModifier;
}

const u32 upgradeModifier_weapon(const CUnit *unit) {
	u32 finalUpgradeModifier = 10;
	if(unit->status & UnitStatus::CooldownUpgrade) {
		finalUpgradeModifier = 5; //by now, no units need special behavior
	}
	return finalUpgradeModifier;
}

const u32 ensnareModifier_weapon(const CUnit *unit){
	u32 finalEnsnareModifier = 10;
	if(unit->ensnareTimer) {
		finalEnsnareModifier = 15;
	}
	return finalEnsnareModifier;
}

namespace hooks {

/// Calculates the unit's weapon cooldown, factoring in upgrades and status effects.
///
/// @return		The modified cooldown value.
u32 getModifiedWeaponCooldownHook(CUnit* unit, u8 weaponId) {
	//KYSXD new behavior
	u32 cooldown = weapons_dat::Cooldown[weaponId];

	cooldown += acidSporeModifier_weapon(unit, weaponId);

	cooldown *= upgradeModifier_weapon(unit);
	cooldown *= stimModifier_weapon(unit);
	cooldown *= ensnareModifier_weapon(unit);

	cooldown = cooldown/1000;

	if (cooldown > 250) cooldown = 250;
	else if (cooldown < 5) cooldown = 5;

	return cooldown;
}

} //hooks
