// $Id: nofCarrier.cpp 5853 2010-01-04 16:14:16Z FloSoft $
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

///////////////////////////////////////////////////////////////////////////////
// Header
#include "main.h"
#include "nofCarrier.h"

#include "GameWorld.h"
#include "Random.h"
#include "SerializedGameData.h"
#include "GameClient.h"
#include "GameClientPlayer.h"
#include "SoundManager.h"

#include "Ware.h"
#include "RoadSegment.h"
#include "noRoadNode.h"
#include "noFlag.h"
#include "noBaseBuilding.h"


///////////////////////////////////////////////////////////////////////////////
// Makros / Defines
#if defined _WIN32 && defined _DEBUG && defined _MSC_VER
	#define new new(_NORMAL_BLOCK, THIS_FILE, __LINE__)
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
#endif

///////////////////////////////////////////////////////////////////////////////
// Konstanten

/// Zeitabstand, in dem die Produktivit�t vom Tr�ger gemessen wird
const unsigned PRODUCTIVITY_GF = 6000;
/// Ab wieviel Prozent Auslastung in Prozent eines Tr�gers ein Esel kommen soll
const unsigned DONKEY_PRODUCTIVITY = 80;
/// Positionen der Ware f�r jede Richtung beim Zeichnen f�r Esel und Boot
const int WARE_POS_DONKEY[96] = 
{
	0,-13, 0,-12, 0,-12, 1,-13, 2,-13, 2,-12, 2,-12, 1,-13,
	3,-12, 3,-11, 3,-11, 3,-12, 4,-12, 5,-11, 5,-11, 5,-12,
	2,-9, 1,-8, 0,-7, 1,-8, 2,-9, 3,-8, 4,-8, 3,-9,
	-1,-8, -2,-7, -3,-7, -2,-8, -1,-8, 0,-7, 1,-7, 0,-8,
	-3,-7, -2,-6, -1,-6, -1,-7, -2,-7, -3,-6, -4,-6, -3,-7,
	-3,-10, -3,-9, -3,-9, -2,-10, -1,-10, -1,-10, -1,-9, -1,-10,
};
/// Positionen der Ware im Boat f�r jede Richtung
const int WARE_POS_BOAT[12] =
{
	11,-4, 11,0, -7,-1, -8,-5, -7,-7, 6,-7
};


/// Abstand zur n�chsten Animation (Wert ergibt sich aus NEXT_ANIMATION + rand(NEXT_ANIMATION_RANDOM) )
const unsigned NEXT_ANIMATION = 200; // fest
const unsigned NEXT_ANIMATION_RANDOM = 200; // was noch dazu zuf�lliges addiert wird


/// L�ngen in Frames der Animationenen
const unsigned ANIMATION_FRAME_LENGTHS[2][4] =
{ {11,10,35,20}, {9,12,12,13} };
/// Dauer in GF eines Frames
const unsigned FRAME_GF = 3;

/// Animations-Index (von map.lst)
const unsigned short ANIMATION[2][4][35] =
{
	{
		// d�nn
		{1745,1746,1747,1748,1749,1750,1751,1748,1748,1747,1746}, // Reifenspringen
		// d�nn
		{1752,1753,1754,1755,1756,1757,1758,1754,1753,1752}, // Winken
		// d�nn
		{1759,1760,1761,1762,1763,1763,1763,1765,1763,1763, // Zeitung lesen
		 1763,1765,1763,1762,1765,1763,1764,1764,1763,1763,
		 1763,1765,1765,1765,1763,1763,1763,1765,1763,1763,
		 1763,1765,1765,1764,1761}, 
		// d�nn 
		{1752,1770,1771,1772,1773,1772,1773,1772,1773,1772,1773,1771,1771,1773,1771,1771,1771,1771,1770,1752} // G�hnen
	},
	{
		// dick
		{1726,1727,1728,1729,1730,1730,1729,1728,1727}, // Niesen?
		// dick
		{1731,1732,1733,1734,1734,1735,1736,1737,1737,1736,1736,1737}, // Kaugummi essen
		// dick
		{1738,1739,1740,1739,1738,1739,1740,1739,1741,1742,1743,1744}, // Kaugummi-Blasen machen
		// dick
		{1726,1766,1767,1768,1769,1768,1769,1768,1769,1766,1767,1766,1726}, // auf Hosentasche klopfen?!
	}
};

///////////////////////////////////////////////////////////////////////////////
/**
 *  Konstruktor von @p nofCarrier.
 *
 *  @author OLiver
 */

const Job JOB_TYPES[3] = { JOB_HELPER, JOB_PACKDONKEY, JOB_BOATCARRIER };

