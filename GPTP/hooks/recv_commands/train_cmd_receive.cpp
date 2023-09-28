#include "train_cmd_receive.h"
#include <SCBW/api.h>

//Helper functions declaration

namespace {

bool HasMoneyCanMake(CUnit* builder, u32 unitToBuild);	//67250

u32 getFighterId(CUnit* trainer);
bool isFightersTrainer(CUnit *trainer);

CUnit *getBestTrainer();
CUnit *getBestBuilder(u16 wUnitType);
CUnit *getLowestQueueUnit(CUnit **units_array, int array_length);

const u32 Can_Create_UnitorBuilding = 0x00428E60; 	//BTNSCOND_CanBuildUnit
const u32 Can_Build_Subunit = 0x00428E00; 			//BTNSCOND_TrainingFighter

s32 req_check(u32 reqFunc, u16 reqVar, u32 playerId, CUnit* unit);

} //unnamed namespace

namespace hooks {

void CMDRECV_TrainFighter()
{
	CUnit *trainer = getBestTrainer();
	if(trainer != NULL)
	{
		if(HasMoneyCanMake(trainer, getFighterId(trainer))
			&& trainer->secondaryOrderState != 2)
		{
			trainer->secondaryOrderId = OrderId::TrainFighter;
			trainer->secondaryOrderPos.y = 0;
			trainer->secondaryOrderPos.x = 0;
			trainer->currentBuildUnit = NULL;
			trainer->secondaryOrderState = 0;
		}
		scbw::refreshConsole();
	}
	return;
};

//Note: the default function isn't compatible with multiple selection
void CMDRECV_Train(u16 wUnitType) {

	CUnit* builder = getBestBuilder(wUnitType);

	if(builder != NULL)
	{
		if(HasMoneyCanMake(builder,wUnitType))
		{
			builder->setSecondaryOrder(OrderId::Train);
		}
		scbw::refreshConsole();
	}
	return;
};

} //namespace hooks

//-------- Helper function definitions. Do NOT modify! --------//

namespace {

const u32 Func_HasMoneyCanMake = 0x00467250;
bool HasMoneyCanMake(CUnit* builder, u32 unitToBuild) {

	static Bool32 bPreResult;

	__asm {
		PUSHAD
		PUSH unitToBuild
		MOV EDI, builder
		CALL Func_HasMoneyCanMake
		MOV bPreResult, EAX
		POPAD
	}

	return (bPreResult != 0);

}

u32 getFighterId(CUnit* trainer)
{
	u32 fighterId;
	switch(trainer->id)
	{
		case UnitId::ProtossCarrier:
		case UnitId::Hero_Gantrithor:
			fighterId = UnitId::ProtossInterceptor;
			break;
		case UnitId::ProtossReaver:
		case UnitId::Hero_Warbringer:
			fighterId = UnitId::ProtossScarab;
			break;
		default:
			fighterId = UnitId::None;
			break;
	}
	return fighterId;
}

bool isFightersTrainer(CUnit *trainer)
{
	switch(trainer->id)
	{
		case UnitId::ProtossCarrier:
		case UnitId::Hero_Gantrithor:
		case UnitId::ProtossReaver:
		case UnitId::Hero_Warbringer:
			return true;
			break;
		default:
			return false;
			break;
	}
}

CUnit *getBestTrainer()
{
	int i = 0;
	CUnit *trainers[SELECTION_ARRAY_LENGTH] = {NULL};

	*selectionIndexStart = 0;
	CUnit *current_unit = getActivePlayerNextSelection();
	while(current_unit != NULL)
	{
		if(isFightersTrainer(current_unit)
			&& current_unit->canMakeUnit(getFighterId(current_unit) == 1,
				*ACTIVE_NATION_ID))
		{
			trainers[i++] = current_unit;
		}
		current_unit = getActivePlayerNextSelection();
	}
	return getLowestQueueUnit(trainers, i);
}


CUnit *getBestBuilder(u16 wUnitType)
{
	int i = 0;
	CUnit *builders[SELECTION_ARRAY_LENGTH] = {NULL};

	*selectionIndexStart = 0;
	int j = 0;
	CUnit *current_unit = clientSelectionGroup->unit[j];
	while(current_unit != NULL)
	{
		if(current_unit->canMakeUnit(wUnitType, *ACTIVE_NATION_ID) == 1
			&& req_check(Can_Create_UnitorBuilding, wUnitType, current_unit->playerId, current_unit) == BUTTON_STATE::Enabled)
		{
			builders[i++] = current_unit;
		}
		current_unit = clientSelectionGroup->unit[++j];
	}
	return getLowestQueueUnit(builders, i);
}

CUnit *getLowestQueueUnit(CUnit **units_array, int array_length)
{
	CUnit *bestUnit = units_array[0];
	int bestLength = 5;

	for(int i = 0; i < array_length; i++)
	{
		if(units_array[i] != NULL)
		{
			int currentLength = 0;
			while(units_array[i]->buildQueue[ (units_array[i]->buildQueueSlot + currentLength)%5 ] != UnitId::None)
			{
				currentLength++;
			}
			if(currentLength < bestLength)
			{
				bestLength = currentLength;
				bestUnit = units_array[i];
			}
		}
	}
	return bestUnit;
}

s32 req_check(u32 reqFunc, u16 reqVar, u32 playerId, CUnit* unit) {

	static s32 return_value;

	__asm {
		PUSHAD
		MOV CX, reqVar
		MOV EDX, playerId
		PUSH unit
		CALL reqFunc
		MOV return_value, EAX
		POPAD
	}

	return return_value;

}

;

} //unnamed namespace

//End of helper functions
