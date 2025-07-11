//	CSpaceObject.cpp
//
//	CSpaceObject class
//	Copyright (c) 2019 Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"

#ifdef DEBUG_ON_CREATE_TIME
#include <stdio.h>
static DWORD g_dwTotalTime = 0;
#endif

#define MAX_DELTA								(2.0 * g_KlicksPerPixel)
#define MAX_DELTA2								(MAX_DELTA * MAX_DELTA)
#define MAX_DELTA_VEL							(g_KlicksPerPixel / 2.0)
#define MAX_DELTA_VEL2							(MAX_DELTA_VEL * MAX_DELTA_VEL)

const int ITEM_UPDATE_CYCLE =					30;
const int HIGHLIGHT_TIMER =						200;
const int HIGHLIGHT_BLINK =						110;
const int HIGHLIGHT_FADE =						30;

const int DAMAGE_BAR_WIDTH =					100;
const int DAMAGE_BAR_HEIGHT =					12;

const Metric g_rMaxCommsRange =					(LIGHT_MINUTE * 60.0);
const Metric g_rMaxCommsRange2 =				(g_rMaxCommsRange * g_rMaxCommsRange);

#define BOUNDS_CHECK_DIST 						(256.0 * g_KlicksPerPixel)
#define BOUNDS_CHECK_DIST2						(BOUNDS_CHECK_DIST * BOUNDS_CHECK_DIST)

#define HIGHLIGHT_CORNER_WIDTH					8
#define HIGHLIGHT_CORNER_HEIGHT					8

#define STR_UNCHARTED							CONSTLIT("uncharted")

#define CAN_DOCK_AS_PLAYER_EVENT				CONSTLIT("CanDockAsPlayer")
#define CAN_INSTALL_ITEM_EVENT					CONSTLIT("CanInstallItem")
#define GET_DOCK_SCREEN_EVENT					CONSTLIT("GetDockScreen")
#define GET_EXPLOSION_TYPE_EVENT				CONSTLIT("GetExplosionType")
#define GET_PLAYER_PRICE_ADJ_EVENT				CONSTLIT("GetPlayerPriceAdj")
#define ON_ATTACKED_EVENT						CONSTLIT("OnAttacked")
#define ON_ATTACKED_BY_PLAYER_EVENT				CONSTLIT("OnAttackedByPlayer")
#define ON_AUTO_LOOT_EVENT						CONSTLIT("OnAutoLoot")
#define ON_CREATE_EVENT							CONSTLIT("OnCreate")
#define ON_CREATE_ORDERS_EVENT					CONSTLIT("OnCreateOrders")
#define ON_DAMAGE_EVENT							CONSTLIT("OnDamage")
#define ON_DATA_TRANSFER_EVENT                  CONSTLIT("OnDataTransfer")
#define ON_DESELECTED_EVENT						CONSTLIT("OnDeselected")
#define ON_DESTROY_EVENT						CONSTLIT("OnDestroy")
#define ON_DESTROY_CHECK_EVENT					CONSTLIT("OnDestroyCheck")
#define ON_DESTROY_OBJ_EVENT					CONSTLIT("OnDestroyObj")
#define ON_DOCK_OBJ_ADJ_EVENT					CONSTLIT("OnDockObjAdj")
#define ON_DOCK_OBJ_DESTROYED_EVENT				CONSTLIT("OnDockObjDestroyed")
#define ON_ENTERED_GATE_EVENT					CONSTLIT("OnEnteredGate")
#define ON_ENTERED_SYSTEM_EVENT					CONSTLIT("OnEnteredSystem")
#define ON_LOAD_EVENT							CONSTLIT("OnLoad")
#define ON_MINING_EVENT							CONSTLIT("OnMining")
#define ON_OBJ_BLACKLISTED_PLAYER_EVENT			CONSTLIT("OnObjBlacklistedPlayer")
#define ON_OBJ_DESTROYED_EVENT					CONSTLIT("OnObjDestroyed")
#define ON_OBJ_DOCKED_EVENT						CONSTLIT("OnObjDocked")
#define ON_OBJ_ENTERED_GATE_EVENT				CONSTLIT("OnObjEnteredGate")
#define ON_OBJ_GATE_EVENT						CONSTLIT("OnObjGate")
#define ON_OBJ_GATE_CHECK_EVENT					CONSTLIT("OnObjGateCheck")
#define ON_OBJ_JUMPED_EVENT						CONSTLIT("OnObjJumped")
#define ON_OBJ_JUMP_POS_ADJ_EVENT				CONSTLIT("OnObjJumpPosAdj")
#define ON_OBJ_RECONNED_EVENT					CONSTLIT("OnObjReconned")
#define ON_ORDER_CHANGED_EVENT					CONSTLIT("OnOrderChanged")
#define ON_ORDERS_COMPLETED_EVENT				CONSTLIT("OnOrdersCompleted")
#define ON_OVERRIDE_INIT_EVENT					CONSTLIT("OnEventHandlerInit")
#define ON_OVERRIDE_TERM_EVENT					CONSTLIT("OnEventHandlerTerm")
#define ON_MISSION_ACCEPTED_EVENT				CONSTLIT("OnMissionAccepted")
#define ON_MISSION_COMPLETED_EVENT				CONSTLIT("OnMissionCompleted")
#define ON_PLAYER_BLACKLISTED_EVENT				CONSTLIT("OnPlayerBlacklisted")
#define ON_PLAYER_ENTERED_SHIP_EVENT			CONSTLIT("OnPlayerEnteredShip")
#define ON_PLAYER_ENTERED_SYSTEM_EVENT			CONSTLIT("OnPlayerEnteredSystem")
#define ON_PLAYER_LEFT_SHIP_EVENT				CONSTLIT("OnPlayerLeftShip")
#define ON_PLAYER_LEFT_SYSTEM_EVENT				CONSTLIT("OnPlayerLeftSystem")
#define ON_RANDOM_ENCOUNTER_EVENT				CONSTLIT("OnRandomEncounter")
#define ON_SELECTED_EVENT						CONSTLIT("OnSelected")
#define ON_SUBORDINATE_ATTACKED_EVENT			CONSTLIT("OnSubordinateAttacked")
#define ON_SYSTEM_EXPLOSION_EVENT				CONSTLIT("OnSystemExplosion")
#define ON_SYSTEM_OBJ_CREATED_EVENT				CONSTLIT("OnSystemObjCreated")
#define ON_SYSTEM_OBJ_DESTROYED_EVENT			CONSTLIT("OnSystemObjDestroyed")
#define ON_SYSTEM_STARTED_EVENT					CONSTLIT("OnSystemStarted")
#define ON_SYSTEM_STOPPED_EVENT					CONSTLIT("OnSystemStopped")
#define ON_SYSTEM_WEAPON_FIRE_EVENT				CONSTLIT("OnSystemWeaponFire")
#define ON_TRANSLATE_MESSAGE_EVENT				CONSTLIT("OnTranslateMessage")
#define ON_UPDATE_EVENT							CONSTLIT("OnUpdate")

#define FIELD_ARMOR_INTEGRITY					CONSTLIT("armorIntegrity")
#define FIELD_HULL_INTEGRITY					CONSTLIT("hullIntegrity")
#define FIELD_OBJ_ID							CONSTLIT("objID")
#define FIELD_POS								CONSTLIT("pos")
#define FIELD_SHIELD_LEVEL						CONSTLIT("shieldLevel")
#define FIELD_STATUS							CONSTLIT("status")

#define ORDER_DOCKED							CONSTLIT("docked")

#define SPECIAL_CHARACTER						CONSTLIT("character:")
#define SPECIAL_DATA							CONSTLIT("data:")
#define SPECIAL_IS_PLANET						CONSTLIT("isPlanet:")
#define SPECIAL_LOCATION						CONSTLIT("location:")
#define SPECIAL_PROPERTY						CONSTLIT("property:")
#define SPECIAL_SERVICE							CONSTLIT("service:")
#define SPECIAL_UNID							CONSTLIT("unid:")

#define SPECIAL_VALUE_TRUE						CONSTLIT("true")

struct SInstallItemResultsData
	{
	const char *pszID;
	int iArmorCompatibleID;
	int iDeviceCompatibleID;
	};

SInstallItemResultsData INSTALL_ITEM_RESULTS_TABLE[] =
	{
		{	"ok",						0,		0,	},

		{	"armorTooHeavy",			1,		-1,	},
		{	"cannotInstall",			-1,		-1,	},
		{	"noDeviceSlotsLeft",		-1,		2	},
		{	"noNonWeaponSlotsLeft",		-1,		13	},
		{	"noWeaponSlotsLeft",		-1,		12	},
		{	"noLauncherSlotsLeft",		-1,		15	},
		{	"notInstallable",			-1,		1	},
		{	"reactorIncompatible",		-1,		11	},
		{	"reactorOverload",			-1,		-1	},
		{	"reactorTooWeak",			-1,		7	},
		{	"notCompatible",			-1,		-1	},
		{	"tooMuchCargo",				-1,		-1	},

		{	"replacementRequired",		-1,		8	},
		{	"replacementRequired",		-1,		5	},
		{	"replacementRequired",		-1,		6	},
		{	"replacementRequired",		-1,		14	},
		{	"replacementRequired",		-1,		9	},
		{	"replacementRequired",		-1,		4	},
	};

CString ParseParam (char **ioPos);

CSpaceObject::CSpaceObject (CUniverse &Universe) : 
		m_Universe(Universe),
		m_iDestiny(mathRandom(0, g_DestinyRange - 1)),
		m_dwID(Universe.CreateGlobalID())

//	CSpaceObject constructor

	{
	}

CSpaceObject::~CSpaceObject (void)

//	CSpaceObject destructor

	{
	//	Can't turn this on until system destroys spaces objects
	//	explicitly.
#if 0
	ASSERT(m_pSystem == NULL);
#endif

	//	Delete the list of effects

	SEffectNode *pNext = m_pFirstEffect;
	while (pNext)
		{
		SEffectNode *pDelete = pNext;
		pNext = pNext->pNext;

		pDelete->pPainter->Delete();
		delete pDelete;
		}

#ifdef DEBUG_ENEMY_CACHE_BUG
	for (int i = 0; i < GetUniverse().GetSovereignCount(); i++)
		GetUniverse().GetSovereign(i)->DebugObjDeleted(this);

	CSystem *pSystem = GetUniverse().GetCurrentSystem();
	if (pSystem)
		pSystem->GetObjectGrid().DebugObjDeleted(this);
#endif

#ifdef DEBUG_OBJ_REFERENCES
	//	Make sure the object is not being held by anyone else

	if (GetUniverse().GetPOV())
		{
		CSystem *pSystem = GetUniverse().GetPOV()->GetSystem();
		for (int i = 0; i < pSystem->GetObjectCount(); i++)
			{
			CSpaceObject *pObj = pSystem->GetObject(i);
			if (pObj)
				{
				if (pObj->m_Data.FindObjRefData(this))
					ASSERT(false);
				}
			}
		}
#endif

	ASSERT(m_ForceDesc.IsEmpty());
	}

void CSpaceObject::Accelerate (const CVector &vPush, Metric rSeconds)

//	Accelerate
//
//	Accelerates the given object along the given vector. The magnitude of
//	the vector is the force used in gigaNewtons(!). The acceleration is
//	maintained for rSeconds

	{
	Metric rMass = GetMass();

	if (rMass)
		{
		//	rAccel needs to be in klicks per second (we assume here
		//	that 1 klick = 1,000 meters).
		CVector rAccel = (vPush * 1000.0) / rMass;
		m_vVel = m_vVel + (rAccel * rSeconds);
		}
	}

void CSpaceObject::AccelerateStop (Metric rPush, Metric rSeconds)

//	AccelerateStop
//
//	Slows down the object with the given thrust

	{
	Metric rMass = GetMass();

	if (rMass)
		{
		Metric rAccel = rPush * 1000.0 / rMass;

		Metric rLength;
		CVector vDir = m_vVel.Normal(&rLength);

		if (rAccel > rLength)
			m_vVel = NullVector;
		else
			m_vVel = m_vVel - (vDir * rAccel);
		}
	}

void CSpaceObject::AddEffect (IEffectPainter *pPainter, const CVector &vPos, int iTick, int iRotation)

//	AddEffect
//
//	Adds an effect to the object

	{
	int xOffset = mathRound((vPos.GetX() - m_vPos.GetX()) / g_KlicksPerPixel);
	int yOffset = mathRound((m_vPos.GetY() - vPos.GetY()) / g_KlicksPerPixel);

	AddEffect(pPainter, xOffset, yOffset, iTick, iRotation);
	}

void CSpaceObject::AddEffect (IEffectPainter *pPainter, int xOffset, int yOffset, int iTick, int iRotation)

//	AddEffect
//
//	Adds an effect to the object

	{
	ASSERT(pPainter->GetCreator()->IsValidUNID());

	SEffectNode *pNewNode = new SEffectNode;
	pNewNode->pPainter = pPainter;
	pNewNode->xOffset = xOffset;
	pNewNode->yOffset = yOffset;
	pNewNode->iTick = iTick;
	pNewNode->iRotation = iRotation;
	pNewNode->pNext = m_pFirstEffect;

	m_pFirstEffect = pNewNode;
	}

void CSpaceObject::AddEventSubscriber (CSpaceObject *pObj)

//	AddEventSubscriber
//
//	Adds an object that wants to subscribe to our events
	
	{
	if (pObj 
			&& !pObj->IsDestroyed()
			&& pObj->NotifyOthersWhenDestroyed())
		m_SubscribedObjs.Add(pObj); 
	}

void CSpaceObject::AddEventSubscribers (const CSpaceObjectList &Objs)

//	AddEventSubscribers
//
//	Adds the given set of objects as subscribers to our events. We check for
//	duplicates before adding.

	{
	for (int i = 0; i < Objs.GetCount(); i++)
		{
		CSpaceObject *pObj = Objs.GetObj(i);
		if (!pObj->IsDestroyed() 
				&& pObj->NotifyOthersWhenDestroyed()
				&& !FindEventSubscriber(*pObj))
			m_SubscribedObjs.Add(pObj);
		}
	}

void CSpaceObject::AddDrag (Metric rDragFactor)

//	AddDrag
//
//	Adds a drag factor.

	{
	if (!m_pSystem || IsAnchored())
		return;

	m_ForceDesc.AddDrag(m_pSystem->GetForceResolver(), *this, rDragFactor);
	}

void CSpaceObject::AddForce (const CVector &vForce)

//	AddForce
//
//	Adds an acceleration force to the system's force resolver.

	{
	if (!m_pSystem || IsAnchored())
		return;

	m_ForceDesc.AddForce(m_pSystem->GetForceResolver(), *this, vForce);
	}

void CSpaceObject::AddForceFromDeltaV (const CVector &vDeltaV)

//	AddForceFromDeltaV
//
//	Adds a force that will result in the given delta-V.

	{
	AddForce(vDeltaV * (GetMass() / 1000.0));
	}

void CSpaceObject::AddForceLimited (const CVector &vForce)

//	AddForceLimited
//
//	Adds an acceleration force to the system's force resolver.

	{
	if (!m_pSystem || IsAnchored())
		return;

	m_ForceDesc.AddForceLimited(m_pSystem->GetForceResolver(), *this, vForce);
	}

EnhanceItemStatus CSpaceObject::AddItemEnhancement (const CItem &itemToEnhance, 
													CItemType *pEnhancement, 
													int iLifetime, 
													DWORD *retdwID)

//	AddItemEnhancement
//
//	Adds an enhancement to the given item

	{
	//	Select the item

	CItemListManipulator ItemList(GetItemList());
	if (!ItemList.SetCursorAtItem(itemToEnhance))
		{
		if (retdwID)
			*retdwID = OBJID_NULL;

		return eisNoEffect;
		}

	//	Add the enhancement

	return AddItemEnhancement(ItemList, pEnhancement, iLifetime, retdwID);
	}

EnhanceItemStatus CSpaceObject::AddItemEnhancement (CItemListManipulator &ItemList, 
													CItemType *pEnhancement, 
													int iLifetime, 
													DWORD *retdwID)

//	AddItemEnhancement
//
//	Adds an enhancement to the given item

	{
	//	Pre-init in case we exit early

	if (retdwID)
		*retdwID = OBJID_NULL;

	//	Compute expire time

	int iExpireTime = (iLifetime != -1 ? GetUniverse().GetTicks() + iLifetime : -1);

	//	Create a new enhancement

	CItemEnhancement NewEnhancement;
	NewEnhancement.SetEnhancementType(pEnhancement);
	NewEnhancement.SetModCode(pEnhancement->GetModCode());
	NewEnhancement.SetExpireTime(iExpireTime);

	//	Enhance

	return EnhanceItem(ItemList, NewEnhancement, retdwID);
	}

void CSpaceObject::AddOverlay (COverlayType *pType, const CVector &vPos, int iRotation, int iPosZ, int iLifetime, DWORD *retdwID)

//	AddOverlay
//
//	Adds an overlay at the given position.

	{
	//	Convert from a hit position to an overlay pos

	int iPosAngle;
	int iPosRadius;
	CalcOverlayPos(pType, vPos, &iPosAngle, &iPosRadius);
			
	//	Add the overlay

	AddOverlay(pType, iPosAngle, iPosRadius, iRotation, iPosZ, iLifetime, retdwID);
	}

void CSpaceObject::AddOverlay (DWORD dwUNID, int iPosAngle, int iPosRadius, int iRotation, int iPosZ, int iLifetime, DWORD *retdwID)

//	AddOverlay
//
//	Adds an overly by UNID

	{
	COverlayType *pType = GetUniverse().FindOverlayType(dwUNID);
	if (pType == NULL)
		{
		if (retdwID) *retdwID = 0;
		return;
		}

	AddOverlay(pType, iPosAngle, iPosRadius, iRotation, iPosZ, iLifetime, retdwID);
	}

ALERROR CSpaceObject::AddToSystem (CSystem &System, bool bNoGlobalInsert)

//	AddToSystem
//
//	Adds the object to the system

	{
	ALERROR error;

	//	We can get here with m_pSystem already set during load

	ASSERT(m_pSystem == NULL || m_pSystem == &System);

	//	Clear the destroyed bit

	m_fDestroyed = false;

	//	Add to system

	if (error = System.AddToSystem(this, &m_iIndex))
		return error;

	m_pSystem = &System;

	//	If this is a ship or station then add to the global list

	if (!bNoGlobalInsert)
		System.GetUniverse().GetGlobalObjects().InsertIfTracked(this);

	return NOERROR;
	}

EConditionResult CSpaceObject::ApplyCondition (ECondition iCondition, const SApplyConditionOptions &Options)

//	ApplyCondition
//
//	Apply a condition

	{
	//	Check to see if we're immune. If not, then we continue.

	EConditionResult iResult = CanApplyCondition(iCondition, Options);
	if (iResult != EConditionResult::ok)
		return iResult;

	//	Handle some conditions ourselves.

	switch (iCondition)
		{
		case ECondition::timeStopped:
			{
			StopTime();
			return EConditionResult::ok;
			}

		//	For others, let the descendant handle it.

		default:
			return OnApplyCondition(iCondition, Options);
		}
	}

ICCItemPtr CSpaceObject::AsCCItem (CCodeChainCtx &Ctx, const CItem::SEnhanceItemResult &Result)

//	AsCCItem
//
//	Encode as a structure.

	{
	//	In debug mode, validate that we've got a proper translation

	if (Ctx.GetUniverse().InDebugMode() && !CLanguage::ValidateTranslation(Result.sDesc))
		return ICCItemPtr::Error(strPatternSubst(CONSTLIT("Missing translation: %s"), Result.sDesc));

	//	Encode in a struct

	ICCItemPtr pResult(ICCItem::SymbolTable);
	pResult->SetStringAt(CONSTLIT("resultCode"), CItemEnhancement::EnhanceItemStatusToString(Result.iResult));

	if (!Result.Enhancement.IsEmpty())
		pResult->SetStringAt(CONSTLIT("enhancement"), strPatternSubst(CONSTLIT("0x%08x"), Result.Enhancement.GetModCode()));

	if (Result.Enhancement.GetID() != OBJID_NULL)
		pResult->SetIntegerAt(CONSTLIT("id"), Result.Enhancement.GetID());

	if (Result.Enhancement.GetExpireTime() != 0xffffffff
			&& Result.Enhancement.GetExpireTime() > Ctx.GetUniverse().GetTicks())
		pResult->SetIntegerAt(CONSTLIT("lifetime"), Result.Enhancement.GetExpireTime() - Ctx.GetUniverse().GetTicks());

	if (!Result.sDesc.IsBlank())
		pResult->SetStringAt(CONSTLIT("desc"), Result.sDesc);

	if (!Result.sNextScreen.IsBlank())
		pResult->SetStringAt(CONSTLIT("nextScreen"), Result.sNextScreen);

	if (Result.bDoNotConsume)
		pResult->SetBooleanAt(CONSTLIT("doNotConsume"), true);

	return pResult;
	}

void CSpaceObject::Ascend (void)

//	Ascend
//
//	Ascend out of system so that it can move to a different system.

	{
	//	Set ascended flag so that when we get called at OnObjEnteredGate
	//	we can test for it.

	SetAscended(true);

	//	To everyone else in the system, it looks like the object entered a gate

	EnterGate(NULL, NULL_STR, NULL, true);

	//	Let subclasses handle this

	OnAscended();

	//	Remove the object from the old system

	Remove(ascended, CDamageSource());
	}

void CSpaceObject::CalcInsideBarrier (void)

//	CalcInsideBarrier
//
//	Figures out if we are currently inside a barrier. If so, we set the 
//	m_fInsideBarrier flag.

	{
	int i;

	if (!m_fCanBounce)
		return;

	//	Compute the bounding rect for this object

	CVector vUR, vLL;
	GetBoundingRect(&vUR, &vLL);

	//	Loop over all other objects and see if we are inside a barrier

	CSystem *pSystem = GetSystem();
	for (i = 0; i < pSystem->GetObjectCount(); i++)
		{
		CSpaceObject *pObj = pSystem->GetObject(i);
		if (pObj == NULL
				|| pObj == this
				|| pObj->IsDestroyed()
				|| !pObj->CanBlock(this))
			continue;

		//	Compute the bounding rect for the barrier.

		CVector vBarrierUR, vBarrierLL;
		pObj->GetBoundingRect(&vBarrierUR, &vBarrierLL);

		//	If we intersect then we're inside at least one object

		if (IntersectRect(vUR, vLL, vBarrierUR, vBarrierLL)
				&& pObj->ObjectInObject(pObj->GetPos(), this, GetPos()))
			{
			m_fInsideBarrier = true;
			break;
			}
		}
	}

void CSpaceObject::CalcOverlayPos (COverlayType *pOverlayType, const CVector &vPos, int *retiPosAngle, int *retiPosRadius)

//	CalcOverlayPos
//
//	Calculates an overlay position from the given absolute position.

	{
	Metric rRadius;
	int iDirection = VectorToPolar(vPos - GetPos(), &rRadius);
	int iRotationOrigin = ((pOverlayType && pOverlayType->RotatesWithSource(*this)) ? GetRotation() : 0);

	if (retiPosAngle)
		*retiPosAngle = AngleMod(iDirection - iRotationOrigin);

	if (retiPosRadius)
		*retiPosRadius = (int)(rRadius / g_KlicksPerPixel);
	}

int CSpaceObject::CalcFireSolution (CSpaceObject *pTarget, Metric rMissileSpeed) const

//	CalcFireSolution
//
//	Returns the angle to aim at to hit the given target. Or -1 if the target cannot be
//	intercepted.

	{
	CVector vPos = pTarget->GetPos() - GetPos();
	CVector vVel = pTarget->GetVel() - GetVel();

	Metric rTimeToIntercept = CalcInterceptTime(vPos, vVel, rMissileSpeed);
	if (rTimeToIntercept < 0.0)
		return -1;

	//	Compute the intercept point and then the direction

	CVector vInterceptPoint = vPos + vVel * rTimeToIntercept;
	return VectorToPolar(vInterceptPoint);
	}

int CSpaceObject::CalcMiningDifficulty (EAsteroidType iType) const

//	CalcMiningDifficulty
//
//	Compute mining difficult based on asteroid (0-100).

	{
	CCodeChainCtx CCX(GetUniverse());
	ICCItemPtr pValue = GetTypeProperty(CCX, PROPERTY_CORE_MINING_DIFFICULTY);
	if (!pValue->IsNil())
		return Max(0, Min(pValue->GetIntegerValue(), 100));

	return CAsteroidDesc::GetDefaultMiningDifficulty(iType);
	}

DWORD CSpaceObject::CalcSRSVisibility (const CSpaceObject &ObserverObj, int iObserverPerception) const

//	CalcSRSVisibility
//
//	Calculates the SRS opacity to paint with.
//
//	0 = Fully visible.
//	1-255 = varying degrees of visibility (1 = lowest, 255 = highest).

	{
	CPerceptionCalc Perception(iObserverPerception);
	return Perception.CalcSRSShimmer(*this, GetDistance(&ObserverObj));
	}

CSpaceObject *CSpaceObject::CalcTargetToAttack (CSpaceObject *pAttacker, CSpaceObject *pOrderGiver)

//	CalcTargetToAttack
//
//	Figure out whether to attack the order giver or not. NOTE: We return NULL if
//	neither can be attacked.

	{
	CPerceptionCalc Perception(GetPerception());

	//	Figure out if we can see the attacker

	Metric rAttackerDist2 = g_InfiniteDistance;
	bool bCanSeeAttacker = false;
	if (pAttacker)
		{
		rAttackerDist2 = GetDistance2(pAttacker);
		if (Perception.CanBeTargeted(pAttacker, rAttackerDist2))
			bCanSeeAttacker = true;
		}

	//	Figure out if we can see the order giver

	Metric rOrderGiverDist2 = g_InfiniteDistance;
	bool bCanSeeOrderGiver = false;
	if (pOrderGiver == pAttacker)
		{
		bCanSeeOrderGiver = bCanSeeAttacker;
		rOrderGiverDist2 = rAttackerDist2;
		}
	else if (pOrderGiver)
		{
		rOrderGiverDist2 = GetDistance2(pOrderGiver);
		if (Perception.CanBeTargeted(pOrderGiver, rOrderGiverDist2))
			bCanSeeOrderGiver = true;
		}

	//	Choose whichever is valid and visible.

	if (pAttacker == pOrderGiver)
		return (bCanSeeAttacker ? pAttacker : NULL);

	else if (pAttacker == NULL || !bCanSeeAttacker)
		return (bCanSeeOrderGiver ? pOrderGiver : NULL);

	else if (pOrderGiver == NULL || !bCanSeeOrderGiver)
		return (bCanSeeAttacker ? pAttacker : NULL);

	//	Both attacker and order giver are valid and visible to us. If one is
	//	twice as close to us, we pick that one.

	else if (rAttackerDist2 < rOrderGiverDist2 * 4.0)
		return pAttacker;
	else if (rOrderGiverDist2 < rAttackerDist2 * 4.0)
		return pOrderGiver;

	//	Otherwise, we pick randomly, weighing the order giver a little more.

	else if (mathRandom(1, 100) <= 60)
		return pOrderGiver;
	else
		return pAttacker;
	}

Metric CSpaceObject::CalculateItemMass (Metric *retrCargoMass) const

//	CalculateCargoMass
//
//	Returns the total mass of the items

	{
	int i;
	Metric rTotal = 0.0;
	Metric rTotalCargo = 0.0;

	for (i = 0; i < m_ItemList.GetCount(); i++)
		{
		const CItem &Item = m_ItemList.GetItem(i);

		Metric rMass = Item.GetMass() * Item.GetCount();

		//	All items count towards item mass

		rTotal += rMass;

		//	Only uninstalled items count in cargo space

		if (!Item.IsInstalled())
			rTotalCargo += rMass;
		}

	if (retrCargoMass)
		*retrCargoMass = rTotalCargo;

	return rTotal;
	}

EConditionResult CSpaceObject::CanApplyCondition (ECondition iCondition, const SApplyConditionOptions &Options) const

//	CanApplyCondition
//
//	Returns result when applying condition

	{
	//	If we're already in this condition...

	if (GetCondition(iCondition))
		return EConditionResult::alreadyApplied;

	//	If we don't need to check immunities, then we continue.

	if (Options.bNoImmunityCheck)
		return EConditionResult::ok;

	//	Let our descendants handle it.

	return OnCanApplyCondition(iCondition, Options); 
	}

bool CSpaceObject::CanCommunicateWith (const CSpaceObject &SenderObj) const

//	CanCommunicateWith
//
//	Returns TRUE if this object can receive communications from pSender

	{
	//	We can't communicate if we don't have a handler

	const CCommunicationsHandler *pHandler = GetCommsHandler();
	if (pHandler == NULL)
		return false;

	//	We can't communicate if we don't know about the object

	if (!IsKnown())
		return false;

	//	We can't communicate if we're suspended, etc. (But it's OK if we're
	//	virtual.)

	if (IsUnreal())
		return false;

	//	Can't communicate if we're part of a different object

	if (IsAttached())
		return false;

	//	We can't communicate if we are out of range

	if ((SenderObj.GetPos() - m_vPos).Length2() > g_rMaxCommsRange2)
		return false;

	//	See if any of the messages are valid. If at least
	//	one is, then we can communicate.

	for (int i = 0; i < pHandler->GetCount(); i++)
		{
		if (pHandler->GetMessage(i).OnShowEvent.pCode == NULL)
			continue;

		CCodeChainCtx Ctx(GetUniverse());

		//	Define parameters

		Ctx.DefineContainingType(this);
		Ctx.SaveAndDefineSourceVar(this);
		Ctx.DefineSpaceObject(CONSTLIT("gSender"), SenderObj);

		//	Execute

		ICCItemPtr pResult = Ctx.RunCode(pHandler->GetMessage(i).OnShowEvent);

		if (pResult->IsNil())
			continue;
		else if (pResult->IsError())
			{
			SenderObj.SendMessage(this, pResult->GetStringValue());
			return false;
			}
		else
			return true;
		}

	return false;
	}

bool CSpaceObject::CanDetect (int iPerception, CSpaceObject *pObj)

//	CanDetect
//
//	Returns TRUE if this object (with given perception) can detect the target

	{
	CVector vDist = pObj->GetPos() - GetPos();
	return (vDist.Length2() < pObj->GetDetectionRange2(iPerception));
	}

CItem::SEnhanceItemResult CSpaceObject::CanEnhanceItem (CItemListManipulator &ItemList, const CItem &EnhancementItem, CString *retsError) const

//	CanEnhanceItem
//
//	Returns an SEnhanceItemResult structure. If the result is eisUnknown, then 
//	we got an error trying to enhance.

	{
	ASSERT(ItemList.IsCursorValid());
	ASSERT(!EnhancementItem.IsEmpty());

	DEBUG_TRY

	const CItem &TargetItem = ItemList.GetItemAtCursor();

	//	See if we have an explicit event. FireCanEnhanceItem returns FALSE if
	//	there is an error.

	CItem::SEnhanceItemResult Result;
	if (!EnhancementItem.FireCanEnhanceItem(*this, TargetItem, Result, retsError))
		{
		Result.iResult = eisUnknown;
		return Result;
		}

	//	If <CanEnhanceItem> worked, then we're done.

	else if (Result.iResult != eisUnknown)
		return Result;

	//	Get the enhancement to confer

	else if (!EnhancementItem.GetEnhancementConferred(*this, TargetItem, CONSTLIT("canEnhance"), Result, retsError))
		{
		Result.iResult = eisUnknown;
		return Result;
		}

	//	If no mod, nothing to do (but Result.sDesc might explain what happened).

	else if (Result.Enhancement.IsEmpty())
		{
		Result.iResult = eisNoEffect;
		return Result;
		}

	//	Figure out the effect of the enhancement on the item

	else
		{
		CItemEnhancement OldEnhancement = TargetItem.GetMods();
		Result.iResult = OldEnhancement.Combine(TargetItem, Result.Enhancement);

		//	Done

		return Result;
		}

	DEBUG_CATCH
	}

bool CSpaceObject::CanFireOnObjHelper (CSpaceObject *pObj) const

//	CanFireOnObjHelper
//
//	Return TRUE if a missile fired by this object can hit the given object

	{
	return (
		//	We cannot hit our friends (if our source can't)
		(CanHitFriends() && pObj->CanBeHitByFriends()) || GetSovereign() == NULL || !GetSovereign()->IsFriend(pObj->GetSovereign())
		);
	}

bool CSpaceObject::CanInstallItem (const CItem &Item, const CDeviceSystem::SSlotDesc &Slot, InstallItemResults *retiResult, CString *retsResult, CItem *retItemToReplace)

//	CanInstallItem
//
//	Must be overridden by subclasses.

	{
	if (retiResult)
		*retiResult = insCannotInstall;

	if (retsResult)
		*retsResult = CONSTLIT("Item installation not supported.");

	return false;
	}

EConditionResult CSpaceObject::CanRemoveCondition (ECondition iCondition, const SApplyConditionOptions &Options) const

//	CanRemoveCondition
//
//	Returns result when attempting to remove condition.

	{
	//	If we're not in this condition...

	if (!GetCondition(iCondition))
		return EConditionResult::alreadyRemoved;

	//	If we don't need to check for cases in which we cannot remove the
	//	condition, then just return.

	else if (Options.bNoImmunityCheck)
		return EConditionResult::ok;

	//	For fouling, we know how to remove it.

	else if (iCondition == ECondition::fouled)
		return EConditionResult::ok;

	//	Let our descendants handle it.

	else
		return OnCanRemoveCondition(iCondition, Options);
	}

void CSpaceObject::CommsMessageFrom (CSpaceObject *pSender, int iIndex)

//	CommsMessageFrom
//
//	Handle comms message from the sender

	{
	CCommunicationsHandler *pHandler = GetCommsHandler();
	if (!pHandler || iIndex >= pHandler->GetCount())
		throw CException(ERR_FAIL);

	const CCommunicationsHandler::SMessage &Msg = pHandler->GetMessage(iIndex);

	if (Msg.InvokeEvent.pCode)
		{
		CCodeChainCtx Ctx(GetUniverse());

		//	Define parameters

		Ctx.DefineContainingType(this);
		Ctx.SaveAndDefineSourceVar(this);
		Ctx.DefineSpaceObject(CONSTLIT("gSender"), pSender);

		//	Execute

		ICCItemPtr pResult = Ctx.RunCode(Msg.InvokeEvent);
		if (pResult->IsError())
			pSender->SendMessage(this, pResult->GetStringValue());
		}
	}