nofCarrier::nofCarrier(const CarrierType ct, unsigned short x,
					   unsigned short y,
					   unsigned char player,
					   RoadSegment *workplace,
					   noRoadNode *const goal)
					   : noFigure(JOB_TYPES[ct], x, y, player, goal), ct(ct),
	state(CARRS_FIGUREWORK), fat( ( RANDOM.Rand(__FILE__,__LINE__,obj_id,2) ? true : false) ),
	workplace(workplace), carried_ware(NULL), productivity_ev(0),
	productivity(0), worked_gf(0), since_working_gf(0xFFFFFFFF), next_animation(0)
{
}

///////////////////////////////////////////////////////////////////////////////
/**
 *
 *
 *  @author OLiver
 */
nofCarrier::nofCarrier(SerializedGameData *sgd, unsigned int obj_id)
	: noFigure(sgd,obj_id),
	ct( CarrierType(sgd->PopUnsignedChar()) ),
	state( CarrierState(sgd->PopUnsignedChar()) ),
	fat( sgd->PopBool() ),
	workplace( sgd->PopObject<RoadSegment>(GOT_ROADSEGMENT) ),
	carried_ware( sgd->PopObject<Ware>(GOT_WARE) ),
	productivity_ev(sgd->PopObject<EventManager::Event>(GOT_EVENT)),
	productivity(sgd->PopUnsignedInt()),
	worked_gf(sgd->PopUnsignedInt()),
	since_working_gf(sgd->PopUnsignedInt()),
	next_animation(0)
{
}

///////////////////////////////////////////////////////////////////////////////
/**
 *
 *
 *  @author OLiver
 */
void nofCarrier::Serialize_nofCarrier(SerializedGameData * sgd) const
{
	Serialize_noFigure(sgd);

	sgd->PushUnsignedChar(static_cast<unsigned char>(ct));
	sgd->PushUnsignedChar(static_cast<unsigned char>(state));
	sgd->PushBool(fat);
	sgd->PushObject(workplace, true);
	sgd->PushObject(carried_ware, true);
	sgd->PushObject(productivity_ev,true);
	sgd->PushUnsignedInt(productivity);
	sgd->PushUnsignedInt(worked_gf);
	sgd->PushUnsignedInt(since_working_gf);
}

///////////////////////////////////////////////////////////////////////////////
/**
 *
 *
 *  @author OLiver
 */
nofCarrier::~nofCarrier()
{
	// Ware vernichten (physisch)
	delete carried_ware;
}



///////////////////////////////////////////////////////////////////////////////
/**
 *
 *
 *  @author OLiver
 */
void nofCarrier::Destroy_nofCarrier()
{
	// Ware vernichten (abmelden)
	if(carried_ware)
	{
		gwg->GetPlayer(player)->RemoveWare(carried_ware);
		gwg->GetPlayer(player)->DecreaseInventoryWare(carried_ware->type,1);
	}

	em->RemoveEvent(productivity_ev);

	Destroy_noFigure();
}


///////////////////////////////////////////////////////////////////////////////
/**
 *
 *
 *  @author OLiver
 */
