/// This is where the magic happens; program your plug-in's core behavior here.

#include "game_hooks.h"
#include <graphics/graphics.h>
#include <SCBW/api.h>
#include <SCBW/ExtendSightLimit.h>

#include "psi_field.h"
#include <cstdio>

#include <SCBW/UnitFinder.h>
#include <logger.h>
#define WRITE_TO_LOG(x) GPTP::logger<<x<<std::endl

namespace
{
	void replaceUnitWithType(CUnit* unit, u16 newUnitId);
	void replaceSpriteImages(CSprite* sprite, u16 imageId, u8 imageDirection);
	void setImageDirection(CImage* image, s8 direction);
	bool unitHasPathToDestOnGround(const CUnit* unit, int x, int y);
	bool unitHasPathToUnitOnGround(const CUnit* unit, CUnit* target);
	void removeOrderFromUnitQueue(CUnit* unit);


	inline void manageUnitStatusFlags(CUnit* unit, u32 flag, bool status);
}

namespace buildingMorph
{
	void morphBuildingInto(CUnit* unit, u16 newUnitId, IscriptAnimation::Enum animation, bool canBeCanceled);
	void manageMorphing(CUnit* unit, u16 seconds);
	void runBuildingMorph(CUnit* unit);
}

namespace plugins
{
	CUnit* nearestTemplarMergePartner(CUnit* unit);
	bool chargeTargetInRange(const CUnit* zealot);
	bool isInHarvestState(const CUnit* worker);
	bool isMineral(const CUnit* resource);
	bool isGasSupply(const CUnit* resource);
	bool hasHarvestOrders(const CUnit* worker);

	class HarvestTargetFinder;
	class ContainerTargetFinder;

	void exploreMap();

	void runFirstFrameBehaviour();
	void showUnitGraphicHelpers(CUnit* unit);
	void manageWorkerCollision(CUnit* unit);

	void runAdeptPsionicTransfer_cast(CUnit* adept);
	void runAdeptPsionicTransfer_behavior(CUnit* shade);
	void runChronoBoost_cast(CUnit* unit);
	void runChronoBoost_behavior(CUnit* unit);
	void runStalkerBlink(CUnit* unit);
	void runZealotCharge(CUnit* unit);
	void runReactorBehaviour(CUnit* unit);
	void runBurrowedMovement(CUnit* unit);
	void showWorkerCount(CUnit* selUnit);
	void showProgressBar(CUnit* unit, u16 remainingBuildTime, u16 totalBuildTime);
}



namespace warpgateMechanic
{
	void manageWarpgateFlags(CUnit* warpgate);
	void cleanWarpgate(CUnit* warpgate);
	void setWarpUnit(CUnit* warpUnit);
	void runWarpgatePlacing(CUnit* unit);
	void runWarpgate(CUnit* warpgate);
}

namespace buildingPreview
{
	void saveUnitAsParent(CUnit* parent, CUnit* unit);
	CUnit* getPrevParent(CUnit* preview);
	u8 getPrevPlayerId(CUnit* preview);
	void cleanPrevPlayerId(CUnit* preview);
	void convertToHallucination(CUnit* unit);
	void useHallucinationDrawfunc(CSprite* sprite, u32 palette);
	bool IsBuildOrder(u16 orderId);
	bool isValidPreview(CUnit* preview);
	void removePreview(CUnit* preview);
	void freeOccupiedTiles(CUnit* building);
	bool placementIsFree(u16 unitId, u16 pos_x, u16 pos_y);
	void setPreview(CUnit* preview);
	CUnit* placePreviewAt(u16 unitId, u16 pos_x, u16 pos_y);
	void createPreview(CUnit* unit);
	void manageBuildingPreview(CUnit* unit);
}

namespace smartCasting
{
	//Here we store the last order for each unit,
	//to do not mess with the members of the CUnit structure
	static COrder lastOrderArray[UNIT_ARRAY_LENGTH + 1];

	const int ORDER_TYPE_COUNT = ((int)(0xBD) + 1);
	//Stores the pointer for the next CUnit (like a linked list)
	static CUnit* custom_unit_array[UNIT_ARRAY_LENGTH + 1];

	//First and last node of the casters lists
	static CUnit* firstCaster[ORDER_TYPE_COUNT][PLAYABLE_PLAYER_COUNT];
	static CUnit* lastCaster[ORDER_TYPE_COUNT][PLAYABLE_PLAYER_COUNT];

	bool isOrderValidForSmartcasting(u16 orderId);

	//Get a COrder pointer with a custom order
	COrder* createOrder(u16 orderId, u16 unitId, CUnit* target, u16 posX, u16 posY);

	//Get a COrder pointer with the current order
	inline COrder* getCurrentOrder(CUnit* unit);

	//Get a COrder pointer with the last order
	inline COrder* getLastOrder(CUnit* unit);

	//Stores all variables of COrder in the unit using unused members of CUnit
	inline void saveAsLastOrder(CUnit* unit, COrder* lastOrder);

	//Stores all variables of COrder in the variables of the current order
	inline void saveAsMainOrder(CUnit* unit, COrder* mainOrder);

	//This function does three things:
	//1) resets the variables of the current order
	//2) issues the COrder newOrder
	//3) stores the newOrder's variables in the unit's last order variables
	inline void setOrderInUnit(CUnit* unit, COrder* newOrder);

	//This function checks three things:
	//1) If the user has interacted with the unit
	//2) if the unit's mainOrderId is the same as the asked orderId
	//3) If the last order is different from the asked orderId or is allowed to overrun orders
	inline bool isCasterValidForOrder(CUnit* unit, u8 orderId);

	//Checks if the new order is different from the last
	inline bool isOverruningLastOrder(CUnit* unit, u8 orderId);

	//Checks of the orderId requires two units (warp archon)
	inline bool isCouplesOrder(u8 orderId);

	//Checks if two units are partners in a 2-units order
	inline bool isPartnerInOrder(CUnit* unit1, CUnit* unit2, u8 orderId);

	void setCasters();

	//Returns the best caster running orderId for player playerId
	CUnit** getBestCasters(u8 orderId);

	//Issued the last order to unit
	//If the last order was completed, orders to idle
	inline void tryLastOrder(CUnit* unit);

	//Smartcast behaviour
	inline void smartCastOrder(u8 orderId);

	//Sets flags and saves last order
	inline void prepareUnitsForNextFrame();

	//Runs smartcast for each order and prepares for next frame
	//Archon's orders doesn't work now:
	//Archon Merge and Dark Archon Meld buttons doesn't set userActionFlags on the unit
	inline void runSmartCast();
} //namespace smartCasting


namespace hooks
{




