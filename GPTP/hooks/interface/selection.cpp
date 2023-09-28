#include "selection.h"
#include <SCBW/api.h>

//Helper functions declaration

namespace {

	bool unitIsOwnedByCurrentPlayer(CUnit* unit); 																//0x00401170
	bool isUnitBurrowed(CUnit* unit);																			//0x00402A70
	void SC_memcpy_0(u32 dest, u32 src, u32 memsize);															//0x00408FD0
	CUnit** getAllUnitsInBounds(Box16* coords);																	//0x0042FF80
	void function_00468670(CUnit* unit);																		//0x00468670
	bool unit_isUnselectable(u16 unitId);																		//00x046ED80
	void function_0046F040(CUnit* current_unit, CUnit** unit_list, CUnit* clicked_unit, u32 list_length);		//00x046F040
	u32 combineLists_Sub_6F290(CUnit* unit,CUnit** unit_list_1,CUnit** unit_list_2,u32 list_length);			//0x0046F290
	void applyNewSelect_Sub_6FA00(CUnit** unit_list,u32 unit_list_length);										//0x0046FA00
	u32 CUnitToUnitID(CUnit* unit);																				//0x0047B1D0
	bool unit_IsStandardAndMovable(CUnit* unit); 																//0x0047B770
	void selectBuildingSFX(CUnit* unit);																		//0x0048F910
	Bool32 selectSingleUnitFromID(u32 unitIndex);																//0x00496D30
	void CreateNewUnitSelectionsFromList(CUnit** unit_list, u32 unit_list_length);								//0x0049AE40
	void CMDACT_Select(CUnit** unit_list, u32 unit_list_length); 												//0x004C0860

	inline void setBox16(Box16 *box, s16 n_left, s16 n_top, s16 n_right, s16 n_bottom);
	bool isStunned(const CUnit *unit);
	bool unit_isMultiselectable(const CUnit *unit);

	inline bool unitsShareSamePlayer(const CUnit *unit1, const CUnit *unit2)
	{
		return unit1->playerId == unit2->playerId;
	};

	inline bool unitsShareSameId(const CUnit *unit1, const CUnit *unit2)
	{
		return unit1->id == unit2->id;
	};

	inline bool isFlagOn(const u32 status, const u32 flag)
	{
		return (status & flag) != 0;
	};

	inline bool checkUnitStatus(const CUnit *unit, u32 flag)
	{
		return (unit->status & flag) != 0;
	};

	inline bool checkUnitVisibilityStatus(const CUnit *unit, u32 flag)
	{
		return (unit->visibilityStatus & flag) != 0;
	};

	inline bool checkSpriteVisibilityFlags(const CSprite *sprite, u8 flag)
	{
		return (sprite->visibilityFlags & flag) != 0;
	};

	//Equivalent to " unit1->status XOR unit2->status "
	//keep the flags appearing in only 1 of the variables
	inline u32 getMixedStatusFlags(const CUnit *unit1, const CUnit *unit2)
	{
		return ( ~(unit1->status) & unit2->status ) | ( unit1->status & ~(unit2->status) );
	};

	bool unitsMatchMultiselection(const CUnit *ref_unit, const CUnit *current_unit)
	{
		bool bUnitMatch = false;
		if(ref_unit != NULL)
		{
			u32 mixed_flags = getMixedStatusFlags(ref_unit, current_unit);
			if(
				unitsShareSamePlayer(ref_unit, current_unit)
				&& unitsShareSameId(ref_unit, current_unit)
				&& !isFlagOn(mixed_flags, UnitStatus::Burrowed)
				&& (isFlagOn(ref_unit->status, UnitStatus::Burrowed)
					|| !isFlagOn(mixed_flags, UnitStatus::RequiresDetection))
				&& !isFlagOn(mixed_flags, UnitStatus::IsHallucination)
				)
				bUnitMatch = true;
		}
		else bUnitMatch = true;

		return bUnitMatch;
	};