void nofCarrier::Draw(int x, int y)
{
	// Unterscheiden, um was f�r eine Art von Tr�ger es sich handelt
	switch(ct)
	{
	case CT_NORMAL:
		{
			if(state == CARRS_WAITFORWARE || (waiting_for_free_node && !pause_walked_gf && !carried_ware))
			{
				bool animation = false;

				// Ist es schon Zeit f�r eine Animation?
				unsigned current_gf = GameClient::inst().GetGFNumber();
				if(current_gf >= next_animation)
				{
					// Animationstype bestimmen
					unsigned animation_id = next_animation%4;

					// Ist die Animation schon vorbei?
					if(current_gf >= next_animation+ANIMATION_FRAME_LENGTHS[fat?1:0][animation_id]*FRAME_GF)
						// Neuen n�chsten Animationszeitpunkt bestimmen
						SetNewAnimationMoment();
					else
					{
						animation = true;

						// Nein, dann Animation abspielen
						LOADER.GetImageN("rom_bobs", ANIMATION[fat?1:0][animation_id][(current_gf-next_animation)/FRAME_GF])
							->Draw(x,y,0,0,0,0,0,0,COLOR_WHITE, COLORS[gwg->GetPlayer(player)->color]);
					}
				}

				if(!animation)
					// Steht und wartet (ohne Ware)
					LOADER.GetBobN("jobs")->Draw(0,dir,fat,2,x,y,COLORS[gwg->GetPlayer(player)->color]);

				DrawShadow(x,y,0,dir);
			}
			else if(state == CARRS_WAITFORWARESPACE || (waiting_for_free_node && !pause_walked_gf && carried_ware))
			{
				// Steht und wartet (mit Ware)
				// Japaner-Schild-Animation existiert leider nicht --> R�merschild nehmen
				LOADER.GetBobN("carrier")->Draw((carried_ware->type==GD_SHIELDJAPANESE)?GD_SHIELDROMANS:carried_ware->type,
					dir,fat,2,x,y,COLORS[gwg->GetPlayer(player)->color]);
				DrawShadow(x,y,0,dir);
			}
			else
			{
				// L�uft normal mit oder ohne Ware
				if(carried_ware)
					DrawWalking(x,y,LOADER.GetBobN("carrier"),(carried_ware->type==GD_SHIELDJAPANESE)?GD_SHIELDROMANS:carried_ware->type,fat);
				else
					DrawWalking(x,y,LOADER.GetBobN("jobs"),0,fat);
			}
		} break;
	case CT_DONKEY:
		{
	
			if(state == CARRS_WAITFORWARE || (waiting_for_free_node && !pause_walked_gf && !carried_ware))
			{
				// Steht und wartet (ohne Ware)

				// Esel
				LOADER.GetMapImageN(2000+((dir+3)%6)*8)->Draw(x,y);
				// Schatten des Esels
				LOADER.GetMapImageN(2048+dir%3)->Draw(x,y,0,0,0,0,0,0,COLOR_SHADOW);
			}
			else if(state == CARRS_WAITFORWARESPACE || (waiting_for_free_node && !pause_walked_gf && carried_ware))
			{
				//// Steht und wartet (mit Ware)
				//// Japaner-Schild-Animation existiert leider nicht --> R�merschild nehmen

				// Esel
				LOADER.GetMapImageN(2000+(((dir)+3)%6)*8)->Draw(x,y);
				// Schatten des Esels
				LOADER.GetMapImageN(2048+(dir)%3)->Draw(x,y,0,0,0,0,0,0,COLOR_SHADOW);

		
				// Ware im Korb zeichnen
				LOADER.GetMapImageN(2350+carried_ware->type)
					->Draw(x+WARE_POS_DONKEY[(dir)*16],y+WARE_POS_DONKEY[(dir)*16+1]);
			}
			else
			{
				// Wenn wir warten auf ein freies Pl�tzchen, m�ssen wir den stehend zeichnen!
				// Wenn event = 0, dann sind wir mittem auf dem Weg angehalten!
				unsigned ani_step = waiting_for_free_node?2:GAMECLIENT.Interpolate(ASCENT_ANIMATION_STEPS[ascent],current_ev)%8;
				
				CalcFigurRelative(x,y);

				// L�uft normal mit oder ohne Ware
			
				// Esel
				LOADER.GetMapImageN(2000+((dir+3)%6)*8+ani_step)->Draw(x,y);
				// Schatten des Esels
				LOADER.GetMapImageN(2048+dir%3)->Draw(x,y,0,0,0,0,0,0,COLOR_SHADOW);

				if(carried_ware)
				{
					// Ware im Korb zeichnen
					LOADER.GetMapImageN(2350+carried_ware->type)
						->Draw(x+WARE_POS_DONKEY[dir*16+ani_step*2],y+WARE_POS_DONKEY[dir*16+ani_step*2+1]);
				}
			}
			
		//			char number[256];
		//sprintf(number,"%u",obj_id);
		//NormalFont->Draw(x,y,number,0,0xFFFF0000);

		} break;
	case CT_BOAT:
		{
			if(state == CARRS_FIGUREWORK)
			{
				// Beim normalen Laufen Tr�ger mit Boot �ber den Schultern zeichnen
				DrawWalking(x,y,LOADER.GetBobN("carrier"),GD_BOAT,fat);
			}
			else if(state == CARRS_WAITFORWARE || (waiting_for_free_node && !pause_walked_gf && !carried_ware))
			{
				// Steht und wartet (ohne Ware)
				LOADER.GetImageN("boat", ((dir+3)%6)*8)->Draw(x,y,0,0,0,0,0,0,COLOR_WHITE, COLORS[gwg->GetPlayer(player)->color]);
				// Schatten des Boots (Eselschatten)
				LOADER.GetMapImageN(2048+dir%3)->Draw(x,y,0,0,0,0,0,0,COLOR_SHADOW);
			}
			else if(state == CARRS_WAITFORWARESPACE || (waiting_for_free_node && !pause_walked_gf && carried_ware))
			{
				// Steht und wartet (ohne Ware)
				LOADER.GetImageN("boat", (((dir)+3)%6)*8)->Draw(x,y,0,0,0,0,0,0,COLOR_WHITE, COLORS[gwg->GetPlayer(player)->color]);
				// Schatten des Boots (Eselschatten)
				LOADER.GetMapImageN(2048+(dir)%3)->Draw(x,y,0,0,0,0,0,0,COLOR_SHADOW);
		
				// Ware im Boot zeichnen
				LOADER.GetMapImageN(2350+carried_ware->type)
					->Draw(x+WARE_POS_BOAT[(dir)*2],y+WARE_POS_BOAT[(dir)*2+1]);
			}
			else
			{
				// Wenn wir warten auf ein freies Pl�tzchen, m�ssen wir den (fest)stehend zeichnen!
				// Wenn event = 0, dann sind wir mittem auf dem Weg angehalten!
				unsigned ani_step = waiting_for_free_node?2:GAMECLIENT.Interpolate(ASCENT_ANIMATION_STEPS[ascent],current_ev)%8;
			
				CalcFigurRelative(x,y);		

				// ruderndes Boot zeichnen
				LOADER.GetImageN("boat", ((dir+3)%6)*8+ani_step)->Draw(x,y,0,0,0,0,0,0,COLOR_WHITE, COLORS[gwg->GetPlayer(player)->color]);
				// Schatten des Boots (Eselschatten)
				LOADER.GetMapImageN(2048+dir%3)->Draw(x,y,0,0,0,0,0,0,COLOR_SHADOW);

				// L�uft normal mit oder ohne Ware
				if(carried_ware)
					// Ware im Boot zeichnen
					LOADER.GetMapImageN(2350+carried_ware->type)
						->Draw(x+WARE_POS_BOAT[dir*2],y+WARE_POS_BOAT[dir*2+1]);

				// Sound ggf. abspielen
				if(ani_step == 2)
					SoundManager::inst().PlayNOSound(84,this,0);

				last_id = ani_step;
			}

		} break;
	}
}