	/// This hook is called every frame; most of your plugin's logic goes here.
	bool nextFrame()
	{

		if (!scbw::isGamePaused())
		{ //If the game is not paused

			scbw::setInGameLoopState(true); //Needed for scbw::random() to work
			graphics::resetAllGraphics();
			hooks::updatePsiFieldProviders();

			//smartCasting::runSmartCast();
			//KYSXD idle worker amount
			bool isOperationCwalEnabled = scbw::isCheatEnabled(CheatFlags::OperationCwal);
			u32 idleWorkerCount = 0;

			//This block is executed once every game.
			if (*elapsedTimeFrames == 0)
			{
				plugins::runFirstFrameBehaviour();

			}

			//Loop through all visible units in the game - start
			for (CUnit* unit = *firstVisibleUnit; unit; unit = unit->link.next)
			{
				plugins::manageWorkerCollision(unit);
				//buildingPreview::manageBuildingPreview(unit);

				//show workers for visible and invisible
				plugins::showWorkerCount(unit);

				//btech - add progress bar for units being built
				if (unit->playerId == *LOCAL_NATION_ID)
				{
					if (unit->id && UnitId::ProtossFleetBeacon)
					{
						u16 test = 0;
					}
					u32 baseProp = units_dat::BaseProperty[unit->id];

					if (unit->status & UnitStatus::GroundedBuilding
						&& unit->status & UnitStatus::Completed)
					{
						switch (unit->mainOrderId)
						{
						case OrderId::Upgrade:
							plugins::showProgressBar(unit, unit->building.upgradeResearchTime, upgrades_dat::TimeCostBase[unit->building.upgradeType]);
						case OrderId::ResearchTech:
							plugins::showProgressBar(unit, unit->building.upgradeResearchTime, techdata_dat::TimeCost[unit->building.techType]);
							break;
						default: break;
						}

					}

					
					if (unit && unit->remainingBuildTime > 0
						&& unit->buildQueue
						&& unit->buildQueue[unit->buildQueueSlot] == unit->orderUnitType
						&& !(unit->status & UnitStatus::Completed))
					{
						plugins::showProgressBar(unit, unit->remainingBuildTime, units_dat::TimeCost[unit->id]);
					}
					else
						if (unit->buildQueue && unit->buildQueue[unit->buildQueueSlot] != unit->orderUnitType && unit->currentBuildUnit)
						{
							plugins::showProgressBar(unit->currentBuildUnit, unit->currentBuildUnit->remainingBuildTime, units_dat::TimeCost[unit->currentBuildUnit->id]);
						}
						else
							if (unit->buildQueue && unit->buildQueue[unit->buildQueueSlot] != unit->orderUnitType && unit->remainingBuildTime > 0)
							{
								plugins::showProgressBar(unit, unit->remainingBuildTime, units_dat::TimeCost[unit->buildQueue[unit->buildQueueSlot]]);
							}
				}

				//KYSXD Idle worker count
				if ((unit->playerId == *LOCAL_NATION_ID || scbw::isInReplay())
					&& units_dat::BaseProperty[unit->id] & UnitProperty::Worker
					&& unit->mainOrderId == OrderId::PlayerGuard)
				{
					++idleWorkerCount;
				}

				//KYSXD - reset warpgates when build
				if ((unit->id == UnitId::Special_WarpGate
					|| unit->id == UnitId::ProtossGateway)
					&& unit->mainOrderId == OrderId::BuildSelf2)
				{
					unit->previousUnitType = UnitId::None;
				}

				//KYSXD - Protoss plugins:
				plugins::runAdeptPsionicTransfer_cast(unit);
				plugins::runAdeptPsionicTransfer_behavior(unit);

				plugins::runChronoBoost_cast(unit);
				plugins::runChronoBoost_behavior(unit);

				plugins::runStalkerBlink(unit);
				plugins::runZealotCharge(unit);

				warpgateMechanic::runWarpgate(unit);

				//KYSXD - Terran plugins
				plugins::runReactorBehaviour(unit);

				//KYSXD - Zerg plugins
				plugins::runBurrowedMovement(unit);


			} //Loop through all visible units in the game - end

			//KYSXD - Selection plugins
			for (int i = 0; i < *clientSelectionCount; ++i)
			{
				CUnit* selUnit = clientSelectionGroup->unit[i];
				plugins::showUnitGraphicHelpers(selUnit);

				//KYSXD Twilight Archon
				if (selUnit->id == UnitId::ProtossDarkTemplar || selUnit->id == UnitId::ProtossHighTemplar)
				{
					if (selUnit->mainOrderId == OrderId::ReaverStop)
					{
						CUnit* templarPartner = plugins::nearestTemplarMergePartner(selUnit);
						if (templarPartner != NULL)
						{
							selUnit->orderTo(OrderId::WarpingDarkArchon, templarPartner);
							templarPartner->orderTo(OrderId::WarpingDarkArchon, selUnit);

							selUnit->mainOrderId = OrderId::WarpingDarkArchon;
							templarPartner->mainOrderId = OrderId::WarpingDarkArchon;
						}
						else selUnit->mainOrderId = OrderId::Stop;
					}
				}
				//KYSXD - For selected units - From SC Transition - end
			}

			//KYSXD Print info on screen
			if (idleWorkerCount != 0)
			{
				char idleworkers[64];
				sprintf_s(idleworkers, "Idle Workers: %d", idleWorkerCount);
				graphics::drawText(5, 5, idleworkers, graphics::FONT_MEDIUM, graphics::ON_SCREEN);
			}

			scbw::setInGameLoopState(false);
		}
		return true;

	}




	bool gameOn()
	{
		return true;
	}

	bool gameEnd()
	{
		return true;
	}

} //hooks

namespace
{
	const u32 Func_ReplaceUnitWithType = 0x0049FED0;
	void replaceUnitWithType(CUnit* unit, u16 newUnitId)
	{
		u32 newUnitId_ = newUnitId;

		__asm {
			PUSHAD
			PUSH newUnitId_
			MOV EAX, unit
			CALL Func_ReplaceUnitWithType
			POPAD
		}

	}

	const u32 Func_ReplaceSpriteImages = 0x00499BB0;
	void replaceSpriteImages(CSprite* sprite, u16 imageId, u8 imageDirection)
	{
		u32 imageId_ = imageId, imageDirection_ = imageDirection;
		__asm {
			PUSHAD
			PUSH imageDirection_
			PUSH imageId_
			MOV EAX, sprite
			CALL Func_ReplaceSpriteImages
			POPAD
		}
	}

	const u32 Func_SetImageDirection = 0x004D5F80;
	void setImageDirection(CImage* image, s8 direction)
	{
		u32 direction_ = direction;

		__asm {
			PUSHAD
			PUSH direction_
			MOV ESI, image
			CALL Func_SetImageDirection
			POPAD
		}

	}

	const u32 Func_unitHasPathToDestOnGround = 0x0042FA00;
	bool unitHasPathToDestOnGround(const CUnit* unit, int x, int y)
	{
		static Bool32 bPreResult;

		__asm {
			PUSHAD
			PUSH y
			MOV EDX, x
			MOV EAX, unit
			CALL Func_unitHasPathToDestOnGround
			MOV bPreResult, EAX
			POPAD
		}

		return (bPreResult != 0);

	}

	bool unitHasPathToUnitOnGround(const CUnit* unit, CUnit* target)
	{
		if (target != NULL)
			return unitHasPathToDestOnGround(unit, target->getX(), target->getY());
		return false;
	}

	const u32 Func_removeOrderFromUnitQueue = 0x004742D0;
	void removeOrderFromUnitQueue(CUnit* unit)
	{

		static COrder* orderQueueHead = unit->orderQueueHead;

		__asm {
			PUSHAD
			MOV ECX, unit
			MOV EAX, orderQueueHead
			CALL Func_removeOrderFromUnitQueue
			POPAD
		}

	}

	inline void manageUnitStatusFlags(CUnit* unit, u32 flag, bool status)
	{
		if (status)
		{
			unit->status |= flag;
		}
		else unit->status &= ~(flag);
	}
} //unnamed namespace

namespace buildingMorph
{
	void morphBuildingInto(CUnit* unit, u16 newUnitId, IscriptAnimation::Enum animation, bool canBeCanceled)
	{
		u16 timeCost = units_dat::TimeCost[newUnitId];

		unit->previousUnitType = unit->id;
		unit->id = newUnitId;
		unit->energy = 0;
		manageUnitStatusFlags(unit, UnitStatus::Completed, false);

		unit->mainOrderId = OrderId::Nothing2;
		unit->remainingBuildTime = timeCost;
		if (canBeCanceled)
			unit->currentButtonSet = UnitId::Buildings;
		else unit->currentButtonSet = UnitId::None;
		scbw::refreshConsole();
		unit->playIscriptAnim(animation);
		unit->_unused_0x106 = 1;

	}

	void manageMorphing(CUnit* unit, u16 seconds = 0)
	{
		//Morphing flag
		if (unit->_unused_0x106 == 1
			&& !unit->isFrozen())
		{
			u16 timeCost = units_dat::TimeCost[unit->id];
			u16 timeGain;
			if (seconds && (timeCost / 15) > seconds)
			{
				timeGain = timeCost / (seconds * 15);
			}
			else timeGain = 1;

			unit->remainingBuildTime -= std::min(unit->remainingBuildTime, timeGain);
			if (!unit->remainingBuildTime
				&& !(unit->status & UnitStatus::NoBrkCodeStart))
			{

				s32 shieldHolder = unit->shields;

				manageUnitStatusFlags(unit, UnitStatus::Completed, true);
				unit->currentButtonSet = unit->id;
				unit->mainOrderState = 0;
				replaceUnitWithType(unit, unit->id);
				unit->shields = shieldHolder;
				unit->_unused_0x106 = 0;
			}
		}
	}

