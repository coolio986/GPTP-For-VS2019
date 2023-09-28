#include "max_energy.h"
#include <SCBW/api.h>

const int ENERGY_INGAME_250 = 64000;
const int ENERGY_INGAME_200 = 51200;

namespace hooks {

/// Replaces the CUnit::getMaxEnergy() function.
/// Return the amount of maximum energy that a unit can have.
/// Note: 1 energy displayed in-game equals 256 energy.
/// Should be equivalent to GetUnitMaxEnergy @ 00491870  
u16 getUnitMaxEnergyHook(CUnit* const unit) {

	if (units_dat::BaseProperty[unit->id] & UnitProperty::Hero)
		return ENERGY_INGAME_250;

	switch (unit->id) {
		case UnitId::science_vessel:
			if (scbw::getUpgradeLevel(unit->playerId, UpgradeId::TitanReactor))
				return ENERGY_INGAME_250;
			break;
		case UnitId::ghost:
			if (scbw::getUpgradeLevel(unit->playerId, UpgradeId::MoebiusReactor))
				return ENERGY_INGAME_250;
			break;
		case UnitId::wraith:
			if (scbw::getUpgradeLevel(unit->playerId, UpgradeId::ApolloReactor))
				return ENERGY_INGAME_250;
			break;
		case UnitId::battlecruiser:
			if (scbw::getUpgradeLevel(unit->playerId, UpgradeId::ColossusReactor))
				return ENERGY_INGAME_250;
			break;
		case UnitId::queen:
			if (scbw::getUpgradeLevel(unit->playerId, UpgradeId::GameteMeiosis))
				return ENERGY_INGAME_250;
			break;
		case UnitId::defiler:
			if (scbw::getUpgradeLevel(unit->playerId, UpgradeId::MetasynapticNode))
				return ENERGY_INGAME_250;
			break;
		case UnitId::high_templar:
			if (scbw::getUpgradeLevel(unit->playerId, UpgradeId::KhaydarinAmulet))
				return ENERGY_INGAME_250;
			break;
		case UnitId::arbiter:
			if (scbw::getUpgradeLevel(unit->playerId, UpgradeId::KhaydarinCore))
				return ENERGY_INGAME_250;
			break;
		case UnitId::corsair:
			if (scbw::getUpgradeLevel(unit->playerId, UpgradeId::ArgusJewel))
				return ENERGY_INGAME_250;
			break;
		case UnitId::medic:
			if (scbw::getUpgradeLevel(unit->playerId, UpgradeId::CaduceusReactor))
				return ENERGY_INGAME_250;
			break;
		case UnitId::dark_archon:
			if (scbw::getUpgradeLevel(unit->playerId, UpgradeId::ArgusTalisman))
				return ENERGY_INGAME_250;
			break;
    //KYSXD for use with WarpGate
      // KYSXD Unit Cooldowns = TimeCost[] - 5 seconds
    case UnitId::Special_WarpGate:
      switch(unit->previousUnitType) {
        case UnitId::ProtossGateway:
        case UnitId::Special_WarpGate:
        case UnitId::None: return 0; break;
        default:
        u16 cooldownVal = units_dat::TimeCost[unit->previousUnitType]/15; //cause 15 frames per second
        return (cooldownVal + 5) << 8; break; //Energy per second
      }
      break;
    //KYSXD for use with the zealot's charge plugin
    case UnitId::ProtossZealot:
      if (scbw::getUpgradeLevel(unit->playerId, UpgradeId::LegEnhancements))
        return 10 << 8; //10
      else return 0;
      break;
    //KYSXD for use with the nexus chrono boost
    case UnitId::ProtossNexus:
      return 50 << 8;
      break;

	}

	return ENERGY_INGAME_200;
}

} //hooks