/// Bestimmt neuen Animationszeitpunkt
void nofCarrier::SetNewAnimationMoment()
{
	next_animation = GameClient::inst().GetGFNumber()+NEXT_ANIMATION+rand()%NEXT_ANIMATION_RANDOM;
}

///////////////////////////////////////////////////////////////////////////////
/**
 *
 *
 *  @author OLiver
 */
void nofCarrier::Walked()
{
	// Bootssounds ggf. l�schen
	if(ct == CT_BOAT && state != CARRS_FIGUREWORK)
		SoundManager::inst().WorkingFinished(this);

	switch(state)
	{
	default:
		break;
	case CARRS_GOTOMIDDLEOFROAD:
		{
			// Gibts an der Flagge in der entgegengesetzten Richtung, in die ich laufe, evtl Waren zu tragen
			// (da wir dar�ber nicht unmittelbar informiert werden!)
			if(workplace->AreWareJobs(rs_dir,ct,false))
			{
				// Dann umdrehen und holen
				rs_dir = !rs_dir;
				rs_pos = workplace->GetLength() - rs_pos;
				state = CARRS_FETCHWARE;

				StartWalking(cur_rs->GetDir(rs_dir,rs_pos));
			}
			else if(rs_pos == cur_rs->GetLength()/2 || rs_pos == cur_rs->GetLength()/2 + cur_rs->GetLength()%2)
			{
				// Wir sind in der Mitte angekommen
				state = CARRS_WAITFORWARE;
				if(dir == 0 || dir == 1 || dir == 5)
					dir = 5;
				else
					dir = 4;

				current_ev = 0;

				// Jetzt wird wieder nur rumgegammelt, dann kriegen wir aber evtl keinen sch�nen IH-AH!
				StopWorking();

				// Animation auf sp�ter verschieben, damit die nicht mittendrin startet
				SetNewAnimationMoment();
			}
			else
			{
				// Eventuell laufen wir in die falsche Richtung?
				if(rs_pos > cur_rs->GetLength()/2)
					{
					rs_dir = !rs_dir;
					rs_pos = cur_rs->GetLength() - rs_pos;
				}

				StartWalking(cur_rs->GetDir(rs_dir,rs_pos));
			}
		} break;
	case CARRS_FETCHWARE:
		{
			// Zur Flagge laufen, um die Ware zu holen

			// Sind wir schon da?
			if(rs_pos == cur_rs->GetLength())
				// Dann Ware aufnehmnen
				FetchWare(false);
			else
				StartWalking(cur_rs->GetDir(rs_dir,rs_pos));


		} break;
	case CARRS_CARRYWARE:
		{
			// Sind wir schon da?
			if(rs_pos == cur_rs->GetLength())
			{

				// Flagge, an der wir gerade stehen
				noFlag * this_flag  = static_cast<noFlag*>(((rs_dir) ? workplace->f1 : workplace->f2));

				// Will die Waren jetzt gleich zur Baustelle neben der Flagge?
				if(WantInBuilding())
				{
					// Erst noch zur Baustelle bzw Geb�ude laufen
					state = CARRS_CARRYWARETOBUILDING;
					StartWalking(1);
					cur_rs = this_flag->routes[1];
					// location wird immer auf n�chste Flagge gesetzt --> in dem Fall aktualisieren
					carried_ware->Carry((cur_rs->f1 == this_flag)? cur_rs->f2 : cur_rs->f1);
				}
				else
				{
					// Ist an der Flagge noch gen�gend Platz (wenn wir wieder eine Ware mitnehmen, kann sie auch voll sein)
					if(this_flag->IsSpaceForWare())
					{
						carried_ware->LieAtFlag(this_flag);

						// Ware soll ihren weiteren Weg berechnen
						carried_ware->RecalcRoute();

						// Ware ablegen
						this_flag->AddWare(carried_ware);
						// Wir tragen erstmal keine Ware mehr
						carried_ware = 0;
						// Gibts an den Flaggen etwas, was ich tragen muss, ansonsten wieder in die Mitte gehen und warten
						LookForWares();
					}
					else if(workplace->AreWareJobs(!rs_dir,ct,true))
					{
						// die Flagge ist voll, aber wir k�nnen eine Ware mitnehmen, daher erst Ware nehmen und dann erst ablegen

						// Ware "merken"
						Ware * tmp_ware = carried_ware;
						// neue Ware aufnehmen
						FetchWare(true);

						// alte Ware ablegen
						tmp_ware->LieAtFlag(this_flag);
						tmp_ware->RecalcRoute();
						this_flag->AddWare(tmp_ware);
					}
					else
					{
						// wenn kein Platz mehr ist --> wieder umdrehen und zur�ckgehen
						state = CARRS_GOBACKFROMFLAG;
						rs_dir = !rs_dir;
						rs_pos = cur_rs->GetLength()-rs_pos;
						StartWalking((dir+3)%6);
					}
				}
			}
			else if(rs_pos == cur_rs->GetLength()-1)
			{
				// Wenn wir fast da sind, gucken, ob an der Flagge noch ein freier Platz ist
				noFlag * this_flag  = static_cast<noFlag*>(((rs_dir) ? workplace->f1 : workplace->f2));

				if(this_flag->IsSpaceForWare() || WantInBuilding() || cur_rs->AreWareJobs(!rs_dir,ct,true))
				{
					// Es ist Platz, dann zur Flagge laufen
					StartWalking(cur_rs->GetDir(rs_dir,rs_pos));
				}
				else
				{
					// Wenn kein Platz ist, stehenbleiben und warten!
					state = CARRS_WAITFORWARESPACE;
					dir=cur_rs->GetDir(rs_dir,rs_pos);
				}
			}
			else
			{
				StartWalking(cur_rs->GetDir(rs_dir,rs_pos));
			}

		} break;
	case CARRS_CARRYWARETOBUILDING:
		{
			// Ware ablegen
			gwg->GetSpecObj<noRoadNode>(x,y)->AddWare(carried_ware);
			// Ich trag' keine Ware mehr
			carried_ware = 0;
			// Wieder zur�ck zu meinem Weg laufen
			state = CARRS_LEAVEBUILDING;
			StartWalking(4);
		} break;
	case CARRS_LEAVEBUILDING:
		{
			// So tun, als ob der Tr�ger gerade vom anderen Ende des Weges kommt, damit alles korrekt funktioniert
			cur_rs = workplace;
			dir = workplace->GetDir(rs_dir,workplace->GetLength()-1);
			LookForWares();
		} break;
	case CARRS_GOBACKFROMFLAG:
		{
			// Wieder umdrehen und so tun, als w�ren wir gerade normal angekommen
			rs_dir = !rs_dir;
			rs_pos = cur_rs->GetLength()-rs_pos;
			state = CARRS_CARRYWARE;
			Walked();
		} break;
	}
}