	void runBuildingMorph(CUnit* unit)
	{
		//Usage exaplme: Gateway/WarpGate morph

		if (unit->id == UnitId::ProtossGateway
			|| unit->id == UnitId::Special_WarpGate)
		{
			if (unit->mainOrderId == OrderId::ReaverStop)
			{
				u16 morphVariant = UnitId::ProtossGateway + UnitId::Special_WarpGate - unit->id;
				morphBuildingInto(unit, morphVariant, IscriptAnimation::Unused1, false);
				unit->rally.unit = unit;
			}
			manageMorphing(unit, 20);
		}

		//Set unit id
		else if (unit->id == UnitId::TerranCommandCenter
			|| unit->id == UnitId::Special_IonCannon
			|| unit->id == UnitId::TerranComsatStation)
		{
			//Set condition to morph
				//Morph into Planetary F.
			if (unit->mainOrderId == OrderId::ReaverStop)
			{
				//unit who is morphing
				//id to morph into
				//animation with the morph animation (unit--->newUnit)
				//Can be canceled?
				morphBuildingInto(unit, UnitId::Special_IonCannon, IscriptAnimation::Unused1, true);
			}
			//Morph into Orbital C.
			else if (unit->id == OrderId::CarrierStop)
			{
				morphBuildingInto(unit, UnitId::TerranComsatStation, IscriptAnimation::Unused1, true);
			}
			//Run morph behaviour: the second value is only if you want any fixed morph time
			manageMorphing(unit);
		}
	}
}

namespace plugins
{
	//KYSXD helpers
	CUnit* nearestTemplarMergePartner(CUnit* unit)
	{

		CUnit* nearest_unit = NULL;
		u32 best_distance = MAXINT32;

		for (u32 i = 0; i < *clientSelectionCount; ++i)
		{
			CUnit* nextUnit = clientSelectionGroup->unit[i];

			if (nextUnit != NULL
				&& nextUnit != unit
				&& unit->id == (UnitId::ProtossDarkTemplar + UnitId::ProtossHighTemplar) - nextUnit->id
				&& nextUnit->mainOrderId == OrderId::ReaverStop)
			{

				u32 x_distance, y_distance;
				x_distance =
					unit->getX() -
					nextUnit->getX();

				x_distance *= x_distance;

				y_distance =
					unit->getY() -
					nextUnit->getY();

				y_distance *= y_distance;

				if ((x_distance + y_distance) < best_distance)
				{

					CUnit* new_nearest_unit = nextUnit;
					nextUnit = nearest_unit;
					nearest_unit = new_nearest_unit;
					best_distance = x_distance + y_distance;

				}
			}
		}

		return nearest_unit;
	}

	//Same as chargeTarget_ in unit_speed.cpp
	bool chargeTargetInRange(const CUnit* zealot)
	{
		if (!zealot->orderTarget.unit)
			return false;
		CUnit* chargeTarget = zealot->orderTarget.unit;
		int maxChargeRange = 3 << 5;
		int minChargeRange = 16;
		int chargeRange = zealot->getDistanceToTarget(zealot->orderTarget.unit);
		if (zealot->mainOrderId != OrderId::AttackUnit)
			return false;
		else if (minChargeRange > chargeRange
			|| chargeRange > maxChargeRange)
			return false;
		else if (unitHasPathToUnitOnGround(zealot, chargeTarget))
			return true;
		return false;
	}

	bool isInHarvestState(const CUnit* worker)
	{
		if (!(worker->status & UnitStatus::IsGathering))
		{
			return false;
		}
		/*if(worker->unusedTimer == 0) {
			return false;*/
			//}
		else return true;
	}

	bool isMineral(const CUnit* resource)
	{
		// this covers the entire range ResourceMineralField, ResourceMineralFieldType2, ResourceMineralFieldType3
		if (UnitId::ResourceMineralField <= resource->id && resource->id <= UnitId::ResourceMineralFieldType3)
		{
			return true;
		}
		else return false;
	}

	bool isGasSupply(const CUnit* resource)
	{
		if (units_dat::BaseProperty[resource->id] & UnitProperty::ResourceContainer)
		{
			return true;
		}
		else return false;

	}

	bool hasHarvestOrders(const CUnit* worker)
	{
		if (OrderId::Harvest1 <= worker->mainOrderId &&
			worker->mainOrderId <= OrderId::Harvest5)
		{
			return true;
		}
		else return false;
	}

	class HarvestTargetFinder : public scbw::UnitFinderCallbackMatchInterface
	{
		CUnit* mainHarvester;
	public:
		void setmainHarvester(CUnit* mainHarvester)
		{
			this->mainHarvester = mainHarvester;
		}
		bool match(CUnit* unit)
		{
			if (!unit)
				return false;

			if (mainHarvester->getDistanceToTarget(unit) > (16 << 5)) //Harvest distance
				return false;

			if (!(isMineral(unit)))
				return false;

			return true;
		}
	};
	HarvestTargetFinder harvestTargetFinder;

	class ContainerTargetFinder : public scbw::UnitFinderCallbackMatchInterface
	{
		CUnit* mineral;
		u32 unitStatusFlags;
	public:
		void setResource(CUnit* resource)
		{
			this->mineral = resource;
		}
		void setStatusFlags(u32 unitStatusFlags)
		{
			this->unitStatusFlags = unitStatusFlags;
		}
		bool match(CUnit* unit)
		{
			if (!unit)
				return false;

			if (!(units_dat::BaseProperty[unit->id] & UnitProperty::ResourceDepot))
				return false;

			if (!(unit->status & UnitStatus::Completed))
				return false;

			if (scbw::getActiveTileAt(mineral->getX(), mineral->getY()).groundHeight
				!= scbw::getActiveTileAt(unit->getX(), unit->getY()).groundHeight)
				return false;

			if (!(unit->hasPathToUnit(mineral)))
				return false;

			if (!(unit->status & unitStatusFlags) && unitStatusFlags > 0)
				return false;

			return true;
		}
	};
	ContainerTargetFinder containerTargetFinder;

	void exploreMap()
	{
		for (int x = 0; x < mapTileSize->width; x++)
		{
			for (int y = 0; y < mapTileSize->height; y++)
			{

				if ((x + y) % 2)
				{
					ActiveTile* currentTile = &(*activeTileArray)[(x)+mapTileSize->width * (y)];
					currentTile->exploredFlags = 0;

				}

			}
		}
		//By RavenWolf:
		//if vespene geyser or a mineral field, reveal it to all players
		for (CUnit* unit = *firstVisibleUnit; unit; unit = unit->link.next)
		{
			if (isMineral(unit)
				|| unit->id == UnitId::ResourceVespeneGeyser)
			{
				unit->sprite->visibilityFlags = 0xFF;
			}
		}
	}

	void runFirstFrameBehaviour()
	{
		scbw::printText("You're now playing:\n     "
			PLUGIN_NAME "\n     By " AUTHOR_NAME);
		//All nexi in 50
		for (CUnit* unit = *firstVisibleUnit; unit; unit = unit->link.next)
		{
			switch (unit->id)
			{
			case UnitId::ProtossNexus:
				unit->energy = 50 << 8;
				break;
			case UnitId::ProtossGateway:
			case UnitId::Special_WarpGate:
				unit->previousUnitType = UnitId::None;
				break;
			default: break;
			}
		}
		//KYSXD - For non-custom games - start
		if (*GAME_TYPE != GameType::UseMapSettings)
		{
			exploreMap();
			CUnit* firstMineral[8];
			u16 initialworkeramount = 12;
			for (CUnit* base = *firstVisibleUnit; base; base = base->link.next)
			{
				//KYSXD - Increase initial amount of Workers - From GagMania
				if (units_dat::BaseProperty[base->id] & UnitProperty::ResourceDepot)
				{
					//KYSXD - Find nearest mineral patch
					harvestTargetFinder.setmainHarvester(base);
					firstMineral[base->playerId] = scbw::UnitFinder::getNearestTarget(
						base->getX() - 512, base->getY() - 512,
						base->getX() + 512, base->getY() + 512,
						base,
						harvestTargetFinder);
				}
			}
			for (CUnit* worker = *firstVisibleUnit; worker; worker = worker->link.next)
			{
				//KYSXD Send all units to harvest on first run
				if (units_dat::BaseProperty[worker->id] & UnitProperty::Worker)
				{
					if (firstMineral[worker->playerId])
					{
						worker->orderTo(OrderId::Harvest1, firstMineral[worker->playerId]);
					}
				}
			}
		}
		//For non-custom games - end  
	}