	const u32* VISIBILITY_CHECK_0057F0B0 = (u32*)0x0057F0B0;
	bool unit_isVisible(const CUnit *unit)
	{
		bool bUnitIsVisible = true;
		if (
			!checkSpriteVisibilityFlags(unit->sprite, (u8)*VISIBILITY_CHECK_0057F0B0)
			||
			//if the unit is cloaked and visibility status test (detection by detectors
			//units probably) fail, the unit is skipped
			(checkUnitStatus(unit, UnitStatus::Cloaked | UnitStatus::RequiresDetection)
			&& !checkUnitVisibilityStatus(unit, *VISIBILITY_CHECK_0057F0B0) )
			)
		{
			bUnitIsVisible = false;
		}
		return bUnitIsVisible;
	};

	u32 getUnitSearchFlag(CUnit *unit)
	{
		u32 searchFlag = 0;
		searchFlag +=
			(!isFlagOn(unit->status, UnitStatus::IsBuilding) ? 0 : 1);
		searchFlag +=
			(units_dat::BaseProperty[unit->id]  & UnitProperty::NeutralAccessories ? 0 : 2);
		return searchFlag;
	}

	u32 getUnitListSearchFlag(CUnit ** unit_list)
	{
		u32 searchFlag = 0;
		u32 current_index_in_unit_list = 0;
		CUnit *current_unit = unit_list[current_index_in_unit_list];
		while(current_unit != NULL)
		{
			u32 currentFlag = getUnitSearchFlag(current_unit);
			if(currentFlag > searchFlag)
			{
				searchFlag = currentFlag;
			}
			current_unit = unit_list[++current_index_in_unit_list];
		}
		return searchFlag;
	}

	bool unitMatchSearchFlag(CUnit *unit, u32 flag)
	{
		return flag == getUnitSearchFlag(unit);
	}

} //unnamed namespace

namespace hooks {
	///
	/// Function used by Ctrl+Click (select all units of same type)
	///
	u32 SortAllUnits(CUnit* ref_unit, CUnit** unit_list, CUnit** units_in_bounds) {
		CUnit* backup_current_unit = NULL;
		u32 current_index_in_unit_list;

		u32 searchFlag = getUnitListSearchFlag(units_in_bounds);

		bool bKeepForEmptyListCase = false;

		//if unit not null, become first entry of unit_list
		if(ref_unit != NULL)
		{
			unit_list[0] = ref_unit;
			current_index_in_unit_list = 1;
		}
		else
		{
			current_index_in_unit_list = 0;
		}

		//work with the first unit from units_in_bounds
		int units_in_bounds_index = 0;
		CUnit* current_unit = units_in_bounds[units_in_bounds_index];

		/// Fill unit_list from 0 to SELECTION_ARRAY_LENGTH -1
		while(current_unit != NULL)
		{
			bKeepForEmptyListCase = false;
			bool bDontAddToList = false;

			if(current_unit != ref_unit)
			{
				if(current_unit->subunit != NULL)
				{
					//if what got selected is the turret or another subunit, swap
					//the ref_unit and the subunit as current_unit
					if(units_dat::BaseProperty[current_unit->id] & UnitProperty::Subunit)
						current_unit = current_unit->subunit;

					if(current_index_in_unit_list != 0)
					{
						u32 index = current_index_in_unit_list;
						do {
							index--;

							//if the ref_unit is already in the list, skip it
							if(unit_list[index] == current_unit)
								bDontAddToList = true;

						}while (index != 0 && !bDontAddToList);
					}
				} //if(current_unit->subunit != NULL)

				if(!bDontAddToList)
				{
					//if the unit sprite is visible, perform additionnal visibility test,
					//else skip the unit
					if( unit_isVisible(current_unit)
						&& !unit_isUnselectable(current_unit->id) )
					{
						if(// Select One case
							!unit_isMultiselectable(current_unit)
							|| (!*IS_IN_REPLAY
							&& current_unit->playerId != *LOCAL_NATION_ID))
							bKeepForEmptyListCase = true; //skip the ref_unit for the list, unless it's the only available
						else
						{
							if( (ref_unit != NULL
								&& unitsMatchMultiselection(ref_unit, current_unit) )
								|| ( ref_unit == NULL
									&& unitMatchSearchFlag(current_unit, searchFlag) )
								)
							{
								if(current_index_in_unit_list >= SELECTION_ARRAY_LENGTH) //action when unit_list is full
									function_0046F040(current_unit, unit_list, ref_unit, current_index_in_unit_list);
								else {
									//Add current_unit to unit_list
									unit_list[current_index_in_unit_list] = current_unit;
									current_index_in_unit_list++;
								}

							}

						} //several ifs
					}

				} //if(!bDontAddToList) => 6F161

			} //if(current_unit != ref_unit)

			if(bKeepForEmptyListCase)
				backup_current_unit = current_unit;
			//select next ref_unit in bounds
			current_unit = units_in_bounds[++units_in_bounds_index];

		} //loop back at 6F237

		if(current_index_in_unit_list == 0)
		{
			if(backup_current_unit == NULL)
			{
				if(ref_unit != NULL)
				{
					unit_list[0] = ref_unit;
					current_index_in_unit_list = 1; //return 1 (the clicked ref_unit alone)
				}
				else
				{
					current_index_in_unit_list = 0; //return 0 (no list)
				}

			} //if(backup_current_unit == NULL)
			else
			{
				if(ref_unit != NULL)
				{
					unit_list[0] = ref_unit;
					current_index_in_unit_list = 1; //return 1 (the clicked ref_unit alone)
				}
				else {
					unit_list[0] = backup_current_unit;
					current_index_in_unit_list = 1; //return 1 (an unit preserved by bKeepForEmptyListCase)
				}

			}

		}

		return current_index_in_unit_list;
	} //u32 SortAllUnits(CUnit* ref_unit,CUnit** unit_list,CUnit* units_in_bounds)