int CSpaceObject::ConvertToCompatibleIndex (const CItem &Item, InstallItemResults iResult)

//	ConvertToCompatibleIndex
//
//	Converts a result to the compatible ID number

	{
	if (iResult < 0 || iResult >= insInstallItemResultsCount)
		{
		ASSERT(false);
		return -1;
		}

	if (Item.IsArmor())
		return INSTALL_ITEM_RESULTS_TABLE[iResult].iArmorCompatibleID;
	else
		return INSTALL_ITEM_RESULTS_TABLE[iResult].iDeviceCompatibleID;
	}

CString CSpaceObject::ConvertToID (InstallItemResults iResult)

//	ConvertToID
//
//	Converts a result to its ID

	{
	if (iResult < 0 || iResult >= insInstallItemResultsCount)
		{
		ASSERT(false);
		return NULL_STR;
		}

	return CString(INSTALL_ITEM_RESULTS_TABLE[iResult].pszID);
	}

void CSpaceObject::CopyDataFromObj (CSpaceObject *pSource)

//	CopyDataFromObj
//
//	Copies data from the source object (this is used when we
//	change ships)

	{
	int i;

	//  Ask our object for a list of data fields that we DO NOT want to copy.
	//  We only ask our class because it should be the only thing that stores
	//  per-ship data. All other uses should store data in other places, such
	//  as overlays, items, etc.
	//
	//  [The other exception is non-player ships, which often have event
	//  handlers that store state, but those are not player ships.]

	SEventHandlerDesc Event;
	TSortMap<CString, CAttributeDataBlock::STransferDesc> Options;
	if (FindEventHandler(ON_DATA_TRANSFER_EVENT, &Event))
		{
		CCodeChainCtx Ctx(GetUniverse());

		Ctx.DefineContainingType(this);
		Ctx.SaveAndDefineSourceVar(this);

		ICCItemPtr pResult = Ctx.RunCode(Event);
		if (pResult->IsError())
			ReportEventError(ON_DATA_TRANSFER_EVENT, pResult);
		else if (pResult->IsSymbolTable())
			{
			for (i = 0; i < pResult->GetCount(); i++)
				{
				CAttributeDataBlock::STransferDesc *pDesc = Options.SetAt(pResult->GetKey(i));
				CString sOption = pResult->GetElement(i)->GetStringValue();
				if (strEquals(sOption, CONSTLIT("ignore")))
					pDesc->iOption = CAttributeDataBlock::transIgnore;
				else if (strEquals(sOption, CONSTLIT("copy")))
					pDesc->iOption = CAttributeDataBlock::transCopy;
				else
					kernelDebugLogPattern("Unknown transfer option: %s", sOption);
				}
			}
		}

	//  Copy

	m_Data.Copy(pSource->m_Data, Options);
	}

void CSpaceObject::CreateDefaultDockingPorts (void)

//	CreateDefaultDockingPorts
//
//	Called to create docking ports on objects that don't already have them. This
//	can be overridden by subclasses to control port position, etc.

	{
	if (CDockingPorts *pDockingPorts = GetDockingPorts())
		{
		if (pDockingPorts->GetPortCount() == 0)
			{
			constexpr int PORT_COUNT = 4;
			int iRadius = GetHitSizePixels() / 2;

			pDockingPorts->InitPorts(this, PORT_COUNT, iRadius * g_KlicksPerPixel);
			}
		}
	}

CSpaceObject *CSpaceObject::CreateFromClassID (CUniverse &Universe, DWORD dwClass)

//	CreateFromClassID
//
//	Creates a new object of the specified class.

	{
	switch (dwClass)
		{
		case CAreaDamage::ClassID():
			return new CAreaDamage(Universe);

		case CBeam::ClassID():
			return new CBeam(Universe);

		case CBoundaryMarker::ClassID():
			return new CBoundaryMarker(Universe);

		case CContinuousBeam::ClassID():
			return new CContinuousBeam(Universe);

		case CDisintegrationEffect::ClassID():
			return new CDisintegrationEffect(Universe);

		case CEffect::ClassID():
			return new CEffect(Universe);

		case CFractureEffect::ClassID():
			return new CFractureEffect(Universe);

		case CMarker::ClassID():
			return new CMarker(Universe);

		case CMissile::ClassID():
			return new CMissile(Universe);

		case CMission::ClassID():
			return new CMission(Universe);

		case CParticleDamage::ClassID():
			return new CParticleDamage(Universe);

		case CParticleEffect::ClassID():
			return new CParticleEffect(Universe);

		case CPOVMarker::ClassID():
			return new CPOVMarker(Universe);

		case CRadiusDamage::ClassID():
			return new CRadiusDamage(Universe);

		case CSequencerEffect::ClassID():
			return new CSequencerEffect(Universe);

		case CShip::ClassID():
			return new CShip(Universe);

		case CStaticEffect::ClassID():
			return new CStaticEffect(Universe);

		case CStation::ClassID():
			return new CStation(Universe);
			
		default:
			throw CException(ERR_FAIL, strPatternSubst(CONSTLIT("Invalid CSpaceObject class: %08x"), dwClass));
		}
	}

void CSpaceObject::CreateFromStream (SLoadCtx &Ctx, CSpaceObject **retpObj)

//	CreateFromStream
//
//	Creates an object from the stream
//
//	DWORD		ObjID
//	DWORD		m_iIndex
//	DWORD		m_dwID
//	DWORD		m_iDestiny
//	Vector		m_vPos
//	Vector		m_vVel
//	Metric		m_rBoundsX
//	Metric		m_rBoundsY
//	DWORD		low = m_iDesiredHighlightChar; hi = m_iHighlightCountdown
//	DWORD		m_pOverride
//	CItemList	m_ItemList
//	DWORD		m_iControlsFrozen
//	DWORD		flags
//	CAttributeDataBlock	m_Data
//	CVector		m_vOldPos (only if !IsAnchored())
//
//	For each effect:
//	IEffectPainter (0 == no more)
//	DWORD		x
//	DWORD		y
//	DWORD		iTick
//	DWORD		iRotation

	{
	ELoadStates iOldLoadState = Ctx.iLoadState;

	//	Create the object

	DWORD dwLoad;
	Ctx.pStream->Read(dwLoad);
	CSpaceObject *pObj = CreateFromClassID(Ctx.GetUniverse(), dwLoad);

	//	Remember the type of object that we're loading (in case of crash)

	Ctx.iLoadState = loadStateObject;
	Ctx.dwObjClassID = dwLoad;

	//	Load the index. This will not be the final index (because the
	//	index will change relative to the new system). But this is the
	//	index that other objects will refer to during load.

	Ctx.pStream->Read(pObj->m_iIndex);

	//	Load the global ID

	if (Ctx.dwVersion >= 13)
		Ctx.pStream->Read(pObj->m_dwID);
	else
		pObj->m_dwID = Ctx.GetUniverse().CreateGlobalID();

	//	Set the system as soon as possible because we rely on it during loading

	pObj->m_pSystem = Ctx.pSystem;

	//	Load other stuff

	Ctx.pStream->Read(pObj->m_iDestiny);
	pObj->m_vPos.ReadFromStream(*Ctx.pStream);
	pObj->m_vVel.ReadFromStream(*Ctx.pStream);
	Ctx.pStream->Read(pObj->m_rBoundsX);
	Ctx.pStream->Read(pObj->m_rBoundsY);
	Ctx.pStream->Read(dwLoad);
	if (Ctx.dwVersion >= 99)
		pObj->m_iDesiredHighlightChar = LOWORD(dwLoad);
	else
		pObj->m_iDesiredHighlightChar = 0;
	pObj->m_iHighlightCountdown = HIWORD(dwLoad);

	//	Override

	if (Ctx.dwVersion >= 48
			&& Ctx.pStream->Read(dwLoad) == NOERROR
			&& dwLoad != 0)
		pObj->m_pOverride = Ctx.GetUniverse().FindDesignType(dwLoad);
	else
		pObj->m_pOverride = NULL;

	//	Item List

	pObj->m_ItemList.ReadFromStream(Ctx);

	//	Load other stuff

	Ctx.pStream->Read(dwLoad);
	pObj->m_iControlsFrozen = dwLoad;

	//	Load flags

	Ctx.pStream->Read(dwLoad);
	pObj->m_fHookObjectDestruction =	((dwLoad & 0x00000001) ? true : false);
	pObj->m_fNoObjectDestructionNotify = ((dwLoad & 0x00000002) ? true : false);
	pObj->m_fCannotBeHit =				((dwLoad & 0x00000004) ? true : false);
	pObj->m_fSelected =					((dwLoad & 0x00000008) ? true : false);
	pObj->m_fInPOVLRS =					((dwLoad & 0x00000010) ? true : false);
	pObj->m_fCanBounce =				((dwLoad & 0x00000020) ? true : false);
	pObj->m_fIsBarrier =				((dwLoad & 0x00000040) ? true : false);
	bool bSavedOldPos =					((dwLoad & 0x00000080) ? false : true);
	pObj->m_fNoFriendlyFire =			((dwLoad & 0x00000100) ? true : false);
	pObj->m_fTimeStop =					((dwLoad & 0x00000200) ? true : false);
	pObj->m_fPlayerTarget =				((dwLoad & 0x00000400) ? true : false);
	pObj->m_fAutoClearDestinationOnGate = (((dwLoad & 0x00000800) ? true : false) && Ctx.dwVersion >= 158);
	pObj->m_fNoFriendlyTarget =			((dwLoad & 0x00001000) ? true : false);
	pObj->m_fPlayerDestination =		((dwLoad & 0x00002000) ? true : false);
	pObj->m_fShowDistanceAndBearing =	((dwLoad & 0x00004000) ? true : false);
	pObj->m_fHasOnObjDockedEvent =		((dwLoad & 0x00008000) ? true : false);
	pObj->m_fHasInterSystemEvent =		((dwLoad & 0x00010000) ? true : false);
	pObj->m_fHasOnDamageEvent =			((dwLoad & 0x00020000) ? true : false);
	pObj->m_fHasOnAttackedEvent =		((dwLoad & 0x00040000) ? true : false);
	pObj->m_fAutoClearDestination =		((dwLoad & 0x00080000) ? true : false);
	pObj->m_fHasOnOrdersCompletedEvent =((dwLoad & 0x00100000) ? true : false);
	pObj->m_fPlayerDocked =				((dwLoad & 0x00200000) ? true : false);
	pObj->m_fNonLinearMove =			((dwLoad & 0x00400000) ? true : false);
	pObj->m_fAscended =					((dwLoad & 0x00800000) ? true : false);
	pObj->m_fOutOfPlaneObj =			((dwLoad & 0x01000000) ? true : false);
	pObj->m_fAutoClearDestinationOnDock = ((dwLoad & 0x02000000) ? true : false);
	pObj->m_fShowHighlight =			((dwLoad & 0x04000000) ? true : false);
	pObj->m_fAutoClearDestinationOnDestroy = ((dwLoad & 0x08000000) ? true : false);
	pObj->m_fShowDamageBar =			((dwLoad & 0x10000000) ? true : false);
	pObj->m_fHasGravity =				((dwLoad & 0x20000000) ? true : false);
	pObj->m_fInsideBarrier =			((dwLoad & 0x40000000) ? true : false);
	pObj->m_fHasOnSubordinateAttackedEvent = ((dwLoad & 0x80000000) ? true : false);

	//	More flags

	if (Ctx.dwVersion >= 136)
		Ctx.pStream->Read(dwLoad);
	else
		dwLoad = 0;

	pObj->m_fQuestTarget =				(((dwLoad & 0x00000001) ? true : false) && Ctx.dwVersion >= 201);
	pObj->m_fHasGetDockScreenEvent =	((dwLoad & 0x00000002) ? true : false);
	pObj->m_fHasOnAttackedByPlayerEvent =	((dwLoad & 0x00000004) ? true : false);
	pObj->m_fHasOnOrderChangedEvent =	((dwLoad & 0x00000008) ? true : false);
	pObj->m_fManualAnchor =				((dwLoad & 0x00000010) ? true : false);
	pObj->m_f3DExtra =					((dwLoad & 0x00000020) ? true : false);
	pObj->m_fAutoCreatedPorts =			((dwLoad & 0x00000040) ? true : false);
	pObj->m_fDebugMode =				((dwLoad & 0x00000080) ? true : false);

	//	No need to save the following

	pObj->m_fOnCreateCalled = true;
	pObj->m_fItemEventsValid = false;
	pObj->m_fInDamage = false;
	pObj->m_fDestroyed = false;
	pObj->m_fPaintNeeded = false;
	pObj->m_fPainted = false;

	//	Load opaque data

	Ctx.iLoadState = loadStateObjData;
	pObj->m_Data.ReadFromStream(Ctx);
	Ctx.iLoadState = loadStateObject;

	//	Load additional data

	if (bSavedOldPos && Ctx.dwVersion >= 65)
		pObj->m_vOldPos.ReadFromStream(*Ctx.pStream);
	else
		pObj->m_vOldPos = pObj->m_vPos - (pObj->m_vVel * g_SecondsPerUpdate);

	//	Subscriptions
	//
	//	NOTE: We ignore any missing objects. This can happen if (e.g.) a mission
	//	gets destroyed while we're out of the system.

	if (Ctx.dwVersion >= 77)
		pObj->m_SubscribedObjs.ReadFromStream(Ctx, true);

	//	Load the effect list

	Ctx.iLoadState = loadStateObjEffects;
	IEffectPainter *pEffect = CEffectCreator::CreatePainterFromStream(Ctx);
	while (pEffect)
		{
		int x, y, iTick, iRotation;
		Ctx.pStream->Read(x);
		Ctx.pStream->Read(y);
		Ctx.pStream->Read(iTick);
		if (Ctx.dwVersion >= 51)
			Ctx.pStream->Read(iRotation);
		else
			iRotation = 0;

		pObj->AddEffect(pEffect, x, y, iTick);

		pEffect = CEffectCreator::CreatePainterFromStream(Ctx);
		}

	//	Let the subclass read its part

	Ctx.iLoadState = loadStateObjSubClass;
	pObj->OnReadFromStream(Ctx);

	//	We must handle this after we load the subclass

	if (Ctx.dwVersion < 137)
		{
		pObj->m_fHasOnAttackedByPlayerEvent = pObj->FindEventHandler(CONSTLIT("OnAttackedByPlayer"));
		pObj->m_fHasOnOrderChangedEvent = pObj->FindEventHandler(CONSTLIT("OnOrderChanged"));
		}

	//	Done

	*retpObj = pObj;
	Ctx.iLoadState = iOldLoadState;
	}

EDamageResults CSpaceObject::Damage (SDamageCtx &Ctx)

//	Damage
//
//	Cause damage to the object

	{
	DEBUG_TRY

	ASSERT(!IsInDamageCode());
	SetInDamageCode();

	//	Let our subclasses handle it

	EDamageResults iResult = OnDamage(Ctx);

	//	Done

	ClearInDamageCode();
	return iResult;

	DEBUG_CATCH
	}

void CSpaceObject::DamageItem (CInstalledDevice *pDevice)

//	DamageItem
//
//	Damages an item

	{
	CItemListManipulator ItemList(GetItemList());
	SetCursorAtDevice(ItemList, pDevice);
	DamageItem(ItemList);
	}

void CSpaceObject::DamageItem (CItemListManipulator &ItemList)

//	DamageItem
//
//	Damages an item

	{
	const CItem &Item = ItemList.GetItemAtCursor();
	if (Item.IsDamaged())
		return;

	//	Figure out the current mods on this item

	CItemEnhancement Mods(Item.GetMods());

	//	If the item has an enhancement mod, then we remove it

	if (Mods.IsEnhancement())
		EnhanceItem(ItemList, etLoseEnhancement);

	//	If the item is enhanced, then damaging it removes the enhancements

	else if (Item.IsEnhanced())
		ItemList.SetEnhancedAtCursor(false);

	//	Otherwise, damage the item

	else if (!Item.IsDamaged())
		ItemList.SetDamagedAtCursor(true);

	//	Done

	ItemEnhancementModified(ItemList);
	}

CString CSpaceObject::DebugDescribe (CSpaceObject *pObj)

//	DebugDescribe
//
//	Describe object

	{
	try
		{
		if (pObj == NULL)
			return CONSTLIT("none");
		else if (pObj->IsDestroyed())
			return strPatternSubst(CONSTLIT("%x %s (%s) [destroyed]"), (DWORD)pObj, pObj->GetNounPhrase(), pObj->GetObjClassName());
		else
			return strPatternSubst(CONSTLIT("%x %s (%s)"), (DWORD)pObj, pObj->GetNounPhrase(), pObj->GetObjClassName());
		}
	catch (...)
		{
		}

	return strPatternSubst(CONSTLIT("%x [invalid]"), (DWORD)pObj);
	}

CString CSpaceObject::DebugLoadError (SLoadCtx &Ctx)

//	DebugLoadError
//
//	Compose error message from loading.

	{
	CString sLine = strPatternSubst(CONSTLIT(
			"Unable to load object.\r\n"
			"State: %s\r\n"
			"ObjectClassID: %x\r\n"),

			GetLoadStateString(Ctx.iLoadState),
			Ctx.dwObjClassID);

	//	If we're loading an effect, output that

	if (Ctx.iLoadState == loadStateEffect)
		sLine.Append(strPatternSubst(CONSTLIT(
			"EffectUNID: %s\r\n"),

			Ctx.sEffectUNID));

	return sLine;
	}

void CSpaceObject::Destroy (DestructionTypes iCause, const CDamageSource &Attacker, CWeaponFireDesc *pWeaponDesc, CSpaceObject **retpWreck)

//	Destroy
//
//	Destroy this object

	{
	DEBUG_TRY

	//	Do not recurse

	if (IsDestroyed())
		return;

	//	Prepare struct

	SDestroyCtx Ctx(*this);
	Ctx.pDesc = pWeaponDesc;
	Ctx.iCause = iCause;
	Ctx.Attacker = Attacker;
	Ctx.pWreck = NULL;
	Ctx.bResurrectPending = false;
	Ctx.pResurrectedObj = NULL;
	if (retpWreck)
		*retpWreck = NULL;

	//	Give our descendants a chance to do something
	//	If necessary, our descendants will set bResurrectPending.

	OnDestroyed(Ctx);

	//	Even if resurrecting, mark object as destroyed. We need to
	//	do this because cached lists will still have the object for a bit
	//	(We will clear this in AddToSystem).
	//
	//	Also, we need to mark this before we start calling OnObjDestroyed
	//	(so that we can detect any attempts at recursion).

	m_fDestroyed = true;

	//	If we were destroyed by a weapon, let the weapon know that it succeeded.

	if (pWeaponDesc)
		pWeaponDesc->FireOnDestroyObj(Ctx);

	if (Attacker.GetObj())
		Attacker.GetObj()->FireOnDestroyObj(Ctx);

	//	Remove from the object from the universal list (NOTE: We must do this
	//	before we clear out m_pSystem.)

	CSpaceObject::Categories iCategory = GetCategory();
	if (iCategory == CSpaceObject::catStation || iCategory == CSpaceObject::catShip)
		GetUniverse().GetGlobalObjects().Delete(this);

	//	Remove from system. This will call OnObjDestroyed to all other
	//	interested objects

	CSystem *pSystem = m_pSystem;
	m_pSystem = NULL;
	if (m_iIndex != -1)
		{
		//	Give our descendants a chance to remove. For example, ships will
		//	deal with attached sections.

		OnRemoved(Ctx);

		//	Remove from system

		pSystem->RemoveObject(Ctx);
		m_iIndex = -1;

		//	Delete

		pSystem->DeleteObject(Ctx);
		}

	//	Return wreck

	if (retpWreck)
		*retpWreck = Ctx.pWreck;

	//	If resurrection is pending, then clear the destroyed flag. Otherwise, 
	//	wingmen might leave the player.
	//
	//	NOTE: This is somewhat a hack, and it works only because we remove the
	//	player ship from lists that might contain it (including the system grid).
	//	In the future it might be cleaner to leave the destroyed flag set but
	//	set an additional resurrection flag and have wingmen check that.

	if (Ctx.bResurrectPending && IsPlayer())
		m_fDestroyed = false;

	DEBUG_CATCH
	}

void CSpaceObject::DisruptItem (CItemListManipulator &ItemList, DWORD dwDuration)

//	DisruptItem
//
//	Disrupts an item

	{
	ItemList.SetDisruptedAtCursor(dwDuration);
	ItemEnhancementModified(ItemList);
	}

EnhanceItemStatus CSpaceObject::EnhanceItem (CItemListManipulator &ItemList, const CItemEnhancement &Mods, DWORD *retdwID)

//	EnhanceItem
//
//	Enhances the item at cursor (either installed or in cargo hold)

	{
	DEBUG_TRY

	//	Pre-init in case we exit early

	if (retdwID)
		*retdwID = OBJID_NULL;

	//	Get the item to enhance

	const CItem &TargetItem = ItemList.GetItemAtCursor();

	//	Figure out the effect of the enhancement on the item

	CItemEnhancement Enhancement = TargetItem.GetMods();
	EnhanceItemStatus iResult = Enhancement.Combine(TargetItem, Mods);

	//	If no enhancement, then we exit

	switch (iResult)
		{
		case eisNoEffect:
		case eisAlreadyEnhanced:
		case eisCantReplaceDefect:
		case eisCantReplaceEnhancement:
			return iResult;
		}

	//	Notify any dock screens that we might modify an item

	DWORD dwID = OBJID_NULL;

	IDockScreenUI::SModifyItemCtx ModifyCtx;
	OnModifyItemBegin(ModifyCtx, TargetItem);

	//	If repairing, then do it here.

	if (iResult == eisItemRepaired)
		{
		ItemList.SetDamagedAtCursor(false);
		}

	//	Enhance

	else
		{
		switch (Enhancement.GetModCode())
			{
			case etBinaryEnhancement:
				dwID = OBJID_NULL;
				ItemList.SetEnhancedAtCursor(true);
				break;

			default:
				//	NOTE: This call handles etNone properly by removing the 
				//	enhancement and returning a null ID.

				dwID = ItemList.AddItemEnhancementAtCursor(Enhancement);
				break;
			}

		//	Deal with installed items

		ItemEnhancementModified(ItemList);

		//	Fire On event to the enhancement

		if (Mods.GetEnhancementType() && ItemList.IsCursorValid())
			{
			CItem theEnhancement(Mods.GetEnhancementType(), 1);
			theEnhancement.FireOnAddedAsEnhancement(this, ItemList.GetItemAtCursor(), iResult);
			}
		}

	//	Update the object

	if (ItemList.IsCursorValid())
		OnModifyItemComplete(ModifyCtx, ItemList.GetItemAtCursor());

	//	Done

	if (retdwID)
		*retdwID = dwID;

	//	Done

	return iResult;

	DEBUG_CATCH
	}

bool CSpaceObject::EnhanceItem (CItemListManipulator &ItemList, const CItem &EnhancementItem, CItem::SEnhanceItemResult &retResult, CString *retsError)

//	EnhanceItem
//
//	Enhances the currently selected item with the given enhancement.

	{
	ASSERT(ItemList.IsCursorValid());
	ASSERT(!EnhancementItem.IsEmpty());

	//	See if we have an <OnEnhanceItem> event.

	retResult = CItem::SEnhanceItemResult();
	if (!EnhancementItem.FireOnEnhanceItem(*this, ItemList.GetItemAtCursor(), retResult, retsError))
		return false;

	if (retResult.iResult != eisUnknown)
		return true;

	//	Get the enhancement to confer

	if (!EnhancementItem.GetEnhancementConferred(*this, ItemList.GetItemAtCursor(), CONSTLIT("onEnhance"), retResult, retsError))
		return false;

	//	If no mod, nothing to do.

	if (retResult.Enhancement.IsEmpty())
		{
		retResult.iResult = eisNoEffect;
		return true;
		}

	//	Otherwise, enhance

	else
		{
		DWORD dwID;
		retResult.iResult = EnhanceItem(ItemList, retResult.Enhancement, &dwID);
		retResult.Enhancement.SetID(dwID);
		}

	//	Done

	return true;
	}

void CSpaceObject::EnterGate (CTopologyNode *pDestNode, const CString &sDestEntryPoint, CSpaceObject *pStargate, bool bAscend)

//	EnterGate
//
//	We enter a stargate.
//
//	NOTE: pDestNode and pStargate may be NULL.

	{
	DEBUG_TRY

	int i;

	//	If we're going to the same system, then do nothing

	if (pDestNode && pDestNode->GetSystem() == m_pSystem)
		return;

	//	Notify subscribers

	m_SubscribedObjs.NotifyOnObjEnteredGate(this, pDestNode, sDestEntryPoint, pStargate);

	//	Handle the case where the player entered the gate

	if (IsPlayer())
		{
		//	Clear destination, if necessary

		if (pStargate && pStargate->IsAutoClearDestinationOnGate())
			pStargate->ClearPlayerDestination();

		//	If some object destroyed the player while gating

		if (IsDestroyed())
			return;
		}

	//	Tell all listeners that this object entered a stargate

	for (i = 0; i < m_pSystem->GetObjectCount(); i++)
		{
		CSpaceObject *pObj = m_pSystem->GetObject(i);

		if (pObj && pObj != this)
			pObj->OnObjEnteredGate(this, pDestNode, sDestEntryPoint, pStargate);
		}

	//	Let the object do the appropriate thing when entering a gate
	//	Note: Objects rely on this happening after other objects
	//	are notified.

	GateHook(pDestNode, sDestEntryPoint, pStargate, bAscend);

	DEBUG_CATCH
	}

int CSpaceObject::FindCommsMessage (const CString &sID)

//	FindCommsMessage
//
//	Returns the index of the given comms message (or -1 if not found)

	{
	CCommunicationsHandler *pHandler = GetCommsHandler();
	if (pHandler == NULL)
		return -1;

	int iIndex = pHandler->FindMessageByID(sID);
	if (iIndex == -1)
		{
		//	For backwards compatibility we handle some special IDs.


		}

	//	Success!

	return iIndex;
	}

int CSpaceObject::FindCommsMessageByName (const CString &sName)

//	FindCommsMessageByName
//
//	Returns the index of the given comms message (or -1 if not found)

	{
	CCommunicationsHandler *pHandler = GetCommsHandler();
	if (pHandler == NULL)
		return -1;

	return pHandler->FindMessageByName(sName);
	}

bool CSpaceObject::FindDevice (const CItem &Item, CInstalledDevice **retpDevice, CString *retsError)

//	FindDevice
//
//	Looks for the device of the item; returns an error if not found.

	{
	CInstalledDevice *pDevice = FindDevice(Item);
	if (pDevice == NULL)
		{
		*retsError = CONSTLIT("Item is not an installed device on object.");
		return false;
		}

	if (retpDevice)
		*retpDevice = pDevice;

	return true;
	}

bool CSpaceObject::FindEventHandler (const CString &sEntryPoint, SEventHandlerDesc *retEvent) const

//	FindEventHandler
//
//	Finds the event handler for the given event

	{
	//	Check our override

	if (m_pOverride && m_pOverride->FindEventHandler(sEntryPoint, retEvent))
		return true;

	//	Check our type

	CDesignType *pType = GetType();
	if (pType)
		return pType->FindEventHandler(sEntryPoint, retEvent);
	
	//	Not found

	return false;
	}

bool CSpaceObject::FindEventHandler (CDesignType::ECachedHandlers iEvent, SEventHandlerDesc *retEvent) const

//	FindEventHandler
//
//	Finds the event handler for the given event

	{
	//	Check our override

	if (m_pOverride && m_pOverride->FindEventHandler(iEvent, retEvent))
		return true;

	//	Check our type

	CDesignType *pType = GetType();
	if (pType)
		return pType->FindEventHandler(iEvent, retEvent);
	
	//	Not found

	return false;
	}

bool CSpaceObject::FireCanDockAsPlayer (CSpaceObject *pDockTarget, CString *retsError)

//	FireCanDockAsPlayer
//
//	Fires an event to ask the object if it should be allowed to dock with the
//	given dock target.
//
//	Returns TRUE if docking is allowed. If FALSE, retsError is initialized with 
//	the message to return to the player.

	{
	SEventHandlerDesc Event;
	if (FindEventHandler(CAN_DOCK_AS_PLAYER_EVENT, &Event))
		{
		CCodeChainCtx Ctx(GetUniverse());
		Ctx.DefineContainingType(this);
		Ctx.SaveAndDefineSourceVar(this);
		Ctx.DefineSpaceObject(CONSTLIT("aDockTarget"), pDockTarget);

		ICCItemPtr pResult = Ctx.RunCode(Event);

		bool bAllowDock;
		if (pResult->IsError())
			{
			ReportEventError(CAN_DOCK_AS_PLAYER_EVENT, pResult);
			*retsError = NULL_STR;
			bAllowDock = false;
			}
		else if (!pResult->IsTrue())
			{
			*retsError = pResult->GetStringValue();
			bAllowDock = false;
			}
		else
			bAllowDock = true;

		return bAllowDock;
		}
	else
		return true;
	}

bool CSpaceObject::FireCanInstallItem (const CItem &Item, const CDeviceSystem::SSlotDesc &Slot, CString *retsResult)

//	FireCanInstallItem
//
//	Asks the object whether we can install the given item.

	{
	SEventHandlerDesc Event;
	if (FindEventHandler(CDesignType::evtCanInstallItem, &Event))
		{
		CCodeChainCtx Ctx(GetUniverse());
		Ctx.DefineContainingType(this);
		Ctx.SaveAndDefineSourceVar(this);
		Ctx.SaveAndDefineItemVar(Item);

		if (Slot.iIndex == -1)
			{
			Ctx.DefineNil(CONSTLIT("aArmorSeg"));
			Ctx.DefineNil(CONSTLIT("aItemToReplace"));
			}
		else if (Item.IsArmor())
			{
			Ctx.DefineInteger(CONSTLIT("aArmorSeg"), Slot.iIndex);
			Ctx.DefineNil(CONSTLIT("aItemToReplace"));
			}
		else
			{
			CItemListManipulator ItemList(GetItemList());
			if (SetCursorAtDevice(ItemList, Slot.iIndex))
				{
				const CItem &ItemToReplace = ItemList.GetItemAtCursor();
				Ctx.DefineItem(CONSTLIT("aItemToReplace"), ItemToReplace);
				}
			else
				Ctx.DefineNil(CONSTLIT("aItemToReplace"));

			Ctx.DefineNil(CONSTLIT("aArmorSeg"));
			}

		ICCItemPtr pResult = Ctx.RunCode(Event);

		bool bCanBeInstalled;
		if (pResult->IsError())
			{
			*retsResult = pResult->GetStringValue();
			ReportEventError(strPatternSubst(CONSTLIT("Ship %x CanInstallItem"), GetType()->GetUNID()), pResult);
			bCanBeInstalled = false;
			}
		else if (!pResult->IsTrue())
			{
			*retsResult = pResult->GetStringValue();
			bCanBeInstalled = false;
			}
		else
			bCanBeInstalled = true;

		return bCanBeInstalled;
		}
	else
		return true;
	}

bool CSpaceObject::FireCanRemoveItem (const CItem &Item, int iSlot, CString *retsResult)

//	FireCanRemoveItem
//
//	Asks the object whether we can remove the given item.

	{
	SEventHandlerDesc Event;
	if (FindEventHandler(CDesignType::evtCanRemoveItem, &Event))
		{
		CCodeChainCtx Ctx(GetUniverse());
		Ctx.DefineContainingType(this);
		Ctx.SaveAndDefineSourceVar(this);
		Ctx.SaveAndDefineItemVar(Item);
		if (iSlot != -1)
			Ctx.DefineInteger(CONSTLIT("aArmorSeg"), iSlot);
		else
			Ctx.DefineNil(CONSTLIT("aArmorSeg"));

		ICCItemPtr pResult = Ctx.RunCode(Event);

		bool bCanBeRemoved;
		if (pResult->IsError())
			{
			*retsResult = pResult->GetStringValue();
			ReportEventError(strPatternSubst(CONSTLIT("Ship %x CanRemoveItem"), GetType()->GetUNID()), pResult);
			bCanBeRemoved = false;
			}
		else if (!pResult->IsTrue())
			{
			*retsResult = pResult->GetStringValue();
			bCanBeRemoved = false;
			}
		else
			bCanBeRemoved = true;

		return bCanBeRemoved;
		}
	else
		return true;
	}

int CSpaceObject::GetNextAutoDefenseDeviceIndex (int iDev)
	{
	if(iDev < 0)
		return iDev;
	for (int i = iDev; i < GetDeviceCount(); i++)
		{
		if (GetDevice(i)->IsAutomatedWeapon())
			return i;
		}
	return -1;
	}

void CSpaceObject::FireCustomEvent (const CString &sEvent, ECodeChainEvents iEvent, ICCItem *pData, ICCItem **retpResult)

//	FireCustomEvent
//
//	Fires a named event and optionally returns result

	{
	CCodeChainCtx Ctx(GetUniverse());

	SEventHandlerDesc Event;
	if (FindEventHandler(sEvent, &Event))
		{
		Ctx.SetEvent(iEvent);
		Ctx.DefineContainingType(this);
		Ctx.SaveAndDefineSourceVar(this);
		Ctx.SaveAndDefineDataVar(pData);

		ICCItemPtr pResult = Ctx.RunCode(Event);
		if (pResult->IsError())
			ReportEventError(sEvent, pResult);

		//	Either return the event result or discard it

		if (retpResult)
			*retpResult = pResult->Reference();
		}
	else
		{
		if (retpResult)
			*retpResult = Ctx.CreateNil();
		}
	}

void CSpaceObject::FireCustomItemEvent (const CString &sEvent, const CItem &Item, ICCItem *pData, ICCItem **retpResult)

//	FireCustomItemEvent
//
//	Fires a named event to an item and optionally returns result

	{
	CItemCtx ItemCtx(&Item, this);
	Item.FireCustomEvent(ItemCtx, sEvent, pData, retpResult);
	}

void CSpaceObject::FireCustomOverlayEvent (const CString &sEvent, DWORD dwOverlayID, ICCItem *pData, ICCItem **retpResult)

//	FireCustomOverlayEvent
//
//	Fires a custom event on an overlay

	{
	CCodeChain &CC = GetUniverse().GetCC();

	//	Find the overlay

	COverlay *pOverlay = GetOverlay(dwOverlayID);
	if (pOverlay == NULL)
		{
		if (retpResult)
			*retpResult = CC.CreateNil();
		return;
		}

	//	Fire event

	pOverlay->FireCustomEvent(this, sEvent, pData, retpResult);
	}