	void showUnitGraphicHelpers(CUnit* unit)
	{
		if (unit->id == UnitId::TerranSiegeTankSiegeMode
			|| unit->id == UnitId::Hero_EdmundDukeSiegeMode)
		{
			graphics::drawCircle(unit->getX(), unit->getY(),
				unit->getMaxWeaponRange(units_dat::GroundWeapon[unit->subunit->id]) + 30,
				graphics::TEAL, graphics::ON_MAP);
		}

		if (unit->id == UnitId::ZergNydusCanal
			&& unit->building.nydusExit != NULL)
		{
			graphics::drawLine(unit->getX(), unit->getY(),
				unit->building.nydusExit->getX(), unit->building.nydusExit->getY(),
				graphics::ORANGE, graphics::ON_MAP);
		}

		//Display rally points for factories selected
		if (unit->status & UnitStatus::GroundedBuilding
			&& unit->rally.pt.x
			&& unit->rally.pt.y
			&& unit->id != UnitId::ProtossPylon
			&& (unit->playerId == *LOCAL_NATION_ID || scbw::isInReplay()))
		{
			const CUnit* rallyUnit = unit->rally.unit;
			//Rallied to self; disable rally altogether
			if (rallyUnit != unit)
			{
				//Is usable rally unit
				if (rallyUnit && rallyUnit->playerId == unit->playerId)
				{
					graphics::drawLine(unit->getX(), unit->getY(),
						rallyUnit->getX(), rallyUnit->getY(),
						graphics::GREEN, graphics::ON_MAP);
					graphics::drawCircle(rallyUnit->getX(), rallyUnit->getY(), 10,
						graphics::GREEN, graphics::ON_MAP);
				}
				//Use map position instead
				else if (unit->rally.pt.x != 0)
				{
					graphics::drawLine(unit->getX(), unit->getY(),
						unit->rally.pt.x, unit->rally.pt.y,
						graphics::YELLOW, graphics::ON_MAP);
					graphics::drawCircle(unit->rally.pt.x, unit->rally.pt.y, 10,
						graphics::YELLOW, graphics::ON_MAP);
				}
			}

			//Show worker rally point
			const CUnit* workerRallyUnit = unit->moveTarget.unit;
			if (workerRallyUnit)
			{
				graphics::drawLine(unit->getX(), unit->getY(),
					workerRallyUnit->getX(), workerRallyUnit->getY(),
					graphics::ORANGE, graphics::ON_MAP);
				graphics::drawCircle(
					workerRallyUnit->getX(), workerRallyUnit->getY(), 10,
					graphics::ORANGE, graphics::ON_MAP);
			}

		}
	}

	//KYSXD worker no collision if harvesting - start
	void manageWorkerCollision(CUnit* unit)
	{
		if (units_dat::BaseProperty[unit->id] & UnitProperty::Worker)
		{
			if (hasHarvestOrders(unit))
			{
				unit->unusedTimer = 2;
				manageUnitStatusFlags(unit, UnitStatus::NoCollide, true);
			}
			else
			{
				manageUnitStatusFlags(unit, UnitStatus::NoCollide, false);
			}
		}
	}

	//KYSXD PsionicTransfer cast
	//Original from Eisetley, KYSXD modification
	void runAdeptPsionicTransfer_cast(CUnit* adept)
	{
		if (adept->id == UnitId::Hero_Raszagal
			&& adept->mainOrderId == OrderId::PlaceScanner
			&& adept->orderSignal == 2)
		{

			adept->mainOrderId = OrderId::Nothing2;
			CUnit* shade = scbw::createUnitAtPos(UnitId::aldaris, adept->playerId, adept->getX(), adept->getY());
			if (shade)
			{
				shade->connectedUnit = adept;
				shade->unusedTimer = 2 * 7;

				shade->status |= UnitStatus::NoCollide;
				shade->status |= UnitStatus::IsGathering;
				if (adept->orderTarget.unit)
				{
					shade->orderTo(OrderId::Follow, adept->orderTarget.unit);
				}
				else
					shade->orderTo(OrderId::Move, adept->orderTarget.pt.x, adept->orderTarget.pt.y);

				if (!scbw::isCheatEnabled(CheatFlags::TheGathering))
				{
					adept->energy = 0;
				}
			}
		}
	}

	//KYSXD PsionicTransfer behavior
	//Original from Eisetley, KYSXD modification
	void runAdeptPsionicTransfer_behavior(CUnit* shade)
	{
		if (shade->id == UnitId::aldaris)
		{
			CUnit* adept = shade->connectedUnit;

			if (!adept)
			{
				shade->orderTo(OrderId::Die);
			}
			else
			{
				if (adept->getCurrentHpInGame() <= 0)
				{
					shade->orderTo(OrderId::Die);
				}

				if (!shade->unusedTimer)
				{
					//      createThingy(375, adept->getX(), adept->getY(), adept->playerId);
					CImage* adeptSprite = adept->sprite->images.head;
					setImageDirection(adeptSprite, shade->currentDirection1);

					u16 thisX = shade->getX();
					u16 thisY = shade->getY();
					scbw::moveUnit(adept, thisX, thisY);
					/*
									shade->userActionFlags |= 0x4;
									shade->remove();
									adept->sprite->createOverlay(557);
									scbw::playSound(1140, adept);
					*/
				}
			}
		}
	}

	//KYSXD Chrono boost cast
	void runChronoBoost_cast(CUnit* unit)
	{
		if (unit->id == UnitId::ProtossNexus
			&& unit->status & UnitStatus::Completed)
		{
			if (unit->mainOrderId == OrderId::PlaceScanner
				|| unit->orderSignal == 2)
			{
				if (unit->orderTarget.unit
					&& scbw::isAlliedTo(unit->playerId, unit->orderTarget.unit->playerId))
				{
					unit->orderTarget.unit->stimTimer = 60; //~30 seconds
					unit->mainOrderId = OrderId::Nothing2;
					if (!scbw::isCheatEnabled(CheatFlags::TheGathering))
					{
						unit->energy -= 50 << 8;
					}
				}
				else unit->mainOrderId = OrderId::Nothing2;
			}
		}
	}

	//KYSXD Chrono boost behavior
	//Should be moved to update_unit_state
	void runChronoBoost_behavior(CUnit* unit)
	{
		if (unit->status & UnitStatus::GroundedBuilding
			&& unit->status & UnitStatus::Completed
			&& unit->stimTimer
			&& !(unit->isFrozen()))
		{
			//If the building is working - start
			switch (unit->mainOrderId)
			{
			case OrderId::Upgrade:
			case OrderId::ResearchTech:
				if (unit->building.upgradeResearchTime)
				{
					unit->building.upgradeResearchTime -= std::min<u16>(unit->building.upgradeResearchTime, 4);
				}
				break;
			default: break;
			}
			switch (unit->secondaryOrderId)
			{
			case OrderId::BuildAddon:
			case OrderId::Train:
				if (unit->currentBuildUnit)
				{
					CUnit* unitInQueue = unit->currentBuildUnit;
					if (unitInQueue->remainingBuildTime)
					{
						static s32 hpHolder;
						static s32 maxHp;
						maxHp = units_dat::MaxHitPoints[unitInQueue->id];
						if (scbw::isCheatEnabled(CheatFlags::OperationCwal))
						{
							unitInQueue->remainingBuildTime -= std::min<u16>(unitInQueue->remainingBuildTime, 16);

							hpHolder = unitInQueue->hitPoints + (unitInQueue->buildRepairHpGain << 4);
							unitInQueue->hitPoints = std::min<s32>(maxHp, hpHolder);
						}
						else
						{
							unitInQueue->remainingBuildTime--;

							hpHolder = unitInQueue->hitPoints + unitInQueue->buildRepairHpGain;
							unitInQueue->hitPoints = std::min<s32>(maxHp, hpHolder);
						}
					}
				}
				break;
			default: break;
			}
			//Case: Warpgate
			if (unit->id == UnitId::Special_WarpGate
				&& unit->previousUnitType != UnitId::None)
			{
				u32 energyHolder = unit->energy + 17; //Should be on update_unit_state.cpp?
				unit->energy = std::min<u32>(unit->getMaxEnergy(), energyHolder);
			}
			//Case: Larva spawn - Check larva_creep_spawn.cpp
		}
	}