///////////////////////////////////////////////////////////////////////////////
/**
 *
 *
 *  @author OLiver
 */
void nofCarrier::LookForWares()
{
	// Gibts an dieser Flagge etwas, das ich tragen muss?
	if(workplace->AreWareJobs(!rs_dir,ct,true))
	{
		// Dann soll das CARRS_FETCHWARE �bernehmen
		FetchWare(false);
	}
	else if(workplace->AreWareJobs(rs_dir,ct,false))
	{
		// Oder evtl auf der anderen Seite?
		state = CARRS_FETCHWARE;
		rs_dir = !rs_dir;
		rs_pos = 0;
		StartWalking(cur_rs->GetDir(rs_dir,rs_pos));
	}
	else
	{
		// Wieder zur�ck in die Mitte gehen
		state = CARRS_GOTOMIDDLEOFROAD;
		rs_dir = !rs_dir;
		rs_pos = 0;
		StartWalking(cur_rs->GetDir(rs_dir,rs_pos));
	}
}

///////////////////////////////////////////////////////////////////////////////
/**
 *
 *
 *  @author OLiver
 */
void nofCarrier::GoalReached()
{
	// Erstes Produktivit�tsevent anmelden
	productivity_ev = em->AddEvent(this,PRODUCTIVITY_GF,1);
	// Wir arbeiten schonmal
	StartWorking();

	noRoadNode * rn = gwg->GetSpecObj<noRoadNode>(x,y);
	for(unsigned char i = 0;i<6;++i)
	{
		//noRoadNode * rn = gwg->GetSpecObj<noRoadNode>(x,y);
		if(rn->routes[i] == workplace)
		{
			// Am neuen Arbeitsplatz angekommen
			StartWalking(i);
			cur_rs = workplace;
			rs_pos = 0;
			rs_dir = (rn == cur_rs->f1) ? false : true;

			state = CARRS_GOTOMIDDLEOFROAD;

			// Wenn hier schon Waren liegen, diese gleich transportieren
			if(workplace->AreWareJobs(rs_dir,ct,true))
			{
				// Ware aufnehmen
				carried_ware = static_cast<noFlag*>(rn)->SelectWare(dir,false,this);

				carried_ware->Carry((rs_dir)?workplace->f1:workplace->f2);

				if(carried_ware != NULL)
					state = CARRS_CARRYWARE;
			}
			// wenn was an der gegen�berliegenden Flaggge liegt, ebenfalls holen
			else if(workplace->AreWareJobs(!rs_dir,ct,false))
				state = CARRS_FETCHWARE;
			return;
		}
	}

	LOG.lprintf("nofCarrier::GoalReached: ERROR: Road of carrier (id: %u) not found!\n",obj_id);
}

