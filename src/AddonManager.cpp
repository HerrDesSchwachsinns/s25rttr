// $Id: AddonManager.cpp 6699 2010-09-01 17:12:06Z jh $
//
// Copyright (c) 2005 - 2010 Settlers Freaks (sf-team at siedler25.org)
//
// This file is part of Return To The Roots.
//
// Return To The Roots is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Return To The Roots is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Return To The Roots. If not, see <http://www.gnu.org/licenses/>.

///////////////////////////////////////////////////////////////////////////////
// Header
#include "main.h"
#include "AddonManager.h"

#include "Settings.h"

#include "AddonLimitCatapults.h"
#include "AddonInexhaustibleMines.h"
#include "AddonRefundMaterials.h"
#include "AddonExhaustibleWells.h"
#include "AddonRefundOnEmergency.h"
#include "AddonManualRoadEnlargement.h"
#include "AddonCharburner.h"

#include "AddonDemolitionProhibition.h"

#include "AddonChangeGoldDeposits.h"
#include "AddonMaxWaterwayLength.h"
#include "AddonCustomBuildSequence.h"
#include "AddonStatisticsVisibility.h"

#include "AddonDefenderBehavior.h"
#include "AddonAIDebugWindow.h"

#include "AddonNoCoinsDefault.h"

///////////////////////////////////////////////////////////////////////////////
/**
 *
 *  @author FloSoft
 */
AddonManager::AddonManager()
{
	// register addons
	reset();
}

///////////////////////////////////////////////////////////////////////////////
/**
 *
 *  @author FloSoft
 */
AddonManager::~AddonManager()
{
	// clear memory and dont register addons again
	reset(false);
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  clears the addon memory.
 *
 *  if @p recreate is @p true then the addons are re-registered 
 *  and set to defaults
 *
 *  @author FloSoft
 */
void AddonManager::reset(bool recreate)
{
	for( std::vector<item>::iterator it = addons.begin(); it != addons.end(); ++it)
		delete it->addon;

	addons.clear();

	if(recreate)
	{
		registerAddon(new AddonLimitCatapults);
		registerAddon(new AddonInexhaustibleMines);
		registerAddon(new AddonRefundMaterials);
		registerAddon(new AddonExhaustibleWells);
		registerAddon(new AddonRefundOnEmergency);
		registerAddon(new AddonManualRoadEnlargement);
		registerAddon(new AddonCharburner);

		registerAddon(new AddonDemolitionProhibition);

		registerAddon(new AddonChangeGoldDeposits);
		registerAddon(new AddonMaxWaterwayLength);
		registerAddon(new AddonCustomBuildSequence);
		registerAddon(new AddonStatisticsVisibility);

		registerAddon(new AddonDefenderBehavior);
		registerAddon(new AddonAIDebugWindow);
		
		registerAddon(new AddonNoCoinsDefault);
	}
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  loads the saved addon configuration from the SETTINGS.
 *
 *  @author FloSoft
 */
void AddonManager::LoadSettings()
{
	reset();

	for( std::map<unsigned int, unsigned int>::iterator it = SETTINGS.addons.configuration.begin(); it != SETTINGS.addons.configuration.end(); ++it)
		setSelection((AddonId)it->first, it->second);
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  saves the current addon configuration to the SETTINGS.
 *
 *  @author FloSoft
 */
void AddonManager::SaveSettings() const
{
	SETTINGS.addons.configuration.clear();
	for( std::vector<item>::const_iterator it = addons.begin(); it != addons.end(); ++it)
		SETTINGS.addons.configuration.insert(std::make_pair(it->addon->getId(), it->status));
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  saves the current addon configuration to a serializer object.
 *
 *  @author FloSoft
 */
void AddonManager::Serialize(Serializer *ser) const
{
	LOG.write(">>> Addon Status:\n");

	ser->PushUnsignedInt(addons.size());
	for( std::vector<item>::const_iterator it = addons.begin(); it != addons.end(); ++it)
	{
		ser->PushUnsignedInt(it->addon->getId());
		ser->PushUnsignedInt(it->status);

		LOG.write("\t%d=%d\n", it->addon->getId(), it->status);
	}
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  reads the current addon configuration from a serializer object.
 *
 *  @author FloSoft
 */
void AddonManager::Deserialize(Serializer *ser)
{
	unsigned int count = ser->PopUnsignedInt();

	reset();

	LOG.write("<<< Addon Status:\n");

	for(unsigned int i = 0; i < count; ++i)
	{
		AddonId addon = AddonId(ser->PopUnsignedInt());
		unsigned int status = ser->PopUnsignedInt();
		setSelection(addon, status);

		LOG.write("\t%d=%d\n", addon, status);
	}
}