	//KYSXD stalker's blink
	void runStalkerBlink(CUnit* unit)
	{
		if (unit->id == UnitId::Hero_FenixDragoon)
		{
			if (unit->mainOrderId == OrderId::CastOpticalFlare
				|| unit->orderSignal == 2)
			{
				u16 thisX = unit->orderTarget.pt.x;
				u16 thisY = unit->orderTarget.pt.y;
				scbw::moveUnit(unit, thisX, thisY);
				unit->mainOrderId = OrderId::Nothing2;
				if (!scbw::isCheatEnabled(CheatFlags::TheGathering))
				{
					unit->energy = 0;
				}
			}
		}
	}

	//KYSXD zealot's charge
	void runZealotCharge(CUnit* unit)
	{
		//Check max_energy.cpp and unit_speed.cpp for more info
		if (unit->id == UnitId::ProtossZealot
			&& unit->status & UnitStatus::SpeedUpgrade)
		{
			//Unit isn't in charge state
			if (!unit->stimTimer)
			{
				if (chargeTargetInRange(unit))
				{
					//Must be: orderTo(CastStimPack)
					if (unit->energy == unit->getMaxEnergy())
					{
						unit->stimTimer = 5;
						unit->energy = 0;
					}
				}
			}
		}
	}

	//KYSXD Reactor behaviour start
	void runReactorBehaviour(CUnit* unit)
	{
		//Check unit state
		if ((unit->id == UnitId::TerranBarracks
			|| unit->id == UnitId::TerranFactory
			|| unit->id == UnitId::TerranStarport)
			&& unit->status & UnitStatus::Completed
			&& unit->mainOrderId != OrderId::Die)
		{
			//Check add-on
			if (unit->building.addon != NULL)
			{
				CUnit* reactor = unit->building.addon;
				reactor->rally = unit->rally; //Update rally

				//If the unit is trainning, do reactor behavior
				if (unit->secondaryOrderId == OrderId::Train)
				{
					u16 changeQueue = (unit->buildQueueSlot + 1) % 5;
					u16 tempId = unit->buildQueue[changeQueue];
					//If the addon isn't trainning
					if (tempId != UnitId::None
						&& reactor->buildQueue[reactor->buildQueueSlot] == UnitId::None)
					{
						reactor->buildQueue[reactor->buildQueueSlot] = tempId;
						reactor->setSecondaryOrder(OrderId::Train);

						//Update remaining queue
						u16 queueId[5];
						for (int i = 0; i < 5; i++)
						{
							if (i != 4)
							{
								queueId[i] = unit->buildQueue[(unit->buildQueueSlot + i + 1) % 5];
							}
							else queueId[i] = UnitId::None;
						}
						for (int i = 1; i < 5; i++)
						{
							unit->buildQueue[(unit->buildQueueSlot + i) % 5] = queueId[i];
						}
					}
				}
				/*
				//If isn't trainning, disable reactor
				else if(reactor->secondaryOrderId == OrderId::Train) {
					unit->buildQueue[unit->buildQueueSlot] = reactor->buildQueue[reactor->buildQueueSlot];
					unit->setSecondaryOrder(OrderId::Train);
					if(unit->currentBuildUnit) {
						unit->currentBuildUnit->remainingBuildTime =
							reactor->currentBuildUnit->remainingBuildTime;
					}
					reactor->setSecondaryOrder(OrderId::Nothing2);
					//Update remaining queue
					u16 queueId[5];
					for (int i = 0; i < 5; i++) {
						if(i != 4) {
							queueId[i] = reactor->buildQueue[(reactor->buildQueueSlot+i+1)%5];
						}
						else queueId[i] = UnitId::None;
					}
					for (int i = 0; i < 5; i++) {
						reactor->buildQueue[(reactor->buildQueueSlot+i)%5] = queueId[i];
					}
				}
				*/
			}
		}
	} //KYSXD Reactor behaviour end

	//KYSXD - Burrow movement start
	void runBurrowedMovement(CUnit* unit)
	{
		if (unit->id == UnitId::ZergZergling
			&& unit->playerId == *LOCAL_HUMAN_ID)
		{
			if (unit->mainOrderId == OrderId::Burrow)
			{
				unit->_unused_0x106 = (u8)true;
			}

			if (unit->_unused_0x106
				&& unit->status & UnitStatus::Burrowed
				&& unit->status & UnitStatus::CanNotReceiveOrders)
			{

				unit->status &= ~UnitStatus::Burrowed;
				unit->status &= ~UnitStatus::CanNotReceiveOrders;
				unit->status |= UnitStatus::IsGathering;

				unit->flingyMovementType = 0;
				unit->flingyTopSpeed = 1280;
				unit->flingyAcceleration = 128;
				unit->flingyTurnSpeed = 40;
			}

			if (unit->_unused_0x106
				&& unit->mainOrderId != OrderId::Unburrow
				&& unit->sprite->mainGraphic->animation != IscriptAnimation::Burrow)
			{
				unit->playIscriptAnim(IscriptAnimation::Burrow);
			}

			if (unit->_unused_0x106
				&& OrderId::Attack1 <= unit->mainOrderId
				&& unit->mainOrderId <= OrderId::AttackMove)
			{
				if (unit->orderTarget.unit)
				{
					unit->orderTo(OrderId::Move, unit->orderTarget.unit);
				}
				else if (unit->orderTarget.pt.x
					&& unit->orderTarget.pt.y)
				{
					unit->orderTo(OrderId::Move, unit->orderTarget.pt.x, unit->orderTarget.pt.y);
				}
				else unit->orderTo(OrderId::Nothing2);
			}

			if (unit->mainOrderId == OrderId::ReaverStop
				&& unit->_unused_0x106)
			{
				unit->_unused_0x106 = (u8)false;

				unit->status &= ~UnitStatus::IsGathering;
				unit->status |= UnitStatus::Burrowed;
				unit->flingyMovementType = 2;

				unit->orderTo(OrderId::Unburrow);
			}
		}
	} //KYSXD - Burrow movement end