///////////////////////////////////////////////////////////////////////////////
/**
 *
 *
 *  @author OLiver
 */
void nofCarrier::AbrogateWorkplace()
{
	if(workplace)
	{
		em->RemoveEvent(productivity_ev);
		productivity_ev = 0;

		// anderen Tr�ger herausfinden
		unsigned other = (ct == CT_DONKEY) ? 0 : 1;

		// wenn ich in ein Geb�ude gegangen bin und dann vom Weg geworfen wurde, muss der andere
		// ggf. die Waren tragen, die ich jetzt nicht mehr tragen kann
		if((state == CARRS_LEAVEBUILDING || state == CARRS_CARRYWARETOBUILDING) && workplace->carrier[other])
		{
			if(workplace->AreWareJobs(false,ct,true))
				workplace->carrier[other]->AddWareJob(workplace->f1);
			else if(workplace->AreWareJobs(true,ct,true))
				workplace->carrier[other]->AddWareJob(workplace->f2);
		}


		workplace->CarrierAbrogated(this);
		workplace = 0;
		 // Wenn ich noch ne Ware in der Hand habe, muss die gel�scht werden
		 if(carried_ware)
		 {
			carried_ware->WareLost(player);
			delete carried_ware;
			carried_ware = 0;
		 }
	}
}

///////////////////////////////////////////////////////////////////////////////
/**
 *
 *
 *  @author OLiver
 */
void nofCarrier::LostWork()
{
	 workplace = 0;
	 em->RemoveEvent(productivity_ev);
	 productivity_ev = 0;

	 if(state == CARRS_FIGUREWORK)
		 GoHome();
	 else
	 {
		 // Wenn ich noch ne Ware in der Hand habe, muss die gel�scht werden
		 if(carried_ware)
		 {
			carried_ware->WareLost(player);
			delete carried_ware;
			carried_ware = 0;
		 }

		 StartWandering();
		 if(state == CARRS_WAITFORWARE || state == CARRS_WAITFORWARESPACE)
			 Wander();
	 }
	 state = CARRS_FIGUREWORK;
}

///////////////////////////////////////////////////////////////////////////////
/**
 *
 *
 *  @author OLiver
 */