void CSpaceObject::FireCustomShipOrderEvent (const CString &sEvent, CSpaceObject *pShip, ICCItem **retpResult)

//	FireCustomShipOrderEvent
//
//	Fires an event in response to a fireEvent order.

	{
	CCodeChainCtx Ctx(GetUniverse());

	SEventHandlerDesc Event;
	if (FindEventHandler(sEvent, &Event))
		{
		Ctx.DefineContainingType(this);
		Ctx.SaveAndDefineSourceVar(this);
		Ctx.SaveAndDefineDataVar(NULL);
		Ctx.DefineSpaceObject(CONSTLIT("aShipObj"), pShip);

		ICCItemPtr pResult = Ctx.RunCode(Event);
		if (pResult->IsError())
			ReportEventError(sEvent, pResult);

		//	Either return the event result or discard it

		if (retpResult)
			*retpResult = pResult->Reference();
		}
	else
		{
		if (retpResult)
			*retpResult = Ctx.CreateNil();
		}
	}

bool CSpaceObject::FireGetDockScreen (CDockScreenSys::SSelector *retSelector) const

//	FireGetDockScreen
//
//	Allows the object to override the first dock screen

	{
	SEventHandlerDesc Event;
	if (!HasGetDockScreenEvent() 
			|| !FindEventHandler(GET_DOCK_SCREEN_EVENT, &Event))
		return false;

	CCodeChainCtx Ctx(GetUniverse());
	Ctx.DefineContainingType(this);
	Ctx.SaveAndDefineSourceVar(this);

	ICCItemPtr pResult = Ctx.RunCode(Event);
	if (pResult->IsError())
		{
		ReportEventError(GET_DOCK_SCREEN_EVENT, pResult);
		return false;
		}

	return CTLispConvert::AsScreenSelector(pResult, retSelector);
	}

bool CSpaceObject::FireGetExplosionType (SExplosionType *retExplosion) const

//	FireGetExplosionType
//
//	Allows the object to compute the kind of explosion

	{
	SEventHandlerDesc Event;

	if (FindEventHandler(GET_EXPLOSION_TYPE_EVENT, &Event))
		{
		CCodeChainCtx Ctx(GetUniverse());
		Ctx.DefineContainingType(this);
		Ctx.SaveAndDefineSourceVar(this);

		ICCItemPtr pResult = Ctx.RunCode(Event);
		if (pResult->IsError())
			ReportEventError(GET_EXPLOSION_TYPE_EVENT, pResult);

		//	Return UNID, bonus, and cause

		DWORD dwUNID;
		int iBonus;
		DestructionTypes iCause;

		//	If the result is a list, then we expect a list with the following values:
		//
		//	The UNID of the explosion
		//	The bonus
		//	The cause (e.g., "explosion" or "playerCreatedExplosion")

		if (pResult->IsNil())
			{
			dwUNID = 0;
			iBonus = 0;
			iCause = killedByExplosion;
			}
		else if (pResult->IsList())
			{
			dwUNID = (DWORD)pResult->GetElement(0)->GetIntegerValue();
			iBonus = pResult->GetElement(1)->GetIntegerValue();
			if (pResult->GetElement(2)->IsNil())
				iCause = killedByExplosion;
			else
				{
				iCause = ::GetDestructionCause(pResult->GetElement(2)->GetStringValue());
				if (iCause == killedNone)
					iCause = killedByExplosion;
				}
			}

		//	Otherwise, expect just an UNID
		else
			{
			dwUNID = (DWORD)pResult->GetIntegerValue();
			iBonus = 0;
			iCause = killedByExplosion;
			}

		//	Return

		retExplosion->pDesc = (dwUNID ? GetUniverse().FindWeaponFireDesc(strPatternSubst(CONSTLIT("%d/0"), dwUNID)) : NULL);
		retExplosion->iBonus = iBonus;
		retExplosion->iCause = iCause;

		return (retExplosion->pDesc != NULL);
		}
	else
		{
		retExplosion->pDesc = NULL;
		retExplosion->iBonus = 0;
		retExplosion->iCause = killedByExplosion;

		return false;
		}
	}

bool CSpaceObject::FireGetPlayerPriceAdj (STradeServiceCtx &ServiceCtx, ICCItem *pData, int *retiPriceAdj) const

//	FireGetPlayerPriceAdj
//
//	Fire GetPlayerPriceAdj

	{
	SEventHandlerDesc Event;

	if (FindEventHandler(GET_PLAYER_PRICE_ADJ_EVENT, &Event))
		{
		CCodeChainCtx Ctx(GetUniverse());

		//	Set up

		Ctx.SetEvent(eventGetTradePrice);
		Ctx.DefineContainingType(this);
		Ctx.SaveAndDefineSourceVar(this);
		if (ServiceCtx.pItem)
			{
			Ctx.SaveAndDefineItemVar(*ServiceCtx.pItem);
			Ctx.SetItemType(ServiceCtx.pItem->GetType());
			}
		else if (ServiceCtx.pObj)
			{
			Ctx.DefineSpaceObject(CONSTLIT("aObj"), ServiceCtx.pObj);
			}

		Ctx.DefineString(CONSTLIT("aService"), CTradingDesc::ServiceToString(ServiceCtx.iService));
		Ctx.DefineSpaceObject(CONSTLIT("aProviderObj"), ServiceCtx.pProvider);
		if (pData)
			Ctx.SaveAndDefineDataVar(pData);

		//	Run

		ICCItemPtr pResult = Ctx.RunCode(Event);

		int iPriceAdj = 100;
		if (pResult->IsError())
			ReportEventError(GET_PLAYER_PRICE_ADJ_EVENT, pResult);
		else if (pResult->IsNil())
			;
		else
			iPriceAdj = pResult->GetIntegerValue();

		//	Done

		if (retiPriceAdj)
			*retiPriceAdj = iPriceAdj;

		return (iPriceAdj != 100);
		}
	else
		return false;
	}

void CSpaceObject::FireItemOnAIUpdate (void)

//	FireItemOnAIUpdate
//
//	Fires OnAIUpdate event for all items

	{
	if (!m_fItemEventsValid)
		InitItemEvents();

	m_ItemEvents.FireEvent(this, eventOnAIUpdate);
	}

void CSpaceObject::FireItemOnDocked (CSpaceObject *pDockedAt)

//	FireItemOnDocked
//
//	Fires OnDocked event for all items

	{
	//	We need to initialize, even if we don't use the event list because we
	//	rely on the flag to tell us if items changed out from under us.

	if (!m_fItemEventsValid)
		InitItemEvents();

	m_ItemEvents.FireOnDocked(this, pDockedAt);
	}

void CSpaceObject::FireItemOnObjDestroyed (const SDestroyCtx &Ctx)

//	FireItemOnObjDestroyed
//
//	Fires OnObjDestroyed event for all items

	{
	//	We need to initialize, even if we don't use the event list because we
	//	rely on the flag to tell us if items changed out from under us.

	if (!m_fItemEventsValid)
		InitItemEvents();

	m_ItemEvents.FireOnObjDestroyed(this, Ctx);
	}

void CSpaceObject::FireItemOnUpdate (void)

//	FireItemOnUpdate
//
//	Fires OnUpdate event for all items

	{
	if (!m_fItemEventsValid)
		InitItemEvents();

	m_ItemEvents.FireUpdateEvents(this);
	}

void CSpaceObject::FireOnAttacked (const SDamageCtx &Ctx)

//	FireOnAttacked
//
//	Fire OnAttacked event

	{
	DEBUG_TRY

	SEventHandlerDesc Event;

	if (FindEventHandler(ON_ATTACKED_EVENT, &Event))
		{
		CCodeChainCtx CCCtx(GetUniverse());
		CCCtx.DefineContainingType(this);
		CCCtx.SaveAndDefineSourceVar(this);
		CCCtx.DefineDamageCtx(Ctx);

		ICCItemPtr pResult = CCCtx.RunCode(Event);
		if (pResult->IsError())
			ReportEventError(ON_ATTACKED_EVENT, pResult);
		}

	DEBUG_CATCH
	}

void CSpaceObject::FireOnAttackedByPlayer (void)

//	FireOnAttackedByPlayer
//
//	Fire OnAttackedByPlayer event

	{
	SEventHandlerDesc Event;

	if (FindEventHandler(ON_ATTACKED_BY_PLAYER_EVENT, &Event))
		{
		CCodeChainCtx Ctx(GetUniverse());
		Ctx.DefineContainingType(this);
		Ctx.SaveAndDefineSourceVar(this);

		ICCItemPtr pResult = Ctx.RunCode(Event);
		if (pResult->IsError())
			ReportEventError(ON_ATTACKED_BY_PLAYER_EVENT, pResult);
		}
	}

void CSpaceObject::FireOnAutoLoot (CSpaceObject &LootedBy, const CItemList &Loot)

//	FireOnAutoLoot
//
//	Fire OnAutoLoot event

	{
	SEventHandlerDesc Event;
	if (!FindEventHandler(ON_AUTO_LOOT_EVENT, &Event))
		return;

	CCodeChainCtx Ctx(GetUniverse());
	Ctx.DefineContainingType(this);
	Ctx.SaveAndDefineSourceVar(this);
	Ctx.DefineSpaceObject(CONSTLIT("aLootedBy"), LootedBy);
	Ctx.DefineItemList(CONSTLIT("aLoot"), Loot);

	ICCItemPtr pResult = Ctx.RunCode(Event);
	if (pResult->IsError())
		ReportEventError(ON_AUTO_LOOT_EVENT, pResult);
	}

void CSpaceObject::FireOnCreate (void)

//	FireOnCreate
//
//	Fire OnCreate event

	{
	FireOnCreate(SOnCreate());
	}

void CSpaceObject::FireOnCreate (const SOnCreate &OnCreate)

//	FireOnCreate
//
//	Fire OnCreate event

	{
	SEventHandlerDesc Event;

	if (!m_fOnCreateCalled 
			&& FindEventHandler(ON_CREATE_EVENT, &Event))
		{
#ifdef DEBUG_ON_CREATE_TIME
		DWORD dwStart = ::GetTickCount();
#endif

		CCodeChainCtx Ctx(GetUniverse());
		Ctx.SetSystemCreateCtx(OnCreate.pCreateCtx);
		Ctx.DefineContainingType(this);
		Ctx.SaveAndDefineSourceVar(this);
		Ctx.SaveAndDefineDataVar(OnCreate.pData);
		Ctx.DefineSpaceObject(CONSTLIT("aBaseObj"), OnCreate.pBaseObj);
		Ctx.DefineSpaceObject(CONSTLIT("aOwnerObj"), OnCreate.pOwnerObj);
		Ctx.DefineSpaceObject(CONSTLIT("aTargetObj"), OnCreate.pTargetObj);
		if (OnCreate.pOrbit)
			Ctx.DefineOrbit(CONSTLIT("aOrbit"), *OnCreate.pOrbit);

		ICCItemPtr pResult = Ctx.RunCode(Event);
		if (pResult->IsError())
			ReportEventError(ON_CREATE_EVENT, pResult);

#ifdef DEBUG_ON_CREATE_TIME
		g_dwTotalTime += ::sysGetTicksElapsed(dwStart);
		printf("[%x] OnCreate: %s (%d)\n", GetID(), (LPSTR)GetNounPhrase(), g_dwTotalTime);
#endif
		}

	//	Remember that we already called OnCreate. This is helpful in case we
	//	create an object inside another object's OnCreate
	//
	//	[We set this even if there is no OnCreate event because we use this
	//	to test whether we're fully initialized.]

	m_fOnCreateCalled = true;
	}

void CSpaceObject::FireOnCreateOrders (CSpaceObject *pBase, CSpaceObject *pTarget)

//	FireOnCreateOrders
//
//	Fire OnCreateOrders event

	{
	SEventHandlerDesc Event;

	if (FindEventHandler(ON_CREATE_ORDERS_EVENT, &Event))
		{
		CCodeChainCtx Ctx(GetUniverse());
		Ctx.DefineContainingType(this);
		Ctx.SaveAndDefineSourceVar(this);
		Ctx.DefineSpaceObject(CONSTLIT("aBaseObj"), pBase);
		Ctx.DefineSpaceObject(CONSTLIT("aTargetObj"), pTarget);

		ICCItemPtr pResult = Ctx.RunCode(Event);
		if (pResult->IsError())
			ReportEventError(ON_CREATE_ORDERS_EVENT, pResult);
		}
	}

int CSpaceObject::FireOnDamage (SDamageCtx &Ctx, int iDamage)

//	FireOnDamage
//
//	Fire OnDamage event

	{
	SEventHandlerDesc Event;

	if (FindEventHandler(ON_DAMAGE_EVENT, &Event))
		{
		CCodeChainCtx CCCtx(GetUniverse());
		CCCtx.DefineContainingType(this);
		CCCtx.SaveAndDefineSourceVar(this);
		CCCtx.DefineDamageCtx(Ctx, iDamage);

		ICCItemPtr pResult = CCCtx.RunCode(Event);
		if (pResult->IsError())
			ReportEventError(ON_DAMAGE_EVENT, pResult);

		//	Result is the amount of damage

		iDamage = pResult->GetIntegerValue();
		}

	return iDamage;
	}

void CSpaceObject::FireOnDeselected (void)

//	FireOnDeselected
//
//	Fire OnDeselected event

	{
	SEventHandlerDesc Event;

	if (FindEventHandler(ON_DESELECTED_EVENT, &Event))
		{
		CCodeChainCtx CCCtx(GetUniverse());
		CCCtx.DefineContainingType(this);
		CCCtx.SaveAndDefineSourceVar(this);
		CCCtx.DefineInteger(CONSTLIT("aPlayer"), g_PlayerSovereignUNID);

		//	Run code

		ICCItemPtr pResult = CCCtx.RunCode(Event);
		if (pResult->IsError())
			ReportEventError(ON_DESELECTED_EVENT, pResult);
		}
	}

void CSpaceObject::FireOnDestroy (const SDestroyCtx &Ctx)

//	FireOnDestroy
//
//	Fire OnDestroy event

	{
	DEBUG_TRY

	SEventHandlerDesc Event;

	if (FindEventHandler(ON_DESTROY_EVENT, &Event))
		{
		CCodeChainCtx CCCtx(GetUniverse());
		CCCtx.DefineContainingType(this);
		CCCtx.SaveAndDefineSourceVar(this);
		CCCtx.DefineSpaceObject(CONSTLIT("aDestroyer"), Ctx.Attacker.GetObj());
		CCCtx.DefineSpaceObject(CONSTLIT("aOrderGiver"), Ctx.GetOrderGiver());
		CCCtx.DefineSpaceObject(CONSTLIT("aWreckObj"), Ctx.pWreck);
		CCCtx.DefineBool(CONSTLIT("aDestroy"), Ctx.WasDestroyed());
		CCCtx.DefineString(CONSTLIT("aDestroyReason"), GetDestructionName(Ctx.iCause));

		//	Run code

		ICCItemPtr pResult = CCCtx.RunCode(Event);
		if (pResult->IsError())
			ReportEventError(ON_DESTROY_EVENT, pResult);
		}

	DEBUG_CATCH
	}

bool CSpaceObject::FireOnDestroyCheck (DestructionTypes iCause, const CDamageSource &Attacker)

//	FireOnDestroyCheck
//
//	Fire OnDestroyCheck event. Returns FALSE if we avoid destruction.

	{
	CCodeChainCtx Ctx(GetUniverse());

	//	If we have code, call it.

	SEventHandlerDesc Event;
	if (FindEventHandler(CDesignType::evtOnDestroyCheck, &Event)
			&& !Ctx.InEvent(eventOnDestroyCheck))
		{
		Ctx.SetEvent(eventOnDestroyCheck);
		Ctx.DefineContainingType(this);
		Ctx.SaveAndDefineSourceVar(this);
		Ctx.DefineSpaceObject(CONSTLIT("aDestroyer"), Attacker.GetObj());
		Ctx.DefineSpaceObject(CONSTLIT("aOrderGiver"), Attacker.GetOrderGiver());
		Ctx.DefineBool(CONSTLIT("aDestroy"), (iCause != enteredStargate && iCause != ascended));
		Ctx.DefineString(CONSTLIT("aDestroyReason"), GetDestructionName(iCause));

		ICCItemPtr pResult = Ctx.RunCode(Event);
		return !pResult->IsNil();
		}

	//	Otherwise, destruction succeeds

	return true;
	}

void CSpaceObject::FireOnDestroyObj (const SDestroyCtx &Ctx)

//	FireOnDestroyObj
//
//	Fire OnDestroyObj event

	{
	SEventHandlerDesc Event;

	if (FindEventHandler(ON_DESTROY_OBJ_EVENT, &Event))
		{
		CCodeChainCtx CCCtx(GetUniverse());
		CCCtx.DefineContainingType(this);
		CCCtx.SaveAndDefineSourceVar(this);
		CCCtx.DefineSpaceObject(CONSTLIT("aObjDestroyed"), Ctx.Obj);
		CCCtx.DefineSpaceObject(CONSTLIT("aDestroyer"), Ctx.Attacker.GetObj());
		CCCtx.DefineSpaceObject(CONSTLIT("aOrderGiver"), Ctx.GetOrderGiver());
		CCCtx.DefineSpaceObject(CONSTLIT("aWreckObj"), Ctx.pWreck);
		CCCtx.DefineBool(CONSTLIT("aDestroy"), Ctx.WasDestroyed());
		CCCtx.DefineString(CONSTLIT("aDestroyReason"), GetDestructionName(Ctx.iCause));

		ICCItemPtr pResult = CCCtx.RunCode(Event);
		if (pResult->IsError())
			ReportEventError(ON_DESTROY_OBJ_EVENT, pResult);
		}
	}

bool CSpaceObject::FireOnDockObjAdj (CSpaceObject **retpObj)

//	FireOnDockObjAdj
//
//	Fires an event to adjust the object that a player will dock with

	{
	SEventHandlerDesc Event;

	if (FindEventHandler(ON_DOCK_OBJ_ADJ_EVENT, &Event))
		{
		CCodeChainCtx Ctx(GetUniverse());
		Ctx.DefineContainingType(this);
		Ctx.SaveAndDefineSourceVar(this);

		ICCItemPtr pResult = Ctx.RunCode(Event);

		if (pResult->IsError())
			{
			ReportEventError(ON_DOCK_OBJ_ADJ_EVENT, pResult);
			return false;
			}
		else if (pResult->IsInteger())
			{
			CSpaceObject *pNewObj = Ctx.AsSpaceObject(pResult);

			if (pNewObj == NULL || pNewObj == this)
				return false;

			*retpObj = pNewObj;
			return true;
			}
		else
			{
			return false;
			}
		}

	return false;
	}

void CSpaceObject::FireOnDockObjDestroyed (CSpaceObject *pDockTarget, const SDestroyCtx &Ctx)

//	FireOnDockObjDestroyed
//
//	Fire event when a object that we are docked with (or in the process of 
//	docking with) got destroyed.

	{
	SEventHandlerDesc Event;
	if (!FindEventHandler(ON_DOCK_OBJ_DESTROYED_EVENT, &Event))
		return;

	CCodeChainCtx CCCtx(GetUniverse());
	CCCtx.DefineContainingType(this);
	CCCtx.SaveAndDefineSourceVar(this);
	CCCtx.DefineSpaceObject(CONSTLIT("aObjDestroyed"), Ctx.Obj);
	CCCtx.DefineSpaceObject(CONSTLIT("aDestroyer"), Ctx.Attacker.GetObj());
	CCCtx.DefineSpaceObject(CONSTLIT("aOrderGiver"), Ctx.GetOrderGiver());
	CCCtx.DefineSpaceObject(CONSTLIT("aWreckObj"), Ctx.pWreck);
	CCCtx.DefineBool(CONSTLIT("aDestroy"), Ctx.WasDestroyed());
	CCCtx.DefineString(CONSTLIT("aDestroyReason"), GetDestructionName(Ctx.iCause));

	ICCItemPtr pResult = CCCtx.RunCode(Event);
	if (pResult->IsError())
		ReportEventError(ON_DOCK_OBJ_DESTROYED_EVENT, pResult);
	}

void CSpaceObject::FireOnEnteredGate (CTopologyNode *pDestNode, const CString &sDestEntryPoint, CSpaceObject *pGate)

//	FireOnEnteredGate
//
//	Fire event when this object has entered a gate

	{
	SEventHandlerDesc Event;

	if (FindEventHandler(ON_ENTERED_GATE_EVENT, &Event))
		{
		CCodeChainCtx Ctx(GetUniverse());
		Ctx.DefineContainingType(this);
		Ctx.SaveAndDefineSourceVar(this);
		Ctx.DefineSpaceObject(CONSTLIT("aGateObj"), pGate);
		Ctx.DefineString(CONSTLIT("aDestNodeID"), (pDestNode ? pDestNode->GetID() : NULL_STR));
		Ctx.DefineString(CONSTLIT("aDestEntryPoint"), sDestEntryPoint);

		//	Run code

		ICCItemPtr pResult = Ctx.RunCode(Event);
		if (pResult->IsError())
			ReportEventError(ON_ENTERED_GATE_EVENT, pResult);
		}
	}

void CSpaceObject::FireOnEnteredSystem (CSpaceObject *pGate)

//	FireOnEnteredSystem
//
//	Fire event when this object has comes out of a gate into a new system

	{
	SEventHandlerDesc Event;

	if (FindEventHandler(ON_ENTERED_SYSTEM_EVENT, &Event))
		{
		CCodeChainCtx Ctx(GetUniverse());
		Ctx.DefineContainingType(this);
		Ctx.SaveAndDefineSourceVar(this);
		Ctx.DefineSpaceObject(CONSTLIT("aGateObj"), pGate);

		//	Run code

		ICCItemPtr pResult = Ctx.RunCode(Event);
		if (pResult->IsError())
			ReportEventError(ON_ENTERED_SYSTEM_EVENT, pResult);
		}
	}

void CSpaceObject::FireOnLoad (SLoadCtx &Ctx)

//	FireOnLoad
//
//	Fire OnLoad event

	{
	SEventHandlerDesc Event;

	if (FindEventHandler(ON_LOAD_EVENT, &Event))
		{
		CCodeChainCtx CCCtx(GetUniverse());
		CCCtx.DefineContainingType(this);
		CCCtx.SaveAndDefineSourceVar(this);
		CCCtx.DefineInteger(CONSTLIT("aVersion"), Ctx.dwVersion);

		ICCItemPtr pResult = CCCtx.RunCode(Event);
		if (pResult->IsError())
			ReportEventError(ON_LOAD_EVENT, pResult);
		}
	}

void CSpaceObject::FireOnMining (const SDamageCtx &Ctx, EAsteroidType iType)

//	FireOnMining
//
//	Fire OnMining event

	{
	SEventHandlerDesc Event;
	if (!FindEventHandler(ON_MINING_EVENT, &Event))
		return;

	//	Compute some mining values:
	//
	//	Mining difficulty 0-100, based on properties or default value
	//		from asteroid type
	//
	//	Max ore level, based on damage type
	//
	//	Chance of success
	//
	//	Yield, based on mining level, asteroid type, and weapon type.

	//	Figure out the mining difficulty.

	const int iMiningLevel = Ctx.Damage.GetMiningAdj();
	const int iMiningDifficulty = CalcMiningDifficulty(iType);

	CAsteroidDesc::SMiningStats MiningStats;
	CAsteroidDesc::CalcMining(iMiningLevel, iMiningDifficulty, iType, Ctx, MiningStats);

	//	Run

	CCodeChainCtx CCX(GetUniverse());
	CCX.DefineContainingType(this);
	CCX.SaveAndDefineSourceVar(this);
	CCX.DefineSpaceObject(CONSTLIT("aCause"), Ctx.pCause);
	CCX.DefineSpaceObject(CONSTLIT("aOrderGiver"), Ctx.GetOrderGiver());
	CCX.DefineSpaceObject(CONSTLIT("aMiner"), Ctx.Attacker.GetObj());
	CCX.DefineVector(CONSTLIT("aMinePos"), Ctx.vHitPos);
	CCX.DefineInteger(CONSTLIT("aMineDir"), Ctx.iDirection);
	CCX.DefineInteger(CONSTLIT("aMineProbability"), iMiningLevel);
	CCX.DefineInteger(CONSTLIT("aMineDifficulty"), iMiningDifficulty);
	CCX.DefineString(CONSTLIT("aAsteroidType"), CAsteroidDesc::CompositionID(iType));
	CCX.DefineInteger(CONSTLIT("aSuccessChance"), MiningStats.iSuccessChance);
	CCX.DefineInteger(CONSTLIT("aMaxOreLevel"), MiningStats.iMaxOreLevel);
	CCX.DefineDouble(CONSTLIT("aYieldAdj"), MiningStats.rYieldAdj);
	CCX.DefineInteger(CONSTLIT("aHP"), Ctx.iDamage);
	CCX.DefineString(CONSTLIT("aDamageType"), GetDamageShortName(Ctx.Damage.GetDamageType()));
	CCX.DefineItemType(CONSTLIT("aWeaponType"), Ctx.GetDesc().GetWeaponType());

	ICCItemPtr pResult = CCX.RunCode(Event);
	if (pResult->IsError())
		ReportEventError(ON_MINING_EVENT, pResult);
	}

void CSpaceObject::FireOnMissionAccepted (CMission *pMission)

//	FireOnMissionAccepted
//
//	Fire <OnMissionAccepted> event

	{
	SEventHandlerDesc Event;

	if (FindEventHandler(ON_MISSION_ACCEPTED_EVENT, &Event))
		{
		CCodeChainCtx CCCtx(GetUniverse());
		CCCtx.DefineContainingType(this);
		CCCtx.SaveAndDefineSourceVar(this);
		CCCtx.DefineSpaceObject(CONSTLIT("aMissionObj"), pMission);

		ICCItemPtr pResult = CCCtx.RunCode(Event);
		if (pResult->IsError())
			ReportEventError(ON_MISSION_ACCEPTED_EVENT, pResult);
		}
	}

void CSpaceObject::FireOnMissionCompleted (CMission *pMission, const CString &sReason)

//	FireOnMissionCompleted
//
//	Fire <OnMissionCompleted> event

	{
	SEventHandlerDesc Event;

	if (FindEventHandler(ON_MISSION_COMPLETED_EVENT, &Event))
		{
		CCodeChainCtx CCCtx(GetUniverse());
		CCCtx.DefineContainingType(this);
		CCCtx.SaveAndDefineSourceVar(this);
		CCCtx.DefineSpaceObject(CONSTLIT("aMissionObj"), pMission);
		CCCtx.DefineString(CONSTLIT("aReason"), sReason);

		ICCItemPtr pResult = CCCtx.RunCode(Event);
		if (pResult->IsError())
			ReportEventError(ON_MISSION_COMPLETED_EVENT, pResult);
		}
	}

void CSpaceObject::FireOnObjBlacklistedPlayer (CSpaceObject *pObj)

//	FireOnObjBlacklistedPlayer
//
//	Fire OnObjBlacklistedPlayer event

	{
	SEventHandlerDesc Event;

	if (FindEventHandler(ON_OBJ_BLACKLISTED_PLAYER_EVENT, &Event))
		{
		CCodeChainCtx Ctx(GetUniverse());
		Ctx.DefineContainingType(this);
		Ctx.SaveAndDefineSourceVar(this);
		Ctx.DefineSpaceObject(CONSTLIT("aObj"), pObj);

		ICCItemPtr pResult = Ctx.RunCode(Event);
		if (pResult->IsError())
			ReportEventError(ON_OBJ_BLACKLISTED_PLAYER_EVENT, pResult);
		}
	}

void CSpaceObject::FireOnObjDestroyed (const SDestroyCtx &Ctx)

//	FireOnObjDestroyed
//
//	Fire OnObjDestroyed event

	{
	DEBUG_TRY

	SEventHandlerDesc Event;

	if (FindEventHandler(ON_OBJ_DESTROYED_EVENT, &Event))
		{
		CCodeChainCtx CCCtx(GetUniverse());
		CCCtx.DefineContainingType(this);
		CCCtx.SaveAndDefineSourceVar(this);
		CCCtx.DefineSpaceObject(CONSTLIT("aObjDestroyed"), Ctx.Obj);
		CCCtx.DefineSpaceObject(CONSTLIT("aDestroyer"), Ctx.Attacker.GetObj());
		CCCtx.DefineSpaceObject(CONSTLIT("aOrderGiver"), Ctx.GetOrderGiver());
		CCCtx.DefineSpaceObject(CONSTLIT("aWreckObj"), Ctx.pWreck);
		CCCtx.DefineBool(CONSTLIT("aDestroy"), Ctx.WasDestroyed());
		CCCtx.DefineString(CONSTLIT("aDestroyReason"), GetDestructionName(Ctx.iCause));

		ICCItemPtr pResult = CCCtx.RunCode(Event);
		if (pResult->IsError())
			ReportEventError(ON_OBJ_DESTROYED_EVENT, pResult);
		}

	DEBUG_CATCH
	}

void CSpaceObject::FireOnObjDocked (CSpaceObject *pObj, CSpaceObject *pDockTarget)

//	FireOnObjDocked
//
//	Fire OnObjDocked event

	{
	SEventHandlerDesc Event;

	if (FindEventHandler(ON_OBJ_DOCKED_EVENT, &Event))
		{
		CCodeChainCtx Ctx(GetUniverse());
		Ctx.DefineContainingType(this);
		Ctx.SaveAndDefineSourceVar(this);
		Ctx.DefineSpaceObject(CONSTLIT("aObjDocked"), pObj);
		Ctx.DefineSpaceObject(CONSTLIT("aDockTarget"), pDockTarget);

		ICCItemPtr pResult = Ctx.RunCode(Event);
		if (pResult->IsError())
			ReportEventError(ON_OBJ_DOCKED_EVENT, pResult);
		}
	}

void CSpaceObject::FireOnObjEnteredGate (CSpaceObject *pObj, CTopologyNode *pDestNode, const CString &sDestEntryPoint, CSpaceObject *pStargate)

//	FireOnObjEnteredGate
//
//	Fire OnObjEnteredGate event

	{
	SEventHandlerDesc Event;

	if (FindEventHandler(ON_OBJ_ENTERED_GATE_EVENT, &Event))
		{
		CCodeChainCtx Ctx(GetUniverse());
		Ctx.DefineContainingType(this);
		Ctx.SaveAndDefineSourceVar(this);
		Ctx.DefineSpaceObject(CONSTLIT("aObj"), pObj);
		Ctx.DefineSpaceObject(CONSTLIT("aGateObj"), pStargate);
		Ctx.DefineString(CONSTLIT("aDestNodeID"), (pDestNode ? pDestNode->GetID() : NULL_STR));
		Ctx.DefineString(CONSTLIT("aDestEntryPoint"), sDestEntryPoint);

		ICCItemPtr pResult = Ctx.RunCode(Event);
		if (pResult->IsError())
			ReportEventError(ON_OBJ_ENTERED_GATE_EVENT, pResult);
		}
	}

bool CSpaceObject::FireOnObjGate (CSpaceObject *pObj)

//	FireOnObjGate
//
//	Fire OnObjGate event. Allows us to manipulate the object that gated. We
//	return TRUE if we handled gating. Otherwise, we return FALSE and the default
//	behavior is to destroy the object inside the gate.

	{
	SEventHandlerDesc Event;

	if (FindEventHandler(ON_OBJ_GATE_EVENT, &Event))
		{
		CCodeChainCtx Ctx(GetUniverse());
		Ctx.DefineContainingType(this);
		Ctx.SaveAndDefineSourceVar(this);
		Ctx.DefineSpaceObject(CONSTLIT("aObj"), pObj);

		bool bResult;
		ICCItemPtr pResult = Ctx.RunCode(Event);
		if (pResult->IsError())
			{
			ReportEventError(ON_OBJ_GATE_EVENT, pResult);
			bResult = false;
			}
		else if (pResult->IsNil())
			bResult = false;
		else
			bResult = true;

		return bResult;
		}

	return false;
	}

bool CSpaceObject::FireOnObjGateCheck (CSpaceObject *pObj, CTopologyNode *pDestNode, const CString &sDestEntryPoint, CSpaceObject *pStargate)

//	FireOnObjGateCheck
//
//	Fire OnObjGateCheck event. Returns TRUE if we allow pObj to gate through pStargate.

	{
	SEventHandlerDesc Event;
	if (!FindEventHandler(ON_OBJ_GATE_CHECK_EVENT, &Event))
		return true;

	//	Run

	CCodeChainCtx Ctx(GetUniverse());
	Ctx.DefineContainingType(this);
	Ctx.SaveAndDefineSourceVar(this);
	Ctx.DefineSpaceObject(CONSTLIT("aObj"), pObj);
	Ctx.DefineSpaceObject(CONSTLIT("aGateObj"), pStargate);
	Ctx.DefineString(CONSTLIT("aDestNodeID"), (pDestNode ? pDestNode->GetID() : NULL_STR));
	Ctx.DefineString(CONSTLIT("aDestEntryPoint"), sDestEntryPoint);

	//	Handle result

	ICCItemPtr pResult = Ctx.RunCode(Event);
	if (pResult->IsError())
		{
		ReportEventError(ON_OBJ_ENTERED_GATE_EVENT, pResult);
		return false;
		}
	else if (pResult->IsNil())
		return false;
	else
		return true;
	}

void CSpaceObject::FireOnObjJumped (CSpaceObject *pObj)

//	FireOnObjJumped
//
//	Fire OnObjJumped event

	{
	SEventHandlerDesc Event;

	if (FindEventHandler(ON_OBJ_JUMPED_EVENT, &Event))
		{
		CCodeChainCtx Ctx(GetUniverse());
		Ctx.DefineContainingType(this);
		Ctx.SaveAndDefineSourceVar(this);
		Ctx.DefineSpaceObject(CONSTLIT("aObj"), pObj);

		ICCItemPtr pResult = Ctx.RunCode(Event);
		if (pResult->IsError())
			ReportEventError(ON_OBJ_JUMPED_EVENT, pResult);
		}
	}

bool CSpaceObject::FireOnObjJumpPosAdj (CSpaceObject *pObj, CVector *iovPos)

