// $Id: AddonDemolitionProhibition.h 6458 2010-05-31 11:38:51Z FloSoft $
//
// Copyright (c) 2005 - 2010 Settlers Freaks (sf-team at siedler25.org)
//
// This file is part of Siedler II.5 RTTR.
//
// Siedler II.5 RTTR is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Siedler II.5 RTTR is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Siedler II.5 RTTR. If not, see <http://www.gnu.org/licenses/>.
#ifndef ADDONDEMOLITIONPROHIBITION_H_INCLUDED
#define ADDONDEMOLITIONPROHIBITION_H_INCLUDED

#pragma once

#include "Addons.h"

///////////////////////////////////////////////////////////////////////////////
/**
 *  Addon for changing the maximum length of waterways.
 *
 *  @author Divan
 */
class AddonDemolitionProhibition : public AddonList
{
public:
	AddonDemolitionProhibition() : AddonList(ADDON_DEMOLITION_PROHIBITION, 
											 ADDONGROUP_MILITARY,
											 gettext_noop("Disable Demolition of military buildings"), 
											 gettext_noop("Allows to disable the demolition of military buildings under attack or near frontiers"),
											 0
										   )
	{
		addOption(gettext_noop("Off"));
		addOption(gettext_noop("Active if attacked"));
		addOption(gettext_noop("Active near frontiers"));	
	}
};

#endif // !ADDONDEMOLITIONPROHIBITION_H_INCLUDED
