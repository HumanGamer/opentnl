//-----------------------------------------------------------------------------------
//
//   Torque Network Library - ZAP example multiplayer vector graphics space game
//   Copyright (C) 2004 GarageGames.com, Inc.
//   For more information see http://www.opentnl.org
//
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; either version 2 of the License, or
//   (at your option) any later version.
//
//   For use in products that are not compatible with the terms of the GNU 
//   General Public License, alternative licensing options are available 
//   from GarageGames.com.
//
//   This program is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//
//   You should have received a copy of the GNU General Public License
//   along with this program; if not, write to the Free Software
//   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//------------------------------------------------------------------------------------

#ifndef _GAMEOBJECTRENDER_H_
#define _GAMEOBJECTRENDER_H_

#include "point.h"
#include "tnl.h"
using namespace TNL;

namespace Zap
{

extern void renderShip(Color c, F32 alpha, F32 thrusts[], F32 health, F32 radius, bool cloakActive, bool shieldActive);
extern void renderTeleporter(U32 time, F32 radiusFraction, F32 radius, F32 alpha);
extern void renderTurret(Color c, Point anchor, Point normal, bool enabled, F32 health, F32 barrelAngle, F32 aimOffset);

extern void glVertex(Point p);

};

#endif