//	FireOnObjJumpPosAdj
//
//	Fires an event to adjust the position of an object that jumped
//	Returns TRUE if the event adjusted the position

	{
	SEventHandlerDesc Event;

	if (FindEventHandler(ON_OBJ_JUMP_POS_ADJ_EVENT, &Event))
		{
		CCodeChainCtx Ctx(GetUniverse());
		Ctx.DefineContainingType(this);
		Ctx.SaveAndDefineSourceVar(this);
		Ctx.DefineSpaceObject(CONSTLIT("aObj"), pObj);
		Ctx.DefineVector(CONSTLIT("aJumpPos"), *iovPos);

		ICCItemPtr pResult = Ctx.RunCode(Event);

		if (pResult->IsError())
			{
			ReportEventError(ON_OBJ_JUMP_POS_ADJ_EVENT, pResult);
			return false;
			}
		else if (pResult->IsNil())
			{
			return false;
			}
		else
			{
			CVector vNewPos = Ctx.AsVector(pResult);

			if (vNewPos == *iovPos)
				return false;

			*iovPos = vNewPos;
			return true;
			}
		}

	return false;
	}

void CSpaceObject::FireOnObjReconned (CSpaceObject *pObj)

//	FireOnObjReconned
//
//	Fire OnObjReconned event

	{
	SEventHandlerDesc Event;

	if (FindEventHandler(ON_OBJ_RECONNED_EVENT, &Event))
		{
		CCodeChainCtx Ctx(GetUniverse());
		Ctx.DefineContainingType(this);
		Ctx.SaveAndDefineSourceVar(this);
		Ctx.DefineSpaceObject(CONSTLIT("aObj"), pObj);

		ICCItemPtr pResult = Ctx.RunCode(Event);
		if (pResult->IsError())
			ReportEventError(ON_OBJ_RECONNED_EVENT, pResult);
		}
	}

void CSpaceObject::FireOnOrderChanged (void)

//	FireOnOrderChanged
//
//	Fire OnOrderChanged event

	{
	SEventHandlerDesc Event;

	if (FindEventHandler(ON_ORDER_CHANGED_EVENT, &Event))
		{
		CCodeChainCtx Ctx(GetUniverse());
		Ctx.DefineContainingType(this);
		Ctx.SaveAndDefineSourceVar(this);

		ICCItemPtr pResult = Ctx.RunCode(Event);
		if (pResult->IsError())
			ReportEventError(ON_ORDER_CHANGED_EVENT, pResult);
		}
	}

void CSpaceObject::FireOnOrdersCompleted (void)

//	FireOnOrdersCompleted
//
//	Fire OnOrdersCompleted event

	{
	SEventHandlerDesc Event;

	if (FindEventHandler(ON_ORDERS_COMPLETED_EVENT, &Event))
		{
		CCodeChainCtx Ctx(GetUniverse());
		Ctx.DefineContainingType(this);
		Ctx.SaveAndDefineSourceVar(this);

		ICCItemPtr pResult = Ctx.RunCode(Event);
		if (pResult->IsError())
			ReportEventError(ON_ORDERS_COMPLETED_EVENT, pResult);
		}
	}

void CSpaceObject::FireOnPlayerBlacklisted (void)

//	FireOnPlayerBlacklisted
//
//	Fire OnPlayerBlacklisted event

	{
	SEventHandlerDesc Event;

	//	Fire an event for ourselves

	if (FindEventHandler(ON_PLAYER_BLACKLISTED_EVENT, &Event))
		{
		CCodeChainCtx Ctx(GetUniverse());
		Ctx.DefineContainingType(this);
		Ctx.SaveAndDefineSourceVar(this);

		ICCItemPtr pResult = Ctx.RunCode(Event);
		if (pResult->IsError())
			ReportEventError(ON_PLAYER_BLACKLISTED_EVENT, pResult);
		}

	//	Now fire an event for all subscribers

	m_SubscribedObjs.NotifyOnPlayerBlacklisted(this);
	}

void CSpaceObject::FireOnPlayerEnteredShip (CSpaceObject *pOldShip)

//	FireOnPlayerEnteredShip
//
//	Fire OnPlayerEnteredShip event

	{
	SEventHandlerDesc Event;

	//	Fire an event for ourselves

	if (FindEventHandler(ON_PLAYER_ENTERED_SHIP_EVENT, &Event))
		{
		CCodeChainCtx Ctx(GetUniverse());
		Ctx.DefineContainingType(this);
		Ctx.SaveAndDefineSourceVar(this);
		Ctx.DefineSpaceObject(CONSTLIT("aOldShip"), pOldShip);

		ICCItemPtr pResult = Ctx.RunCode(Event);
		if (pResult->IsError())
			ReportEventError(ON_PLAYER_ENTERED_SHIP_EVENT, pResult);
		}
	}

CSpaceObject::InterSystemResults CSpaceObject::FireOnPlayerEnteredSystem (CSpaceObject *pPlayer)

//	FireOnPlayerEnteredSystem
//
//	Fire OnPlayerEnteredSystem event

	{
	SEventHandlerDesc Event;

	if (FindEventHandler(ON_PLAYER_ENTERED_SYSTEM_EVENT, &Event))
		{
		CCodeChainCtx Ctx(GetUniverse());
		Ctx.DefineContainingType(this);
		Ctx.SaveAndDefineSourceVar(this);

		ICCItemPtr pResult = Ctx.RunCode(Event);
		if (pResult->IsError())
			ReportEventError(ON_PLAYER_ENTERED_SYSTEM_EVENT, pResult);

		InterSystemResults iResult = GetInterSystemResult(pResult->GetStringValue());
		return iResult;
		}

	return interNoAction;
	}

void CSpaceObject::FireOnPlayerLeftShip (CSpaceObject *pNewShip)

//  FireOnPlayerLeftShip
//
//	Fire OnPlayerLeftShip event

	{
	SEventHandlerDesc Event;

	//	Fire an event for ourselves

	if (FindEventHandler(ON_PLAYER_LEFT_SHIP_EVENT, &Event))
		{
		CCodeChainCtx Ctx(GetUniverse());
		Ctx.DefineContainingType(this);
		Ctx.SaveAndDefineSourceVar(this);
		Ctx.DefineSpaceObject(CONSTLIT("aNewShip"), pNewShip);

		ICCItemPtr pResult = Ctx.RunCode(Event);
		if (pResult->IsError())
			ReportEventError(ON_PLAYER_LEFT_SHIP_EVENT, pResult);
		}
	}

CSpaceObject::InterSystemResults CSpaceObject::FireOnPlayerLeftSystem (CSpaceObject *pPlayer, CTopologyNode *pDestNode, const CString &sDestEntryPoint, CSpaceObject *pStargate)

//	FireOnPlayerLeftSystem
//
//	Fire OnPlayerLeftSystem event

	{
	SEventHandlerDesc Event;

	if (FindEventHandler(ON_PLAYER_LEFT_SYSTEM_EVENT, &Event))
		{
		CCodeChainCtx Ctx(GetUniverse());
		Ctx.DefineContainingType(this);
		Ctx.SaveAndDefineSourceVar(this);
		Ctx.DefineSpaceObject(CONSTLIT("aGateObj"), pStargate);
		Ctx.DefineString(CONSTLIT("aDestNodeID"), (pDestNode ? pDestNode->GetID() : NULL_STR));
		Ctx.DefineString(CONSTLIT("aDestEntryPoint"), sDestEntryPoint);

		ICCItemPtr pResult = Ctx.RunCode(Event);
		if (pResult->IsError())
			ReportEventError(ON_PLAYER_LEFT_SYSTEM_EVENT, pResult);

		InterSystemResults iResult = GetInterSystemResult(pResult->GetStringValue());
		return iResult;
		}

	return interNoAction;
	}

void CSpaceObject::FireOnSelected (void)

//	FireOnSelected
//
//	Fire OnSelected event

	{
	SEventHandlerDesc Event;

	if (FindEventHandler(ON_SELECTED_EVENT, &Event))
		{
		CCodeChainCtx CCCtx(GetUniverse());
		CCCtx.DefineContainingType(this);
		CCCtx.SaveAndDefineSourceVar(this);
		CCCtx.DefineInteger(CONSTLIT("aPlayer"), g_PlayerSovereignUNID);

		//	Run code

		ICCItemPtr pResult = CCCtx.RunCode(Event);
		if (pResult->IsError())
			ReportEventError(ON_SELECTED_EVENT, pResult);
		}
	}

bool CSpaceObject::FireOnSubordinateAttacked (const SDamageCtx &Ctx)

//	FireOnSubordinateAttacked
//
//	Fire OnSubordinateAttacked event. Return TRUE if we handle it (and we should 
//	skip the default action).

	{
	SEventHandlerDesc Event;

	bool bHandled;
	if (bHandled = (HasOnSubordinateAttackedEvent() && FindEventHandler(ON_SUBORDINATE_ATTACKED_EVENT, &Event)))
		{
		CCodeChainCtx CCCtx(GetUniverse());
		CCCtx.DefineContainingType(this);
		CCCtx.SaveAndDefineSourceVar(this);
		CCCtx.DefineDamageCtx(Ctx);
		CCCtx.DefineSpaceObject(CONSTLIT("aObjAttacked"), Ctx.pObj);

		ICCItemPtr pResult = CCCtx.RunCode(Event);
		if (pResult->IsError())
			ReportEventError(ON_SUBORDINATE_ATTACKED_EVENT, pResult);
		else if (pResult->IsNil())
			bHandled = false;
		}

	return bHandled;
	}

void CSpaceObject::FireOnSystemExplosion (CSpaceObject *pExplosion, CSpaceObject *pSource, DWORD dwItemUNID)

//	FireOnSystemExplosion
//
//	Fire OnSystemExplosion event

	{
	SEventHandlerDesc Event;

	if (FindEventHandler(ON_SYSTEM_EXPLOSION_EVENT, &Event))
		{
		CCodeChainCtx Ctx(GetUniverse());
		Ctx.DefineContainingType(this);
		Ctx.SaveAndDefineSourceVar(this);
		Ctx.DefineSpaceObject(CONSTLIT("aExplosionObj"), pSource);
		Ctx.DefineInteger(CONSTLIT("aExplosionUNID"), dwItemUNID);
		Ctx.DefineVector(CONSTLIT("aExplosionPos"), pExplosion->GetPos());

		ICCItemPtr pResult = Ctx.RunCode(Event);
		if (pResult->IsError())
			ReportEventError(ON_SYSTEM_EXPLOSION_EVENT, pResult);
		}
	}

void CSpaceObject::FireOnSystemObjAttacked (SDamageCtx &Ctx)

//	FireOnSystemObjAttacked
//
//	Fire OnSystemObjAttacked event

	{
	SEventHandlerDesc Event;

	if (FindEventHandler(CDesignType::evtOnSystemObjAttacked, &Event))
		{
		CCodeChainCtx CCCtx(GetUniverse());
		CCCtx.DefineContainingType(this);
		CCCtx.SaveAndDefineSourceVar(this);
		CCCtx.DefineDamageCtx(Ctx);
		CCCtx.DefineSpaceObject(CONSTLIT("aObjAttacked"), Ctx.pObj);

		ICCItemPtr pResult = CCCtx.RunCode(Event);
		if (pResult->IsError())
			ReportEventError(ON_ATTACKED_EVENT, pResult);
		}
	}

void CSpaceObject::FireOnSystemObjCreated (const CSpaceObject &Obj)

//	FireOnSystemObjCreated
//
//	Fire OnSystemObjCreated event

	{
	SEventHandlerDesc Event;

	if (FindEventHandler(ON_SYSTEM_OBJ_CREATED_EVENT, &Event))
		{
		CCodeChainCtx CCX(GetUniverse());
		CCX.DefineContainingType(this);
		CCX.SaveAndDefineSourceVar(this);
		CCX.DefineSpaceObject(CONSTLIT("aObjCreated"), Obj);

		ICCItemPtr pResult = CCX.RunCode(Event);
		if (pResult->IsError())
			ReportEventError(ON_SYSTEM_OBJ_CREATED_EVENT, pResult);
		}
	}

void CSpaceObject::FireOnSystemObjDestroyed (SDestroyCtx &Ctx)

//	FireOnSystemObjDestroyed
//
//	Fire OnSystemObjDestroyed event

	{
	SEventHandlerDesc Event;

	if (FindEventHandler(ON_SYSTEM_OBJ_DESTROYED_EVENT, &Event))
		{
		CCodeChainCtx CCCtx(GetUniverse());
		CCCtx.DefineContainingType(this);
		CCCtx.SaveAndDefineSourceVar(this);
		CCCtx.DefineSpaceObject(CONSTLIT("aObjDestroyed"), Ctx.Obj);
		CCCtx.DefineSpaceObject(CONSTLIT("aDestroyer"), Ctx.Attacker.GetObj());
		CCCtx.DefineSpaceObject(CONSTLIT("aOrderGiver"), Ctx.GetOrderGiver());
		CCCtx.DefineSpaceObject(CONSTLIT("aWreckObj"), Ctx.pWreck);
		CCCtx.DefineBool(CONSTLIT("aDestroy"), Ctx.WasDestroyed());
		CCCtx.DefineString(CONSTLIT("aDestroyReason"), GetDestructionName(Ctx.iCause));

		ICCItemPtr pResult = CCCtx.RunCode(Event);
		if (pResult->IsError())
			ReportEventError(ON_SYSTEM_OBJ_DESTROYED_EVENT, pResult);
		}
	}

void CSpaceObject::FireOnSystemStarted (DWORD dwElapsedTime)

//	FireOnSystemStarted
//
//	Fire OnSystemStarted event

	{
	SEventHandlerDesc Event;
	if (!FindEventHandler(CDesignType::evtOnSystemStarted, &Event))
		return;

	CCodeChainCtx Ctx(GetUniverse());
	Ctx.DefineContainingType(this);
	Ctx.SaveAndDefineSourceVar(this);
	Ctx.DefineInteger(CONSTLIT("aElapsedTime"), dwElapsedTime);

	ICCItemPtr pResult = Ctx.RunCode(Event);
	if (pResult->IsError())
		ReportEventError(ON_SYSTEM_STARTED_EVENT, pResult);
	}

void CSpaceObject::FireOnSystemStopped (void)

//	FireOnSystemStopped
//
//	Fire OnSystemStopped event

	{
	SEventHandlerDesc Event;
	if (!FindEventHandler(CDesignType::evtOnSystemStopped, &Event))
		return;

	CCodeChainCtx Ctx(GetUniverse());
	Ctx.DefineContainingType(this);
	Ctx.SaveAndDefineSourceVar(this);

	ICCItemPtr pResult = Ctx.RunCode(Event);
	if (pResult->IsError())
		ReportEventError(ON_SYSTEM_STOPPED_EVENT, pResult);
	}

void CSpaceObject::FireOnSystemWeaponFire (CSpaceObject *pShot, CSpaceObject *pSource, DWORD dwItemUNID, int iRepeatingCount)

//	FireOnSystemWeaponFire
//
//	Fire OnSystemWeaponFire event

	{
	SEventHandlerDesc Event;

	if (FindEventHandler(CDesignType::evtOnSystemWeaponFire, &Event))
		{
		CCodeChainCtx Ctx(GetUniverse());
		Ctx.DefineContainingType(this);
		Ctx.SaveAndDefineSourceVar(this);
		Ctx.DefineInteger(CONSTLIT("aFireRepeat"), iRepeatingCount);
		Ctx.DefineSpaceObject(CONSTLIT("aShotObj"), pShot);
		Ctx.DefineSpaceObject(CONSTLIT("aWeaponObj"), pSource);
		Ctx.DefineInteger(CONSTLIT("aWeaponUNID"), dwItemUNID);
		Ctx.DefineVector(CONSTLIT("aWeaponPos"), pShot->GetPos());
		Ctx.DefineItemType(CONSTLIT("aWeaponType"), (pShot->GetWeaponFireDesc() ? pShot->GetWeaponFireDesc()->GetWeaponType() : GetUniverse().GetItemType(dwItemUNID)));

		ICCItemPtr pResult = Ctx.RunCode(Event);
		if (pResult->IsError())
			ReportEventError(ON_SYSTEM_WEAPON_FIRE_EVENT, pResult);
		}
	}

bool CSpaceObject::FireOnTranslateMessage (const CString &sMessage, CString *retsMessage)

//	FireOnTranslateMessage
//
//	Fire OnTranslateMessage event

	{
	bool bHandled = false;
	CString sResult;
	SEventHandlerDesc Event;

	if (FindEventHandler(ON_TRANSLATE_MESSAGE_EVENT, &Event))
		{
		CCodeChainCtx Ctx(GetUniverse());
		Ctx.DefineContainingType(this);
		Ctx.SaveAndDefineSourceVar(this);
		Ctx.DefineString(CONSTLIT("aMessage"), sMessage);

		ICCItemPtr pResult = Ctx.RunCode(Event);
		if (!pResult->IsNil())
			{
			bHandled = true;
			sResult = pResult->GetStringValue();
			}
		}

	if (retsMessage)
		*retsMessage = sResult;

	return bHandled;
	}

void CSpaceObject::FireOnUpdate (void)

//	FireOnUpdate
//
//	Fire OnUpdate event

	{
	SEventHandlerDesc Event;

	if (FindEventHandler(CDesignType::evtOnUpdate, &Event))
		{
		CCodeChainCtx Ctx(GetUniverse());
		Ctx.DefineContainingType(this);
		Ctx.SaveAndDefineSourceVar(this);

		ICCItemPtr pResult = Ctx.RunCode(Event);
		if (pResult->IsError())
			ReportEventError(ON_UPDATE_EVENT, pResult);
		}
	}

void CSpaceObject::GetBoundingRect (CVector *retvUR, CVector *retvLL) const

//	GetBoundingRect
//
//	Returns the bounding RECT of the object image centered on the object position

	{
	CVector vDiag(m_rBoundsX, m_rBoundsY);
	*retvUR = m_vPos + vDiag;
	*retvLL = m_vPos - vDiag;
	}

CCommunicationsHandler *CSpaceObject::GetCommsHandler (void)

//	GetCommsHandler
//
//	Returns the comms handler for the object

	{
	CCommunicationsHandler *pHandler;

	if (m_pOverride && (pHandler = m_pOverride->GetCommsHandler()))
		return pHandler;

	CDesignType *pType = GetType();
	if (pType && (pHandler = pType->GetCommsHandler()))
		return pHandler;

	return NULL;
	}

const CCommunicationsHandler *CSpaceObject::GetCommsHandler (void) const

//	GetCommsHandler
//
//	Returns the comms handler for the object

	{
	CCommunicationsHandler *pHandler;

	if (m_pOverride && (pHandler = m_pOverride->GetCommsHandler()))
		return pHandler;

	CDesignType *pType = GetType();
	if (pType && (pHandler = pType->GetCommsHandler()))
		return pHandler;

	return NULL;
	}

int CSpaceObject::GetCommsMessageCount (void)

//	GetCommsMessageCount
//
//	Returns the number of messages that his object understands

	{
	CCommunicationsHandler *pHandler = GetCommsHandler();
	if (pHandler == NULL)
		return 0;
	else
		return pHandler->GetCount();
	}

bool CSpaceObject::GetCondition (ECondition iCondition) const

//	GetCondition
//
//	Returns TRUE if the object is in the given condition.

	{
	//	We handle certain conditions.

	switch (iCondition)
		{
		case ECondition::timeStopped:
			if (m_fTimeStop)
				return true;
			break;
		}

	//	See if an overlay imparts this condition

	const COverlayList *pList = GetOverlays();
	if (pList && pList->GetConditions().IsSet(iCondition))
		return true;

	//	Finally, let the sub-class handle it.

	return OnGetCondition(iCondition);
	}

CConditionSet CSpaceObject::GetConditions (void) const

//	GetConditions
//
//	Returns the set of all conditions.

	{
	CConditionSet Conditions;
	DWORD dwFlag = 1;
	for (int i = 0; i < (int)ECondition::count; i++)
		{
		if (GetCondition((ECondition)dwFlag))
			Conditions.Set((ECondition)dwFlag);

		dwFlag = dwFlag << 1;
		}

	return Conditions;
	}

CDesignCollection &CSpaceObject::GetDesign (void) const

//	GetDesign
//
//	Returns the design collection.

	{
	return GetUniverse().GetDesignCollection();
	}

CString CSpaceObject::GetDesiredCommsKey (void) const

//	GetDesiredCommsKey
//
//	Returns the key that we want to use for comms. If NULL_STR then we will use
//	the default.

	{
	if (m_iDesiredHighlightChar == 0)
		return NULL_STR;

	char chChar = (char)m_iDesiredHighlightChar;
	return CString(&chChar, 1);
	}

Metric CSpaceObject::GetDetectionRange2 (int iPerception) const

//	GetDetectionRange2
//
//	Returns the square of the range at which this object can be detected by
//	the given perception.

	{
	Metric rRange = CPerceptionCalc::GetRange(GetDetectionRangeIndex(iPerception));
	return rRange * rRange;
	}

int CSpaceObject::GetDetectionRangeIndex (int iPerception) const

//	GetDetectionRangeIndex
//
//	Returns the range index at which this object can be detected by
//	the given perception.

	{
	return CPerceptionCalc::GetRangeIndex(GetStealth(), iPerception);
	}

CSovereign::Disposition CSpaceObject::GetDispositionTowards (const CSpaceObject &Obj) const

//	GetDispositionTowards
//
//	Returns the disposition of this objects towards the given object

	{
	const CSovereign *pOurSovereign = GetSovereign();
	if (pOurSovereign)
		return pOurSovereign->GetDispositionTowards(Obj.GetSovereign());
	else
		return CSovereign::dispFriend;
	}

CSovereign::Disposition CSpaceObject::GetDispositionTowards (const CSovereign &Sovereign) const

//	GetDispositionTowards
//
//	Returns the disposition of this objects towards the given object

	{
	const CSovereign *pOurSovereign = GetSovereign();
	if (pOurSovereign)
		return pOurSovereign->GetDispositionTowards(&Sovereign);
	else
		return CSovereign::dispFriend;
	}

CDesignType *CSpaceObject::GetFirstDockScreen (CString *retsScreen, ICCItemPtr *retpData)

//	GetFirstDockScreen
//
//	Returns the dock screen to show when the player docks with
//	this object.
//
//	NOTE: Caller must discard *retpData.

	{
	CDesignType *pLocalScreens = GetType();

	//	First see if any global types override this

	CDockScreenSys::SSelector Screen;
	if (!GetDesign().FireGetGlobalDockScreen(this, 0, &Screen))
		Screen.iPriority = -1;

	//	See if any overlays have dock screens

	COverlayList *pOverlays;
	if (pOverlays = GetOverlays())
		{
		CDockScreenSys::SSelector OverlayScreen;
		CDesignType *pOverlayLocalScreens;
		if (pOverlays->FireGetDockScreen(this, &OverlayScreen, &pOverlayLocalScreens)
				&& OverlayScreen.iPriority > Screen.iPriority)
			{
			Screen = OverlayScreen;
			pLocalScreens = pOverlayLocalScreens;
			}
		}

	//	Next see if we have an event that handles this

	CDockScreenSys::SSelector CustomScreen;
	if (FireGetDockScreen(&CustomScreen)
			&& CustomScreen.iPriority > Screen.iPriority)
		{
		Screen = CustomScreen;
		pLocalScreens = GetType();
		}

	//	If an event has overridden the dock screen, then resolve
	//	the screen now.

	if (Screen.iPriority != -1)
		{
		CDesignType *pScreen = GetDesign().ResolveDockScreen(pLocalScreens, Screen.sScreen, retsScreen);
		if (pScreen)
			{
			if (retpData)
				*retpData = Screen.pData;

			return pScreen;
			}
		else
			{
			::kernelDebugLogPattern("Unable to resolve screen: %s", Screen.sScreen);
			}
		}

	//	Otherwise, we return the default screen associated with the object

	return GetDefaultDockScreen(retsScreen, retpData);
	}

ICCItemPtr CSpaceObject::GetGlobalData (const CString &sAttribute) const

//	GetGlobalData
//
//	Returns data from the type

	{
	CDesignType *pType = GetType();
	if (pType == NULL)
		return ICCItemPtr(ICCItem::Nil);

	return pType->GetGlobalData(sAttribute);
	}

void CSpaceObject::GetHitRect (CVector *retvUR, CVector *retvLL) const

//	GetHitRect
//
//	Returns the RECT that bounds the object's hit size.

	{
	Metric rHalfSize = 0.5 * GetHitSize();
	CVector vDiag(rHalfSize, rHalfSize);
	*retvUR = m_vPos + vDiag;
	*retvLL = m_vPos - vDiag;
	}

int CSpaceObject::GetHitSizePixels (void) const

//	GetHitSizePixels
//
//	Returns the size of the object (in pixels) for purposes of determining the
//	size that can be hit. This is much larger than the real size of the	object
//	(since object images are greatly magnified) but it is less than the	object 
//	bounds, which includes engine effects.

	{
	const CObjectImageArray &Image = GetImage();
	if (Image.IsEmpty())
		return 32;

	const RECT &rcRect = Image.GetImageRect();
	return Max(RectWidth(rcRect), RectHeight(rcRect));
	}

Metric CSpaceObject::GetMaxGateDist2 (void) const

//	GetMaxGateDist2
//
//	Returns the maximum distance at which we can gate through this object.

	{
	Metric rHitSize = GetHitSize();
	return (0.25 * rHitSize * rHitSize);
	}

const CObjectImageArray &CSpaceObject::GetImage (int *retiRotationFrameIndex) const

//	GetImage
//
//	Returns the image for the object

	{
	static CObjectImageArray NullImage;

	if (retiRotationFrameIndex)
		*retiRotationFrameIndex = 0;

	return NullImage;
	}

int CSpaceObject::GetImageScale (void) const

//	GetImageScale
//
//	Returns the scale.

	{
	if (m_iImageScale == -1)
		{
		const CObjectImageArray &Image = GetImage();
		if (!Image.IsEmpty())
			m_iImageScale = Image.GetImageViewportSize();
		else
			{
			//	If there is no image, we return a default value, but we don't
			//	cache the value, in case the image gets set later.

			return 512;
			}
		}

	return m_iImageScale;
	}

int CSpaceObject::GetNearestDockPort (CSpaceObject *pRequestingObj, CVector *retvPort)

//	GetNearestDockPort
//
//	Returns the index of the nearest dock point (or -1)
	
	{
	CDockingPorts *pPorts = GetDockingPorts();
	if (pPorts == NULL)
		return -1;

	int iPort = pPorts->FindNearestEmptyPort(this, pRequestingObj);

	if (retvPort)
		*retvPort = pPorts->GetPortPos(this, iPort, pRequestingObj);

	return iPort;
	}

CSpaceObject *CSpaceObject::GetNearestEnemyStation (Metric rMaxRange)

//	GetNearestEnemyStation
//
//	Returns the nearest enemy station that could threaten the object

	{
	DEBUG_TRY

	int i;

	//	Get the sovereign

	CSovereign *pSovereign = GetSovereign();
	if (pSovereign == NULL || GetSystem() == NULL)
		return NULL;

	//	Get the list of enemy objects

	const CSpaceObjectList &ObjList = pSovereign->GetEnemyObjectList(GetSystem());

	//	Start a max range

	Metric rBestDist = rMaxRange * rMaxRange;
	CSpaceObject *pBestObj = NULL;

	//	Loop for all enemy objects

	int iCount = ObjList.GetCount();
	for (i = 0; i < iCount; i++)
		{
		CSpaceObject *pObj = ObjList.GetObj(i);

		if ((pObj->GetCategory() == catStation)
				&& pObj->CanAttack()
				&& pObj != this)
			{
			CVector vDist = GetPos() - pObj->GetPos();
			Metric rDist = vDist.Length2();

			if (rDist < rBestDist)
				{
				rBestDist = rDist;
				pBestObj = pObj;
				}
			}
		}

	return pBestObj;

	DEBUG_CATCH
	}

CSpaceObject *CSpaceObject::GetNearestStargate (bool bExcludeUncharted)

//	GetNearestStargate
//
//	Returns the nearest stargate

	{
	int i;
	Metric rNearestGateDist2 = (g_InfiniteDistance * g_InfiniteDistance);
	CSpaceObject *pNearestGate = NULL;
	for (i = 0; i < GetSystem()->GetObjectCount(); i++)
		{
		CSpaceObject *pObj = GetSystem()->GetObject(i);

		if (pObj 
				&& pObj->IsActiveStargate()
				//	Do not include uncharted stargates (such as Huaramarca gate)
				&& (!bExcludeUncharted || !pObj->HasAttribute(STR_UNCHARTED)))
			{
			Metric rDist2 = (GetPos() - pObj->GetPos()).Length2();
			if (rDist2 < rNearestGateDist2)
				{
				rNearestGateDist2 = rDist2;
				pNearestGate = pObj;
				}
			}
		}

	return pNearestGate;
	}

CSpaceObject *CSpaceObject::GetNearestVisibleEnemy (Metric rMaxRange, bool bIncludeStations, CSpaceObject *pExcludeObj)

//	GetNearestVisibleEnemy
//
//	Returns the nearest enemy that is visible to the given center point.

	{
	CSystem *pSystem = GetSystem();
	if (pSystem == NULL)
		return NULL;

	CVisibleObjSelector Selector(*this);
	Selector.SetIncludeStations(bIncludeStations);
	Selector.SetExcludeObj(pExcludeObj);

	CNearestInRadiusRange Range(GetPos(), rMaxRange);

	return CSpaceObjectEnum::FindNearestEnemyObj(*pSystem, *this, Range, Selector);
	}

CSpaceObject *CSpaceObject::GetNearestVisibleEnemyInArc (int iMinFireArc, int iMaxFireArc, Metric rMaxRange, bool bIncludeStations, CSpaceObject *pExcludeObj)

//	GetNearestVisibleEnemy
//
//	Returns the nearest enemy that is visible to us and within the given fire
//	arc.

	{
	CSystem *pSystem = GetSystem();
	if (pSystem == NULL)
		return NULL;

	CVisibleObjSelector Selector(*this);
	Selector.SetIncludeStations(bIncludeStations);
	Selector.SetExcludeObj(pExcludeObj);

	CNearestInArcAndRadiusRange Range(GetPos(), rMaxRange, iMinFireArc, iMaxFireArc);

	return CSpaceObjectEnum::FindNearestEnemyObj(*pSystem, *this, Range, Selector);
	}

CString CSpaceObject::GetNounPhrase (DWORD dwFlags) const

//	GetNounPhrase
//
//	Get the name of the object as a noun phrase modified by the
//	given flags

	{
	//	Get the name and modifiers from the actual object

	DWORD dwNounFlags;
	CString sName = GetNamePattern(dwFlags, &dwNounFlags);

	return CLanguage::ComposeNounPhrase(sName, 1, CString(), dwNounFlags, dwFlags);
	}

CSpaceObject *CSpaceObject::GetOrderGiver (DestructionTypes iCause)

//	GetOrderGiver
//
//	Returns the object that is responsible for this object's attack

	{
	if (iCause == killedByPlayerCreatedExplosion)
		{
		CSpaceObject *pPlayerShip = GetPlayerShip();
		if (pPlayerShip)
			return pPlayerShip;
		else
			return this;
		}
	else
		return OnGetOrderGiver();
	}

ICCItemPtr CSpaceObject::GetOverlayData (DWORD dwID, const CString &sAttrib) const

//	GetOverlayData
//
//	Returns data for overlay

	{
	const COverlayList *pOverlays = GetOverlays();
	if (pOverlays == NULL)
		return ICCItemPtr(GetUniverse().GetCC().CreateNil());

	return pOverlays->GetData(dwID, sAttrib);
	}

ICCItem *CSpaceObject::GetOverlayProperty (CCodeChainCtx *pCCCtx, DWORD dwID, const CString &sName)

//	GetOverlayProperty
//
//	Returns a property

	{
	COverlayList *pOverlays = GetOverlays();
	if (pOverlays)
		return pOverlays->GetProperty(pCCCtx, this, dwID, sName);
	else
		{
		CCodeChain &CC = GetUniverse().GetCC();
		return CC.CreateNil();
		}
	}

bool CSpaceObject::GetPlayerDestinationOptions (SPlayerDestinationOptions &retOptions) const

//	GetPlayerDestinationOptions
//
//	If this option is a player destination, we return TRUE and fill in 
//	retOptions appropriately. Otherwise, we return FALSE.

	{
	if (!m_fPlayerDestination)
		return false;

	retOptions.bShowDistanceAndBearing = (m_fShowDistanceAndBearing ? true : false);
	retOptions.bAutoClearDestination = (m_fAutoClearDestination ? true : false);
	retOptions.bAutoClearOnDestroy = (m_fAutoClearDestinationOnDestroy ? true : false);
	retOptions.bAutoClearOnDock = (m_fAutoClearDestinationOnDock ? true : false);
	retOptions.bAutoClearOnGate = (m_fAutoClearDestinationOnGate ? true : false);
	retOptions.bShowHighlight = (m_fShowHighlight ? true : false);

	return true;
	}

CXMLElement *CSpaceObject::GetScreen (const CString &sName)

//	GetScreen
//
//	Returns a screen object

	{
	CDockScreenTypeRef Screen;
	Screen.LoadUNID(sName);
	Screen.Bind(NULL);
	return Screen.GetDesc();
	}

int CSpaceObject::GetShieldLevel (void) const

//	GetShieldLevel
//
//	Returns the % shield level of the ship (or -1 if the ship has no shields)

	{
	const CInstalledDevice *pShields = GetNamedDevice(devShields);
	if (pShields == NULL)
		return -1;

	return pShields->GetHitPointsPercent(this);
	}

CSovereign *CSpaceObject::GetSovereignToDefend (void) const

//	GetSovereignToDefend
//
//	Returns either our sovereign or the sovereign of the ship that we're
//	escorting (if we're escorting)

	{
	CSpaceObject *pPrincipal = GetEscortPrincipal();
	if (pPrincipal)
		return pPrincipal->GetSovereign();
	else
		return GetSovereign();
	}

#define SO_ATTACK_IN_FORMATION				CONSTLIT("Attack in formation")
#define SO_BREAK_AND_ATTACK					CONSTLIT("Break & attack")
#define SO_FORM_UP							CONSTLIT("Form up")
#define SO_ATTACK_TARGET					CONSTLIT("Attack target")
#define SO_WAIT								CONSTLIT("Wait")
#define SO_CANCEL_ATTACK					CONSTLIT("Cancel attack")
#define SO_ALPHA_FORMATION					CONSTLIT("Alpha formation")
#define SO_BETA_FORMATION					CONSTLIT("Beta formation")
#define SO_GAMMA_FORMATION					CONSTLIT("Gamma formation")

DWORD CSpaceObject::GetSquadronCommsStatus () const