	///
	/// Function used by Click on unit (possibly with key pressed)
	///
	/* KYSXD
	 * void getSelectedUnitsAtPoint()
	 */
	void function_0046FB40(CUnit* clicked_unit) {

		Bool32* const IS_DOUBLE_CLICKING =	(Bool32*)	0x0066FF58;

		Bool8* const bCanUpdateSelectedUnitData		= (Bool8*)	0x0059723C;
		Bool8* const bCanUpdateSelectedUnitPortrait	= (Bool8*)	0x0068AC74;
		Bool32* const bCanUpdateCurrentButtonSet	= (Bool32*)	0x0068C1B0;
		BinDlg** const someDialogUnknown			= (BinDlg**)0x0068C1E8; //related to MouseOver?
		Bool8* const bCanUpdateStatDataDialog		= (Bool8*)	0x0068C1F8;
		BinDlg** const someDialogUnknownUser		= (BinDlg**)0x0068C1EC; //related to MouseOver? Usually someDialogUnknown->user if not 0 or -1

		CUnit* local_temp_array_1[SELECTION_ARRAY_LENGTH];	//used instead of an array starting from [EBP-3C]
		CUnit* local_temp_array_2[SELECTION_ARRAY_LENGTH];	//used instead of an array starting from [EBP-6C]

		Box16 local_temp_box16_structure;	//used instead of using from [EBP-0C] to [EBP-06]

		bool isHoldingCtrl_OR_isDoubleClickingSelectedClickedUnit = 
			*IS_HOLDING_CTRL || 
			( *IS_DOUBLE_CLICKING && (clicked_unit->sprite->flags & CSprite_Flags::Selected) );

		for(int i = 0; i < SELECTION_ARRAY_LENGTH; i++)
			local_temp_array_1[i] = NULL;

		if( // Is multiselecting
			isHoldingCtrl_OR_isDoubleClickingSelectedClickedUnit ||
			( *IS_HOLDING_SHIFT && (activePlayerSelection->unit[0] != NULL) )
			) 
		{

			if( !*IS_HOLDING_SHIFT && isHoldingCtrl_OR_isDoubleClickingSelectedClickedUnit) {

				//Ctrl+click on unit or double-click on already
				//selected unit without other key
				// =>	select surrounding units with same type as 
				//		clicked unit, making them the current 
				//		selection

				CUnit** units_in_bounds;
				u32 sorted_list_length;

				//locate surrounding units for selection
				setBox16(&local_temp_box16_structure, *MoveToX, *MoveToY, *MoveToX + 640, *MoveToY + 400);
				units_in_bounds = getAllUnitsInBounds(&local_temp_box16_structure);
				sorted_list_length = SortAllUnits(clicked_unit,local_temp_array_1,units_in_bounds);

				//reload the previous temporary unit list from before the call to getAllUnitsInBounds
				*tempUnitsListArraysCountsListLastIndex = *tempUnitsListArraysCountsListLastIndex - 1;
				*tempUnitsListCurrentArrayCount = tempUnitsListArraysCountsList[*tempUnitsListArraysCountsListLastIndex];

				if(sorted_list_length != 0) {

					//turn the local array as the newly applied selection
					applyNewSelect_Sub_6FA00(local_temp_array_1,sorted_list_length);

					//this is what make the rally point briefly appear when
					//selecting a building with one.
					//maybe can do others things?
					if(
						sorted_list_length == 1 &&
						clicked_unit->playerId == *ACTIVE_NATION_ID &&
						clicked_unit->isFactory()
						) 
						function_00468670(clicked_unit);

				}


			} //if( !*IS_HOLDING_SHIFT && isHoldingCtrl_OR_isDoubleClickingSelectedClickedUnit )
			else {

				//holding SHIFT (if not holding CTRL, then there's an active selection too)

				if(isHoldingCtrl_OR_isDoubleClickingSelectedClickedUnit) {

					//save existing selection to a temporary array
					for(int i = 0; i < SELECTION_ARRAY_LENGTH; i++)
						local_temp_array_2[i] = activePlayerSelection->unit[i];

					//{Ctrl+Shift+click on unit} OR {Shift+double-click on 
					//selected unit WHILE having an active selection}
					// =>	select units same type as clicked unit, adding them to the
					//		active selection

					CUnit** units_in_bounds;
					u32 sorted_list_length;

					//locate surrounding units for selection
					setBox16(&local_temp_box16_structure, *MoveToX, *MoveToY, *MoveToX + 640, *MoveToY + 400);
					units_in_bounds = getAllUnitsInBounds(&local_temp_box16_structure);
					sorted_list_length = SortAllUnits(clicked_unit,local_temp_array_1,units_in_bounds);

					//reload the previous temporary unit list from before the call to getAllUnitsInBounds
					*tempUnitsListArraysCountsListLastIndex = *tempUnitsListArraysCountsListLastIndex - 1;
					*tempUnitsListCurrentArrayCount = tempUnitsListArraysCountsList[*tempUnitsListArraysCountsListLastIndex];

					if(sorted_list_length != 0) {

						//6FCD4

						//recombine current selection and newly selectable items in local_temp_array_2
						u32 list_remixed_length = combineLists_Sub_6F290(clicked_unit,local_temp_array_1,local_temp_array_2,sorted_list_length);

						//turn local_temp_array_2 as the newly applied selection
						applyNewSelect_Sub_6FA00(local_temp_array_2,list_remixed_length);

					}

				} //if(isHoldingCtrl_OR_isDoubleClickingSelectedClickedUnit)
				else {

					//6FCF6

					//Shift + click while there's an existing selection 
					//(because hold on CTRL case was already dealt with
					//so there's an existing selection at this point)
					// => add/remove unit from selection

					int arrayIndex;

					//copy the existing selection into the local array
					for(arrayIndex = 0 ; arrayIndex < SELECTION_ARRAY_LENGTH && activePlayerSelection->unit[arrayIndex] != NULL; arrayIndex++)
						local_temp_array_1[arrayIndex] = activePlayerSelection->unit[arrayIndex];

					if( !(clicked_unit->sprite->flags & CSprite_Flags::Selected) ) {	
						
						//0x0046FD13 (getting unit sprite) and 0x0046FD18 (middle of the test with the flag)
						
						//unit not selected, so it's added to current selection if valid

						if(
							arrayIndex < SELECTION_ARRAY_LENGTH && 
							unit_isMultiselectable(local_temp_array_1[0]) && 
							unitIsOwnedByCurrentPlayer(local_temp_array_1[0]) &&
							unit_isMultiselectable(clicked_unit) && 
							unitIsOwnedByCurrentPlayer(clicked_unit)
							)
						{

							//new unit is added at end of array, and the index
							//is increased to be used as new array length
							local_temp_array_1[arrayIndex] = clicked_unit;
							arrayIndex++;

							//turn local_temp_array_1 as the newly applied selection
							applyNewSelect_Sub_6FA00(local_temp_array_1,arrayIndex);

						}

					}
					else {

						//6FD77

						//unit already selected, remove it from selection

						u32 memcpy_size;
						bool bUpdateSelection = true;

						//decrease it so it's equal to the index of last element
						arrayIndex--;

						//each element is 4 bytes, multiply it by the number of elements
						//after the one to remove
						memcpy_size = (arrayIndex - clicked_unit->sprite->selectionIndex) * 4;

						//Copy the elements on the right of the one to delete one step left
						//to overwrite the one to delete
						SC_memcpy_0(
							(u32)&local_temp_array_1[clicked_unit->sprite->selectionIndex], 
							(u32)&local_temp_array_1[clicked_unit->sprite->selectionIndex+1], 
							memcpy_size
						);

						//either erase the element to delete or one that was repeated
						//after memcpy
						local_temp_array_1[arrayIndex] = NULL;
						
						if(arrayIndex == 1) {

							if(*IS_HOLDING_ALT) {

								//It seems you can unselect things with Shift+Alt like
								//with shift alone, UNTIL you have 2 units selected.
								//It is the code below that is causing the
								//deselection to fail where simple shift+click would
								//continue to deselect.

								u32 unitIndex = CUnitToUnitID(local_temp_array_1[0]);

								if(selectSingleUnitFromID(unitIndex) != 0)
									bUpdateSelection = false;

							}

						}

						if(bUpdateSelection) {

							//Create a new unit selection (since the clicked unit is unselected)
							//and choose it as current selection)
							CreateNewUnitSelectionsFromList(local_temp_array_1,arrayIndex);
							CMDACT_Select(local_temp_array_1,arrayIndex);

							//tell the GUI to update stuff
							*someDialogUnknown = 0;
							*someDialogUnknownUser = 0;
							*bCanUpdateSelectedUnitData = 1;
							*bCanUpdateCurrentButtonSet = 1;
							*bCanUpdateSelectedUnitPortrait = 1;
							*bCanUpdateStatDataDialog = 1;

						}

					} //if (clicked_unit->sprite->flags & CSprite_Flags::Selected)

				} //if(!isHoldingCtrl_OR_isDoubleClickingSelectedClickedUnit)

			} //if(*IS_HOLDING_SHIFT)

		} //using double clicks, shift, ctrl...

		else {

			//6FBB7

			//Simple click...almost (there's still ALT key)

			bool bRecreatedSelectionThroughAltClick = false;

			if(*IS_HOLDING_ALT) {

				//Recall the selection the unit belong to if there's one
				//In this case, skip the code for a single unit handling

				u32 temp_unit_index = CUnitToUnitID(clicked_unit);

				if(selectSingleUnitFromID(temp_unit_index) != 0)
					bRecreatedSelectionThroughAltClick = true;

			}

			if(!bRecreatedSelectionThroughAltClick) {

				//6FBD1

				//Create a new selection containing only the clicked unit
				CreateNewUnitSelectionsFromList(&clicked_unit,1);

				//play the sound of the unit (don't need to be a building)
				selectBuildingSFX(clicked_unit);

				//select the unit
				CMDACT_Select(&clicked_unit,1);

				//tell the GUI to update stuff
				*bCanUpdateSelectedUnitData = 1;
				*bCanUpdateCurrentButtonSet = 1;
				*bCanUpdateSelectedUnitPortrait = 1;
				*bCanUpdateStatDataDialog = 1;
				*someDialogUnknown = 0;
				*someDialogUnknownUser = 0;

			}

			//6FC2C

			//this is what make the rally point briefly appear when
			//selecting a building with one.
			//maybe can do others things?
			if( clicked_unit->playerId == *ACTIVE_NATION_ID &&
				clicked_unit->isFactory()
				)
				function_00468670(clicked_unit);

		}

	}




} //namespace hooks