	void showWorkerCount(CUnit* unit)
	{
		u32 baseProp = units_dat::BaseProperty[unit->id];
		int gasWorkers = 0;
		//
		if (units_dat::BaseProperty[unit->id] & UnitProperty::ResourceContainer && unit->playerId == *LOCAL_NATION_ID)
		{

			//this gets all visible units
			static scbw::UnitFinder resourcefinder;
			resourcefinder.search(unit->getX() - 256, unit->getY() - 256, unit->getX() + 256, unit->getY() + 256);

			for (int k = 0; k < resourcefinder.getUnitCount(); ++k)
			{
				CUnit* thisUnit = resourcefinder.getUnit(k);
				if (thisUnit->playerId == *LOCAL_NATION_ID
					&& (units_dat::BaseProperty[thisUnit->id] & UnitProperty::Worker)
					&& isInHarvestState(thisUnit)
					&& thisUnit->worker.targetResource.unit != NULL
					&& thisUnit->worker.targetResource.unit == unit)
				{
					gasWorkers++;
				}

			}

			for (CUnit* invisibleunit = *firstHiddenUnit; invisibleunit; invisibleunit = invisibleunit->link.next)
			{
				//btech - Worker Count both visible and invisible (updated from KYSXD code)
				//show workers for visible and invisible
				if (invisibleunit->playerId == *LOCAL_NATION_ID
					&& units_dat::BaseProperty[invisibleunit->id] & UnitProperty::Worker
					&& invisibleunit->worker.targetResource.unit != NULL
					&& invisibleunit->worker.targetResource.unit == unit)
				{
					gasWorkers++;
				}
			}
		}

		//KYSXD unit worker count start  
		if (unit->playerId == *LOCAL_NATION_ID
			&& unit->status & UnitStatus::Completed //ResourceContainer is items like Refinery
			&& (units_dat::BaseProperty[unit->id] & UnitProperty::ResourceDepot || units_dat::BaseProperty[unit->id] & UnitProperty::ResourceContainer))
		{

			int nearmineral = 0;
			int gasSupply = 0;
			int nearworkers = 0;
			CUnit* nearestContainer = NULL;

			//KYSXD count the nearest minerals
			static scbw::UnitFinder resourcefinder;
			resourcefinder.search(unit->getX() - 300, unit->getY() - 300, unit->getX() + 300, unit->getY() + 300);
			for (int k = 0; k < resourcefinder.getUnitCount(); ++k)
			{
				CUnit* currentResource = resourcefinder.getUnit(k);
				if (isMineral(currentResource))
				{
					containerTargetFinder.setResource(currentResource);
					containerTargetFinder.setStatusFlags(UnitStatus::GroundedBuilding);
					nearestContainer = scbw::UnitFinder::getNearestTarget(
						currentResource->getX() - 300, currentResource->getY() - 300,
						currentResource->getX() + 300, currentResource->getY() + 300,
						currentResource,
						containerTargetFinder);
					if (nearestContainer != NULL
						&& nearestContainer == unit)
					{
						nearmineral++;
					}
				}

				//start gas supply
				if (isGasSupply(currentResource))
				{
					gasSupply = 3; //btech: capped at 3 for now. This could be improved by detecting distance and dividing that distance by gather time 
				}


				for (int k = 0; k < resourcefinder.getUnitCount(); ++k)
				{
					CUnit* thisUnit = resourcefinder.getUnit(k);
					if (thisUnit->playerId == *LOCAL_NATION_ID
						&& (units_dat::BaseProperty[thisUnit->id] & UnitProperty::Worker)
						&& isInHarvestState(thisUnit)
						&& thisUnit->worker.targetResource.unit != NULL
						&& thisUnit->worker.targetResource.unit == currentResource
						&& currentResource->id != UnitId::TerranRefinery
						&& currentResource->id != UnitId::ProtossAssimilator
						&& currentResource->id != UnitId::ZergExtractor)
					{
						nearworkers++;
					}
				}
				//end gas supply
			}
			//KYSXD count the nearest Workers
			if (nearmineral > 0 && units_dat::BaseProperty[unit->id] & UnitProperty::ResourceDepot)
			{
				static char unitcount[64];
				sprintf_s(unitcount, "%d/%d", nearworkers, nearmineral * 2); //* 2 since 2 units can use a single mineral at a time
				graphics::drawText(unit->getX() - 12, unit->getY() - 72, unitcount, graphics::FONT_LARGE, graphics::ON_MAP);

			}
			if (gasSupply > 0 && units_dat::BaseProperty[unit->id] & UnitProperty::ResourceContainer)
			{
				static char unitcount[64];
				sprintf_s(unitcount, "%d/%d", gasWorkers, gasSupply); //* 2 since 2 units can use a single mineral at a time
				graphics::drawText(unit->getX() - 12, unit->getY() - 72, unitcount, graphics::FONT_LARGE, graphics::ON_MAP);
			}
		}
		//KYSXD unit worker count end

	} //namespace plugins

	void showProgressBar(CUnit* unit, u16 remainingBuildTime, u16 totalBuildTime)
	{
		totalBuildTime = totalBuildTime == 0 ? totalBuildTime = units_dat::TimeCost[unit->id] : totalBuildTime;

		if (remainingBuildTime > totalBuildTime)
			return;
		if (totalBuildTime == 0)
			return; //prevent divide by zero

		//this condition happens when a zerg egg morphs into something, multiple progress bars are built which shouldn't happen
		if (unit->status & UnitStatus::CanTurnAroundToAttack
			&& unit->status & UnitStatus::IsBuilding
			&& unit->status & UnitStatus::IgnoreTileCollision)
		{
			return;
		}

		graphics::drawFilledBox(unit->getX() - 59, unit->getY() - 51, unit->getX() + 59, unit->getY() - 53, graphics::GREY, graphics::ON_MAP);
		graphics::drawBox(unit->getX() + 60, unit->getY() - 50, unit->getX() - 60, unit->getY() - 54, graphics::BLUE, graphics::ON_MAP);
		u16 buildTimeSoFar = totalBuildTime - remainingBuildTime;
		int pixelsBuilt = ((buildTimeSoFar * 118) / totalBuildTime) - 59;
		graphics::drawFilledBox(unit->getX() - 59, unit->getY() - 51, unit->getX() + pixelsBuilt, unit->getY() - 53, graphics::GREEN, graphics::ON_MAP);
	}
}

namespace warpgateMechanic
{
	void manageWarpgateFlags(CUnit* warpgate)
	{
		if (warpgate->getMaxEnergy()
			&& warpgate->getMaxEnergy() == warpgate->energy)
		{
			warpgate->previousUnitType = UnitId::None;
			warpgate->building.addon = NULL;
		}
	}

	void cleanWarpgate(CUnit* warpgate)
	{
		warpgate->buildQueue[warpgate->buildQueueSlot] = UnitId::None;
		warpgate->mainOrderId = OrderId::Nothing2;
		warpgate->secondaryOrderId = OrderId::Nothing2;
		warpgate->building.addon = warpgate;
	}

	void setWarpUnit(CUnit* warpUnit)
	{
		if (warpUnit)
		{

			u16 warpSeconds = 5;
			u16 warpTime = warpSeconds * 15;

			s32 maxShield = (s32)(units_dat::MaxShieldPoints[warpUnit->id]) << 8;
			s32 maxHp = units_dat::MaxHitPoints[warpUnit->id];

			s32 hpRegen = maxHp / warpTime;
			s32 shieldRegen = maxShield / warpTime;

			//Set unit
			warpUnit->buildRepairHpGain = hpRegen;
			warpUnit->shieldGain = shieldRegen;
			warpUnit->remainingBuildTime = warpTime;

			warpUnit->hitPoints = 1;
			warpUnit->shields = 1;

			replaceSpriteImages(warpUnit->sprite,
				ImageId::ArchonBirth_Unused, warpUnit->currentDirection1);
			warpUnit->status &= ~UnitStatus::Completed;
			warpUnit->orderTo(OrderId::BuildSelf2);
			warpUnit->currentButtonSet = UnitId::Buildings;
		}
	}

	void runWarpgatePlacing(CUnit* unit)
	{
		if (unit->buildQueue[unit->buildQueueSlot] != UnitId::None)
		{

			u16 warpUnitId = unit->buildQueue[unit->buildQueueSlot];
			unit->previousUnitType = warpUnitId;

			CUnit* warpUnit =
				scbw::createUnitAtPos(warpUnitId,
					unit->playerId,
					unit->orderTarget.pt.x,
					unit->orderTarget.pt.y);
			setWarpUnit(warpUnit);

			cleanWarpgate(unit);
		}
	}

	void runWarpgate(CUnit* warpgate)
	{
		buildingMorph::runBuildingMorph(warpgate);

		if (warpgate->id == UnitId::Special_WarpGate)
		{
			runWarpgatePlacing(warpgate);
			manageWarpgateFlags(warpgate);
		}
	}
} //namespace warpagteMechanic

namespace buildingPreview
{
	u16 previewParent[UNIT_ARRAY_LENGTH + 1];
	u8 previewPlayerId[UNIT_ARRAY_LENGTH + 1];

	void saveUnitAsParent(CUnit* parent, CUnit* unit)
	{
		previewParent[unit->getIndex()] = parent->getIndex();
		previewPlayerId[unit->getIndex()] = parent->playerId;
	}

	CUnit* getPrevParent(CUnit* preview)
	{
		return CUnit::getFromIndex(previewParent[preview->getIndex()]);
	}

	u8 getPrevPlayerId(CUnit* preview)
	{
		return previewPlayerId[preview->getIndex()];
	}

	void cleanPrevPlayerId(CUnit* preview)
	{
		previewPlayerId[preview->getIndex()] = NULL;
	}