//	GetSquadronCommsStatus
//
//	Returns the set of messages accepted by this ship's squadron.

	{
	CSystem *pSystem = GetSystem();
	if (!pSystem)
		return 0;

	DWORD dwStatus = 0;

	for (int i = 0; i < pSystem->GetObjectCount(); i++)
		{
		CSpaceObject *pObj = pSystem->GetObject(i);

		if (pObj && IsInOurSquadron(*pObj))
			{
			//	First add messages from the old-style "fleet" controller.

			dwStatus |= Communicate(pObj, msgQueryCommunications);

			//	Next add in messages accepted through the communications struct

			int iIndex;
			if ((iIndex = pObj->FindCommsMessageByName(SO_ATTACK_IN_FORMATION)) != -1
					&& pObj->IsCommsMessageValidFrom(*this, iIndex))
				dwStatus |= resCanAttackInFormation;

			if ((iIndex = pObj->FindCommsMessageByName(SO_BREAK_AND_ATTACK)) != -1
					&& pObj->IsCommsMessageValidFrom(*this, iIndex))
				dwStatus |= resCanBreakAndAttack;

			if ((iIndex = pObj->FindCommsMessageByName(SO_FORM_UP)) != -1
					&& pObj->IsCommsMessageValidFrom(*this, iIndex))
				dwStatus |= resCanFormUp;

			if ((iIndex = pObj->FindCommsMessageByName(SO_ATTACK_TARGET)) != -1
					&& pObj->IsCommsMessageValidFrom(*this, iIndex))
				dwStatus |= resCanAttack;

			if ((iIndex = pObj->FindCommsMessageByName(SO_WAIT)) != -1
					&& pObj->IsCommsMessageValidFrom(*this, iIndex))
				dwStatus |= resCanWait;

			if ((iIndex = pObj->FindCommsMessageByName(SO_CANCEL_ATTACK)) != -1
					&& pObj->IsCommsMessageValidFrom(*this, iIndex))
				dwStatus |= resCanAbortAttack;

			if ((iIndex = pObj->FindCommsMessageByName(SO_ALPHA_FORMATION)) != -1
					&& pObj->IsCommsMessageValidFrom(*this, iIndex))
				dwStatus |= resCanBeInFormation;
			}
		}

	return dwStatus;
	}

ICCItemPtr CSpaceObject::GetStaticData (const CString &sAttrib)

//	GetStaticData
//
//	Returns static data

	{
	ICCItemPtr pData;

	//	Check our override

	if (m_pOverride && m_pOverride->FindStaticData(sAttrib, pData))
		return pData;

	//	Check our type

	CDesignType *pType = GetType();
	if (pType)
		return pType->GetStaticData(sAttrib);
	
	//	Not found

	return ICCItemPtr(GetUniverse().GetCC().CreateNil());
	}

CG32bitPixel CSpaceObject::GetSymbolColor (void) const

//	GetSymbolColor
//
//	Returns the color to paint this object in the player's scanner

	{
	CAccessibilitySettings cAccessibilitySettings = GetUniverse().GetAccessibilitySettings();
	CSovereign *pPlayer = GetUniverse().GetPlayerSovereign();
	CSovereign *pSovereign = GetSovereign();
	CSpaceObject *pPlayerShip;
	CG32bitPixel rgbColor;

	//	Player & player's assets

	if (pSovereign 
			&& ((pSovereign == pPlayer) || pSovereign->IsPlayerOwned()))
		rgbColor = cAccessibilitySettings.GetIFFColor(CAccessibilitySettings::IFFType::player);

	//	Angered ships

	else if ((pPlayerShip = GetUniverse().GetPlayerShip()) 
			&& IsAngryAt(pPlayerShip) && (IsFriend(*pPlayer) || IsNeutral(*pPlayer)))
		rgbColor = cAccessibilitySettings.GetIFFColor(CAccessibilitySettings::IFFType::angry);

	//	Assigned escorts (ex, fleet wingmates)

	else if ((pPlayerShip = GetUniverse().GetPlayerShip()) && pPlayer && IsEscorting(pPlayerShip))
		rgbColor = cAccessibilitySettings.GetIFFColor(CAccessibilitySettings::IFFType::escort);

	//	Enemies

	else if (pPlayer && IsEnemy(*pPlayer))
		rgbColor = cAccessibilitySettings.GetIFFColor(CAccessibilitySettings::IFFType::enemy);

	//	Friendlies

	else if (pPlayer && IsFriend(*pPlayer))
		rgbColor = cAccessibilitySettings.GetIFFColor(CAccessibilitySettings::IFFType::friendly);

	//	Neutrals (Anger more easily)

	else if (pPlayer && IsNeutral(*pPlayer))
		rgbColor = cAccessibilitySettings.GetIFFColor(CAccessibilitySettings::IFFType::neutral);

	//	Fallback (magenta indicates error/uncategorized ship)

	else if (GetCategory() == CSpaceObject::catShip)
		rgbColor = CG32bitPixel(255, 80, 255);
	else
		rgbColor = CG32bitPixel(255, 80, 255);

	//	Dim the color if it is a wreck

	if (IsWreck() || IsAbandoned())
		rgbColor = CG32bitPixel::Blend(0, rgbColor, (BYTE)192);

	return rgbColor;
	}

const CEnhancementDesc *CSpaceObject::GetSystemEnhancements (void) const

//	GetSystemEnhancements
//
//	Returns the list of system enhancements (or NULL if none).

	{
	CSystem *pSystem = GetSystem();
	if (pSystem == NULL)
		return NULL;

	CSystemType *pType = pSystem->GetType();
	if (pType == NULL)
		return NULL;

	const CEnhancementDesc &Enhancements = pType->GetItemEnhancements();
	if (Enhancements.IsEmpty())
		return NULL;

	return &Enhancements;
	}

const CImageFilterStack *CSpaceObject::GetSystemFilters (void) const

//	GetSystemFilters
//
//	Returns the paint filters to apply to this object.

	{
	CSystem *pSystem = GetSystem();
	if (pSystem == NULL)
		return NULL;

	CSystemType *pType = pSystem->GetType();
	if (pType == NULL)
		return NULL;

	if (!pType->GetImageFiltersCriteria().IsEmpty()
			&& !MatchesCriteria(pType->GetImageFiltersCriteria()))
		return NULL;

	const CImageFilterStack *pFilters = &pType->GetImageFilters();
	if (pFilters->IsEmpty())
		return NULL;

	return pFilters;
	}

void CSpaceObject::GetVisibleEnemies (DWORD dwFlags, TArray<CSpaceObject *> *retList, CSpaceObject *pExcludeObj)

//	GetVisibleEnemies
//
//	Returns a list of visible enemies.

	{
	int i;

	CVector vCenter = GetPos();

	//	Get the sovereign

	CSovereign *pSovereign = GetSovereignToDefend();
	if (pSovereign == NULL || GetSystem() == NULL)
		return;

	//	Include stations

	bool bIncludeStations = ((dwFlags & CTargetList::FLAG_INCLUDE_STATIONS) ? true : false);

	//	If a ship has fired its weapon after this time, then it counts
	//	as an aggressor

	int iAggressorThreshold;
	if (dwFlags & CTargetList::FLAG_INCLUDE_NON_AGGRESSORS)
		iAggressorThreshold = -1;
	else
		iAggressorThreshold = GetUniverse().GetTicks() - AGGRESSOR_THRESHOLD;

	//	Get the list of enemy objects

	const CSpaceObjectList &ObjList = pSovereign->GetEnemyObjectList(GetSystem());

	//	Compute this object's perception and perception range

	CPerceptionCalc Perception(GetPerception());

	//	Loop over all objects

	int iObjCount = ObjList.GetCount();
	for (i = 0; i < iObjCount; i++)
		{
		CSpaceObject *pObj = ObjList.GetObj(i);

		if ((pObj->GetCategory() == catShip
					|| (bIncludeStations && pObj->GetCategory() == catStation))
				&& pObj->CanAttack()
				&& !pObj->IsDestroyed()
				&& pObj != this)
			{
			CVector vDist = vCenter - pObj->GetPos();
			Metric rDist2 = vDist.Length2();

			if (Perception.CanBeTargeted(pObj, rDist2)
					&& pObj != pExcludeObj
					&& pObj->GetLastFireTime() > iAggressorThreshold
					&& !pObj->IsEscortingFriendOf(this))
				{
				retList->Insert(pObj);
				}
			}
		}
	}

CSpaceObject *CSpaceObject::GetVisibleEnemyInRange (CSpaceObject *pCenter, Metric rMaxRange, bool bIncludeStations, CSpaceObject *pExcludeObj)

//	GetVisibleEnemyInRange
//
//	Returns the first enemy that we find in range.

	{
	DEBUG_TRY_OBJ_LOOP

	Metric rMaxRange2 = rMaxRange * rMaxRange;

	//	Compute this object's perception and perception range

	CPerceptionCalc Perception(GetPerception());

	//	The player is a special case (because sometimes a station is angry at the 
	//	player even though she is not an enemy)

	CSpaceObject *pPlayer = GetPlayerShip();
	if (pPlayer 
			&& pCenter->IsAngryAt(pPlayer)
			&& pPlayer != pExcludeObj
			&& !pPlayer->IsEscortingFriendOf(this))
		{
		CVector vRange = pPlayer->GetPos() - pCenter->GetPos();
		Metric rDistance2 = vRange.Dot(vRange);

		if (rDistance2 < rMaxRange2
				&& Perception.CanBeTargeted(pPlayer, rDistance2))
			return pPlayer;
		}

	//	Get the sovereign

	CSovereign *pSovereign = GetSovereignToDefend();
	if (pSovereign == NULL || GetSystem() == NULL)
		return NULL;

	//	Loop

	const CSpaceObjectList &ObjList = pSovereign->GetEnemyObjectList(GetSystem());
	int iCount = ObjList.GetCount();
	for (int i = 0; i < iCount; i++)
		{
		pObj = ObjList.GetObj(i);

		if ((pObj->GetCategory() == catShip
					|| (bIncludeStations && pObj->GetCategory() == catStation))
				&& pObj->CanAttack()
				&& pObj != this)
			{
			CVector vRange = pObj->GetPos() - pCenter->GetPos();
			Metric rDistance2 = vRange.Dot(vRange);

			if (rDistance2 < rMaxRange2
					&& Perception.CanBeTargeted(pObj, rDistance2)
					&& pObj != pExcludeObj
					&& !pObj->IsEscortingFriendOf(this))
				return pObj;
			}
		}

	return NULL;

	DEBUG_CATCH_OBJ_LOOP
	}

bool CSpaceObject::HasBeenHitLately (int iTicks)

//	HasBeenHitLately
//
//	Returns TRUE if we've been hit in the last iTicks.

	{
	int iLastHit = GetLastHitTime();
	if (iLastHit == 0)
		return false;

	int iNow = GetUniverse().GetTicks();
	if ((iNow - iLastHit) <= iTicks)
		return true;

	return false;
	}

bool CSpaceObject::HasDockScreen (void) const

//	HasDockScreen
//
//	Returns TRUE if we have at least one dock screen. This should match ::GetFirstDockScreen()

	{
	if (GetDefaultDockScreen())
		return true;

	if (FireGetDockScreen())
		return true;

	const COverlayList *pOverlays;
	if ((pOverlays = GetOverlays()) 
			&& pOverlays->FireGetDockScreen(this))
		return true;

	//	If we don't have any docking screens so far (not even from overlays) and
	//	if we don't have any docking ports, then don't bother calling
	//	<GetGlobalDockScreen>. The performance hit of calling these global 
	//	events at create time (e.g., on asteroids) is too much.

	if (const CDockingPorts *pDockingPorts = GetDockingPorts())
		{
		if (pDockingPorts->GetPortCount() == 0)
			return false;
		}

	//	If we still have no screens, we call <GetGlobalDockScreen>, but we're
	//	only interested in non-override screens. Override screens are screens
	//	like decontamination screens, which should only show up if the station
	//	or ship has other screens.
	//
	//	If we don't do this, then some ships would automatically get screens
	//	because we auto-create docking ports for ships if they have screens.

	if (GetDesign().FireGetGlobalDockScreen(this, CDesignCollection::FLAG_NO_OVERRIDE))
		return true;

	return false;
	}

bool CSpaceObject::HasFiredLately (int iTicks)

//	HasFiredHitLately
//
//	Returns TRUE if we've fired our weapons lately.

	{
	int iLastFire = GetLastFireTime();
	if (iLastFire == 0)
		return false;

	int iNow = GetUniverse().GetTicks();
	if ((iNow - iLastFire) <= iTicks)
		return true;

	return false;
	}

bool CSpaceObject::HasFuelItem (void)

//	HasFuelItem
//
//	Returns TRUE if the object has fuel on board

	{
	CItemListManipulator Search(GetItemList());
	while (Search.MoveCursorForward())
		{
		const CItem &Item = Search.GetItemAtCursor();
		if (Item.GetType()->IsFuel())
			return true;
		}

	return false;
	}

bool CSpaceObject::HasMinableItem (void) const

//	HasMinableItem
//
//	Returns TRUE if the object has any minable items.

	{
	CItemListManipulator Search(const_cast<CSpaceObject *>(this)->GetItemList());
	while (Search.MoveCursorForward())
		{
		const CItem &Item = Search.GetItemAtCursor();
		if (Item.HasAttribute(CONSTLIT("ore"))
				|| Item.HasAttribute(CONSTLIT("minable")))
			return true;
		}

	if (const COverlayList *pOverlays = GetOverlays())
		{
		if (pOverlays->HasMinableItem())
			return true;
		}

	return false;
	}

bool CSpaceObject::HasSpecialAttribute (const CString &sAttrib) const

//	HasSpecialAttribute
//
//	Returns TRUE if object has the special attribute
//
//	NOTE: Subclasses may override this, but they must call the
//	base class if they do not handle the attribute.

	{
	if (strStartsWith(sAttrib, SPECIAL_CHARACTER))
		{
		CString sCharacter = strSubString(sAttrib, SPECIAL_CHARACTER.GetLength());
		DWORD dwUNID = (DWORD)strToInt(sCharacter, 0);
		if (dwUNID == 0)
			return false;

		CDesignType *pCharacter = GetCharacter();
		if (pCharacter == NULL)
			return false;

		return (dwUNID == pCharacter->GetUNID());
		}
	else if (strStartsWith(sAttrib, SPECIAL_DATA))
		{
		CString sDataField = strSubString(sAttrib, SPECIAL_DATA.GetLength());
		return !m_Data.IsDataNil(sDataField);
		}
	else if (strStartsWith(sAttrib, SPECIAL_IS_PLANET))
		{
		CString sValue = strSubString(sAttrib, SPECIAL_IS_PLANET.GetLength());

		//	Figure out if we are a planet or not.

		const COrbit *pOrbit;
		bool bIsPlanet = (HasAttribute(CONSTLIT("planet"))
				||  (GetScale() == scaleWorld
					&& (GetPlanetarySize() >= MIN_PLANET_SIZE)
					&& (pOrbit = GetMapOrbit())
					&& (pOrbit->GetFocus().IsNull() || GetSystem()->IsStarAtPos(pOrbit->GetFocus()))));

		//	Check value

		return (bIsPlanet == strEquals(sValue, SPECIAL_VALUE_TRUE));
		}
	else if (strStartsWith(sAttrib, SPECIAL_LOCATION))
		{
		CString sLocationAttrib = strSubString(sAttrib, SPECIAL_LOCATION.GetLength());
		CSystem *pSystem = GetSystem();
		if (pSystem == NULL)
			return false;

		//	See if we have this as a system attribute or a territory attribute.

		if (::HasModifier(pSystem->GetAttribsAtPos(GetPos()), sLocationAttrib))
			return true;

		//	See if we have this as a location

		const CLocationDef *pLoc = pSystem->GetLocations().GetLocationByObjID(GetID());
		if (pLoc && ::HasModifier(pLoc->GetAttributes(), sLocationAttrib))
			return true;

		//	Otherwise, we don't have this location attribute.

		return false;
		}
	else if (strStartsWith(sAttrib, SPECIAL_PROPERTY))
		{
		CString sProperty = strSubString(sAttrib, SPECIAL_PROPERTY.GetLength());

		CString sError;
		CPropertyCompare Compare;
		CCodeChainCtx CCX(GetUniverse());
		if (!Compare.Parse(CCX, sProperty, &sError))
			{
			::kernelDebugLogPattern("ERROR: Unable to parse property expression: %s", sError);
			return false;
			}

		ICCItemPtr pValue = GetProperty(CCX, Compare.GetProperty());
		return Compare.Eval(pValue);
		}
	else if (strStartsWith(sAttrib, SPECIAL_SERVICE))
		{
		CString sValue = strSubString(sAttrib, SPECIAL_SERVICE.GetLength());
		ETradeServiceTypes iService = CTradingDesc::ParseService(sValue);
		if (iService == serviceNone)
			return false;

		CTradingServices Services(*this);
		return Services.HasService(iService);
		}
	else
		{
		CDesignType *pType = GetType();
		if (pType == NULL)
			return false;

		return pType->HasSpecialAttribute(sAttrib);
		}
	}

void CSpaceObject::Highlight (const CString &sText)

//	Highlight
//
//	Highlight object.
	
	{
	m_sHighlightText = sText;
	m_iHighlightCountdown = HIGHLIGHT_TIMER;
	}

void CSpaceObject::HighlightAppend (const CString &sText)

//	HighlightAppend
//
//	Appends highlight.

	{
	if (m_sHighlightText.IsBlank())
		m_sHighlightText = sText;
	else
		m_sHighlightText = strPatternSubst(CONSTLIT("%s\n%s"), m_sHighlightText, sText);

	m_iHighlightCountdown = HIGHLIGHT_TIMER;
	}

CSpaceObject *CSpaceObject::HitTest (const CVector &vStart, 
									 const DamageDesc &Damage, 
									 CVector *retvHitPos, 
									 int *retiHitDir)

//	HitTest
//
//	Returns the object that the beam hit or NULL if no object was hit.
//	If rThreshold > 0 and the object passes within the threshold distance
//	to some target, then retiHitDir = -1 and retvHitPos is the nearest point.

	{
	DEBUG_TRY_OBJ_LOOP

	//	Get the list of objects that intersect the object

	SSpaceObjectGridEnumerator i;
	GetSystem()->EnumObjectsInBoxStart(i, GetPos(), g_SecondsPerUpdate * LIGHT_SECOND);

	//	We need some variables for stepping

	int iSteps = 0;
	CVector vStep;

	//	Remember what we hit

	CSpaceObject *pHit = NULL;
	CVector vHitPos;

	//	See if the beam hit anything. We start with a crude first pass.
	//	Any objects near the beam are then analyzed further to see if
	//	the beam hit them.
	//
	//	LATER: Note that this is an NxM loop, where N is the number of objects
	//	in an area and M is the number of steps to increment so we can find the
	//	precise point of intersection.
	//
	//	We should probably loop over steps first and then for each step loop 
	//	over objects, because that should guarantee the smallest number of
	//	loops. For example, assume that the object hit is the Ith object on
	//	the Jth step. In that case we would do a maximum of JxN loops.
	//
	//	In contrast, looping over objects first means we do IxM + (N-I)xJ
	//	loops. If on average I is N/2 and J is M/2, then we end up doing
	//	MxN/2 + MxN/4 or 3xMxN/4 total loops for the current code versus
	//	MxN/2 total loops for the optimal case.
	//
	//	Of course, this doesn't matter if M and N are small (as they should be).

	int k;
	while (GetSystem()->EnumObjectsInBoxHasMore(i))
		{
		pObj = GetSystem()->EnumObjectsInBoxGetNext(i);

		//	Skip objects that we cannot hit

		if (!CanHit(pObj)
				|| !pObj->CanBeHitBy(Damage)
				|| pObj == this)
			continue;

		//	Step towards this object and see if we hit it. Start by computing 
		//	the step vector, which should be 2 pixels long.

		if (iSteps == 0)
			{
			CVector vMissileTravel = (GetPos() - vStart);
			Metric rMissileTravel = vMissileTravel.Length();
			iSteps = (int)(rMissileTravel / (2.0 * g_KlicksPerPixel)) + 1;
			vStep = vMissileTravel / iSteps;
			}

		//	Prepare for point in object calculations

		SPointInObjectCtx PiOCtx;
		pObj->PointInObjectInit(PiOCtx);

		//	Step

		CVector vTest = vStart;
		for (k = 0; k < iSteps; k++)
			{
			//	If we hit this object then keep track.

			if (pObj->PointInObject(PiOCtx, pObj->GetPos(), vTest))
				{
				//	If we hit this on the first step, then we're done

				if (k == 0)
					{
					if (retvHitPos)
						*retvHitPos = vTest;

					//	Figure out the direction that the hit came from

					if (retiHitDir)
						*retiHitDir = VectorToPolar(-vStep, NULL);

					return pObj;
					}

				//	Otherwise, we remember it and keep looping in case there
				//	is another closer object.

				else
					{
					pHit = pObj;
					vHitPos = vTest;

					//	No need to look at steps beyond this one.

					iSteps = k;
					}
				}

			//	Next

			vTest = vTest + vStep;
			}
		}

	pObj = NULL;

	//	See if we hit anything

	if (pHit)
		{
		if (retvHitPos)
			*retvHitPos = vHitPos;

		if (retiHitDir)
			*retiHitDir = VectorToPolar(-vStep, NULL);

		return pHit;
		}
	else
		return NULL;

	DEBUG_CATCH_OBJ_LOOP
	}

CSpaceObject* CSpaceObject::HitTestProximity(
		const CVector& vStart,
		const CWeaponFireDesc* pDesc,
		const CTargetList::STargetOptions& TargetOptions,
		const CSpaceObject* pTarget,
		CVector* retvHitPos,
		int* retiHitDir)

//	HitTest (API 54+)
//
//	Returns the object that the beam hit or NULL if no object was hit.
//	retiHitDir is -1 for all hit types except direct hits

	{
	DEBUG_TRY_OBJ_LOOP

	CUsePerformanceCounter Counter(GetUniverse(), CONSTLIT("update.hitTestProximity"));

	const Metric OBJ_RADIUS_ADJ_MIN = 0.25;
	const Metric OBJ_RADIUS_ADJ_MAX = 1.0;
	const Metric IMPACT_SCAN_SECONDS = 2.0 * g_SecondsPerUpdate; //universe seconds (half tick), not real seconds
	const Metric IMPACT_SECONDS = g_SecondsPerUpdate; //universe seconds, not real seconds

	//	Get our proximity detection settings
	//	Note: (the source proximity failsafe option should be handled prior to calling HitTestProximity)

	bool bIsStatic = GetVel().IsNull();

	Metric rSensorRange = max(pDesc->GetFragDistanceArmed(), pDesc->GetFragmentationMaxThreshold()); //ok this DOES seem to detect handle intersecting big objects
	Metric rDetonatorAutoActivationRange = max(pDesc->GetFragDistanceAutoTrigger(), pDesc->GetFragmentationMinThreshold());
	Metric rDetonatorFailRange = pDesc->GetFragDistanceFail();
	Metric rDetonatorImpactActivationRange = pDesc->GetFragDistanceImpactTarget();
	Metric rDetonatorAutoActivationRange2 = rDetonatorAutoActivationRange * rDetonatorAutoActivationRange;
	Metric rDetonatorFailRange2 = rDetonatorFailRange * rDetonatorFailRange;
	int iSensorAngle = (int)pDesc->GetFragSensorArc();
	int iFacingDir = GetRotation();

	//	Update sensor range if we have a larger impact activation range and are not static
	
	Metric rMaxSensorRange = bIsStatic ? rSensorRange : max(rSensorRange, rDetonatorImpactActivationRange);
	Metric rSensorRange2 = rSensorRange * rSensorRange;

	//	Get the list of objects that intersect the object
	//	Technically GetPos() is ahead of where we were last painted

	SSpaceObjectGridEnumerator i;
	GetSystem()->EnumObjectsInBoxStart(i, GetPos(), max(rMaxSensorRange, IMPACT_SCAN_SECONDS * LIGHT_SECOND));

	//	We need some variables for stepping

	int iSteps = 0;
	int iScanSteps = 0;
	CVector vStep;
	CVector vMissileTravel = bIsStatic ? GetVel() : (GetVel() * g_SecondsPerUpdate);
	Metric rMissileTravel = bIsStatic ? 0.0 : vMissileTravel.Length();
	CVector vEnd = bIsStatic ? vStart : (vStart + vMissileTravel);

	//	We need some variables to track the closest object

	Metric rClosestApproach2 = rSensorRange * rSensorRange;	//	We track the square of the closest approach to avoid doing a lot of expensive sqrt ops
	CVector vClosestPos;
	CSpaceObject *pClosestHit = NULL;
	CVector vCurrentClosestPos;
	int iCurrentClosestPosAngle = 0;

	//	See if the beam hit anything. We start with a crude first pass.
	//	Any objects near the beam are then analyzed further to see if
	//	the beam hit them.

	CVector vClosestFinalPos;
	while (GetSystem()->EnumObjectsInBoxHasMore(i))
		{
		pObj = GetSystem()->EnumObjectsInBoxGetNext(i);

		//	Skip objects that we cannot hit

		if (!CanHit(pObj)
			|| !pObj->CanBeHitBy(pDesc->GetDamage())
			|| pObj == this)
			continue;

		//	Skip anything that isnt visible to our sensor

		if (!(pDesc->GetFragSensorArc() == 360))
			{
			int iSensorStart = AngleMod((iSensorAngle / 2) - iFacingDir);
			int iSensorEnd = AngleMod(iSensorStart + iSensorAngle);
			int iObjDir = VectorToPolar(pObj->GetPos());
			int iClosestSensorSide = abs(iObjDir - iSensorStart) < abs(iObjDir - iSensorEnd) ? iSensorStart : iSensorEnd;
			bool bRunBorderlineTest = false;

			//	Check if the object is outside of sensor range

			if (iSensorStart < iSensorEnd && (iObjDir > iSensorEnd || iObjDir < iSensorStart))
					bRunBorderlineTest = true;
			else if (iObjDir < iSensorEnd && iObjDir > iSensorStart)
					bRunBorderlineTest = true;

			//	Figure out if its just on the border of the scanner's range with a low res hit detection
			//	skip it if it doesnt show up. Only miniscule targets will tend to get missed by this.

			if (bRunBorderlineTest && !IntersectionTestScan(pObj, vStart, PolarToVector(iClosestSensorSide, LIGHT_SECOND / 2), (int)(2 * rSensorRange / LIGHT_SECOND)))
				continue;
			}

		//	Prepare to skip anything that we dont directly impact and cant otherwise detonate on

		bool bCanTriggerDetonation = CTargetList::CanDetonate(*this, pTarget, TargetOptions, *pObj);

		//	Get our adjusted size thresholds for pre-emptive detonation
		//max(OBJ_RADIUS_ADJ_MIN * pClosestHit->GetHitSize(), rMaxThreshold + (OBJ_RADIUS_ADJ_MAX * max(1.0, (pClosestHit->GetHitSize() * g_KlicksPerPixel - rMaxThreshold)) / rMaxThreshold))
		Metric rAutoActivationRangeAdj = rDetonatorAutoActivationRange ? max(
			OBJ_RADIUS_ADJ_MIN * pObj->GetHitSize(),
			rDetonatorAutoActivationRange + (OBJ_RADIUS_ADJ_MAX * max(1.0, (pObj->GetHitSize() * g_KlicksPerPixel - rDetonatorAutoActivationRange)) / rDetonatorAutoActivationRange)) : 0.0;
		Metric rAutoActivationRangeAdj2 = rAutoActivationRangeAdj * rAutoActivationRangeAdj;
		Metric rDetonatorImpactActivationRangeAdj = rDetonatorImpactActivationRange ? max(
			OBJ_RADIUS_ADJ_MIN * pObj->GetHitSize(),
			rDetonatorImpactActivationRange + (OBJ_RADIUS_ADJ_MAX * max(1.0, (pObj->GetHitSize() * g_KlicksPerPixel - rDetonatorImpactActivationRange)) / rDetonatorImpactActivationRange)) : 0.0;

		//	If we are static (non-moving), then check if anything else collides with us or passes close by

		if (bIsStatic)
			{
			//	If the object is also static, we just do a simple check

			if (pObj->GetVel().IsNull())
				{

				//	Check if we are within auto activation range
				//	This is cheaper so we check this before actually checking collision

				Metric rObjDistance2 = (pObj->GetPos() - vStart).Length2();

				if (rObjDistance2 < rAutoActivationRangeAdj2)
					{
					//	We detonate

					if (retvHitPos)
						*retvHitPos = vStart;

					if (retiHitDir)
						*retiHitDir = -1;

					return pObj;
					}

				//	Check if we are already collided (ex, someone dropped a mine on top of a station)

				if (pTarget->PointInBounds(GetPos()))
					{
					SPointInObjectCtx PiOCtx;
					pTarget->PointInObjectInit(PiOCtx);

					if (pObj->PointInObject(PiOCtx, pObj->GetPos(), vStart))
						{
						//	We are already collided

						if (retvHitPos)
							*retvHitPos = vStart;

						if (retiHitDir)
							*retiHitDir = 0; //has to be >=0 to register as a direct hit //VectorToPolar(pObj->GetPos() - GetPos());

						return pObj;
						}
					}

				//	Otherwise record it if it is our best candidate

				if (rObjDistance2 < rClosestApproach2 && rObjDistance2 >= rDetonatorFailRange2 && bCanTriggerDetonation)
					{
					vClosestPos = pObj->GetPos();
					pClosestHit = pObj;
					rClosestApproach2 = rObjDistance2;
					}
				}

			//	If the object is not static, we need to do a reverse-proximity check because otherwise we cant see its motion

			else
				{
				//	We have to do this separately for each candidate
				CVector vObjPos = pObj->GetPos();
				CVector vObjTravel = pObj->GetVel() * g_SecondsPerUpdate;
				CVector vObjEnd = vObjPos + vObjTravel;
				Metric rObjTravel = vObjTravel.Length();
				iSteps = (int)(rObjTravel / (2.0 * g_KlicksPerPixel)) + 1;

				//	Discard anything too close if we have a fail range
				if (rDetonatorFailRange2)
					{
					Metric rStartDistance2 = (vObjPos - vStart).Length2();
					Metric rEndDistance2 = (vObjEnd - vStart).Length2();

					//	Only skip if the entire line segment is within the fail range
					if (rStartDistance2 < rDetonatorFailRange2 && rEndDistance2 < rDetonatorFailRange2)
						continue;
					}

				//	Check if they collide with us or come within auto activation range

				if (pObj->IntersectionTestScan(this, vObjPos, vObjEnd, iSteps, true, &vCurrentClosestPos, &iCurrentClosestPosAngle))
					{

					//	If they collided with us

					if (retvHitPos)
						*retvHitPos = vStart;

					if (retiHitDir)
						*retiHitDir = VectorToPolar(vStart - pObj->GetPos());

					return pObj;
					}
				else if ((GetPos() - vCurrentClosestPos).Length2() <= rAutoActivationRangeAdj2)
					{

					//	If they activated automatic activation range

					if (retvHitPos)
						*retvHitPos = vStart;

					if (retiHitDir)
						*retiHitDir = -1;

					return pObj;
					}
				else
					{
					//	Skip if we dont track proximity for this object

					if (!bCanTriggerDetonation)
						continue;

					//	we check if this triggers our proximity sensor
					CVector vDistance = GetPos() - vCurrentClosestPos;
					Metric rObjDistance2 = vDistance.Length2();

					if (vCurrentClosestPos != vObjEnd && rObjDistance2 < rClosestApproach2)
						{
						vClosestPos = pObj->GetPos();
						pClosestHit = pObj;
						rClosestApproach2 = vDistance.Length2();
						}
					}
				}
			}

		//	Else, we check if anything nearby sets off our proximity or we collide

		else
			{

			//	Discard anything too close if we have a fail range

			if (rDetonatorFailRange2)
				{
				Metric rStartDistance2 = (vStart - pObj->GetPos()).Length2();
				Metric rEndDistance2 = (vEnd - pObj->GetPos()).Length2();

				//	Only skip if the entire line segment is within the fail range
				if (rStartDistance2 < rDetonatorFailRange2 && rEndDistance2 < rDetonatorFailRange2)
					continue;
				}

			//	If we have a pre-emptive impact detection range, do a med-granularity check on that first
			if (rDetonatorImpactActivationRangeAdj)
				{
				int iImpactSteps = (int)(rMissileTravel / (4.0 * g_KlicksPerPixel)) + 1;
				CVector vImpactDistance = PolarToVector(GetRotation(), rDetonatorImpactActivationRangeAdj);
				CVector vImpactStep = vImpactDistance / iImpactSteps;

				if (IntersectionTestScan(pObj, vStart, vImpactStep, iImpactSteps, true, &vCurrentClosestPos, &iCurrentClosestPosAngle))
					{
					//	We have detected a target in front of us
					//	In this case we want to treat it as a proximity detonation

					if (retvHitPos)
						*retvHitPos = vStart;

					if (retiHitDir)
						*retiHitDir = -1;

					return pObj;
					}
				}

			//	If we have a pre-emptive impact detection range, and its bigger than our normal sensor range
			//	skip the stuff that we cant normally see
			if (rDetonatorAutoActivationRange > rSensorRange && (pObj->GetPos() - vStart).Length2() > rSensorRange2)
				{
				CVector vLL;
				CVector vUR;
				pObj->GetBoundingRect(&vUR, &vLL);
				if ((vLL - vStart).Length2() > rSensorRange2 && (vUR - vStart).Length2() > rSensorRange2)
					{
					CVector vUL = CVector(vLL.GetX(), vUR.GetY());
					CVector vLR = CVector(vUR.GetX(), vLL.GetY());
					if ((vLR - vStart).Length2() > rSensorRange2 && (vUL - vStart).Length2() > rSensorRange2)
						continue;
					}
				}

			//	Step towards this object and see if we hit it. Start by computing 
			//	the step vector, which should be 2 pixels long.

			if (iSteps == 0)
				{
				iSteps = (int)(rMissileTravel / (2.0 * g_KlicksPerPixel)) + 1;
				vStep = vMissileTravel / iSteps;
				}

			//	Check if we hit it directly

			if (IntersectionTestScan(pObj, vStart, vStep, iSteps, true, &vCurrentClosestPos, &iCurrentClosestPosAngle))
				{
				//	We have direct impact

				if (retvHitPos)
					*retvHitPos = vCurrentClosestPos;

				if (retiHitDir)
					*retiHitDir = VectorToPolar(-1 * vStep);

				return pObj;
				}

			//	Check if we are in auto activation range vs proximity

			else if ((pObj->GetPos() - vCurrentClosestPos).Length2() <= rAutoActivationRangeAdj2)
				{
				//	We are close enough to something we immediately detonate

				if (retvHitPos)
					*retvHitPos = vStart;

				if (retiHitDir)
					*retiHitDir = -1;

				return pObj;
				}

			//	Check if this is our most promising proximity candidate

			else
				{
				//	Skip if we dont track proximity for this object
				if (!bCanTriggerDetonation)
					continue;

				//	we check if this triggers our proximity sensor
				CVector vDistance = pObj->GetPos() - vCurrentClosestPos;
				Metric rDistance2 = vDistance.Length2();

				if (vCurrentClosestPos != vEnd && rDistance2 < rClosestApproach2)
					{
					vClosestPos = vCurrentClosestPos;
					pClosestHit = pObj;
					rClosestApproach2 = vDistance.Length2();
					}
				}
			}
		}

	pObj = NULL;

	//	See if we are going to be passing by anything sufficiently close

	if (pClosestHit)
		{
		if (retvHitPos)
			*retvHitPos = vClosestPos;

		if (retiHitDir)
			*retiHitDir = -1; //iCurrentClosestPosAngle;

		return pClosestHit;
		}

	//	We didn't hit anything.

	return NULL;

	DEBUG_CATCH_OBJ_LOOP
	}