//-------- Helper function definitions. Do NOT modify! --------//

namespace {

	//Identical to 0x00401170
	bool unitIsOwnedByCurrentPlayer(CUnit* unit) {

		bool return_value;

		if(*IS_IN_REPLAY != 0 || unit->playerId != *LOCAL_NATION_ID)
			return_value = false;
		else
			return_value = true;

		return return_value;

	}

	;
	
	//Identical to 0x00402A70
	bool isUnitBurrowed(CUnit* unit) {
		return (unit->status & (UnitStatus::CloakingForFree + UnitStatus::Burrowed)) != 0;
	}

	;
	
	const u32 Func_memcpy_0 = 0x00408FD0;
	void SC_memcpy_0(u32 dest, u32 src, u32 memsize) {

		__asm {
			PUSHAD
			PUSH memsize
			PUSH src
			PUSH dest
			CALL Func_memcpy_0
			ADD ESP, 0x0C
			POPAD
		}

	}

	;
	
	const u32 Func_GetAllUnitsInBounds = 0x0042FF80;
	CUnit** getAllUnitsInBounds(Box16* coords) {

		static CUnit** units_in_bounds;

		__asm {
			PUSHAD
			MOV EAX, coords
			CALL Func_GetAllUnitsInBounds
			MOV units_in_bounds, EAX
			POPAD
		}

		return units_in_bounds;

	}

