/**
* This work is distributed under the General Public License,
* see LICENSE for details
*
* @author Gwenna�l ARBONA
**/

#include "test.h"


/*----------------------------------------------
	Methods
----------------------------------------------*/

void Test::construct()
{
	mScene->setAmbientLight(Ogre::ColourValue(0.5f, 0.5f, 0.5f));
	Actor penguin = Actor(mScene, "penguin", "penguin.mesh");
}

void Test::destruct()
{
}