	const u32 Func_ConvertToHallucination = 0x004F6180;
	void convertToHallucination(CUnit* unit)
	{

		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_ConvertToHallucination
			POPAD
		}

	}

	const u32 Func_UseHallucinationDrawfunc = 0x00497DC0;;
	void useHallucinationDrawfunc(CSprite* sprite, u32 palette = 0)
	{

		__asm {
			PUSHAD
			MOV EAX, sprite
			MOV EBX, palette
			CALL Func_UseHallucinationDrawfunc
			POPAD
		}

	}

	bool IsBuildOrder(u16 orderId)
	{
		switch (orderId)
		{
		case OrderId::BuildTerran:
		case OrderId::BuildProtoss1:
			return true;
		default: return false;

		}
	}

	bool isValidPreview(CUnit* preview)
	{
		CUnit* parent = CUnit::getFromIndex(previewParent[preview->getIndex()]);
		if (preview->status & UnitStatus::IsHallucination
			&& preview->playerId == 11
			&& (!parent
				|| !IsBuildOrder(parent->mainOrderId)
				|| parent->orderTarget.unit != preview
				|| parent->orderTarget.pt.x != preview->getX()
				|| parent->orderTarget.pt.y != preview->getY()))
		{
			return false;
		}
		return true;
	}

	void removePreview(CUnit* preview)
	{
		if (!isValidPreview(preview))
		{

			preview->sprite->visibilityFlags = 0;
			ActiveTile* tile = &scbw::getActiveTileAt(preview->getX(), preview->getY());
			tile->visibilityFlags &= ~(1 << getPrevPlayerId(preview));
			cleanPrevPlayerId(preview);

			preview->remove();

		}
	}

	void freeOccupiedTiles(CUnit* building)
	{
		Point16 const dimensions = units_dat::BuildingDimensions[building->id];;
		u16 init_x = building->getX();
		init_x -= std::min(init_x, (u16)(dimensions.x / 2));
		u16 init_y = building->getY();
		init_y -= std::min(init_y, (u16)(dimensions.y / 2));

		for (int tile_x = 0; tile_x < dimensions.x; tile_x++)
		{
			for (int tile_y = 0; tile_y < dimensions.y; tile_y++)
			{
				ActiveTile* tile = &scbw::getActiveTileAt(init_x + tile_x, init_y + tile_y);
				if (tile) tile->currentlyOccupied &= ~1;
			}
		}
	}

	bool placementIsFree(u16 unitId, u16 pos_x, u16 pos_y)
	{
		Point16 const dimensions = units_dat::BuildingDimensions[unitId];;
		u16 init_x = pos_x;
		init_x -= std::min(init_x, (u16)(dimensions.x / 2));
		u16 init_y = pos_y;
		init_y -= std::min(init_y, (u16)(dimensions.y / 2));

		for (int tile_x = 0; tile_x < dimensions.x; tile_x += 32)
		{
			for (int tile_y = 0; tile_y < dimensions.y; tile_y += 32)
			{
				ActiveTile* tile = &scbw::getActiveTileAt(init_x + tile_x, init_y + tile_y);
				if (tile && tile->currentlyOccupied == 1) return false;
			}
		}
		return true;
	}

	void setPreview(CUnit* preview)
	{
		preview->userActionFlags |= 0x4;

		convertToHallucination(preview);
		manageUnitStatusFlags(preview, UnitStatus::NoCollide, true);
		manageUnitStatusFlags(preview, UnitStatus::Invincible, true);

		if (preview->sprite)
		{
			useHallucinationDrawfunc(preview->sprite);
			preview->sprite->visibilityFlags = (1 << getPrevPlayerId(preview));

			if (preview->sprite->mainGraphic)
			{
				preview->sprite->mainGraphic->flags &= ~CImage_Flags::Clickable;
			}
		}
	}

	CUnit* placePreviewAt(u16 unitId, u16 pos_x, u16 pos_y)
	{

		bool freeTiles = placementIsFree(unitId, pos_x, pos_y);
		CUnit* preview = scbw::createUnitAtPos(unitId,
			11,
			pos_x,
			pos_y);
		if (freeTiles) freeOccupiedTiles(preview);

		return preview;
	}

	void createPreview(CUnit* unit)
	{
		u16 creationId = unit->buildQueue[unit->buildQueueSlot];
		if (creationId != UnitId::None
			&& unit->orderTarget.unit == NULL
			&& IsBuildOrder(unit->mainOrderId))
		{

			CUnit* preview = placePreviewAt(creationId,
				unit->orderTarget.pt.x,
				unit->orderTarget.pt.y);

			if (preview)
			{
				unit->orderTarget.unit = preview;
				saveUnitAsParent(unit, preview);

				setPreview(preview);
			}
		}
	}

	void manageBuildingPreview(CUnit* unit)
	{
		removePreview(unit);
		createPreview(unit);
	}
}

namespace smartCasting
{
	bool isOrderValidForSmartcasting(u16 orderId)
	{
		switch (orderId)
		{
		case OrderId::WarpingArchon:
		case OrderId::FireYamatoGun1:
		case OrderId::MagnaPulse:
		case OrderId::DarkSwarm:
		case OrderId::CastParasite:
		case OrderId::SummonBroodlings:
		case OrderId::EmpShockwave:
		case OrderId::NukePaint:
		case OrderId::PlaceMine:
		case OrderId::DefensiveMatrix:
		case OrderId::PsiStorm:
		case OrderId::Irradiate:
		case OrderId::Plague:
		case OrderId::Consume:
		case OrderId::Ensnare:
		case OrderId::StasisField:
		case OrderId::Hallucianation1:
		case OrderId::Restoration:
		case OrderId::CastDisruptionWeb:
		case OrderId::CastMindControl:
		case OrderId::WarpingDarkArchon:
		case OrderId::CastFeedback:
		case OrderId::CastOpticalFlare:
		case OrderId::CastMaelstrom:
			return true;
			break;
		default:
			return false;
			break;
		}
	}

	//Adds a node to the order's stack
	void addToOrderList(CUnit* unit)
	{
		int currentOrder = unit->mainOrderId;
		int currentPlayer = unit->playerId;
		int index;

		if (!firstCaster[currentOrder][currentPlayer])
		{
			firstCaster[currentOrder][currentPlayer] = unit;
			lastCaster[currentOrder][currentPlayer] = unit;

			index = lastCaster[currentOrder][currentPlayer]->getIndex();

			custom_unit_array[index] = NULL;

		}
		else
		{
			index = lastCaster[currentOrder][currentPlayer]->getIndex();
			custom_unit_array[index] = unit;
			lastCaster[currentOrder][currentPlayer] = unit;

			index = lastCaster[currentOrder][currentPlayer]->getIndex();
			custom_unit_array[index] = NULL;
		}
	}

	//Get a COrder pointer with a custom order
	COrder* createOrder(u16 orderId = OrderId::Nothing2, u16 unitId = 0, CUnit* target = NULL, u16 posX = 0, u16 posY = 0)
	{
		static COrder thisOrder;
		thisOrder.prev = NULL;
		thisOrder.next = NULL;
		thisOrder.orderId = orderId;
		thisOrder.unitId = unitId;
		thisOrder.target.unit = target;
		thisOrder.target.pt.x = posX;
		thisOrder.target.pt.y = posY;

		return &thisOrder;
	}

	//Get a COrder pointer with the current order
	inline COrder* getCurrentOrder(CUnit* unit)
	{
		return createOrder((u16)unit->mainOrderId,
			unit->buildQueue[unit->buildQueueSlot],
			unit->orderTarget.unit,
			unit->orderTarget.pt.x,
			unit->orderTarget.pt.y);
	}

	//Get a COrder pointer with the last order
	inline COrder* getLastOrder(CUnit* unit)
	{
		int index = unit->getIndex();
		COrder* unitLastOrder = &lastOrderArray[index];
		return createOrder(unitLastOrder->orderId,
			unitLastOrder->unitId,
			unitLastOrder->target.unit,
			unitLastOrder->target.pt.x,
			unitLastOrder->target.pt.y);
	}