	;
	
	const u32 Func_Sub_468670 = 0x00468670;
	void function_00468670(CUnit* unit) {

		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_Sub_468670
			POPAD
		}

	}

	;
	
	const u32 Func_Unit_isUnselectable = 0x0046ED80;
	bool unit_isUnselectable(u16 unitId) {

		Bool32 bHalfReturnValue;

		__asm {
			PUSHAD
			XOR EBX, EBX
			MOV BX, unitId
			MOV EAX, EBX
			CALL Func_Unit_isUnselectable
			MOV bHalfReturnValue, EAX
			POPAD
		}

		return (bHalfReturnValue != 0);

	}

	;
	
	const u32 Func_Sub_6F040 = 0x0046F040;
	void function_0046F040(CUnit* current_unit, CUnit** unit_list, CUnit* clicked_unit, u32 list_length) {																//

		__asm {
			PUSHAD
			PUSH clicked_unit
			PUSH current_unit
			MOV ECX, unit_list
			MOV EAX, list_length
			CALL Func_Sub_6F040
			POPAD
		}

	}

	;
	
	const u32 Func_Sub_46F290 = 0x0046F290;
	u32 combineLists_Sub_6F290(CUnit* unit,CUnit** unit_list_1,CUnit** unit_list_2,u32 list_length) {

		u32 return_value;

		__asm {
			PUSHAD
			PUSH unit
			PUSH list_length
			PUSH unit_list_1
			MOV EDI, unit_list_2
			CALL Func_Sub_46F290
			MOV return_value, EAX
			POPAD
		}

		return return_value;

	}

	;
	
	const u32 Func_Sub_46FA00 = 0x0046FA00;
	void applyNewSelect_Sub_6FA00(CUnit** unit_list,u32 unit_list_length) {

		__asm {
			PUSHAD
			PUSH 0x01
			PUSH 0x01
			MOV EBX, unit_list
			MOV EDI, unit_list_length
			CALL Func_Sub_46FA00
			POPAD
		}

	}
	
	;

	const u32 Func_CUnitToUnitID = 0x0047B1D0;
	u32 CUnitToUnitID(CUnit* unit) {

		static u32 return_value;

		__asm {
			PUSHAD
			MOV ESI, unit
			CALL Func_CUnitToUnitID
			MOV return_value, EAX
			POPAD
		}

		return return_value;

	}

	;
	
	const u32 Func_Unit_IsStandardAndMovable = 0x0047B770;
	bool unit_IsStandardAndMovable(CUnit* unit) {

		Bool32 return_value_unconverted;

		__asm {
			PUSHAD
			MOV ECX, unit
			CALL Func_Unit_IsStandardAndMovable
			MOV return_value_unconverted, EAX
			POPAD
		}

		return (return_value_unconverted != 0);

	}

	;
	
	const u32 Func_SelectBuildingSFX = 0x0048F910;
	void selectBuildingSFX(CUnit* unit) {

		__asm {
			PUSHAD
			PUSH unit
			CALL Func_SelectBuildingSFX
			POPAD
		}

	}

	;
	
	const u32 Func_SelectSingleUnitFromID = 0x00496D30;
	Bool32 selectSingleUnitFromID(u32 unitIndex) {

		Bool32 bReturnedValue;

		__asm {
			PUSHAD
			PUSH unitIndex
			CALL Func_SelectSingleUnitFromID
			MOV bReturnedValue, EAX
			POPAD
		}

		return bReturnedValue;

	}

	;
	
	const u32 Func_CreateNewUnitSelectionsFromList = 0x0049AE40;
	void CreateNewUnitSelectionsFromList(CUnit** unit_list, u32 unit_list_length) {

		__asm {
			PUSHAD
			PUSH unit_list_length
			MOV EAX, unit_list
			CALL Func_CreateNewUnitSelectionsFromList
			POPAD
		}

	}	

	;
	
	const u32 Func_CMDACT_Select = 0x004C0860;
	void CMDACT_Select(CUnit** unit_list, u32 unit_list_length) {

		__asm {
			PUSHAD
			PUSH unit_list
			PUSH unit_list_length
			CALL Func_CMDACT_Select
			POPAD
		}

	}

	;

	inline void setBox16(Box16 *box, s16 n_left, s16 n_top, s16 n_right, s16 n_bottom)
	{
	  box->left = n_left;
	  box->top = n_top;
	  box->right = n_right;
	  box->bottom = n_bottom;
	  return;
	};

	bool isStunned(const CUnit *unit)
	{
		return (unit->lockdownTimer != 0
			|| unit->stasisTimer != 0
			|| unit->maelstromTimer != 0);
	};

	bool unit_isMultiselectable(const CUnit *unit)
	{
		bool bIsMultiselectable = true;

/*
		if(activePlayerSelection->unit[0] != NULL)
		{
			const CUnit *selectedUnit = activePlayerSelection->unit[0];
			u32 mixed_flags = getMixedStatusFlags(selectedUnit, unit);
			if(isFlagOn(mixed_flags, UnitStatus::GroundedBuilding)
				|| (units_dat::BaseProperty[unit->id] ^ units_dat::BaseProperty[selectedUnit->id]) & UnitProperty::NeutralAccessories)
			{
				bIsMultiselectable = false;
			}
		}
*/
		if(isStunned(unit)) //Should be CUnit::isFrozen()?
		{
			bIsMultiselectable = false;
		}

		else if (203 <= unit->id && unit->id <= 213) // 203 = Floor Gun Trap, 213 = Right Wall Flame Trap 
		{
			bIsMultiselectable = false;
		}

		else if ((unit->id == UnitId::TerranVultureSpiderMine
			&& unit->status & UnitStatus::Burrowed)
				|| unit->id == UnitId::TerranNuclearMissile
				|| unit->id == UnitId::ProtossScarab
				|| unit->id == UnitId::Spell_DarkSwarm
				|| unit->id == UnitId::Spell_DisruptionWeb)
		{
				bIsMultiselectable = false;
		}

		return bIsMultiselectable;
	};

} //unnamed namespace

//End of helper functions
