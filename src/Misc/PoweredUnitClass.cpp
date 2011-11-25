#include <BuildingClass.h>
#include <HouseClass.h>
#include <GeneralStructures.h>
#include "../Ext/Building/Body.h"
#include "../Ext/TechnoType/Body.h"
#include "Debug.h"
#include "EMPulse.h"
#include "PoweredUnitClass.h"

bool PoweredUnitClass::IsPoweredBy(HouseClass* Owner) const
{
	for(int i = 0; i < Owner->Buildings.Count; ++i)	{
		auto Building  = Owner->Buildings.GetItem(i);
		auto BExt = TechnoExt::ExtMap.Find(Building);
		auto inArray = this->Ext->PoweredBy.FindItemIndex(&Building->Type) != -1;

		if(inArray && !Building->BeingWarpedOut && !Building->IsUnderEMP() && BExt->IsOperated() && Building->IsPowerOnline()) {
			return true;
		}
	}
	
	return false;
}

void PoweredUnitClass::PowerUp()
{
	TechnoExt::ExtData* e = TechnoExt::ExtMap.Find(this->Techno);
	if( !this->Techno->IsUnderEMP() && e->IsOperated() ) {
		EMPulse::DisableEMPEffect2(this->Techno);
	}
}

void PoweredUnitClass::PowerDown()
{
	if( EMPulse::IsDeactivationAdvisable(this->Techno) && !EMPulse::EnableEMPEffect2(this->Techno) ) {
		// for EMP.Threshold=inair
		if( this->Ext->EMP_Threshold < 0 && this->Techno->IsInAir() )	{
			this->Techno->Destroyed(NULL);
			this->Techno->Crash(NULL);
			
			if (this->Techno->Owner == HouseClass::Player) {
				VocClass::PlayAt(this->Techno->GetTechnoType()->VoiceCrashing, &this->Techno->Location, NULL);
			}
		}
	}
}

void PoweredUnitClass::Update()
{
	if( (Unsorted::CurrentFrame - this->LastScan) < this->ScanInterval ) return;
	
	HouseClass* Owner = this->Techno->Owner;
	bool HasPower     = this->IsPoweredBy(Owner);
	
	this->Powered = HasPower;
	
	if(HasPower && this->Techno->Deactivated) {
		this->PowerUp();
	} else if(!HasPower && !this->Techno->Deactivated) {
		// don't shutdown units inside buildings (warfac, barracks, shipyard) because that locks up the factory and the robot tank did it
		auto WhatAmI = this->Techno->WhatAmI();
		if((WhatAmI != InfantryClass::AbsID && WhatAmI != UnitClass::AbsID) || (!this->Techno->GetCell()->GetBuilding())) {
			this->PowerDown();
		}
	}
	
	LastScan = Unsorted::CurrentFrame;
}