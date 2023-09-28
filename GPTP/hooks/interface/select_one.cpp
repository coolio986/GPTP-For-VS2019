#include <SCBW/api.h>
#include <SCBW/scbwdata.h>

//by TheBest(khkgn)
namespace hooks {

bool setSelectOneHook(const CUnit *unit){

	using units_dat::BaseProperty;
	bool selectGroup = true;


	if((unit->status & UnitStatus::GroundedBuilding)
		|| (BaseProperty[unit->id] & UnitProperty::NeutralAccessories))
			selectGroup = false;

	else if(unit->lockdownTimer 
		|| unit->stasisTimer 
		|| unit->maelstromTimer)
			selectGroup = false;

	else if (unit->id >= 203 && unit->id <= 213) // 203 = Floor Gun Trap, 213 = Right Wall Flame Trap 
			selectGroup = false;

	else if ((unit->id == UnitId::spider_mine&&unit->status & UnitStatus::Burrowed)
		|| unit->id == UnitId::nuclear_missile
		|| unit->id == UnitId::scarab
		|| unit->id == UnitId::Spell_DarkSwarm
		|| unit->id == UnitId::Spell_DisruptionWeb){
			selectGroup = false;
	}


	return selectGroup;

}

}