CSpaceObject *CSpaceObject::HitTestProximityLegacy (const CVector &vStart, 
											  Metric rMinThreshold, 
											  Metric rMaxThreshold, 
											  const DamageDesc &Damage, 
											  const CTargetList::STargetOptions &TargetOptions,
											  const CSpaceObject *pTarget,
											  CVector *retvHitPos, 
											  int *retiHitDir)

//	HitTest (API 53 and below)
//
//	Returns the object that the beam hit or NULL if no object was hit.
//	If rThreshold > 0 and the object passes within the threshold distance
//	to some target, then retiHitDir = -1 and retvHitPos is the nearest point.

	{
	DEBUG_TRY_OBJ_LOOP

	CUsePerformanceCounter Counter(GetUniverse(), CONSTLIT("update.hitTestProximity"));

	const Metric OBJ_RADIUS_ADJ_MIN = 0.25;
	const Metric OBJ_RADIUS_ADJ_MAX = 1.00;

	//	Get the list of objects that intersect the object

	SSpaceObjectGridEnumerator i;
	GetSystem()->EnumObjectsInBoxStart(i, GetPos(), Max(rMaxThreshold, g_SecondsPerUpdate * LIGHT_SECOND));

	//	We need some variables for stepping

	int iSteps = 0;
	CVector vStep;
	bool bIsStatic = GetVel().IsNull();

	//	We need some variables to track the closest object

	Metric rClosestApproach = rMaxThreshold;
	CVector vClosestPos;
	CSpaceObject *pClosestHit = NULL;

	//	See if the beam hit anything. We start with a crude first pass.
	//	Any objects near the beam are then analyzed further to see if
	//	the beam hit them.

	int k;
	CVector vClosestFinalPos;
	while (GetSystem()->EnumObjectsInBoxHasMore(i))
		{
		pObj = GetSystem()->EnumObjectsInBoxGetNext(i);

		//	Skip objects that we cannot hit

		if (!CanHit(pObj)
				|| !pObj->CanBeHitBy(Damage)
				|| pObj == this)
			continue;

		//	Step towards this object and see if we hit it. Start by computing 
		//	the step vector, which should be 2 pixels long.

		if (iSteps == 0)
			{
			CVector vMissileTravel = (GetPos() - vStart);
			Metric rMissileTravel = vMissileTravel.Length();
			iSteps = (int)(rMissileTravel / (2.0 * g_KlicksPerPixel)) + 1;
			vStep = vMissileTravel / iSteps;
			}

		//	Prepare for point in object calculations

		SPointInObjectCtx PiOCtx;
		bool bInitNeeded = true;

		//	Do we need to calculate proximity detonation for this object?

		bool bCanTriggerDetonation = CTargetList::CanDetonate(*this, pTarget, TargetOptions, *pObj);
		
		//	Step

		CVector vPrev = vStart;
		CVector vTest = vStart;
		for (k = 0; k < iSteps; k++)
			{
			//	If we hit this object then we're done.

			if (pObj->PointInBounds(vTest))
				{
				if (bInitNeeded)
					{
					pObj->PointInObjectInit(PiOCtx);
					bInitNeeded = false;
					}

				if (pObj->PointInObject(PiOCtx, pObj->GetPos(), vTest))
					{
					if (retvHitPos)
						*retvHitPos = vPrev;

					//	Figure out the direction that the hit came from

					if (retiHitDir)
						*retiHitDir = VectorToPolar(-vStep, NULL);

					return pObj;
					}
				}

			//	Otherwise, if we're calculating proximity, calculate

			if (bCanTriggerDetonation)
				{
				CVector vDist = vTest - pObj->GetPos();
				Metric rDist = vDist.Length();

				if (rDist < rClosestApproach)
					{
					rClosestApproach = rDist;
					vClosestPos = vTest;
					pClosestHit = pObj;
					}
				}

			//	Store the best final pos if this is the best candidate
			if (pClosestHit == pObj)
				vClosestFinalPos = vTest;

			//	Next

			vPrev = vTest;
			vTest = vTest + vStep;
			}

		//	If we're calculating proximity and if we're going to hit this object next
		//	tick anyways, then explode now.
		//
		//	NOTE that in this case we do this for all objects (including asteroids, etc.).

		if (!bIsStatic)
			{
			CVector vNextPos = GetPos() + (GetVel() * g_SecondsPerUpdate);

			if (pObj->PointInBounds(vNextPos))
				{
				if (bInitNeeded)
					{
					pObj->PointInObjectInit(PiOCtx);
					bInitNeeded = false;
					}

				if (pObj->PointInObject(PiOCtx, pObj->GetPos(), vNextPos))
					{
					if (retvHitPos)
						*retvHitPos = GetPos();

					if (retiHitDir)
						*retiHitDir = -1;

					return pObj;
					}
				}
			}
		}

	pObj = NULL;

	//	See if we need to detonate

	if (pClosestHit)
		{
		//	If we're inside our minimum fragmentation radius, then we always
		//	detonate, regardless of whether or not we get closer later.

		if (rClosestApproach <= rMinThreshold)
			{
			if (retvHitPos)
				*retvHitPos = vClosestPos;

			if (retiHitDir)
				*retiHitDir = -1;

			return pClosestHit;
			}

		//	If we got inside the threshold radius for some object check to see if 
		//	we are now farther away. If so, then we	reached the closest point.
		//	(If not, then it means that next tick we will get closer still.)

		else
			{
			//Compute the projected distance to the best candidate at next tick
			CVector vDist = vClosestFinalPos - pClosestHit->GetPos();
			Metric rDist = vDist.Length() + (OBJ_RADIUS_ADJ_MIN * pClosestHit->GetHitSize());

			if (rDist > rClosestApproach)
				{
				if (retvHitPos)
					*retvHitPos = vClosestPos;

				if (retiHitDir)
					*retiHitDir = -1;

				return pClosestHit;
				}
			}
		}

	//	We didn't hit anything.

	return NULL;

	DEBUG_CATCH_OBJ_LOOP
	}

bool CSpaceObject::IntersectionTestScan(const CSpaceObject* pTarget, const CVector& vStart, const CVector& vStep, const int iSteps, const bool bComputeProximity, CVector* retvHitPos, int* retiHitDir, CVector* retvDetectPos, int* retiTriangulationDir)

//	IntersectionTestScan
//
//  Check if a scan vector over a number of steps intersects an object's image
//	Returns true if an intersection was detected - returns intersection data
//		retvHitPos: the coordinate of the detected hit
//		retiHitDir: the direction of the detected hit (the direction it came from, not the direction it was going)
//		retvDetectPos: retvHitPos
//		retiTriangulationDir: the direction from the hit to the target
//	Returns false if there was no intersection.
//	If bComputeProximity is set, with the closest point to the target - returns triangulation data to the target's pivot
//		retvHitPos: vStart
//		retiHitDir: -1
//		retvDetectPos: the closest scan step to the target's pivot
//		retiTriangulationDir: the direction from the hit to the target
//	If bComputeProximity is not set to save on cpu cycles by not doing a bunch of extra trigonometry
//		retvHitPos: vStart
//		retvHitDir: -1
//		retvDetectPos: vStart + iSteps * vStep
//		retiTriangulationDir: -1

	{
	SPointInObjectCtx PiOCtx;
	pTarget->PointInObjectInit(PiOCtx);

	CVector vTarget = pTarget->GetPos();
	CVector vEnd = vStart + iSteps * vStep;
	CVector vPath = vEnd - vStart;
	bool bCanIntersect = true;

	//	Is it even possible to hit the target? If not, we can skip some checks later
	//	Its automatically possible to hit if either start or end are already inside of the bounds
	
	if (!(pTarget->PointInBounds(vStart) || pTarget->PointInBounds(vEnd)))
		{
		//	Check if we cross through the bounding box

		CVector vTargetUR; //	upper right corner of bounds
		CVector vTargetLL; //	lower left corner of bounds

		//	Easy low-cost pre-checks

		Metric rEX = vEnd.GetX();
		Metric rSX = vStart.GetX();
		Metric rEY = vEnd.GetY();
		Metric rSY = vStart.GetY();
		Metric rLX = vTargetLL.GetX();
		Metric rLY = vTargetLL.GetY();
		Metric rRX = vTargetUR.GetX();
		Metric rUY = vTargetUR.GetY();

		//	Too far to the left to intersect

		if (rEX < rLX && rSX < rLX)
			bCanIntersect = false;

		//	Too far to the right to intersect

		else if (rEX > rRX && rSX > rRX)
			bCanIntersect = false;

		//	Too far up to intersect

		else if (rEY > rUY && rSY > rUY)
			bCanIntersect = false;

		//	Too far down to intersect

		else if (rEY < rLY && rSY < rLY)
			bCanIntersect = false;

		//	Check for all the way through horiz intersect

		else if (rEY < rUY && rSY < rUY && rEY > rLY && rSY > rLY)
			bCanIntersect = true;	//	optimized away as a no-op

		//	Check for all the way through vert intersect

		else if (rEX < rRX && rSX < rRX && rEX > rLX && rSX > rLX)
			bCanIntersect = true;	//	optimized away as a op-op

		//	Only remaining cases are cutting through one of the corners
		//	Narrow down which corner to check
		//	Check if we can cut either of the left corners

		else
			{
			bool bCanCutLeft = (rEX < rLX && rSX > rLX) || (rEX > rLX && rSX < rLX);
			bool bCanCutBottom = (rEY < rLY && rSY > rLY) || (rEY > rLY && rSY < rLY);
			Metric rSlope = vPath.GetY() / vPath.GetX();

			//	Check where our line is relative to that corner

			if (bCanCutBottom && bCanCutLeft)
				{
				//  check if our line goes below bottom left corner
				CVector vLeftPoint = rEX < rSX ? vEnd : vStart;
				CVector vLeftToLL = vTargetLL - vLeftPoint;
				Metric rSlopeToLL = vLeftToLL.GetY() / vLeftToLL.GetX();
				if (rSlope < rSlopeToLL)
					bCanIntersect = false;
				}
			else if (bCanCutBottom && !bCanCutLeft)
				{
				//	check if our line goes below bottom right corner
				CVector vLeftPoint = rEX < rSX ? vEnd : vStart;
				CVector vLeftToLR = CVector(rRX, rLY) - vLeftPoint;
				Metric rSlopeToLR = vLeftToLR.GetY() / vLeftToLR.GetX();
				if (rSlope < rSlopeToLR)
					bCanIntersect = false;
				}
			else if (!bCanCutBottom && bCanCutLeft)
				{
				//	check if our line goes above top left corner
				CVector vLeftPoint = rEX < rSX ? vEnd : vStart;
				CVector vLeftToUL = CVector(rLX, rUY) - vLeftPoint;
				Metric rSlopeToUL = vLeftToUL.GetY() / vLeftToUL.GetX();
				if (rSlope > rSlopeToUL)
					bCanIntersect = false;
				}
			else
				{
				//	check if our line goes above top right corner
				CVector vLeftPoint = rEX < rSX ? vEnd : vStart;
				CVector vLeftToUR = vTargetUR - vLeftPoint;
				Metric rSlopeToUR = vLeftToUR.GetY() / vLeftToUR.GetX();
				if (rSlope > rSlopeToUR)
					bCanIntersect = false;
				}
			}
		}

	//	Check if we should probe for proximity or not
	//	If the inner angles of start to target and end to target vs start to end are on either side of 90 degrees because they form an acute triangle
	//	Otherwise we just pick the closer of vStart or vEnd
	
	bool bCheckProximity = bComputeProximity && (retvHitPos || retiHitDir);
	if (bComputeProximity && (retvHitPos || retiHitDir))
		{
		//	We only do this on command to save on CPU cycles doing trigonometry

		CVector vEndToTarget = vTarget - vEnd;
		CVector vStartToTarget = vTarget - vStart;
		int iPathAngle = VectorToPolar(vPath);

		//	Both of these will be either positive 0-180 or negative -180-0 beccause they form a triangle

		int iEndToTargetAngle = VectorToPolar(vEndToTarget)-iPathAngle;
		int iStartToTargetAngle = VectorToPolar(vStartToTarget)-iPathAngle;

		if ((iEndToTargetAngle > 90 && iStartToTargetAngle < 90)
			|| (iEndToTargetAngle < 90 && iStartToTargetAngle > 90)
			|| (iEndToTargetAngle > -90 && iStartToTargetAngle < -90)
			|| (iEndToTargetAngle < -90 && iStartToTargetAngle > -90))
			bCheckProximity = true;
		else
			{
			//	Our best point is either a hit, our start, or our end

			Metric rDistEnd = vEndToTarget.Distance(CVector());
			Metric rDistStart = vStartToTarget.Distance(CVector());
			//	Set these now, they will be overwritten if we actually have a hit
			if (retvHitPos)
				*retvHitPos = vStart;
			if (retiHitDir)
				*retiHitDir = -1;
			if (retvDetectPos)
				*retvDetectPos = rDistEnd < rDistStart ? vEnd : vStart;
			if (retiTriangulationDir)
				*retiTriangulationDir = rDistEnd < rDistStart ? iEndToTargetAngle : iStartToTargetAngle;
			}
		}
	else
		{
		if (retvHitPos)
			*retvHitPos = vStart;
		if (retiHitDir)
			*retiHitDir = -1;
		if (retvDetectPos)
			*retvDetectPos = vEnd;
		if (retiTriangulationDir)
			*retiTriangulationDir = -1;
		}

	//	Step

	CVector vPrev = vStart;
	CVector vTest = vStart;

	CVector vClosestPos;
	Metric rClosestApproach2 = INFINITY; //	This stores a squared value for efficiency purposes
	for (int k = 0; k < iSteps; k++)
		{
		//	Check for collision
		//	First pass to check if we are in the bounds

		if (bCanIntersect && pTarget->PointInBounds(vTest))
			{
			if (pTarget->PointInObject(PiOCtx, pTarget->GetPos(), vTest))
				{
				if (retvHitPos)
					*retvHitPos = vPrev;

				//	Figure out the direction that the hit came from

				if (retiHitDir)
					*retiHitDir = VectorToPolar(-vStep, NULL);

				if (retvDetectPos)
					*retvDetectPos = vPrev;

				if (retiTriangulationDir)
					*retiTriangulationDir = VectorToPolar(vTarget - vPrev);

				return true;
				}
			}

		//	If we know our closest point is between beginning and end
		
		if (bCheckProximity)
			{
			CVector vDist = vTest - pTarget->GetPos();
			Metric rDist2 = vDist.Length2();

			if (rDist2 < rClosestApproach2)
				{
				rClosestApproach2 = rDist2;
				vClosestPos = vTest;
				}
			}

		//	Next

		vPrev = vTest;
		vTest = vTest + vStep;
		}
	
	//	Didnt hit, so set our best point if we had to check for proximity
	if (bCheckProximity)
		{
		if (retvHitPos)
			*retvHitPos = vClosestPos;
		if (retiHitDir)
			*retiHitDir = -1;
		if (retvDetectPos)
			*retvDetectPos = vClosestPos;
		if (retiTriangulationDir)
			*retiTriangulationDir = VectorToPolar(vTarget - vClosestPos);
		}

	return false;
	}

bool CSpaceObject::ImagesIntersect (const CObjectImageArray &Image1, int iTick1, int iRotation1, const CVector &vPos1,
									const CObjectImageArray &Image2, int iTick2, int iRotation2, const CVector &vPos2)

//	ImagesIntersect
//
//	Returns TRUE if the two images intersect

	{
	//	Compute the offset of Image2 relative to Image1 in pixels

	CVector vOffset = vPos2 - vPos1;
	int x = mathRound(vOffset.GetX() / g_KlicksPerPixel);
	int y = -mathRound(vOffset.GetY() / g_KlicksPerPixel);
	
	//	Images intersect

	return Image1.ImagesIntersect(iTick1, iRotation1, x, y, Image2, iTick2, iRotation2);
	}

bool CSpaceObject::InBarrier (const CVector &vPos)

//	InBarrier
//
//	Returns TRUE if the given position is in a barrier

	{
	int i;

	//	Compute the bounding rect for this object

	CVector vUR, vLL;
	GetBoundingRect(&vUR, &vLL);

	//	Loop over all other objects and see if we bounce off

	for (i = 0; i < m_pSystem->GetObjectCount(); i++)
		{
		CSpaceObject *pBarrier = m_pSystem->GetObject(i);
		if (pBarrier 
				&& pBarrier->m_fIsBarrier 
				&& pBarrier != this 
				&& pBarrier->CanBlock(this))
			{
			//	Compute the bounding rect for the barrier.

			CVector vBarrierUR, vBarrierLL;
			pBarrier->GetBoundingRect(&vBarrierUR, &vBarrierLL);

			//	If we intersect then block

			if (IntersectRect(vUR, vLL, vBarrierUR, vBarrierLL)
					&& pBarrier->ObjectInObject(pBarrier->GetPos(), this, GetPos()))
				return true;
			}
		}

	return false;
	}

bool CSpaceObject::IncProperty (const CString &sProperty, ICCItem *pInc, ICCItemPtr &pResult)

//	IncProperty
//
//	Increment the given property.

	{
	CDesignType *pType = GetType();

	//	See if this is a custom property

	ICCItemPtr pDummy;
	EPropertyType iType;
	if (pType && pType->FindCustomProperty(sProperty, pDummy, &iType))
		{
		switch (iType)
			{
			case EPropertyType::propGlobal:
				pResult = pType->IncGlobalData(sProperty, pInc);
				return true;

			case EPropertyType::propData:
			case EPropertyType::propObjData:
				pResult = IncData(sProperty, pInc);
				return true;

			default:
				return false;
			}
		}

	//	Otherwise, see if our sub-classes handle this property

	else if (OnIncProperty(sProperty, pInc, pResult))
		return true;

	//	Lastly, see if we can increment this ourselves.

	else
		{
		pResult = GetProperty(sProperty);
		if (!pResult->IsNumber())
			return false;

		ICCItemPtr pDefaultInc(1);
		if (pInc == NULL || pInc->IsNil())
			pInc = pDefaultInc;

		if (pResult->IsDouble() || pInc->IsDouble())
			pResult = ICCItemPtr(pResult->GetDoubleValue() + pInc->GetDoubleValue());
		else
			pResult = ICCItemPtr(pResult->GetIntegerValue() + pInc->GetIntegerValue());

		CString sError;
		if (!SetProperty(sProperty, pResult, &sError))
			pResult = ICCItemPtr::Error(sError);

		return true;
		}
	}

bool CSpaceObject::IsAngryAt (const CDamageSource &Obj) const

//	IsAngryAt
//
//	Returns TRUE if we're angry at the given objet

	{
	//	If we have a real object, then we can ask about it.

	CSpaceObject *pObj = Obj.GetObj();
	if (pObj)
		return IsAngryAt(pObj);

	//	Otherwise we just go by the sovereign relationship.

	return IsEnemy(Obj);
	}

bool CSpaceObject::IsEnemyInRange (Metric rMaxRange, bool bIncludeStations)

//	IsEnemyInRange
//
//	Returns TRUE if there is an enemy in the range

	{
	int i;

	//	Get the sovereign

	CSovereign *pSovereign = GetSovereignToDefend();
	if (pSovereign == NULL || GetSystem() == NULL)
		return false;

	//	Get the list of enemy objects

	const CSpaceObjectList &ObjList = pSovereign->GetEnemyObjectList(GetSystem());

	//	Start a max range

	Metric rBestDist = rMaxRange * rMaxRange;
	CSpaceObject *pBestObj = NULL;

	//	Loop for all enemy objects

	int iCount = ObjList.GetCount();
	for (i = 0; i < iCount; i++)
		{
		CSpaceObject *pObj = ObjList.GetObj(i);

		if ((pObj->GetCategory() == catShip
					|| (bIncludeStations && pObj->GetCategory() == catStation))
				&& pObj->CanAttack()
				&& pObj != this)
			{
			CVector vDist = GetPos() - pObj->GetPos();
			Metric rDist = vDist.Length2();

			if (rDist < rBestDist
					&& !pObj->IsEscortingFriendOf(this))
				return true;
			}
		}

	return false;
	}

bool CSpaceObject::IsEscortingFriendOf (const CSpaceObject *pObj) const

//	IsEscortingFriendOf
//
//	Returns TRUE if we're escorting a friend of pObj

	{
	CSpaceObject *pPrincipal = GetEscortPrincipal();
	if (pPrincipal)
		return pObj->IsFriend(pPrincipal);
	else
		return false;
	}

bool CSpaceObject::IsEscorting (const CSpaceObject* pObj) const

//	IsEscorting
//
//	Returns TRUE if we're escorting pObj

	{
	CSpaceObject* pPrincipal = GetEscortPrincipal();
	if (pPrincipal)
		return pObj == pPrincipal;
	else
		return false;
	}

bool CSpaceObject::IsInOurSquadron (const CSpaceObject &Obj) const

//	IsInOurSquadron
//
//	Returns TRUE if the given object is part of our squadron.

	{
	return (Obj.GetEscortPrincipal() == this
			&& Obj != this
			&& !Obj.IsSuspended()
			&& !Obj.IsDestroyed());
	}

bool CSpaceObject::IsPlayerAttackJustified (void) const

//	IsPlayerAttackJustified
//
//	Returns TRUE if we think the player attacked us by accident (e.g., while
//	defending us or themselves from attack).
//
//	NOTE: This assumes the player is not an enemy or already blacklisted.

	{
	CSpaceObject *pPlayer = GetPlayerShip();
	if (pPlayer == NULL)
		return false;

	//	Iterate over all objects to see who has this object as a target

	int i;
	for (i = 0; i < GetSystem()->GetObjectCount(); i++)
		{
		CSpaceObject *pObj = GetSystem()->GetObject(i);
		CSpaceObject *pTarget;

		if (pObj
				&& IsEnemy(pObj)
				&& ((pTarget = pObj->GetTarget()) == this
						|| pTarget == pPlayer)
				&& pObj != pPlayer
				&& pObj != this)
			return true;
		}

	return false;
	}

bool CSpaceObject::IsPlayerEscortTarget (CSpaceObject *pPlayer)

//	IsPlayerEscortTarget
//
//	Returns TRUE if we are being escorted by the player.

	{
	//	If we're not a player destination, then we're not being escorted.

	if (!IsPlayerDestination())
		return false;

	//	Get the player as a ship object

	if (pPlayer == NULL)
		{
		pPlayer = GetUniverse().GetPlayerShip();
		if (pPlayer == NULL)
			return false;
		}

	CShip *pPlayerShip = pPlayer->AsShip();
	if (pPlayerShip == NULL)
		return false;

	//	Check the player's target

	const COrderDesc &OrderDesc = pPlayerShip->GetCurrentOrderDesc();

	return (OrderDesc.GetTarget() == this
			&& (OrderDesc.GetOrder() == IShipController::orderGuard || OrderDesc.GetOrder() == IShipController::orderEscort));
	}

bool CSpaceObject::IsStargateInRange (Metric rMaxRange)

//	IsStargateInRange
//
//	Returns TRUE if stargate is in range

	{
	int i;
	Metric rNearestGateDist2 = (rMaxRange * rMaxRange);
	for (i = 0; i < GetSystem()->GetObjectCount(); i++)
		{
		CSpaceObject *pObj = GetSystem()->GetObject(i);

		if (pObj && pObj->IsActiveStargate())
			{
			Metric rDist2 = (GetPos() - pObj->GetPos()).Length2();
			if (rDist2 < rNearestGateDist2)
				return true;
			}
		}

	return false;
	}

bool CSpaceObject::IsUnderAttack (void) const

//	IsUnderAttack
//
//	Returns TRUE if the space object is being attacked
//	by another object.

	{
	//	Iterate over all objects to see who has this object as a target

	int i;
	for (i = 0; i < GetSystem()->GetObjectCount(); i++)
		{
		CSpaceObject *pObj = GetSystem()->GetObject(i);

		if (pObj
				&& pObj->GetTarget() == this
				&& IsEnemy(pObj)
				&& pObj != this)
			return true;
		}

	return false;
	}

bool CSpaceObject::IsCommsMessageValidFrom (const CSpaceObject &SenderObj, int iIndex, CString *retsMsg, CString *retsKey) const

//	IsCommsMessageValidFrom
//
//	Returns TRUE if the given object can send the given comms message to
//	this object

	{
	const CCommunicationsHandler *pHandler = GetCommsHandler();
	if (!pHandler || iIndex >= pHandler->GetCount())
		throw CException(ERR_FAIL);

	const CCommunicationsHandler::SMessage &Msg = pHandler->GetMessage(iIndex);

	//	If we have an OnShow code block then see if it evaluates to TRUE. If not,
	//	then we bail.

	if (Msg.OnShowEvent.pCode)
		{
		CCodeChainCtx Ctx(GetUniverse());

		//	Define parameters

		Ctx.DefineContainingType(this);
		Ctx.SaveAndDefineSourceVar(this);
		Ctx.DefineSpaceObject(CONSTLIT("gSender"), SenderObj);

		//	Execute

		ICCItemPtr pResult = Ctx.RunCode(Msg.OnShowEvent);

		if (pResult->IsNil())
			return false;
		else if (pResult->IsError())
			{
			SenderObj.SendMessage(this, pResult->GetStringValue());
			return false;
			}

		//	<OnShow> returns non-Nil, so we continue.
		}

	//	If we don't have a message name then use the ID to lookup the name and
	//	key (shortcut).

	if (Msg.sMessage.IsBlank() && !Msg.sID.IsBlank())
		{
		CString sLabelDesc;
		if (TranslateText(Msg.sID, NULL, &sLabelDesc))
			{
			CLanguage::ParseLabelDesc(sLabelDesc, retsMsg, retsKey);

			//	If no short cut is defined, then use the one provided. [This 
			//	should not be a common case.]

			if (retsKey && retsKey->IsBlank())
				*retsKey = Msg.sShortcut;
			}
		else
			{
			if (retsMsg)
				*retsMsg = Msg.sID;

			if (retsKey)
				*retsKey = Msg.sShortcut;
			}
		}

	//	Otherwise, we expect a valid name and shortcut

	else
		{
		if (retsMsg)
			*retsMsg = Msg.sMessage;

		if (retsKey)
			*retsKey = Msg.sShortcut;
		}

	return true;
	}

bool CSpaceObject::IsCovering (CSpaceObject *pObj)

//	IsCovering
//
//	Returns TRUE if this object is covering the given object. That is,
//	we return TRUE if pObj is on top of (or below) this object and
//	this object is bigger than pObj

	{
	if (this != pObj)
		{
		CVector vThisUR;
		CVector vThisLL;

		GetBoundingRect(&vThisUR, &vThisLL);
		return pObj->PosInBox(vThisUR, vThisLL);
		}
	else
		return false;
	}

bool CSpaceObject::IsDestinyTime (int iCycle, int iOffset)

//	IsDestinyTime
//
//	Returns TRUE if the space object's destiny aligns with
//	the given cycle. A cycle of n aligns with a space object
//	once every n ticks. Each object aligns at different times
//	depending on its destiny.
 
	{
	return (((GetUniverse().GetTicks() + GetDestiny()) % iCycle) == iOffset);
	}

bool CSpaceObject::IsEnemy (const CSovereign &Sovereign) const

//	IsEnemy
//
//	Returns TRUE if the given object is our enemy

	{
	const CSovereign *pOurSovereign = GetSovereign();

	if (pOurSovereign == NULL)
		return false;
	else
		return pOurSovereign->IsEnemy(Sovereign);
	}

bool CSpaceObject::IsEnemy (const CSpaceObject *pObj) const

//	IsEnemy
//
//	Returns TRUE if the given object is our enemy

	{
	CSovereign *pOurSovereign = GetSovereign();
	CSovereign *pEnemySovereign = pObj->GetSovereign();

	if (pOurSovereign == NULL || pEnemySovereign == NULL)
		return false;
	else
		return pOurSovereign->IsEnemy(pEnemySovereign);
	}

bool CSpaceObject::IsEnemy (const CDamageSource &Obj) const

//	IsEnemy
//
//	Returns TRUE if the given object is our enemy

	{
	CSovereign *pOurSovereign = GetSovereign();
	CSovereign *pEnemySovereign = Obj.GetSovereign();

	if (pOurSovereign == NULL || pEnemySovereign == NULL)
		return false;
	else
		return pOurSovereign->IsEnemy(pEnemySovereign);
	}

bool CSpaceObject::IsFriend (const CSpaceObject *pObj) const

//	IsFriend
//
//	Returns TRUE if the given object is our friend. Note that this
//	is not equal to !IsEnemy. It is also possible for an object to
//	be "neutral"

	{
	CSovereign *pOurSovereign = GetSovereign();
	CSovereign *pEnemySovereign = pObj->GetSovereign();

	if (pOurSovereign == NULL || pEnemySovereign == NULL)
		return false;
	else
		return pOurSovereign->IsFriend(pEnemySovereign);
	}

bool CSpaceObject::IsNeutral (const CSpaceObject* pObj) const

//	IsFriend
//
//	Returns TRUE if the given object is neutral.

	{
	CSovereign* pOurSovereign = GetSovereign();
	CSovereign* pEnemySovereign = pObj->GetSovereign();

	if (pOurSovereign == NULL || pEnemySovereign == NULL)
		return false;
	else
		return pOurSovereign->IsNeutral(pEnemySovereign);
	}

bool CSpaceObject::IsFriend (const CSovereign &Sovereign) const

//	IsFriend
//
//	Returns TRUE if the given object is our friend. Note that this
//	is not equal to !IsEnemy. It is also possible for an object to
//	be "neutral"

	{
	CSovereign* pOurSovereign = GetSovereign();

	if (pOurSovereign == NULL || &Sovereign == NULL)
		return false;
	else
		return pOurSovereign->IsFriend(Sovereign);
	}

bool CSpaceObject::IsNeutral (const CSovereign &Sovereign) const

//	IsFriend
//
//	Returns TRUE if the given object is neutral.

	{
	CSovereign* pOurSovereign = GetSovereign();

	if (pOurSovereign == NULL || &Sovereign == NULL)
		return false;
	else
		return pOurSovereign->IsNeutral(Sovereign);
	}

bool CSpaceObject::IsLineOfFireClear (const CInstalledDevice *pWeapon,
									  CSpaceObject *pTarget, 
									  int iAngle, 
									  Metric rDistance,
									  CSpaceObject **retpFriend) const