	//Stores all variables of COrder in the unit using unused members of CUnit
	inline void saveAsLastOrder(CUnit* unit, COrder* lastOrder = NULL)
	{
		if (lastOrder)
		{
			int index = unit->getIndex();
			COrder* unitLastOrder = &lastOrderArray[index];

			unitLastOrder->orderId = (u8)lastOrder->orderId;
			unitLastOrder->unitId = lastOrder->unitId;
			unitLastOrder->target = lastOrder->target;
		}
		else
		{
			saveAsLastOrder(unit, getCurrentOrder(unit));
		}
	}

	//Stores all variables of COrder in the variables of the current order
	inline void saveAsMainOrder(CUnit* unit, COrder* mainOrder)
	{
		if (mainOrder)
		{
			unit->mainOrderId = (u8)mainOrder->orderId;
			unit->buildQueue[unit->buildQueueSlot] = mainOrder->unitId;
			unit->orderTarget.unit = mainOrder->target.unit;
			unit->orderTarget.pt.x = mainOrder->target.pt.x;
			unit->orderTarget.pt.y = mainOrder->target.pt.y;
		}
	}

	//This function does three things:
	//1) resets the variables of the current order
	//2) issues the COrder newOrder
	//3) stores the newOrder's variables in the unit's last order variables
	inline void setOrderInUnit(CUnit* unit, COrder* newOrder = NULL)
	{
		if (newOrder)
		{
			//If the order is the same, just change the current targets
			if (newOrder->orderId == unit->mainOrderId)
			{
				saveAsMainOrder(unit, newOrder);
			}
			//If the order is the different, clean and queue...
			else
			{
				saveAsMainOrder(unit, createOrder());
				unit->order((u8)newOrder->orderId, newOrder->target.pt.x, newOrder->target.pt.y, newOrder->target.unit, newOrder->unitId, true);
			}
			//Save the variables
			saveAsLastOrder(unit, newOrder);
		}
		//If no order... do the same with a Nothing2 order
		else
		{
			setOrderInUnit(unit, createOrder());
		}
	}

	//This function checks three things:
	//1) If the user has interacted with the unit
	//2) if the unit's mainOrderId is the same as the asked orderId
	//3) If the last order is different from the asked orderId or is allowed to overrun orders
	inline bool isCasterValidForOrder(CUnit* unit, u8 orderId)
	{
		if (unit
			&& unit->userActionFlags == 2
			&& unit->mainOrderId == orderId)
		{
			return true;
		}
		return false;
	}

	//Checks if the new order is different from the last
	inline bool isOverruningLastOrder(CUnit* unit, u8 orderId)
	{
		if (getLastOrder(unit)->orderId == orderId)
		{
			return true;
		}
		return false;
	}

	//Checks of the orderId requires two units (warp archon)
	inline bool isCouplesOrder(u8 orderId)
	{
		switch (orderId)
		{
		case OrderId::WarpingArchon:
		case OrderId::WarpingDarkArchon:
			return true;
			break;
		default:
			break;
		}
		return false;
	}

	//Checks if two units are partners in a 2-units order
	inline bool isPartnerInOrder(CUnit* unit1, CUnit* unit2, u8 orderId)
	{
		if (unit1 && unit2 && isCouplesOrder(orderId)
			&& unit1->orderTarget.unit == unit2
			&& unit2->orderTarget.unit == unit1)
		{
			return true;
		}
		else return false;
	}

	void setCasters()
	{
		for (CUnit* caster = *firstVisibleUnit; caster; caster = caster->link.next)
		{
			if (isOrderValidForSmartcasting(caster->mainOrderId))
			{
				addToOrderList(caster);
			}
		}
	}

	//Returns the best caster running orderId for player playerId
	CUnit** getBestCasters(u8 orderId)
	{
		static CUnit* bestCasterClean[8];
		static CUnit* bestCasterOverrun[8];

		static CUnit* bestCasterArray[8];

		for (int i = 0; i < 8; i++)
		{
			bestCasterClean[i] = NULL;
			bestCasterOverrun[i] = NULL;

			for (CUnit* caster = firstCaster[orderId][i]; caster; caster = custom_unit_array[caster->getIndex()])
			{
				if (isCasterValidForOrder(caster, orderId))
				{

					u8 p_id = caster->playerId;

					//If is clean order (different than the last one), we have a winner
					if (!isOverruningLastOrder(caster, orderId)
						&& (!bestCasterClean[p_id]
							|| bestCasterClean[p_id]->energy < caster->energy))
					{
						bestCasterClean[p_id] = caster;
					}


					//Store in case we don't have a winner (clean caster)
					//We're going to need this if we can't find a new caster
					if (!bestCasterOverrun[p_id]
						|| bestCasterOverrun[p_id]->energy < caster->energy)
					{
						bestCasterOverrun[p_id] = caster;
					}

				}
			}
		}


		//For each player, store the caster
		for (int i = 0; i < 8; i++)
		{
			bestCasterArray[i] = (bestCasterClean[i] == NULL ? bestCasterOverrun[i] : bestCasterClean[i]);
		}

		return bestCasterArray;
	}

	//Issued the last order to unit
	//If the last order was completed, orders to idle
	inline void tryLastOrder(CUnit* unit)
	{
		if (unit->orderSignal == 2)
		{
			setOrderInUnit(unit);
			unit->orderSignal = 0;
		}
		else
			setOrderInUnit(unit, getLastOrder(unit));
	}

	//Smartcast behaviour
	inline void smartCastOrder(u8 orderId)
	{
		CUnit** bestCasterArray = getBestCasters(orderId);
		long int target_x[8];
		long int target_y[8];
		u16 totalCasters[8];

		//Set if we can find at leat one unit new-casting the order
		bool atLeastOneCaster = false;
		for (int i = 0; i < 8; i++)
		{
			if (bestCasterArray[i])
			{
				bestCasterArray[i]->userActionFlags = 0;
				atLeastOneCaster = true;
			}
			target_x[i] = 0;
			target_y[i] = 0;
			totalCasters[i] = 0;
		}

		if (atLeastOneCaster)
		{
			//For each player
			for (int i = 0; i < PLAYABLE_PLAYER_COUNT; i++)
			{
				//Order other units to continue with the lasts orders
				for (CUnit* caster = firstCaster[orderId][i]; caster; caster = custom_unit_array[caster->getIndex()])
				{
					if (caster->userActionFlags == 2
						&& caster->mainOrderId == orderId)
					{

						target_x[caster->playerId] += caster->orderTarget.pt.x;
						target_y[caster->playerId] += caster->orderTarget.pt.y;
						totalCasters[caster->playerId]++;

						//Checks if the unit is the partner of the current best
						//This is going to help (eventually) with smart casting archon warp and dark archon meld
						if (!isPartnerInOrder(bestCasterArray[caster->playerId], caster, orderId))
						{
							tryLastOrder(caster);
						}

						//Unset the user flags (To avoid mistakes in the next frame)
						caster->userActionFlags = 0;
					}
				}
				firstCaster[orderId][i] = NULL;
			}

			//Fix the target point for ground spells
			//Ground spells like psi storm, ensnare, lockdown
			for (int j = 0; j < 8; j++)
			{
				if (bestCasterArray[j]
					&& !bestCasterArray[j]->orderTarget.unit
					&& totalCasters[j])
				{
					target_x[j] /= totalCasters[j];
					target_y[j] /= totalCasters[j];

					bestCasterArray[j]->orderTarget.pt.x = (u16)target_x[j];
					bestCasterArray[j]->orderTarget.pt.y = (u16)target_y[j];
				}
			}
		}
	}

	//Sets flags and saves last order
	inline void prepareUnitsForNextFrame()
	{
		for (CUnit* unit = *firstVisibleUnit; unit; unit = unit->link.next)
		{
			//Stores the last order for smartCast
			saveAsLastOrder(unit);
		}
	}

	//Runs smartcast for each order and prepares for next frame
	//Archon's orders doesn't work now:
	//Archon Merge and Dark Archon Meld buttons doesn't set userActionFlags on the unit
	inline void runSmartCast()
	{
		setCasters();
		for (int i = 0; i < ORDER_TYPE_COUNT; i++)
		{
			if (isOrderValidForSmartcasting(i))
			{
				smartCastOrder(i);
			}
		}

		prepareUnitsForNextFrame();
	}
} //namespace smartCasting
