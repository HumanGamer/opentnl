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

#include "moveObject.h"
#include "SweptEllipsoid.h"
#include "sparkManager.h"
#include "sfx.h"

namespace Zap
{

MoveObject::MoveObject(Point pos, float radius, float mass)
{
   for(U32 i = 0; i < MoveStateCount; i++)
   {
      mMoveState[i].pos = pos;
      mMoveState[i].angle = 0;
   }
   mRadius = radius;
   mMass = mass;
   mInterpolating = false;
}

static const float MoveObjectCollisionElasticity = 1.7f;

void MoveObject::updateExtent()
{
   Rect r(mMoveState[ActualState].pos, mMoveState[RenderState].pos);
   r.expand(Point(mRadius + 10, mRadius + 10));
   setExtent(r);
}

// Ship movement system
// Identify the several cases in which a ship may be moving:
// if this is a client:
//   Ship controlled by this client.  Pos may have been set to something else by server, leaving renderPos elsewhere
//     all movement updates affect pos

// collision process for ships:
//

//
// ship *theShip;
// F32 time;
// while(time > 0)
// {
//    ObjHit = findFirstCollision(theShip);
//    advanceToCollision();
//    if(velocitiesColliding)
//    {
//       doCollisionResponse();
//    }
//    else
//    {
//       computeMinimumSeperationTime(ObjHit);
//       displaceObject(ObjHit, seperationTime);
//    }
// }
//
// displaceObject(Object, time)
// {
//    while(time > 0)
//    {
//       ObjHit = findFirstCollision();
//       advanceToCollision();
//       if(velocitiesColliding)
//       {
//          doCollisionResponse();
//          return;
//       }
//       else
//       {
//          computeMinimumSeperationTime(ObjHit);
//          displaceObject(ObjHit, seperationTime);
//       }
//    }
// }

extern bool FindLowestRootInInterval(Point::member_type inA, Point::member_type inB, Point::member_type inC, Point::member_type inUpperBound, Point::member_type &outX);
static Vector<GameObject *> fillVector;

F32 MoveObject::computeMinSeperationTime(U32 stateIndex, MoveObject *contactShip, Point intendedPos)
{
   // ok, this ship wants to move to intendedPos
   // so we need to figure out how far contactShip has to move so it won't be in the way

   F32 myRadius;
   F32 contactShipRadius;

   Point myPos;
   Point contactShipPos;

   getCollisionCircle(stateIndex, myPos, myRadius);
   contactShip->getCollisionCircle(stateIndex, contactShipPos, contactShipRadius);

   Point v = contactShip->mMoveState[stateIndex].vel;
   Point posDelta = contactShipPos - intendedPos;

   F32 R = myRadius + contactShipRadius;

   F32 a = v.dot(v);
   F32 b = 2 * v.dot(posDelta);
   F32 c = posDelta.dot(posDelta) - R * R;

   F32 t;

   bool result = FindLowestRootInInterval(a, b, c, 100000, t);
   if(!result)
      return 0;
   return t;
}

const F32 moveTimeEpsilon = 0.000001f;
const F32 velocityEpsilon = 0.00001f;

void MoveObject::move(F32 moveTime, U32 stateIndex, bool displacing)
{
   U32 tryCount = 0;
   while(moveTime > moveTimeEpsilon && tryCount < 8)
   {
      tryCount++;
      if(!displacing && mMoveState[stateIndex].vel.len() < velocityEpsilon)
         return;

      F32 collisionTime = moveTime;
      Point collisionPoint;

      GameObject *objectHit = findFirstCollision(stateIndex, collisionTime, collisionPoint);
      if(!objectHit)
      {
         mMoveState[stateIndex].pos += mMoveState[stateIndex].vel * moveTime;
         return;
      }
      // advance to the point of collision
      mMoveState[stateIndex].pos += mMoveState[stateIndex].vel * collisionTime;

      if(objectHit->getObjectTypeMask() & MoveableType)
      {
         MoveObject *shipHit = (MoveObject *) objectHit;
         Point velDelta = shipHit->mMoveState[stateIndex].vel -
                          mMoveState[stateIndex].vel;
         Point posDelta = shipHit->mMoveState[stateIndex].pos -
                          mMoveState[stateIndex].pos;

         if(posDelta.dot(velDelta) < 0) // there is a collision
         {
            computeCollisionResponseMoveObject(stateIndex, shipHit);
            if(displacing)
               return;
         }
         else
         {
            Point intendedPos = mMoveState[stateIndex].pos +
                  mMoveState[stateIndex].vel * moveTime;

            F32 displaceEpsilon = 0.002f;
            F32 t = computeMinSeperationTime(stateIndex, shipHit, intendedPos);
            if(t <= 0)
               return; // some kind of math error - just stop simulating this ship
            shipHit->move(t + displaceEpsilon, stateIndex, true);
         }
      }
      else if(objectHit->getObjectTypeMask() & (BarrierType | EngineeredType | ForceFieldType))
      {
         computeCollisionResponseBarrier(stateIndex, collisionPoint);
      }
      moveTime -= collisionTime;
   }
}

bool MoveObject::collide(GameObject *otherObject)
{
   return true;
}

GameObject *MoveObject::findFirstCollision(U32 stateIndex, F32 &collisionTime, Point &collisionPoint)
{
   // check for collisions against other objects
   Point delta = mMoveState[stateIndex].vel * collisionTime;

   Rect queryRect(mMoveState[stateIndex].pos, mMoveState[stateIndex].pos + delta);
   queryRect.expand(Point(mRadius, mRadius));

   fillVector.clear();
   findObjects(AllObjectTypes, fillVector, queryRect);

   float collisionFraction;

   GameObject *collisionObject = NULL;

   for(S32 i = 0; i < fillVector.size(); i++)
   {
      if(!fillVector[i]->isCollisionEnabled())
         continue;

      static Vector<Point> poly;
      poly.clear();
      if(fillVector[i]->getCollisionPoly(poly))
      {
         Point cp;
         if(PolygonSweptCircleIntersect(&poly[0], poly.size(), mMoveState[stateIndex].pos,
               delta, mRadius, cp, collisionFraction))
         {
            if((cp - mMoveState[stateIndex].pos).dot(mMoveState[stateIndex].vel) > velocityEpsilon)
            {
               bool collide1 = collide(fillVector[i]);
               bool collide2 = fillVector[i]->collide(this);

               if(!(collide1 && collide2))
                  continue;
               collisionPoint = cp;
               delta *= collisionFraction;
               collisionTime *= collisionFraction;
               collisionObject = fillVector[i];
               if(!collisionTime)
                  break;
            }
         }
      }
      else if(fillVector[i]->getObjectTypeMask() & MoveableType)
      {
         MoveObject *otherShip = (MoveObject *) fillVector[i];

         F32 myRadius;
         F32 otherRadius;
         Point myPos;
         Point shipPos;

         getCollisionCircle(stateIndex, myPos, myRadius);
         otherShip->getCollisionCircle(stateIndex, shipPos, otherRadius);

         Point v = mMoveState[stateIndex].vel;
         Point p = myPos - shipPos;

         if(v.dot(p) < 0)
         {
            F32 R = myRadius + otherRadius;
            if(p.len() <= R)
            {
               bool collide1 = collide(otherShip);
               bool collide2 = otherShip->collide(this);

               if(!(collide1 && collide2))
                  continue;

               collisionTime = 0;
               collisionObject = fillVector[i];
               delta.set(0,0);
            }
            else
            {
               F32 a = v.dot(v);
               F32 b = 2 * p.dot(v);
               F32 c = p.dot(p) - R * R;
               F32 t;
               if(FindLowestRootInInterval(a, b, c, collisionTime, t))
               {
                  bool collide1 = collide(otherShip);
                  bool collide2 = otherShip->collide(this);

                  if(!(collide1 && collide2))
                     continue;

                  collisionTime = t;
                  collisionObject = fillVector[i];
                  delta = mMoveState[stateIndex].vel * collisionTime;
               }
            }
         }
      }
   }
   return collisionObject;
}

void MoveObject::computeCollisionResponseBarrier(U32 stateIndex, Point &collisionPoint)
{
   // reflect the velocity along the collision point
   Point normal = mMoveState[stateIndex].pos - collisionPoint;
   normal.normalize();

   mMoveState[stateIndex].vel -= normal * MoveObjectCollisionElasticity * normal.dot(mMoveState[stateIndex].vel);

   // Emit some bump particles (try not to if we're running server side)
   if(isGhost())
   {
      F32 scale = normal.dot(mMoveState[stateIndex].vel) * 0.01f;
      if(scale > 0.5f)
      {
         // Make a noise...
         SFXObject::play(SFXBounceWall, collisionPoint, Point(), getMin(1.0f, scale - 0.25f));

         Color bumpC(scale/3, scale/3, scale);

         for(S32 i=0; i<4*pow((F32)scale, 0.5f); i++)
         {
            Point chaos(Random::readF(), Random::readF());
            chaos *= scale + 1;

            if(Random::readF() > 0.5)
               FXManager::emitSpark(collisionPoint, normal * chaos.len() + Point(normal.y, -normal.x)*scale*5  + chaos + mMoveState[stateIndex].vel*0.05f, bumpC);

            if(Random::readF() > 0.5)
               FXManager::emitSpark(collisionPoint, normal * chaos.len() + Point(normal.y, -normal.x)*scale*-5 + chaos + mMoveState[stateIndex].vel*0.05f, bumpC);
         }
      }
   }
}

void MoveObject::computeCollisionResponseMoveObject(U32 stateIndex, MoveObject *shipHit)
{
   Point collisionVector = shipHit->mMoveState[stateIndex].pos -
                           mMoveState[stateIndex].pos;

   collisionVector.normalize();
   F32 m1 = getMass();
   F32 m2 = shipHit->getMass();

   F32 v1i = mMoveState[stateIndex].vel.dot(collisionVector);
   F32 v2i = shipHit->mMoveState[stateIndex].vel.dot(collisionVector);

   F32 v1f, v2f;

   F32 e = 0.9f;
   v2f = ( e * (v1i - v2i) + v1i + v2i) / (2);
   v1f = ( v1i + v2i - v2f);

   mMoveState[stateIndex].vel += collisionVector * (v1f - v1i);
   shipHit->mMoveState[stateIndex].vel += collisionVector * (v2f - v2i);

   if(v1i > 0.25)
      SFXObject::play(SFXBounceObject, shipHit->mMoveState[stateIndex].pos, Point());
}

void MoveObject::updateInterpolation()
{
   U32 deltaT = mCurrentMove.time;
   {
      mMoveState[RenderState].angle = mMoveState[ActualState].angle;

      if(mInterpolating)
      {
         // first step is to constrain the render velocity to
         // the vector of difference between the current position and
         // the actual position.
         // we can also clamp to zero, the actual velocity, or the
         // render velocity, depending on which one is best.

         Point deltaP = mMoveState[ActualState].pos - mMoveState[RenderState].pos;
         F32 distance = deltaP.len();

         if(!distance)
            goto interpDone;

         deltaP.normalize();
         F32 vel = deltaP.dot(mMoveState[RenderState].vel);
         F32 avel = deltaP.dot(mMoveState[ActualState].vel);

         if(avel > vel)
            vel = avel;
         if(vel < 0)
            vel = 0;

         bool hit = true;
         float time = deltaT * 0.001f;
         if(vel * time > distance)
            goto interpDone;

         float requestVel = distance / time;
         float interpMaxVel = InterpMaxVelocity;
         float currentActualVelocity = mMoveState[ActualState].vel.len();
         if(interpMaxVel < currentActualVelocity)
            interpMaxVel = currentActualVelocity;
         if(requestVel > interpMaxVel)
         {
            hit = false;
            requestVel = interpMaxVel;
         }
         F32 a = (requestVel - vel) / time;
         if(a > InterpAcceleration)
         {
            a = InterpAcceleration;
            hit = false;
         }

         if(hit)
            goto interpDone;

         vel += a * time;
         mMoveState[RenderState].vel = deltaP * vel;
         mMoveState[RenderState].pos += mMoveState[RenderState].vel * time;
      }
      else
      {
   interpDone:
         mInterpolating = false;
         mMoveState[RenderState] = mMoveState[ActualState];
      }
   }
}

};