//	IsLineOfFireClear
//
//	Returns TRUE if there are no friendly objects along the given
//	direction for the given distance

	{
	static const Metric TOO_CLOSE = 20.0 * g_KlicksPerPixel;

	//	If we can't hit friends, then this whole function is moot

	if (!CanHitFriends() || !pWeapon->CanHitFriends())
		return true;

	CUsePerformanceCounter Counter(GetUniverse(), CONSTLIT("update.lineOfFire"));

	//	Compute some values

	const CDeviceItem WeaponItem = pWeapon->GetDeviceItem();
	CVector vSource = pWeapon->GetPos(this);
	bool bAreaWeapon = WeaponItem.IsAreaWeapon();
	bool bShockwaveWeapon = WeaponItem.IsShockwaveWeapon();

	//	We need to adjust the angle to compensate for the fact that the shot
	//	will take on the velocity of the ship.

	CItemCtx ItemCtx(this, pWeapon);
	Metric rShotSpeed = pWeapon->GetShotSpeed(ItemCtx);
	if (rShotSpeed > 0.0)
		{
		CVector vShotVel = GetVel() + PolarToVector(iAngle, rShotSpeed);
		iAngle = VectorToPolar(vShotVel);
		}

	//	We look for objects in the range of the weapon

	CSystem *pSystem = GetSystem();
	SSpaceObjectGridEnumerator i;

	//	Create a box around the path of the shot

	CVector vEndPoint = vSource + PolarToVector(iAngle, rDistance);
	CVector vLL = CVector(Min(vSource.GetX(), vEndPoint.GetX()), Min(vSource.GetY(), vEndPoint.GetY()));
	CVector vUR = CVector(Max(vSource.GetX(), vEndPoint.GetX()), Max(vSource.GetY(), vEndPoint.GetY()));
	pSystem->EnumObjectsInBoxStart(i, vUR, vLL, gridNoBoxCheck);

	//	Compute position of target

	CVector vTarget = (pTarget ? pTarget->GetPos() : vSource);
	Metric rMaxDist2 = rDistance * rDistance;
	bool bCheckTargetOverStation = (pTarget && pTarget->CanThrust());
	bool bTargetIsStation = (pTarget && pTarget->GetCategory() == catStation);

	//	See if any friendly object is in the line of fire

	bool bResult = true;
	while (pSystem->EnumObjectsInBoxHasMore(i))
		{
		CSpaceObject *pObj = pSystem->EnumObjectsInBoxGetNextFast(i);

		if (!pObj->IsDestroyed()
				&& pObj->CanBeAttacked()
				//	Only check for ships, structures
				&& (pObj->GetScale() == scaleStructure 
					|| pObj->GetScale() == scaleShip)
				&& CanFireOn(pObj)
				&& !pObj->IsImmutable()
				&& pObj != this
				&& pObj != pTarget)
			{
			CSpaceObject::Categories iCategory = pObj->GetCategory();

			//	If this is an enemy then see if it is OK to hit them.

			if (IsEnemy(pObj))
				{
				//	If this is another ship, then it is OK to hit.

				if (iCategory == catShip)
					continue;

				//	If the target is a station, then it is OK to hit other 
				//	stations of the same sovereign.

				if (bTargetIsStation && pTarget->GetSovereign() == pObj->GetSovereign())
					continue;
				}

			//	If the target is right on top of a station, then we
			//	cannot fire.

			if (iCategory == catStation
					&& bCheckTargetOverStation)
				{
				//	Compute the distance of the object from us and from
				//	the target.

				CVector vDistFromTarget = pObj->GetPos() - vTarget;
				Metric rDistFromTarget2 = vDistFromTarget.Length2();

				if (rDistFromTarget2 < BOUNDS_CHECK_DIST2)
					{
					CVector vUR, vLL;
					pObj->GetBoundingRect(&vUR, &vLL);

					if (rDistFromTarget2 < 2.0 * vUR.Length2()
							&& pObj->PointInObject(pObj->GetPos(), vTarget))
						{
						if (retpFriend) *retpFriend = pObj;
						bResult = false;
						break;
						}
					}
				}

			//	Skip if we're too far

			CVector vCurObjPos = pObj->GetPos();
			CVector vCurDist = vCurObjPos - vSource;
			Metric rCurDist2 = vCurDist.Length2();
			if (rCurDist2 > rMaxDist2)
				continue;

			//	If this is a shockwave weapon and a friendly is in range, then 
			//	we always abort.

			if (bShockwaveWeapon)
				{
				if (retpFriend) *retpFriend = pObj;
				bResult = false;
				break;
				}

			//	Get the current distance. Because this is all a heuristic, we
			//	assume that this is not too different from the object distance
			//	when the shot gets there.

			Metric rCurDist = sqrt(rCurDist2);

			//	If we're inside the object radius, then we would hit it no matter what
			//	the angle.

			Metric rHalfSize = 0.5 * pObj->GetHitSize();
			if (rCurDist < rHalfSize)
				{
				if (retpFriend) *retpFriend = pObj;
				bResult = false;
				break;
				}

			//	Figure out where the object will be by the time the shot gets
			//	to it.

			CVector vFutureDist;
			if (rShotSpeed > 0.0 && pObj->CanThrust())
				{
				Metric rTimeToReachObj = rCurDist / rShotSpeed;
				CVector vFutureObjPos = vCurObjPos + (rTimeToReachObj * pObj->GetVel());
				vFutureDist = vFutureObjPos - vSource;
				}
			else
				vFutureDist = vCurDist;

			//	Figure out the object's bearing relative to us

			int iFutureAngle = VectorToPolar(vFutureDist);

			//	Figure out how big the object is from that distance.
			//
			//	NOTE: For now we use the current (not future) distance because
			//	we assume it is close enough (as opposed to the angle).

			int iHalfAngularSize;
			if (rCurDist < rHalfSize + TOO_CLOSE)
				iHalfAngularSize = 90;
			else if (bAreaWeapon)
				iHalfAngularSize = 45;
			else
				iHalfAngularSize = pObj->GetHitSizeHalfAngle(rCurDist);

			//	See if it is in our line of fire

			if (AreAnglesAligned(iAngle, iFutureAngle, iHalfAngularSize))
				{
				if (retpFriend) *retpFriend = pObj;
				bResult = false;
				break;
				}
			}
		}

	return bResult;
	}

void CSpaceObject::Jump (const CVector &vPos)

//	Jump
//
//	Object jumps to a different position in the system

	{
	//	Create a gate effect at the old position

	CEffectCreator *pEffect = GetUniverse().FindEffectType(g_StargateInUNID);
	if (pEffect)
		pEffect->CreateEffect(m_pSystem,
				NULL,
				GetPos(),
				NullVector,
				0);

	//	Ask to see if any object wants to alter the jump position

	CVector vNewPos = GetSystem()->OnJumpPosAdj(this, vPos);

	//	Move the ship to the new position

	Place(vNewPos);

	//	Create a gate effect at the destination

	pEffect = GetUniverse().FindEffectType(g_StargateOutUNID);
	if (pEffect)
		pEffect->CreateEffect(m_pSystem,
				NULL,
				vNewPos,
				NullVector,
				0);

	//	Set the ship to hide while coming out of gate

	CShip *pShip = AsShip();
	if (pShip)
		pShip->SetInGate(NULL, 0);

	//	Notify subscribers

	m_SubscribedObjs.NotifyOnObjJumped(this);
	}

bool CSpaceObject::MatchesCriteria (CSpaceObjectCriteria::SCtx &Ctx, const CSpaceObjectCriteria &Crit) const

//	MatchesCriteria
//
//	Returns TRUE if this object matches the criteria

	{
	CSpaceObject *pSource = Ctx.pSource;
	if (pSource == this)
		return false;

	//	If we have an OR expression and that matches, then we're done.

	if (Crit.HasORExpression() && MatchesCriteria(Ctx, Crit.GetORExpression()))
		return true;

	//	Match player

	if (!(Crit.MatchesCategory(GetCategory()))
			&& (!Crit.MatchesPlayer() || !IsPlayer()))
		return false;

	//	NOTE: Virtual objects always count as active. E.g., we want the virtual
	//	St. Victoria station to count as active, but not as CanAttack
	//	[An alternative is to add !IsVirtual() to all places that look for
	//	enemies to attack.]

	if (Crit.MatchesActiveOnly() && IsInactive() && !IsVirtual())
		return false;

	if (Crit.MatchesStargatesOnly() && !IsActiveStargate())
		return false;

	if (Crit.MatchesStructureScaleOnly() 
			&& GetCategory() == CSpaceObject::catStation
			&& (GetScale() == scaleStar || GetScale() == scaleWorld))
		return false;

	if (Crit.MatchesManufacturedOnly()
			&& (GetScale() == scaleWorld || GetScale() == scaleStar))
		return false;

	if (Crit.MatchesKilledOnly() && (CanBeAttacked() || IsVirtual()))
		return false;

	if (!Crit.MatchesSovereign(GetSovereign()))
		return false;

	if (Crit.MatchesSourceSovereign() 
			&& pSource 
			&& GetSovereignUNID() != pSource->GetSovereignUNID())
		return false;

	if (Crit.MatchesDockedWithSource())
		if (pSource == NULL || !pSource->IsObjDocked(this))
			return false;

	if (Crit.MatchesFriendlyOnly() 
			&& pSource
			&& pSource->IsEnemy(this) 
			&& !IsEscortingFriendOf(pSource) 
			&& !pSource->IsEscortingFriendOf(this))
		return false;

	if (Crit.MatchesEnemyOnly() 
			&& pSource
			&& (!pSource->IsEnemy(this) || IsEscortingFriendOf(pSource) || pSource->IsEscortingFriendOf(this)))
		return false;

	if (Crit.MatchesAngryOnly() 
			&& pSource
			&& (!IsAngryAt(pSource) || IsEscortingFriendOf(pSource) || pSource->IsEscortingFriendOf(this)))
		return false;

	if (!Crit.MatchesStargate(GetStargateID()))
		return false;

	if (Crit.MatchesHomeBaseIsSource() && GetBase() != pSource)
		return false;

	if (Crit.MatchesTargetableMissilesOnly()
			&& GetCategory() == CSpaceObject::catMissile
			&& !(IsTargetableProjectile()))
		return false;

	if (Crit.MatchesTargetIsSource() && GetTarget() != pSource)
		return false;

	//	Check level

	if (!Crit.MatchesLevel(GetLevel()))
		return false;

	//	Check attributes

	if (!Crit.MatchesAttributes(*this))
		return false;

	//	Other checks

	if (Crit.ExcludesPlayer() && IsPlayer())
		return false;

	if (!Crit.MatchesData().IsBlank() && m_Data.IsDataNil(Crit.MatchesData()))
		return false;

	if (Crit.ExcludesVirtual() && IsVirtual())
		return false;

	//	Virtual objects are also intangible, so if we want to find virtual 
	//	objects (but not all intangible objects) then we need the extra check
	//	about !IsVirtual.

	if (Crit.ExcludesIntangible() && IsIntangible() && !IsVirtual())
		return false;

	//	With a particular order

	if (!Crit.MatchesOrder(pSource, *this))
		return false;

	//	If necessary, compute the distance and angle from the source to the obj.

	int iObjAngle = -1;
	Metric rObjDist = -1.0;
	Metric rObjDist2 = -1.0;
	if (Ctx.bCalcPolar)
		{
		CVector vCenter = (pSource ? pSource->GetPos() : CVector());
		CVector vDist = GetPos() - vCenter;

		iObjAngle = VectorToPolar(vDist, &rObjDist);
		rObjDist2 = rObjDist * rObjDist;
		}
	else if (Ctx.bCalcDist2)
		{
		CVector vCenter = (pSource ? pSource->GetPos() : CVector());
		CVector vDist = GetPos() - vCenter;

		iObjAngle = 0;
		rObjDist = 0.0;
		rObjDist2 = vDist.Length2();
		}

	ASSERT(!Crit.MatchesNearerThan() || rObjDist2 >= 0.0);
	ASSERT(!Crit.MatchesFartherThan() || rObjDist2 >= 0.0);
	ASSERT(!Crit.MatchesPerceivableOnly() || rObjDist2 >= 0.0);
	ASSERT(Crit.MatchesIntersectAngle() == -1 || (rObjDist >= 0.0 && iObjAngle >= 0));
	ASSERT(!Ctx.bNearestOnly || rObjDist2 >= 0.0);
	ASSERT(!Ctx.bFarthestOnly || rObjDist2 >= 0.0);
	ASSERT(Ctx.iSort == CSpaceObjectCriteria::sortNone || rObjDist2 >= 0.0);

	//	Ranges

	if (Crit.MatchesNearerThan() && rObjDist2 > Crit.MatchesMaxRadius2())
		return false;

	if (Crit.MatchesFartherThan() && rObjDist2 < Crit.MatchesMinRadius2())
		return false;

	//	Visible only

	if (Crit.MatchesPerceivableOnly() && rObjDist2 > GetDetectionRange2(Ctx.iSourcePerception))
		return false;

	if (Crit.MatchesCanPerceiveSourceOnly() 
			&& rObjDist2 > CPerceptionCalc::GetRange2(CPerceptionCalc::GetRangeIndex(Ctx.iSourceStealth, GetPerception())))
		return false;

	//	Angle
	//
	//	Only bother checking if rDist > 0 (we always intersect with an
	//	object that is 0 distance from us :).

	if (Crit.MatchesIntersectAngle() != -1 && rObjDist > 0.0)
		{
		//	Figure out how large the object is at this distance

		int iHalfAngularSize = GetHitSizeHalfAngle(rObjDist);
		
		//	If we do not intersect the line then we're done

		if (!AreAnglesAligned(Crit.MatchesIntersectAngle(), iObjAngle, iHalfAngularSize))
			return false;
		}

	//	Position checks

	if (!Crit.MatchesPosition(*this))
		return false;

	//	If we're looking for the nearest or farthest, do that computation now

	if (Ctx.bNearestOnly)
		{
		if (rObjDist2 < Ctx.rBestDist2
				&& (!IsIntangible() || IsVirtual()))
			{
			Ctx.pBestObj = const_cast<CSpaceObject *>(this);
			Ctx.rBestDist2 = rObjDist2;
			}
		}
	else if (Ctx.bFarthestOnly)
		{
		if (rObjDist2 > Ctx.rBestDist2
				&& (!IsIntangible() || IsVirtual()))
			{
			Ctx.pBestObj = const_cast<CSpaceObject *>(this);
			Ctx.rBestDist2 = rObjDist2;
			}
		}

	//	If we're sorting by distance, then add the object to the list

	if (Ctx.iSort == CSpaceObjectCriteria::sortByDistance)
		Ctx.DistSort.Insert(rObjDist2, const_cast<CSpaceObject *>(this));

	return true;
	}

bool CSpaceObject::MatchesCriteriaCategory (CSpaceObjectCriteria::SCtx &Ctx, const CSpaceObjectCriteria &Crit) const

//	MatchesCriteriaCategory
//
//	Returns TRUE if this object matches just the category portion of the 
//	criteria. We expose this for optimizations.

	{
	if (Crit.MatchesCategory(GetCategory()))
		return true;

	if (Crit.HasORExpression())
		return MatchesCriteriaCategory(Ctx, Crit.GetORExpression());

	return false;
	}

bool CSpaceObject::MissileCanHitObj (const CSpaceObject &Obj, const CDamageSource &Source, const CWeaponFireDesc &Desc, CSpaceObject *pTarget) const

//	MissileCanHitObj
//
//	Return TRUE if this object (a missile) can hit the given object (and assuming
//	that this object was fired by Source).

	{
	DEBUG_TRY

	//	If we have a source...

	if (Source.HasSource())
		{
		//	If we can damage our source, then we don't need to check further

		if (Desc.CanDamageSource())
			{
			if (//	We cannot hit another beam/missile from the same source
				//	(otherwise we get fratricide on fragmentation weapons).
				Source.IsEqual(Obj.GetDamageSource())

					//	See if the missile has rules about what it cannot hit
					|| !Desc.CanHit(Obj))

				return false;
			}

		//	Otherwise, we can only hit if we're not hitting our source, etc.

		else
			{
			if (//	We cannot hit the source of the beam...
				Source.IsEqual(Obj)

					//	We cannot hit another beam/missile from the same source...
					|| Source.IsEqual(Obj.GetDamageSource())

					//	See if the missile has rules about what it cannot hit
					|| !Desc.CanHit(Obj)

					//	We cannot hit our friends (if our source can't)
					|| ((!CanHitFriends() || !Source.CanHitFriends() || !Obj.CanBeHitByFriends()) 
							&& !Source.IsAngryAt(Obj, GetSovereign())
							//	But we can always hit planets, stargates, etc. (Otherwise
							//	the player can't hide from Quantumsphere shots.)
							&& !Obj.IsImmutable() 
							&& Obj.GetScale() != scaleWorld 
							&& Obj.GetScale() != scaleStar)

					//	If our source is the player, then we cannot hit player wingmen

					|| !Source.CanHit(Obj))
				return false;
			}
		}

	//	If we don't have a source...

	else
		{
		//	If we don't have a primary source, then don't hit our secondary source either
		//	(For ship explosions, the secondary source is the wreck; the wreck cannot be the
		//	primary source or else the tombstone message will be wrong)

		if (Obj == GetSecondarySource())
			return false;

		//	Make sure we can hit

		else if (!Desc.CanHit(Obj))
			return false;

		//	If we are part of an explosion, then we cannot hit other parts of an explosion
		//	that also have no source. This is so that fragments from an explosion where the source
		//	got destroyed (i.e., pSource == NULL) do not hit each other.

		else if (Source.IsEqual(Obj.GetDamageSource()))
			return false;
		}

	//	If we get this far then it means that we can hit. Now we need to check 
	//	the interaction.

	return Desc.GetInteraction().CalcCanInteractWith(Obj.GetInteraction(), pTarget && pTarget == Obj);

	DEBUG_CATCH
	}

void CSpaceObject::Move (SUpdateCtx &Ctx, Metric rSeconds)

//	Move
//
//	Moves the object in a straight line based on its current
//	velocity

	{
	DEBUG_TRY

#ifdef DEBUG_MOVE_PERFORMANCE
	Ctx.bCalledMove = true;
	Ctx.bCalledShipOnMove = false;
	Ctx.bCalledShipEffectMove = false;
#endif

	//	Remember the old position

	m_vOldPos = m_vPos;

	//	Move the object on a straight line along the velocity vector

	if (!m_vVel.IsNull() && !m_fNonLinearMove && !IsAnchored())
		m_vPos = m_vPos + (m_vVel * rSeconds);

	//	Let descendants process the move (if necessary)

	OnMove(Ctx, m_vOldPos, rSeconds);

	//	Clear painted (until the next tick)

	ClearPainted();

	//	Set a flag so we check collisions

	if (IsAnchored() || m_vVel.IsNull())
		SetCollisionTestNeeded(false);
	else if (Ctx.bHasShipBarriers)
		SetCollisionTestNeeded(GetCategory() == CSpaceObject::catShip || GetCategory() == CSpaceObject::catStation);
	else
		SetCollisionTestNeeded(GetCategory() == CSpaceObject::catStation);

	DEBUG_CATCH;
	}

void CSpaceObject::NotifyOnNewSystem (CSystem *pNewSystem)

//	NotifyOnNewSystem
//
//	The object has been moved to a new system (this is commonly done for wingmen
//	and other followers of the player). We guarantee that the old system is 
//	still loaded at this point.

	{
	int i;

	//	If any objects in the old system subscribe to us, then we need to
	//	cancel the subscription (missions are OK because they cross systems).

	for (i = 0; i < m_SubscribedObjs.GetCount(); i++)
		{
		CSpaceObject *pSubscriber = m_SubscribedObjs.GetObj(i);
		if (pSubscriber->IsDestroyed()
				|| (!pSubscriber->IsMission() 
					&& pSubscriber->GetSystem() != pNewSystem))
			{
			m_SubscribedObjs.Delete(i);
			i--;
			}
		}

	//	Let our subclasses handle it

	OnNewSystem(pNewSystem);
	}

void CSpaceObject::NotifyOnObjDestroyed (SDestroyCtx &Ctx)

//	NotifyOnObjDestroyed
//
//	Notify subscribers OnObjDestroyed

	{
	DEBUG_TRY

	m_SubscribedObjs.NotifyOnObjDestroyed(Ctx);

	//	Notify any docked objects that their dock target got destroyed.

	CDockingPorts *pPorts = GetDockingPorts();
	if (pPorts)
		pPorts->OnDockObjDestroyed(this, Ctx);

	DEBUG_CATCH
	}

void CSpaceObject::NotifyOnObjDocked (CSpaceObject *pDockTarget)

//	NotifyOnObjDocked
//
//	Notify subscribers OnObjDocked

	{
	m_SubscribedObjs.NotifyOnObjDocked(this, pDockTarget);
	}

bool CSpaceObject::NotifyOnObjGateCheck (CSpaceObject *pGatingObj, CTopologyNode *pDestNode, const CString &sDestEntryPoint, CSpaceObject *pGateObj)

//	NotifyOnObjGateCheck
//
//	Notify subscribers OnObjGateCheck

	{
	return m_SubscribedObjs.NotifyOnObjGateCheck(pGatingObj, pDestNode, sDestEntryPoint, pGateObj);
	}

bool CSpaceObject::ObjRequestDock (CSpaceObject *pObj, int iPort)

//	ObjRequestDock
//
//	pObj requests docking services with this object. Returns TRUE if docking 
//	is engaged.

	{
	switch (CanObjRequestDock(pObj))
		{
		case dockingOK:
			{
			CDockingPorts *pPorts = GetDockingPorts();
			if (pPorts == NULL)
				{
				//	Should never happen; we would not return dockingOK in this case.

				ASSERT(false);
				pObj->SendMessage(this, CONSTLIT("No docking services available"));
				return false;
				}

			//	Request dock

			return pPorts->RequestDock(this, pObj, iPort);
			}

		case dockingNotSupported:
			pObj->SendMessage(this, CONSTLIT("No docking services available"));
			return false;

		case dockingDisabled:
			pObj->SendMessage(this, CONSTLIT("Unable to dock"));
			return false;

		case dockingDenied:
			{
			CString sText;
			if (!pObj->TranslateText(LANGID_DOCKING_REQUEST_DENIED, NULL, &sText))
				sText = CONSTLIT("Docking request denied");

			pObj->SendMessage(this, sText);
			return false;
			}
			
		default:
			//	Should never happen. This means that we missed a result type.
			ASSERT(false);
			return false;
		}
	}

void CSpaceObject::OnObjLoadComplete (SLoadCtx &Ctx)

//	OnObjLoadComplete
//
//	Called after all objects have been loaded.

	{
	SetEventFlags();

	if (Ctx.pSystem)
		{
		LoadObjReferences(Ctx.pSystem);
		OnSystemLoaded(Ctx);
		}

	FireOnLoad(Ctx);
	}

void CSpaceObject::OnModifyItemBegin (IDockScreenUI::SModifyItemCtx &ModifyCtx, const CItem &Item) const

//	OnModifyItemBegin
//
//	The given Item (which must be part of the object) is about to be modified.

	{
	GetUniverse().GetDockSession().OnModifyItemBegin(ModifyCtx, *this, Item);
	}

void CSpaceObject::OnModifyItemComplete (IDockScreenUI::SModifyItemCtx &ModifyCtx, const CItem &Result)

//	OnModifyItemComplete
//
//	An item was modified. Result is the newly modified item.

	{
	InvalidateItemListState();
	GetUniverse().GetDockSession().OnModifyItemComplete(ModifyCtx, *this, Result);
	}

void CSpaceObject::OnObjDestroyed (const SDestroyCtx &Ctx)

//	OnObjDestroyed
//
//	Called whenever another object in the system is destroyed

	{
	DEBUG_TRY

	//	Give our subclasses a chance to do something (note that we need
	//	to do this before we NULL-out the references because some objects
	//	need to check their references.

	if (IsObjectDestructionHooked())
		ObjectDestroyedHook(Ctx);

	//	NULL-out any references to the object

	m_Data.OnObjDestroyed(&Ctx.Obj);

	//	Remove the object if it had a subscription to us

	m_SubscribedObjs.Delete(&Ctx.Obj);

	DEBUG_CATCH
	}

void CSpaceObject::SetConditionDueToDamage (SDamageCtx &DamageCtx, ECondition iCondition)

//	SetConditionDueToDamage
//
//	The given damage has imparted the given condition. NOTE: The caller is 
//	responsible for checking to see if the object is immune or not.

	{
	switch (iCondition)
		{
		case ECondition::blind:
			{
			SApplyConditionOptions Options;
			Options.iTimer = DamageCtx.GetBlindTime();
			Options.bNoImmunityCheck = true;

			ApplyCondition(iCondition, Options);
			break;
			}

		case ECondition::paralyzed:
			{
			SApplyConditionOptions Options;
			Options.iTimer = DamageCtx.GetParalyzedTime();
			Options.bNoImmunityCheck = true;

			ApplyCondition(iCondition, Options);
			break;
			}

		case ECondition::radioactive:
			{
			SApplyConditionOptions Options;
			Options.Cause = DamageCtx.Attacker;
			Options.bNoImmunityCheck = true;

			ApplyCondition(iCondition, Options);
			break;
			}
		}
	}

void CSpaceObject::Paint (CG32bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx)

//	Paint
//
//	Paint the object

	{
	if (IsPaintDeferred(Ctx))
		return;
	else if (IsHidden())
		{
		SetPainted();
		ClearPaintNeeded();
		return;
		}

	//	Initialize the object bounds

	Ctx.rcObjBounds = GetImage().GetImageRectAtPoint(x, y);
	Ctx.yAnnotations = Ctx.rcObjBounds.bottom + ANNOTATION_INNER_SPACING_Y + 1;

	//	Paint annotations under the object

	PaintDebugVector(Dest, x, y, Ctx);

	if (m_fShowHighlight && !Ctx.fNoSelection && !m_fShowDamageBar)
		PaintTargetHighlight(Dest, x, y, Ctx);

	//	Paint the object

	OnPaint(Dest, x, y, Ctx);

	//	Paint effects (e.g., muzzle flash)

	if (m_pFirstEffect)
		PaintEffects(Dest, x, y, Ctx);

	//	Mark this object's joints as needed to be painted

	if (m_pFirstJoint)
		m_pFirstJoint->SetObjListPaintNeeded(this);

	//	Paint annotations about the object (damage bar, etc.)

	if (!Ctx.fNoSelection)
		{
		if (IsHighlighted())
			PaintHighlight(Dest, x, y, Ctx);

		//	Paint damage bar

		if (m_fShowDamageBar)
			{
			int cyHeight;

			CPaintHelper::PaintStatusBar(Dest,
					x,
					Ctx.yAnnotations,
					GetUniverse().GetPaintTick(),
					GetSymbolColor(),
					NULL_STR,
					100 - GetVisibleDamage(),
					100,
					&cyHeight);

			Ctx.yAnnotations += cyHeight + ANNOTATION_INNER_SPACING_Y;
			}

		//	Show bounds

		if (Ctx.bShowBounds 
				|| InDebugMode()
				|| (GetUniverse().GetDebugOptions().IsShowNavPathsEnabled() && BlocksShips()))
			{
			CG32bitPixel rgbColor = GetSymbolColor();
			int xHalf = mathRound(m_rBoundsX / g_KlicksPerPixel);
			int yHalf = mathRound(m_rBoundsY / g_KlicksPerPixel);
			CGDraw::RectOutline(Dest, x - xHalf, y - yHalf, 2 * xHalf, 2 * yHalf,rgbColor);

			Dest.DrawDot(x, y, rgbColor, MarkerTypes::markerMediumCross);
			}

		//	Let the object paint additional annotations

		OnPaintAnnotations(Dest, x, y, Ctx);
		}

	//	Done

	SetPainted();
	ClearPaintNeeded();
	}

void CSpaceObject::PaintEffects (CG32bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx)

//	PaintEffects
//
//	Paints the list of effects for this object

	{
	SEffectNode *pEffect = m_pFirstEffect;
	if (pEffect)
		{
		CViewportPaintCtxSmartSave Save(Ctx);
		Ctx.iVariant = 0;
		Ctx.iDestiny = GetDestiny();

		while (pEffect)
			{
			Ctx.iTick = pEffect->iTick;
			Ctx.iRotation = pEffect->iRotation;

			pEffect->pPainter->Paint(Dest, 
					x + pEffect->xOffset,
					y + pEffect->yOffset,
					Ctx);

			pEffect = pEffect->pNext;
			}
		}
	}

void CSpaceObject::PaintHighlight (CG32bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx)

//	PaintHighlight
//
//	Paints a highlight around the object

	{
	//	Figure out the color of the highlight

	CG32bitPixel rgbColor = GetSymbolColor();

	//	Paint the corners

	Dest.DrawLine(Ctx.rcObjBounds.left, Ctx.rcObjBounds.top,
			Ctx.rcObjBounds.left + HIGHLIGHT_CORNER_WIDTH, Ctx.rcObjBounds.top,
			1, rgbColor);

	Dest.DrawLine(Ctx.rcObjBounds.left, Ctx.rcObjBounds.top,
			Ctx.rcObjBounds.left, Ctx.rcObjBounds.top + HIGHLIGHT_CORNER_HEIGHT,
			1, rgbColor);

	Dest.DrawLine(Ctx.rcObjBounds.right, Ctx.rcObjBounds.top,
			Ctx.rcObjBounds.right - HIGHLIGHT_CORNER_WIDTH, Ctx.rcObjBounds.top,
			1, rgbColor);

	Dest.DrawLine(Ctx.rcObjBounds.right, Ctx.rcObjBounds.top,
			Ctx.rcObjBounds.right, Ctx.rcObjBounds.top + HIGHLIGHT_CORNER_HEIGHT,
			1, rgbColor);

	Dest.DrawLine(Ctx.rcObjBounds.left, Ctx.rcObjBounds.bottom,
			Ctx.rcObjBounds.left, Ctx.rcObjBounds.bottom - HIGHLIGHT_CORNER_HEIGHT,
			1, rgbColor);

	Dest.DrawLine(Ctx.rcObjBounds.left, Ctx.rcObjBounds.bottom,
			Ctx.rcObjBounds.left + HIGHLIGHT_CORNER_WIDTH, Ctx.rcObjBounds.bottom,
			1, rgbColor);

	Dest.DrawLine(Ctx.rcObjBounds.right, Ctx.rcObjBounds.bottom,
			Ctx.rcObjBounds.right - HIGHLIGHT_CORNER_WIDTH, Ctx.rcObjBounds.bottom,
			1, rgbColor);

	Dest.DrawLine(Ctx.rcObjBounds.right, Ctx.rcObjBounds.bottom,
			Ctx.rcObjBounds.right, Ctx.rcObjBounds.bottom - HIGHLIGHT_CORNER_HEIGHT,
			1, rgbColor);

	//	Paint message, if we have one

	if (!m_sHighlightText.IsBlank() || m_iHighlightChar)
		{
		int cyHeight;
		PaintHighlightText(Dest, x, Ctx.yAnnotations, Ctx, alignCenter, rgbColor, &cyHeight);
		Ctx.yAnnotations += cyHeight + ANNOTATION_INNER_SPACING_Y;
		}
	}

void CSpaceObject::PaintAnnotationText (CG32bitImage &Dest, int x, int y, const CString &sText, SViewportPaintCtx &Ctx) const

//	PaintAnnotationText
//
//	Paints an annotation.

	{
	const CG16bitFont &MessageFont = GetUniverse().GetNamedFont(CUniverse::fontSRSMessage);
	MessageFont.DrawText(Dest,
			x,
			Ctx.yAnnotations,
			GetSymbolColor(),
			sText,
			CG16bitFont::AlignCenter);

	Ctx.yAnnotations += MessageFont.GetHeight() + ANNOTATION_INNER_SPACING_Y;
	}

void CSpaceObject::PaintHighlightText (CG32bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx, AlignmentStyles iAlign, CG32bitPixel rgbColor, int *retcyHeight)

//	PaintHighlightText
//
//	Paint highlight text

	{
	int yOriginal = y;

	const int KEY_BOX_SIZE = 18;
	const CG16bitFont &NameFont = GetUniverse().GetNamedFont(CUniverse::fontSRSObjName);
	const CG16bitFont &MessageFont = GetUniverse().GetNamedFont(CUniverse::fontSRSMessage);

	if (iAlign & alignBottom)
		{
		y -= NameFont.GetHeight()
				+ (IsShowingDistanceAndBearing() ? NameFont.GetHeight() : 0)
				+ (!m_sHighlightText.IsBlank() ? MessageFont.GetHeight() : 0)
				+ (m_iHighlightChar ? KEY_BOX_SIZE + 2 : 0);
		}

	DWORD dwFontFlags = CG16bitFont::AdjustToFit;
	if (iAlign & alignCenter)
		dwFontFlags |= CG16bitFont::AlignCenter;
	else if (iAlign & alignRight)
		dwFontFlags |= CG16bitFont::AlignRight;

	//	Figure out what name to paint

	CString sName;
	if (IsIdentified())
		sName = GetNounPhrase(nounTitleCapitalize);
	else if (Ctx.pCenter == NULL)
		sName = CONSTLIT("Unknown Object");
	else if (Ctx.pCenter->IsEnemy(this))
		sName = CONSTLIT("Unknown Hostile");
	else
		sName = CONSTLIT("Unknown Friendly");

	//	Paint it

	NameFont.DrawText(Dest, 
			x,
			y,
			rgbColor, 
			sName,
			dwFontFlags);
	y += NameFont.GetHeight();

	//	Paint distance and bearing, if required

	if (IsShowingDistanceAndBearing() && Ctx.pCenter)
		{
		Metric rDist = (GetPos() - Ctx.pCenter->GetPos()).Length();
		CString sText = strPatternSubst(CONSTLIT("Distance: %d"), (int)(rDist / LIGHT_SECOND));
		NameFont.DrawText(Dest,
				x,
				y,
				rgbColor,
				sText,
				dwFontFlags);

		y += NameFont.GetHeight();
		}

	//	Paint the message, if we have one

	if (!m_sHighlightText.IsBlank())
		{
		RECT rcRect;
		rcRect.left = x - 1000;
		rcRect.right = x + 1000;
		rcRect.top = y;
		rcRect.bottom = y + 1000;

		//	Paint message

		CG32bitPixel rgbMessageColor;
		if (m_iHighlightCountdown > HIGHLIGHT_BLINK)
			rgbMessageColor = CG32bitPixel::Blend(rgbColor, 0xffff, (BYTE)(255 * (m_iHighlightCountdown - HIGHLIGHT_BLINK) / (HIGHLIGHT_TIMER - HIGHLIGHT_BLINK)));
		else
			rgbMessageColor = rgbColor;

		DWORD dwOpacity;
		if (m_iHighlightCountdown < HIGHLIGHT_FADE)
			dwOpacity = 255 * m_iHighlightCountdown / HIGHLIGHT_FADE;
		else
			dwOpacity = 255;

		int cyHeight;
		MessageFont.DrawText(Dest,
				rcRect,
				CG32bitPixel(rgbMessageColor, (BYTE)dwOpacity),
				m_sHighlightText,
				0,
				dwFontFlags,
				&cyHeight);

		y += cyHeight;
		}

	//	Paint the highlight key

	if (m_iHighlightChar)
		{
		const CG16bitFont &KeyFont = GetUniverse().GetNamedFont(CUniverse::fontSRSObjName);
		char chChar = (char)m_iHighlightChar;
		CString sKey = CString(&chChar, 1);

		y += ANNOTATION_INNER_SPACING_Y;

		Dest.Fill(x - KEY_BOX_SIZE / 2, y, KEY_BOX_SIZE, KEY_BOX_SIZE, rgbColor);

		int xText = x - (KeyFont.MeasureText(sKey) / 2);
		int yText = y + (KEY_BOX_SIZE / 2) - (KeyFont.GetHeight() / 2);

		KeyFont.DrawText(Dest,
				xText,
				yText,
				0,
				sKey);

		y += KEY_BOX_SIZE;
		}

	//	Done

	if (retcyHeight)
		*retcyHeight = y - yOriginal;
	}

void CSpaceObject::PaintLRSForeground (CG32bitImage &Dest, int x, int y, const ViewportTransform &Trans)

//	PaintLRSForeground
//
//	Paints the object on an LRS

	{
	Dest.DrawDot(x, y, 
			GetUniverse().GetAccessibilitySettings().GetIFFColor(CAccessibilitySettings::IFFType::projectile),
			markerRoundDot);
	}

void CSpaceObject::PaintMap (CMapViewportCtx &Ctx, CG32bitImage &Dest, int x, int y)

//	PaintMap
//
//	Paints the object on a system map

	{
	OnPaintMap(Ctx, Dest, x, y);

	if (IsPlayerDestination() || (IsHighlighted() && !m_sHighlightText.IsBlank()) || IsSelected())
		{
		int iTick = GetUniverse().GetPaintTick();
		int iRadius = 10;
		int iRingSpacing = 4;
		CG32bitPixel rgbColor = GetSymbolColor();

		CPaintHelper::PaintTargetHighlight(Dest, x, y, iTick, iRadius, iRingSpacing, 6, rgbColor);

		if (IsHighlighted() && !m_sHighlightText.IsBlank())
			{
			SViewportPaintCtx ViewportCtx;
			ViewportCtx.pCenter = Ctx.GetCenterObj();

			PaintHighlightText(Dest, x, y, ViewportCtx, alignCenter, rgbColor);
			}
		}
	}