void nofCarrier::RoadSplitted(RoadSegment * rs1, RoadSegment * rs2)
{
	// Bin ich schon auf meinem Arbeitsplatz (=Stra�e) oder bin ich erst noch auf dem Weg dorthin?
	if(state == CARRS_FIGUREWORK)
	{
		// ich gehe erst noch hin, also gucken, welche Flagge ich anvisiert habe und das jeweilige Teilst�ck dann als Arbeitsstra�e
		if(GetGoal() == rs1->f1)
			workplace = rs1;
		else
			workplace = rs2;
	}
	else if(state == CARRS_CARRYWARETOBUILDING || state == CARRS_LEAVEBUILDING)
	{
		// Wenn ich in ein Geb�ude gehen oder rauskomme, auf den Weg gehen, der an dieses Geb�ude grenzt
		if(cur_rs->f1 == rs1->f1 || cur_rs->f1 == rs1->f2)
			workplace = rs1;
		else
			workplace = rs2;
	}
	else
	{
		// sonst wurde es ja schon entschieden
		workplace = (cur_rs == rs1) ? rs1 : rs2;
	}

	// Sonstige Sachen f�r jeweilige States unternehmen
	switch(state)
	{
	default:
		break;
	case CARRS_WAITFORWARE:
		{
			// Wenn wir stehen, m�ssen wir in die Mitte laufen
			state = CARRS_GOTOMIDDLEOFROAD;
			Walked();
		} break;
	case CARRS_FETCHWARE:
		{
			// Wenn wir zur 2. Flagge vom 1. Wegst�ck gelaufen sind, k�nnen wir das nun vergessen
			if(!workplace->AreWareJobs(!rs_dir,ct,false))
				state = CARRS_GOTOMIDDLEOFROAD;
		} break;
	}

	// Mich als Tr�ger f�r meinen neuen Arbeitsplatz zuweisen
	workplace->carrier[ct==CT_DONKEY?1:0] = this;

	// F�r andere Stra�e neuen Tr�ger/Esel rufen
	RoadSegment * uc_road = ((rs1==workplace)?(rs2):(rs1));
	uc_road->carrier[ct==CT_DONKEY?1:0] = 0;

	if(ct == CT_NORMAL)
		gwg->GetPlayer(player)->FindCarrierForRoad(uc_road);
	else if(ct == CT_DONKEY)
		uc_road->carrier[1] = gwg->GetPlayer(player)->OrderDonkey(uc_road);

}

///////////////////////////////////////////////////////////////////////////////
/**
 *
 *
 *  @author OLiver
 */
void nofCarrier::HandleDerivedEvent(const unsigned int id)
{
	switch(id)
	{
	// Produktivit�tsevent
	case 1:
		{
			productivity_ev = 0;

			// Gucken, ob bis jetzt gearbeitet wurde/wird oder nicht, je nachdem noch was dazuz�hlen
			if(since_working_gf != 0xFFFFFFFF)
			{
				// Es wurde bis jetzt nicht mehr gearbeitet, das also noch dazuz�hlen
				worked_gf += static_cast<unsigned short>(GAMECLIENT.GetGFNumber()-since_working_gf);
				// Z�hler zur�cksetzen
				since_working_gf = GAMECLIENT.GetGFNumber();
			}

			// Produktivit�t ausrechnen
			productivity = worked_gf*100/PRODUCTIVITY_GF;

			// Z�hler zur�cksetzen
			worked_gf = 0;

			// N�chstes Event anmelden
			productivity_ev = em->AddEvent(this,PRODUCTIVITY_GF,1);

			// Reif f�r einen Esel?
			if(productivity >= DONKEY_PRODUCTIVITY && ct == CT_NORMAL)
				workplace->UpgradeDonkeyRoad();

		} break;
	}
}

///////////////////////////////////////////////////////////////////////////////
/**
 *
 *
 *  @author OLiver
 */
bool nofCarrier::AddWareJob(const noRoadNode * rn)
{
	// Wenn wir rumstehen, sollten wir mal loslaufen! ^^und ggf umdrehen, genauso wie beim Laufen in die Mitte
	if(state == CARRS_WAITFORWARE || state == CARRS_GOTOMIDDLEOFROAD)
	{
		// Stimmt die Richtung nicht? Dann umdrehen (geht aber nur, wenn wir stehen!)
		if(rs_dir == workplace->GetNodeID(rn) && state == CARRS_WAITFORWARE)
		{
			rs_dir = !rs_dir;
			// wenn wir zur Mitte laufen, m�ssen noch 2 von der pos abgezogen werden wegen dem Laufen
			rs_pos = cur_rs->GetLength()-rs_pos-((state == CARRS_GOTOMIDDLEOFROAD) ? 2 : 0);
		}
		// beim Gehen in die Mitte nicht sofort umdrehen!
		else if(rs_dir == workplace->GetNodeID(rn))
			return false;

		// Und loslaufen, wenn wir stehen
		if(state == CARRS_WAITFORWARE)
		{
			StartWalking(cur_rs->GetDir(rs_dir,rs_pos));
			// Endlich wird wieder ordentlich gearbeitet!
			StartWorking();
		}


		state = CARRS_FETCHWARE;

		// Wir �bernehmen den Job
		return true;
	}
	else if(state == CARRS_WAITFORWARESPACE && rs_dir == !workplace->GetNodeID(rn))
	{
		// Wenn wir auf einen freien Platz warten, k�nnen wir nun losgehen, da wir ja die Waren dann "tauschen" k�nnen
		StartWalking(cur_rs->GetDir(rs_dir,rs_pos));
		state = CARRS_CARRYWARE;

		// Wir �bernehmen den Job
		return true;
	}

	// Wir �bernehmen den Job nicht
	return false;

}

