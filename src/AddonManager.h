// $Id$
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
#ifndef ADDONMANAGER_H_INCLUDED
#define ADDONMANAGER_H_INCLUDED

#include "Addons.h"

#pragma once

class AddonManager : public Singleton<AddonManager>
{
public:
	AddonManager();
	~AddonManager();

	void Serialize(Serializer *ser) const;
	void Deserialize(Serializer *ser);

	/// clears the addon memory.
	void reset(bool recreate = true);

	const Addon *getAddon(unsigned int nr, unsigned int &status) const
	{
		if(nr >= addons.size())
			return NULL;

		const item *i = &addons.at(nr);

		if(!i->addon)
			return NULL;

		status = i->status;
		return i->addon;
	}

	unsigned int getCount() const { return addons.size(); }

	bool isEnabled(AddonId id) const
	{
		std::vector<item>::const_iterator it = std::find(addons.begin(), addons.end(), id);
		if(it == addons.end() || it->status == it->addon->getDefaultStatus())
			return false;
		return true;
	}

	unsigned int getSelection(AddonId id) const
	{
		std::vector<item>::const_iterator it = std::find(addons.begin(), addons.end(), id);
		if(it == addons.end())
			return 0;
		return it->status;
	}

	void setSelection(AddonId id, unsigned int selection)
	{
		std::vector<item>::iterator it = std::find(addons.begin(), addons.end(), id);
		if(it == addons.end())
			return;

		it->status = selection;
	}

	/// loads the saved addon configuration from the SETTINGS.
	void LoadSettings();
	/// saves the current addon configuration to the SETTINGS.
	void SaveSettings() const;

private:
	void registerAddon(Addon *addon)
	{
		if(!addon)
			return;

		if(std::find(addons.begin(), addons.end(), addon->getId()) == addons.end())
			addons.push_back(item(addon));
	}

	struct item
	{
		item(void) : addon(NULL), status(0) {}
		item(Addon *addon) : addon(addon), status(addon->getDefaultStatus()) {}

		Addon *addon;
		unsigned int status;

		bool operator==(const AddonId &o) const { return (addon ? addon->getId() == o : false); }
	};

	std::vector<item> addons;
};

#define ADDONMANAGER AddonManager::inst()

#endif // !ADDONMANAGER_H_INCLUDED