void CSpaceObject::PaintTargetHighlight (CG32bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx)

//	PaintTargetHighlight
//
//	Paints an animated highlight

	{
	int iTick = GetUniverse().GetPaintTick();
	int iRadius = (int)(0.5 * GetHitSize() / g_KlicksPerPixel);
	int iRingSpacing = 10;
	CG32bitPixel rgbColor = GetSymbolColor();

	CPaintHelper::PaintTargetHighlight(Dest, x, y, iTick, iRadius, iRingSpacing, 3, rgbColor);
	}

bool CSpaceObject::ParseConditionOptions (const ICCItem &Options, SApplyConditionOptions &retOptions) const

//	ParseObjectPart
//
//	Parse options for setting conditions.

	{
	retOptions = SApplyConditionOptions();

	if (Options.IsNil())
		return true;
	else if (Options.IsSymbolTable())
		{
		CCodeChainCtx CCX(GetUniverse());

		if (ICCItem *pApplyTo = Options.GetElement(CONSTLIT("applyTo")))
			{
			CString sApplyTo = pApplyTo->GetStringValue();
			if (strEquals(sApplyTo, CONSTLIT("interior")))
				retOptions.ApplyTo.iPart = EObjectPart::interior;
			else
				return false;
			}
		else if (ICCItem *pApplyToItem = Options.GetElement(CONSTLIT("applyToItem")))
			{
			retOptions.ApplyTo.iPart = EObjectPart::item;
			retOptions.ApplyTo.Item = CCX.AsItem(pApplyToItem);
			}

		if (ICCItem *pDuration = Options.GetElement(CONSTLIT("duration")))
			{
			retOptions.iTimer = pDuration->GetIntegerValue();
			}

		return true;
		}
	else if (Options.IsNumber())
		{
		retOptions.iTimer = Options.GetIntegerValue();

		return true;
		}
	else
		return false;
	}

void CSpaceObject::Reconned (void)

//	Reconned
//
//	Tell all other objects that this object has been reconned
//	(used for missions)

	{
	m_SubscribedObjs.NotifyOnObjReconned(this);
	}

void CSpaceObject::RecordBuyItem (CSpaceObject *pSellerObj, const CItem &Item, const CCurrencyAndValue &Price)

//	RecordBuyItem
//
//	Transfers funds from the buyer to the seller and calls appropriate events.
//	NOTE: This does not transfer the item.

	{
	if (!pSellerObj)
		throw CException(ERR_FAIL);

	if (!Price.GetCurrencyType())
		throw CException(ERR_FAIL);

	//	NOTE: It is OK if the Item is null.

	//	Charge the buyer (us)

	ChargeMoney(Price.GetCurrencyType()->GetUNID(), Price.GetValue());

	//	Give money to the seller

	pSellerObj->CreditMoney(Price.GetCurrencyType()->GetUNID(), Price.GetValue());

	//	If the player is buying, then record it as the player buying.

	if (IsPlayer())
		{
		CShip *pPlayerShip = AsShip();
		if (pPlayerShip)
			pPlayerShip->GetController()->OnItemBought(Item, Price);

		//	If the player is buying, then allow types to keep track
		//	(e.g., Black Market gives out experience points).

		if (CDesignType *pSellerType = pSellerObj->GetType())
			{
			pSellerType->FireOnPlayerBoughtItem(Item, Price);
			}

		GetUniverse().FireOnGlobalPlayerBoughtItem(pSellerObj, Item, Price);
		}

	//	Otherwise, if the seller is the player, record it as the player
	//	selling.

	else if (pSellerObj->IsPlayer())
		{
		CShip *pPlayerShip = pSellerObj->AsShip();
		if (pPlayerShip)
			pPlayerShip->GetController()->OnItemSold(Item, Price);

		//	If the player is selling, then allow types to keep track.

		if (CDesignType *pBuyerType = GetType())
			{
			pBuyerType->FireOnPlayerSoldItem(Item, Price);
			}

		GetUniverse().FireOnGlobalPlayerSoldItem(this, Item, Price);
		}
	}

void CSpaceObject::Remove (DestructionTypes iCause, const CDamageSource &Attacker, bool bRemovedByOwner)

//	Remove
//
//	Removes this object from the system
//
//	pCause is the object that caused us to be removed (may be NULL)

	{
	if (m_iIndex != -1 && !m_fDestroyed)
		{
		//	Set the destroyed flag so that this object 
		//	doesn't show up in cached lists like the system object grid
		//
		//	Also, we use this to prevent recursion

		m_fDestroyed = true;

		//	Remove from the object from the universal list (NOTE: We must do this
		//	before we clear out m_pSystem.)

		CSpaceObject::Categories iCategory = GetCategory();
		if (iCategory == CSpaceObject::catStation || iCategory == CSpaceObject::catShip)
			GetUniverse().GetGlobalObjects().Delete(this);

		//	Remove

		SDestroyCtx Ctx(*this);
		Ctx.iCause = iCause;
		Ctx.Attacker = Attacker;
		Ctx.pWreck = NULL;
		Ctx.bResurrectPending = false;
		Ctx.bRemovedByOwner = bRemovedByOwner;
		Ctx.pResurrectedObj = NULL;

		CSystem *pSystem = m_pSystem;
		m_pSystem = NULL;

		//	Give our descendants a chance to remove. For example, ships will
		//	deal with attached sections.

		OnRemoved(Ctx);

		//	This will call OnObjDestroyed for all interested objects

		pSystem->RemoveObject(Ctx);

		//	Delete all subscriptions. We are leaving the system, so we can't
		//	hold on to pointers to the old system.
		//
		//	NOTE: We leave mission objects intact

		m_SubscribedObjs.DeleteSystemObjs();

		//	Done

		m_iIndex = -1;
		}
	}

void CSpaceObject::RemoveAllEventSubscriptions (CSystem *pSystem, TArray<DWORD> *retRemoved)

//	RemoveAllEventSubscriptions
//
//	Removes all of this object's subscriptions to other objects. Optionally
//	returns an array of object IDs that we subscribed to.

	{
	int i;

	if (retRemoved)
		retRemoved->DeleteAll();

	for (i = 0; i < pSystem->GetObjectCount(); i++)
		{
		CSpaceObject *pObj = pSystem->GetObject(i);
		if (pObj && pObj != this)
			{
			if (pObj->m_SubscribedObjs.Delete(this))
				{
				if (retRemoved)
					retRemoved->Insert(pObj->GetID());
				}
			}
		}
	}

EConditionResult CSpaceObject::RemoveCondition (ECondition iCondition, const SApplyConditionOptions &Options)

//	RemoveCondition
//
//	Removes the given condition

	{
	//	Check to see if we can remove the condition

	EConditionResult iResult = CanRemoveCondition(iCondition, Options);
	if (iResult != EConditionResult::ok)
		return iResult;

	//	Handle some conditions ourselves.

	switch (iCondition)
		{
		case ECondition::fouled:
			{
			ScrapeOverlays();
			break;
			}

		case ECondition::timeStopped:
			{
			RestartTime();
			break;
			}

		//	Otherwise, let our descendant handle it.

		default:
			OnRemoveCondition(iCondition, Options);
			break;
		}

	//	See if we removed the condition

	if (GetCondition(iCondition))
		return EConditionResult::stillApplied;
	else
		return EConditionResult::ok;
	}

void CSpaceObject::RemoveItemEnhancement (const CItem &itemToEnhance, DWORD dwID, bool bExpiredOnly)

//	RemoveItemEnhancement
//
//	Removes the given item enhancement

	{
	DEBUG_TRY

	//	Find the item

	CItemListManipulator ItemList(GetItemList());
	if (!ItemList.SetCursorAtItem(itemToEnhance))
		return;

	//	Make sure it is the right mod

	const CItemEnhancement &Mod = ItemList.GetItemAtCursor().GetMods();
	if (Mod.GetID() != dwID)
		return;

	//	If we're removed expired only, check for expiration

	if (bExpiredOnly)
		{
		if (Mod.GetExpireTime() > GetUniverse().GetTicks())
			return;
		}

	//	Get the enhancement type now because Mod will be invalid after
	//	we remove it.

	CItemType *pEnhancementType = Mod.GetEnhancementType();

	//	Remove the enhancement

	ItemList.RemoveItemEnhancementAtCursor(dwID);

	//	Notify source

	ItemEnhancementModified(ItemList);

	//	Fire an event to the enhancement to tell it

	if (ItemList.IsCursorValid())
		{
		CItem theEnhancement(pEnhancementType, 1);
		theEnhancement.FireOnRemovedAsEnhancement(this, ItemList.GetItemAtCursor());
		}

	DEBUG_CATCH
	}

void CSpaceObject::RepairItem (CItemListManipulator &ItemList)

//	RepairItem
//
//	Repairs the selected item

	{
	const CItem &Item = ItemList.GetItemAtCursor();

	if (Item.IsDamaged())
		{
		ItemList.SetDamagedAtCursor(false);

		//	Raise event

		ItemEnhancementModified(ItemList);
		}
	}

void CSpaceObject::ReportEventError (const CString &sEvent, const ICCItem *pError) const

//	ReportEventError
//
//	Report an error during an event

	{
	CString sError = strPatternSubst(CONSTLIT("%s [%s]: %s"), sEvent, GetNounPhrase(), pError->GetStringValue());
	CSpaceObject *pPlayer = GetUniverse().GetPlayerShip();
	if (pPlayer)
		pPlayer->SendMessage(this, sError);

	kernelDebugLogString(sError);
	}

bool CSpaceObject::RequestGate (CSpaceObject *pObj)

//	RequestGate
//
//	Requests that the given object be transported through the gate

	{
	pObj->EnterGate(NULL, NULL_STR, this);
	return true;
	}

void CSpaceObject::ScrapeOverlays ()

//	ScrapeOverlays
//
//	Remove attached, foreign overlays

	{
	COverlayList *pOverlays = GetOverlays();
	if (pOverlays == NULL)
		return;

	pOverlays->ScrapeHarmfulOverlays(this);
	}

void CSpaceObject::SendSquadronMessage (const CString &sMsg)

//	SendSquadronMessage
//
//	Sends the message to all deployed objects in our squadron.

	{
	CSystem *pSystem = GetSystem();
	if (!pSystem)
		return;

	//	Add all autons/wingmates in the system.

	for (int i = 0; i < pSystem->GetObjectCount(); i++)
		{
		CSpaceObject *pObj = pSystem->GetObject(i);

		if (pObj && IsInOurSquadron(*pObj))
			SendSquadronMessage(*pObj, sMsg);
		}
	}

void CSpaceObject::SendSquadronMessage (CSpaceObject &ReceiverObj, const CString &sMsg)

//	SendSquadronMessage
//
//	Sends the message to the given object in our squadron.

	{

	}

void CSpaceObject::SetCursorAtArmor (CItemListManipulator &ItemList, CInstalledArmor *pArmor)

//	SetCursorAtArmor
//
//	Sets the given cursor at the item for the armor.

	{
	ItemList.ResetCursor();

	while (ItemList.MoveCursorForward())
		{
		const CItem &Item = ItemList.GetItemAtCursor();
		if (Item.IsInstalled()
				&& Item.GetType() == pArmor->GetClass()->GetItemType()
				&& Item.GetInstalled() == pArmor->GetSect())
			{
			ASSERT(Item.GetCount() == 1);
			break;
			}
		}
	}

bool CSpaceObject::SetCursorAtDevice (CItemListManipulator &ItemList, int iDevSlot)

//	SetCursorAtDevice
//
//	Sets the given cursor at the item for the device. Returns TRUE if successful.

	{
	ItemList.ResetCursor();

	while (ItemList.MoveCursorForward())
		{
		const CItem &Item = ItemList.GetItemAtCursor();
		if (Item.IsInstalled()
				&& Item.IsDevice()
				&& Item.GetInstalled() == iDevSlot)
			{
			ASSERT(Item.GetCount() == 1);
			return true;
			}
		}

	return false;
	}

bool CSpaceObject::SetCursorAtDevice (CItemListManipulator &ItemList, CInstalledDevice *pDevice)

//	SetCursorAtDevice
//
//	Sets the given cursor at the item for the device.

	{
	return SetCursorAtDevice(ItemList, pDevice->GetDeviceSlot());
	}

void CSpaceObject::OnObjDestroyUpdateDevices (const SDestroyCtx& Ctx)
	{
	for (int i = 0; i < GetDeviceCount(); i++)
		{
		CDeviceClass* pDeviceClass = GetDevice(i)->GetClass();
		if(pDeviceClass)
			pDeviceClass->OnObjDestroyed(GetDevice(i), this, Ctx);
		}
	}

void CSpaceObject::SetCursorAtRandomItem (CItemListManipulator &ItemList, const CItemCriteria &Crit)

//	SetCursorAtRandomItem
//
//	Sets the given cursor at a random item that matches the criteria
//	(We may leave the cursor invalid if there is no item that matches
//	the criteria.)

	{
	TArray<int> ItemPos;

	//	Make a list of the position of all the items that match
	//	the criteria.

	int iCount = 0;
	ItemList.ResetCursor();
	while (ItemList.MoveCursorForward())
		{
		if (ItemList.GetItemAtCursor().MatchesCriteria(Crit))
			ItemPos.Insert(iCount);

		iCount++;
		}

	//	Now choose a random device

	ItemList.ResetCursor();
	if (ItemPos.GetCount() > 0)
		{
		int iRoll = ItemPos[mathRandom(0, ItemPos.GetCount() - 1)];

		while (ItemList.MoveCursorForward() && iRoll > 0)
			iRoll--;
		}

	//	Done
	}

void CSpaceObject::SetEventFlags (void)

//	SetEventFlags
//
//	Sets cached flags for events

	{
	SetHasGetDockScreenEvent(FindEventHandler(CONSTLIT("GetDockScreen")));
	SetHasOnAttackedEvent(FindEventHandler(CONSTLIT("OnAttacked")));
	SetHasOnAttackedByPlayerEvent(FindEventHandler(CONSTLIT("OnAttackedByPlayer")));
	SetHasOnDamageEvent(FindEventHandler(CONSTLIT("OnDamage")));
	SetHasOnObjDockedEvent(FindEventHandler(CONSTLIT("OnObjDocked")));

	//	See if we have a handler for dock screens.

	if (CDockingPorts *pDockingPorts = GetDockingPorts())
		{
		bool bHasGetDockScreen = HasDockScreen();

		//	If we have an event for a dock screen, but we don't have docking ports
		//	then we create some default ports. This is useful for when overlays or
		//	event handlers create dock screens.

		if (bHasGetDockScreen && pDockingPorts->GetPortCount() == 0)
			{
			CreateDefaultDockingPorts();
			m_fAutoCreatedPorts = true;
			}

		//	If we DON'T have dock screens and we auto-created some docking ports,
		//	then we need to remove them.

		else if (!bHasGetDockScreen && m_fAutoCreatedPorts)
			{
			pDockingPorts->DeleteAll(this);
			m_fAutoCreatedPorts = false;
			}
		}

	//	Let subclasses do their bit

	OnSetEventFlags();
	}

void CSpaceObject::SetGlobalData (const CString &sAttribute, ICCItem *pData)

//	SetGlobalData
//
//	Sets data for the type

	{
	CDesignType *pType = GetType();
	if (pType == NULL)
		return;

	pType->SetGlobalData(sAttribute, pData);
	}

void CSpaceObject::SetOverride (CDesignType *pOverride)

//	SetOverride
//
//	Sets the override.
	
	{
	//	NULL means go back to the default event handler (which could be NULL).

	if (pOverride == NULL)
		pOverride = GetDefaultOverride();

	//	Short-circuit if no change

	if (pOverride == m_pOverride)
		{
		//	Always set the event flags because we call this when we create the
		//	universe and we want to make sure we set the flags in that case.

		SetEventFlags();
		return;
		}

	//	Let the previous event handler terminate

	if (m_pOverride)
		{
		SEventHandlerDesc Event;
		if (FindEventHandler(ON_OVERRIDE_TERM_EVENT, &Event))
			{
			CCodeChainCtx Ctx(GetUniverse());

			Ctx.DefineContainingType(this);
			Ctx.SaveAndDefineSourceVar(this);

			ICCItemPtr pResult = Ctx.RunCode(Event);
			if (pResult->IsError())
				ReportEventError(ON_OVERRIDE_TERM_EVENT, pResult);
			}
		}

	//	Set it.

	m_pOverride = pOverride;
	SetEventFlags();

	if (m_pOverride)
		{
		//	Initialize any custom properties.

		m_pOverride->InitObjectData(*this, GetData());

		//	Fire OnOverrideInit

		SEventHandlerDesc Event;
		if (FindEventHandler(ON_OVERRIDE_INIT_EVENT, &Event))
			{
			CCodeChainCtx Ctx(GetUniverse());

			Ctx.DefineContainingType(this);
			Ctx.SaveAndDefineSourceVar(this);

			ICCItemPtr pResult = Ctx.RunCode(Event);
			if (pResult->IsError())
				ReportEventError(ON_OVERRIDE_INIT_EVENT, pResult);
			}
		}
	}

void CSpaceObject::SetPlayerDestination (const SPlayerDestinationOptions &Options)

//	SetPlayerDestination
//
//	Sets the object as a player destination.

	{
	m_fPlayerDestination = true;

	if (Options.bShowDistanceAndBearing)
		m_fShowDistanceAndBearing = true;

	if (Options.bAutoClearDestination)
		m_fAutoClearDestination = true;

	if (Options.bAutoClearOnDestroy)
		m_fAutoClearDestinationOnDestroy = true;

	if (Options.bAutoClearOnDock)
		m_fAutoClearDestinationOnDock = true;

	if (Options.bAutoClearOnGate)
		m_fAutoClearDestinationOnGate = true;

	if (Options.bShowHighlight)
		m_fShowHighlight = true;
	}

void CSpaceObject::SetSovereign (CSovereign *pSovereign)

//	SetSovereign
//
//	Sets the object sovereign

	{
	OnSetSovereign(pSovereign);

	//	If we're part of a system, we need to flush the enemy object cache when
	//	we change sovereigns.

	CSystem *pSystem;
	if (ClassCanAttack() && (pSystem = GetSystem()))
		pSystem->FlushEnemyObjectCache();
	}

void CSpaceObject::ShowDamage (const SDamageCtx &Ctx)

//	ShowDamage
//
//	Shows damage done by the player.

	{
	CMarker::SCreateOptions Options;
	Options.pSovereign = GetSovereign();
	Options.iLifetime = 150;
	Options.iStyle = CMarker::EStyle::Message;
	Options.sName = strPatternSubst(CONSTLIT("%d hp %s"), Ctx.iDamage, ::GetDamageName(Ctx.Damage.GetDamageType()));

	CMarker::Create(*GetSystem(), Ctx.vHitPos, NullVector, Options);
	}

bool CSpaceObject::Translate (const CString &sID, ICCItem *pData, ICCItemPtr &retResult) const

//	Translate
//
//	Translate a message by ID. The caller is responsible for discarding the 
//	result.

	{
	//	First we ask the override

	if (m_pOverride && m_pOverride->Translate(*this, sID, pData, retResult))
		return true;

	//	Ask the type

	CDesignType *pType = GetType();
	if (pType && pType->Translate(*this, sID, pData, retResult))
		return true;

	//	Otherwise, see if the sovereign has it

	CSovereign *pSovereign = GetSovereign();
	if (pSovereign && pSovereign->Translate(*this, sID, pData, retResult))
		return true;

	//	Otherwise, we can't find it.

	return false;
	}

bool CSpaceObject::TranslateText (const CString &sID, ICCItem *pData, CString *retsText) const

//	Translate
//
//	Translate a message by ID.

	{
	//	Ask the override

	if (m_pOverride && m_pOverride->TranslateText(*this, sID, pData, retsText))
		return true;

	//	Then the type

	CDesignType *pType = GetType();
	if (pType && pType->TranslateText(*this, sID, pData, retsText))
		return true;

	//	Otherwise, see if the sovereign has it

	CSovereign *pSovereign = GetSovereign();
	if (pSovereign && pSovereign->TranslateText(*this, sID, pData, retsText))
		return true;

	//	Otherwise, we can't find it.

	return false;
	}

void CSpaceObject::Update (SUpdateCtx &Ctx)

//	Update
//
//	Update the object

	{
	CUsePerformanceCounter Counter(GetUniverse(), GetUpdatePerformanceID(GetCategory()));

	//	Update as long as we are not time-stopped.

	if (!Ctx.IsTimeStopped())
		{
		//	Update effects

		if (m_pFirstEffect)
			UpdateEffects();

		//	Update items

		if (IsDestinyTime(ITEM_ON_UPDATE_CYCLE, ITEM_ON_UPDATE_OFFSET))
			{
			FireItemOnUpdate();

			//	We could have gotten destroyed here.

			if (IsDestroyed())
				return;
			}

		//	Update object

		CDesignType *pType;
		if (FindEventHandler(CDesignType::evtOnUpdate)
				&& IsDestinyTime(OBJECT_ON_UPDATE_CYCLE, OBJECT_ON_UPDATE_OFFSET)
				&& (pType = GetType())
				//	Skip missiles, because we can't tell the difference between OnUpdate
				//	for the item and OnUpdate for the missile object.
				&& pType->GetType() != designItemType
				&& pType->GetAPIVersion() >= 31)
			{
			FireOnUpdate();

			//	We could have gotten destroyed here, so we check and leave if 
			//	necessary.

			if (IsDestroyed())
				return;
			}

		//	Update the specific object subclass.

		OnUpdate(Ctx, g_SecondsPerUpdate);
		if (IsDestroyed())
			return;
		}

	//	Otherwise, if we're time-stopped we need to update any overlays that
	//	cause time stop (so that they can expire properly).

	else
		{
		COverlayList *pOverlays = GetOverlays();
		if (pOverlays)
			pOverlays->UpdateTimeStopped(this, GetImageScale(), GetRotation());
		}

	//	Now update some variables that are only used by the player but relate
	//	to this object.
	//
	//	NOTE: These should update even if we're time-stopped.

	if (m_iHighlightCountdown > 0)
		{
		if (--m_iHighlightCountdown == 0)
			m_sHighlightText = NULL_STR;
		}

	//	Update some player-needed calculations, such as whether this object is the
	//	nearest target.

	Ctx.UpdatePlayerCalc(*this);

	//	See if we have a dock screen. We only check every 20 ticks or so, so this
	//	information might be stale. Use this for the docking ports animation, but
	//	not for actually determining if we can dock.
	//
	//	NOTE: <GetDockScreen> and <GetGlobalDockScreen> must not have side-
	//	effects, so we assume we cannot be destroyed after the call.

	if (IsDestinyTime(21, 8))
		{
		m_fHasDockScreenMaybe = (CanObjRequestDock(Ctx.GetPlayerShip()) == dockingOK);
		}
	}

void CSpaceObject::UpdateEffects (void)

//	UpdateEffects
//
//	Updates old-style effects

	{
	SEffectMoveCtx MoveCtx;
	MoveCtx.pObj = this;

	SEffectUpdateCtx UpdateCtx(GetUniverse());
	UpdateCtx.pSystem = GetSystem();
	UpdateCtx.pObj = this;

	//	Update the effects

	SEffectNode *pEffect = m_pFirstEffect;
	SEffectNode *pPrev = NULL;
	while (pEffect)
		{
		SEffectNode *pNext = pEffect->pNext;

		if (++pEffect->iTick >= pEffect->pPainter->GetLifetime())
			{
			if (pPrev)
				pPrev->pNext = pNext;
			else
				m_pFirstEffect = pNext;

			pEffect->pPainter->Delete();
			delete pEffect;
			}
		else
			{
			UpdateCtx.iTick = pEffect->iTick;
			UpdateCtx.iRotation = pEffect->iRotation;

			pEffect->pPainter->OnUpdate(UpdateCtx);
			pEffect->pPainter->OnMove(MoveCtx);

			pPrev = pEffect;
			}

		pEffect = pNext;
		}
	}

void CSpaceObject::UpdateExtended (const CTimeSpan &ExtraTime)

//	UpdateExtended
//
//	Update the object after a long time.
	
	{
	UpdateTradeExtended(ExtraTime);

	//	Let subclasses update

	OnUpdateExtended(ExtraTime);
	}

bool CSpaceObject::InvokePower (CPower &Power, CSpaceObject *pTarget)

//	InvokePower
//
//	Invokes the given power.

	{
	if (IsPlayer())
		{
		CString sError;
		Power.InvokeByPlayer(this, pTarget, &sError);
		if (!sError.IsBlank())
			{
			SendMessage(NULL, sError);
			::kernelDebugLogString(sError);
			return false;
			}
		}
	else
		{
		CString sError;
		Power.InvokeByNonPlayer(this, pTarget, &sError);
		if (!sError.IsBlank())
			{
			::kernelDebugLogString(sError);
			return false;
			}
		}

	return true;
	}

bool CSpaceObject::UseItem (const CItem &Item, CString *retsError)

//	UseItem
//
//	Uses the given item

	{
	CCodeChainCtx Ctx(GetUniverse());

	//	If this item is a device and it is installed, get the
	//	installed device structure.

	if (Item.GetType()->IsDevice() && Item.IsInstalled())
		{
		CInstalledDevice *pDevice = FindDevice(Item);

		ASSERT(pDevice);
		if (pDevice)
			{
			//	If the device is disabled, then we can't use it

			if (!pDevice->IsEnabled())
				{
				if (retsError) *retsError = strPatternSubst(CONSTLIT("%s not enabled"), Item.GetNounPhrase(nounCapitalize));
				return false;
				}

			//	If the device is not ready, then we can't use it

			if (!pDevice->IsReady())
				{
				if (retsError) *retsError = strPatternSubst(CONSTLIT("%s not yet recharged"), Item.GetNounPhrase(nounCapitalize));
				return false;
				}

			//	Reset the activation delay, if necessary

			int iActivationDelay = pDevice->GetActivateDelay(this);
			if (iActivationDelay)
				{
				pDevice->SetTimeUntilReady(iActivationDelay);

				if (pDevice->ShowActivationDelayCounter(this))
					OnComponentChanged(comDeviceCounter);
				}
			}
		}

	//	Get the code from the item

	CItemType::SUseDesc UseDesc;
	if (Item.GetType()->GetUseDesc(&UseDesc) && UseDesc.pCode)
		{
		//	Define parameters

		Ctx.DefineContainingType(Item);
		Ctx.SaveAndDefineSourceVar(this);
		Ctx.SaveAndDefineItemVar(Item);
		Ctx.SetScreensRoot(Item.GetType());
		Ctx.SetExtension(UseDesc.pExtension);

		//	Invoke

		ICCItemPtr pResult = Ctx.RunCode(UseDesc.pCode);
		if (pResult->IsError())
			{
			if (retsError) *retsError = pResult->GetStringValue();
			return false;
			}
		}

	return true;
	}

void CSpaceObject::WriteObjRefToStream (CSpaceObject *pObj, IWriteStream *pStream) const

//	WriteObjRefToStream
//
//	Writes an object reference.

	{
	//	If we have a system, save through that. We do this because it has some
	//	extra debug checks.

	if (CSystem *pSystem = GetSystem())
		pSystem->WriteObjRefToStream(pObj, pStream, this);

	//	Otherwise, we can write it without the debug checks.

	else
		CSystem::WriteObjRefToStream(*pStream, pObj);
	}

void CSpaceObject::WriteToStream (IWriteStream *pStream)

//	WriteToStream
//
//	Write the object to a stream
//
//	DWORD		ObjID
//	DWORD		m_iIndex
//	DWORD		m_dwID
//	DWORD		m_iDestiny
//	Vector		m_vPos
//	Vector		m_vVel
//	Metric		m_rBoundsX
//	Metric		m_rBoundsY
//	DWORD		low = unused; hi = m_iHighlightCountdown
//	DWORD		m_pOverride
//	CItemList	m_ItemList
//	DWORD		m_iControlsFrozen
//	DWORD		flags
//	CAttributeDataBlock	m_Data
//	CVector		m_vOldPos (only if !IsAnchored())
//	CSpaceObjectList m_SubscribedObjs
//
//	For each effect:
//	IEffectPainter (0 == no more)
//	DWORD		x
//	DWORD		y
//	DWORD		iTick
//	DWORD		iRotation

	{
	//	Write out the Kernel object ID

	DWORD dwSave = GetClassID();

	//	Save out stuff

	pStream->Write(dwSave);
	pStream->Write(m_iIndex);
	pStream->Write(m_dwID);
	pStream->Write(m_iDestiny);
	m_vPos.WriteToStream(*pStream);
	m_vVel.WriteToStream(*pStream);
	pStream->Write(m_rBoundsX);
	pStream->Write(m_rBoundsY);
	dwSave = MAKELONG((WORD)(BYTE)m_iDesiredHighlightChar, m_iHighlightCountdown);
	pStream->Write(dwSave);

	//	Override

	dwSave = (m_pOverride ? m_pOverride->GetUNID() : 0);
	pStream->Write(dwSave);

	//	Write out the list of items

	m_ItemList.WriteToStream(pStream);

	//	More Data

	dwSave = m_iControlsFrozen;
	pStream->Write(dwSave);

	//	Write out flags

	dwSave = 0;
	dwSave |= (m_fHookObjectDestruction		? 0x00000001 : 0);
	dwSave |= (m_fNoObjectDestructionNotify ? 0x00000002 : 0);
	dwSave |= (m_fCannotBeHit				? 0x00000004 : 0);
	dwSave |= (m_fSelected					? 0x00000008 : 0);
	dwSave |= (m_fInPOVLRS					? 0x00000010 : 0);
	dwSave |= (m_fCanBounce					? 0x00000020 : 0);
	dwSave |= (m_fIsBarrier					? 0x00000040 : 0);
	dwSave |= (IsAnchored()					? 0x00000080 : 0);
	dwSave |= (m_fNoFriendlyFire			? 0x00000100 : 0);
	dwSave |= (m_fTimeStop					? 0x00000200 : 0);
	dwSave |= (m_fPlayerTarget				? 0x00000400 : 0);
	dwSave |= (m_fAutoClearDestinationOnGate ? 0x00000800 : 0);
	dwSave |= (m_fNoFriendlyTarget			? 0x00001000 : 0);
	dwSave |= (m_fPlayerDestination			? 0x00002000 : 0);
	dwSave |= (m_fShowDistanceAndBearing	? 0x00004000 : 0);
	dwSave |= (m_fHasOnObjDockedEvent		? 0x00008000 : 0);
	dwSave |= (m_fHasInterSystemEvent		? 0x00010000 : 0);
	dwSave |= (m_fHasOnDamageEvent			? 0x00020000 : 0);
	dwSave |= (m_fHasOnAttackedEvent		? 0x00040000 : 0);
	dwSave |= (m_fAutoClearDestination		? 0x00080000 : 0);
	dwSave |= (m_fHasOnOrdersCompletedEvent	? 0x00100000 : 0);
	dwSave |= (m_fPlayerDocked				? 0x00200000 : 0);
	dwSave |= (m_fNonLinearMove				? 0x00400000 : 0);
	dwSave |= (m_fAscended					? 0x00800000 : 0);
	dwSave |= (m_fOutOfPlaneObj				? 0x01000000 : 0);
	dwSave |= (m_fAutoClearDestinationOnDock ? 0x02000000 : 0);
	dwSave |= (m_fShowHighlight				? 0x04000000 : 0);
	dwSave |= (m_fAutoClearDestinationOnDestroy ? 0x08000000 : 0);
	dwSave |= (m_fShowDamageBar				? 0x10000000 : 0);
	dwSave |= (m_fHasGravity				? 0x20000000 : 0);
	dwSave |= (m_fInsideBarrier				? 0x40000000 : 0);
	dwSave |= (m_fHasOnSubordinateAttackedEvent	? 0x80000000 : 0);
	//	No need to save m_fHasName because it is set by CSystem on load.
	pStream->Write(dwSave);

	//	More flags

	dwSave = 0;
	dwSave |= (m_fQuestTarget				? 0x00000001 : 0);
	dwSave |= (m_fHasGetDockScreenEvent		? 0x00000002 : 0);
	dwSave |= (m_fHasOnAttackedByPlayerEvent	? 0x00000004 : 0);
	dwSave |= (m_fHasOnOrderChangedEvent	? 0x00000008 : 0);
	dwSave |= (m_fManualAnchor				? 0x00000010 : 0);
	dwSave |= (m_f3DExtra					? 0x00000020 : 0);
	dwSave |= (m_fAutoCreatedPorts			? 0x00000040 : 0);
	dwSave |= (m_fDebugMode					? 0x00000080 : 0);
	pStream->Write(dwSave);

	//	Write out the opaque data

	m_Data.WriteToStream(pStream, m_pSystem);

	//	Write out other stuff

	if (!IsAnchored())
		m_vOldPos.WriteToStream(*pStream);

	//	Subscriptions

	m_SubscribedObjs.WriteToStream(pStream);

	//	Write out the effect list

	SEffectNode *pNext = m_pFirstEffect;
	while (pNext)
		{
		CEffectCreator::WritePainterToStream(pStream, pNext->pPainter);

		pStream->Write(pNext->xOffset);
		pStream->Write(pNext->yOffset);
		pStream->Write(pNext->iTick);
		pStream->Write(pNext->iRotation);

		pNext = pNext->pNext;
		}

	//	Effects list ends in a NULL

	CEffectCreator::WritePainterToStream(pStream, NULL);

	//	Let the subclass write out its part

	OnWriteToStream(pStream);
	}

CString ParseParam (char **ioPos)
	{
	char *pPos = *ioPos;
	if (pPos[1] == ':')
		{
		pPos++;
		pPos++;

		char *pStart = pPos;
		while (*pPos != ';' && *pPos != '\0')
			pPos++;

		//	If we hit the end, we backup one character because our
		//	caller will advance the position by one.

		*ioPos = (*pPos == '\0' ? (pPos - 1) : pPos);

		//	Return the string

		return CString(pStart, pPos - pStart);
		}
	else
		return NULL_STR;
	}

#ifdef DEBUG_VECTOR

void CSpaceObject::PaintDebugVector (CG32bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx)
	{
	if (!m_vDebugVector.IsNull())
		{
		int xDest, yDest;

		Ctx.XForm.Transform(GetPos() + m_vDebugVector, &xDest, &yDest);

		Dest.DrawLine(x, y,
				xDest, yDest,
				3,
				CG32bitPixel(0,255,0));
		}
	}

#endif

