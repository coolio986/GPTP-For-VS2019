//Contains hooks that control unit movement speed, acceleration, and turn speed.

#include "unit_speed.h"
#include "../SCBW/enumerations.h"
#include "../SCBW/scbwdata.h"
#include <SCBW/api.h>

//V241 for VS2008

//KYSXD helpers:
namespace {
	static const u32 hydraliskModifier_speed = 16;
	static const u32 terranStimModifier_speed = 15;
	bool chargeTargetInRange(const CUnit *zealot);
	u32 upgradeModifier_speed(const CUnit *unit);
	u32 stimModifier_speed(const CUnit *unit);
	u32 creepModifier_speed(const CUnit *zergUnit);
	u32 ensnareModifier_speed(const CUnit *unit);
	u32 unitModifier_speed(const CUnit *unit);
};

namespace hooks {

/// Calculates the unit's modified movement speed, factoring in upgrades and status effects.
///
/// @return		The modified speed value.
u32 getModifiedUnitSpeedHook(CUnit* unit, u32 baseSpeed) {
	//KYSXD new behavior
	u32 speed = baseSpeed;
	speed *= unitModifier_speed(unit);
	speed = speed/10000;

	return speed;

}

/// Calculates the unit's acceleration, factoring in upgrades and status effects.
///
/// @return		The modified acceleration value.
u32 getModifiedUnitAccelerationHook(CUnit* unit) {
	//KYSXD new behavior
	u32 acceleration = flingy_dat::Acceleration[units_dat::Graphic[unit->id]];
	acceleration *= unitModifier_speed(unit);
	acceleration = acceleration/10000;
	return acceleration;


}

/// Calculates the unit's turn speed, factoring in upgrades and status effects.
///
/// @return		The modified turning speed value.
u32 getModifiedUnitTurnSpeedHook(CUnit* unit) {
	//KYSXD new behavior
	u32 turnSpeed = flingy_dat::TurnSpeed[units_dat::Graphic[unit->id]];
	turnSpeed *= unitModifier_speed(unit);
	turnSpeed = turnSpeed/10000;
	return turnSpeed;

}

} //hooks

namespace {
	bool chargeTargetInRange(const CUnit *zealot) {
	  if (!zealot->orderTarget.unit)
	    return false;
	  CUnit *chargeTarget = zealot->orderTarget.unit;
	  int maxChargeRange = 3 << 5;
	  int minChargeRange = 16;
	  int chargeRange = zealot->getDistanceToTarget(zealot->orderTarget.unit);
	  if (zealot->mainOrderId != OrderId::AttackUnit)
	    return false;
	  if (minChargeRange > chargeRange
	    || chargeRange > maxChargeRange)
	    return false;
	  return true;
	}

	//upgrade, stim, creep and ensnare modifiers.
	u32 upgradeModifier_speed(const CUnit *unit) {
		u32 finalUpgradeModifier = 10;
		if(unit->status & UnitStatus::SpeedUpgrade) {
			static ActiveTile actTile;
			switch(unit->id) {
				case UnitId::ZergZergling:
					finalUpgradeModifier = 16;
					break;
				case UnitId::ZergOverlord:
					finalUpgradeModifier = 35; //Was finalUpgradeModifier = 32
					break;
				case UnitId::ZergHydralisk:
				case UnitId::Hero_HunterKiller:
					actTile = scbw::getActiveTileAt(unit->getX(), unit->getY());
					if (!actTile.hasCreep) {
						finalUpgradeModifier = hydraliskModifier_speed;
					}
					break;
				case UnitId::ProtossZealot:
					finalUpgradeModifier = 13;
					break;
				case UnitId::ProtossScout:
					finalUpgradeModifier = 16;
					break;
				case UnitId::TerranVulture:
					finalUpgradeModifier = 15;
					break;
		        default:
		        	finalUpgradeModifier = 13;
		        	break;
			}
		}
		return finalUpgradeModifier;
	}
	u32 stimModifier_speed(const CUnit *unit) {
		u32 finalStimModifier = 10;
		if(unit->stimTimer) {
			switch(unit->id) {
				//Normal stim for regular units:
				case UnitId::TerranMarine:
					finalStimModifier = terranStimModifier_speed;
					break;
				//New speed for some units;
				case UnitId::ProtossZealot:
					if(chargeTargetInRange(unit)) {
						finalStimModifier = 20;
					}
					break;
				default:
					break;
			}
		}
		return finalStimModifier;
	}
	u32 creepModifier_speed(const CUnit *zergUnit) {
		u32 finalCreepModifier = 10;
		if (zergUnit->getRace() == RaceId::Zerg
			&& !(zergUnit->status & UnitStatus::InAir)) {
			ActiveTile actTile = scbw::getActiveTileAt(zergUnit->getX(), zergUnit->getY());
			if(actTile.hasCreep) {
				switch(zergUnit->id) {
					case UnitId::ZergHydralisk:
					case UnitId::Hero_HunterKiller:
						finalCreepModifier = hydraliskModifier_speed;
						break;
					default:
						finalCreepModifier = 13;
						break;
				}
			}
		}
		return finalCreepModifier;
	}
	u32 ensnareModifier_speed(const CUnit *unit) {
		u32 finalEnsnareModifier = 10;
		if(unit->ensnareTimer) {
			finalEnsnareModifier = 5;
		}
		return finalEnsnareModifier;
	}

	//KYSXD speed/acceleration/turnspeed modifier, must divide by 10000 in the getModified-hooks
	u32 unitModifier_speed(const CUnit *unit) {
		u32 finalmodifier = 1;

		finalmodifier *= upgradeModifier_speed(unit);
		finalmodifier *= stimModifier_speed(unit);
		finalmodifier *= creepModifier_speed(unit);
		finalmodifier *= ensnareModifier_speed(unit);

		return finalmodifier;
	}
}