///////////////////////////////////////////////////////////////////////////////
/**
 *
 *
 *  @author OLiver
 */
void nofCarrier::RemoveWareJob()
{
	if(state == CARRS_FETCHWARE)
	{
		// ACHTUNG!!! 
		// Muss das if dorthin oder nicht?!
		//// Ist gar keine Ware mehr an der Flagge, zu der ich gehe?
		if(!workplace->AreWareJobs(!rs_dir,ct,false))
		//{
			//// Wenn es an der anderen Flagge noch einen gibt, dort hin gehen
			//if(workplace->AreWareJobs(rs_dir))
			//{
			//	rs_pos = cur_rs->GetLength()-rs_pos-2;
			//	rs_dir = !rs_dir;
			//}
			//else
			//{
				// Gibt garnix mehr zu tragen --> wieder in die Mitte gehen
				state = CARRS_GOTOMIDDLEOFROAD;
			/*}*/
		/*}*/
	}
}

///////////////////////////////////////////////////////////////////////////////
/**
 *
 *
 *  @author OLiver
 */
void nofCarrier::FetchWare(const bool swap_wares)
{
	// Ware aufnehmnen
	carried_ware = gwg->GetSpecObj<noFlag>(x,y)->SelectWare((dir+3)%6,swap_wares,this);
	
	if(carried_ware)
	{
		carried_ware->Carry((rs_dir)?workplace->f2:workplace->f1);
		// Und zum anderen Ende laufen
		state = CARRS_CARRYWARE;
		rs_dir = !rs_dir;
		rs_pos = 0;

		StartWalking(cur_rs->GetDir(rs_dir,rs_pos));
	}
	else // zur�cklaufen lassen
		state = CARRS_GOTOMIDDLEOFROAD;
}

///////////////////////////////////////////////////////////////////////////////
/**
 *
 *
 *  @author OLiver
 */
bool nofCarrier::SpaceAtFlag(const bool flag)
{
	// Interessiert uns nur, wenn wir auf einen freien Platz warten
	if(state == CARRS_WAITFORWARESPACE && rs_dir == !flag)
	{
		// In Richtung Flagge laufen, um Ware dort abzulegen
		StartWalking(cur_rs->GetDir(rs_dir,rs_pos));
		state = CARRS_CARRYWARE;
		return true;
	}
	else
		return false;

}

///////////////////////////////////////////////////////////////////////////////
/**
 *
 *
 *  @author OLiver
 */
bool nofCarrier::WantInBuilding()
{
	RoadSegment * rs = static_cast<noFlag*>((rs_dir?cur_rs->f1:cur_rs->f2))->routes[1];
	if(!rs)
		return false;

	if(rs->GetLength() != 1)
		return false;


	carried_ware->RecalcRoute();
	return (carried_ware->GetNextDir() == 1);
}

/// F�r Produktivit�tsmessungen: f�ngt an zu arbeiten
void nofCarrier::StartWorking()
{
	// Wenn noch kein Zeitpunkt festgesetzt wurde, jetzt merken
	if(since_working_gf == 0xFFFFFFFF)
		since_working_gf = GAMECLIENT.GetGFNumber();
}

/// F�r Produktivit�tsmessungen: h�rt auf zu arbeiten
void nofCarrier::StopWorking()
{
	// Falls wir vorher nicht gearbeitet haben, diese Zeit merken f�r die Produktivit�t
	if(since_working_gf != 0xFFFFFFFF)
	{
		worked_gf += static_cast<unsigned short>(GAMECLIENT.GetGFNumber() - since_working_gf);
		since_working_gf = 0xFFFFFFFF;
	}
}


noRoadNode * nofCarrier::GetFirstFlag() const
{ return workplace ? workplace->f1 : 0; }
noRoadNode * nofCarrier::GetSecondFlag() const
{ return workplace ? workplace->f2 : 0; }
