/**
* This work is distributed under the General Public License,
* see LICENSE for details
*
* @author Gwenna�l ARBONA
**/


#include "Engine/Rendering/renderer.hpp"
#include "Engine/game.hpp"
#include "Engine/actor.hpp"
#include "Engine/player.hpp"


#define OGRE_CONF			"Config/soyouz.cfg"
#define SYSTEM_CONF			"Config/system.xml"
#if OGRE_DEBUG_MODE
#  define PLUGINS_CONF		"Config/plugins_d.cfg"
#else
#  define PLUGINS_CONF		"Config/plugins.cfg"
#endif
#define RESOURCES_CONF		"Config/resources.cfg"
#define LOGFILE_NAME		"Config/soyouz.log"


/*----------------------------------------------
	Constructor & destructor
----------------------------------------------*/

Game::Game()
{
	mRoot = NULL;
	mIOManager = NULL;
	mPhysWorld = NULL;
	mOverlaySystem = NULL;
}


Game::~Game()
{
	if (mIOManager)
	{
		delete mIOManager;
	}
	if (mOverlaySystem)
	{
		if(mScene) mScene->removeRenderQueueListener(mOverlaySystem);
		delete mOverlaySystem;
	}
	if (mRoot)
	{
		delete mRoot;
	}
}


/*----------------------------------------------
	Public methods
----------------------------------------------*/

void Game::run()
{
	setup();
	mRoot->startRendering();
	while (!mWindow->isClosed() && mRoot->renderOneFrame())
	{
	    Ogre::WindowEventUtilities::messagePump();
	}
	destruct();
}


void Game::tick(const Ogre::FrameEvent& evt)
{
	Actor* ref;
	
	// Physics tick
	if (mPhysWorld)
	{
		mPhysWorld->stepSimulation(evt.timeSinceLastFrame, 4, btScalar(1.)/btScalar(60.));
	}

	// Actor pre-tick
	for (Ogre::list<Actor*>::iterator it = mAllActors.begin(); it != mAllActors.end(); it++)
    {
		ref = *it;
        ref->preTick(evt);
    }

	// Actor tick
	for (Ogre::list<Actor*>::iterator it = mAllActors.begin(); it != mAllActors.end(); it++)
	{
		ref = *it;
		ref->tick(evt);
	}

	// Actor garbage collector
	Actor* previousItem = NULL;
	for (Ogre::list<Actor*>::iterator it = mToRemoveActors.begin(); it != mToRemoveActors.end();it++)
	{
		deleteActor(previousItem);
		previousItem = *it;
	}
	mToRemoveActors.clear();
	deleteActor(previousItem);

	// Debug physics
	mPhysDrawer->step();
	
	//const Ogre::RenderTarget::FrameStats& stats = mWindow->getStatistics();
	//gameLog("FPS:" + Ogre::StringConverter::toString(stats.lastFPS));
}
	

void Game::registerActor(Actor* ref)
{
	mAllActors.push_back(ref);
}
	

void Game::unregisterActor(Actor* ref)
{
	mToRemoveActors.push_back(ref);
}

void Game::deleteActor(Actor* target)
{
	if(target != NULL) {
		mAllActors.remove(target);
		delete target;
	}
}

Ogre::SceneNode* Game::createGameNode(String name)
{
	return mScene->getRootSceneNode()->createChildSceneNode(name);
}


void Game::deleteGameNode(Ogre::SceneNode* node)
{
	mScene->destroySceneNode(node);
}


Ogre::Entity* Game::createGameEntity(String name, String file)
{
	return mScene->createEntity(name, file);
}


void Game::deleteGameEntity(Ogre::Entity* entity)
{
	mScene->destroyEntity(entity);
}


void Game::registerRigidBody(btRigidBody* body)
{
	mPhysWorld->addRigidBody(body);
}


void Game::unregisterRigidBody(btRigidBody* body)
{
	mPhysWorld->removeRigidBody(body);
}


void Game::quit()
{
	bRunning = false;
}


Ogre::SceneManager* Game::getScene()
{
	return mScene;
}


tinyxml2::XMLElement* Game::getConfig()
{
	return mConfig;
}


/*----------------------------------------------
	Events
----------------------------------------------*/

bool Game::frameRenderingQueued(const Ogre::FrameEvent& evt)
{
	tick(evt);
	mIOManager->prerender(evt);
	return bRunning;
}


bool Game::frameEnded(const Ogre::FrameEvent& evt)
{
	mIOManager->postrender(evt);
	return bRunning;
}


/*----------------------------------------------
	Debug utility
----------------------------------------------*/
 
void Game::gameLog(String text)
{
	Ogre::LogManager::getSingletonPtr()->logMessage(text.c_str());
}


void Game::setDebugMode(int newStatus)
{
	mPhysDrawer->setDebugMode(0);
	switch (newStatus)
	{
	case 0:
		mRenderer->setMode(Renderer::DSM_SHOWLIT);
		break;

	case 1:
		mRenderer->setMode(Renderer::DSM_SHOWLIT);
		mPhysDrawer->setDebugMode(1);
		break;

	case 2:
		mRenderer->setMode(Renderer::DSM_SHOWGBUFFER);
		break;

	case 3:
		mRenderer->setMode(Renderer::DSM_SHOWSSAO);
		break;

	default:
		break;
	}
}


Ogre::ManualObject* Game::getDebugLine(Vector3 line, String name, String material)
{
	Ogre::ManualObject* tmp = mScene->createManualObject(name); 
	tmp->begin(material, Ogre::RenderOperation::OT_LINE_LIST); 
	tmp->position(0, 0, 0); 
	tmp->position(line.x, line.y, line.z); 
	tmp->end(); 
	return tmp;
}


void Game::dumpAllNodes()
{
	std::stringstream ss;
	ss << std::endl << "Game::dumpNodes" << std::endl;
	dumpNodes(ss, mScene->getRootSceneNode(), 0);
	gameLog(ss.str());
}


/*----------------------------------------------
	Protected methods
----------------------------------------------*/

bool Game::setup()
{
	setupResources();
	setupSystem("OpenGL");
	setupPhysics(Vector3(0, 0, 0), false);
	setupRender(true);
	construct();
	return true;
}


void Game::setupResources()
{
	Ogre::ConfigFile cf;
	tinyxml2::XMLError res;
	String pluginsPath, secName, typeName, archName;
	
	// Main setup
	pluginsPath = PLUGINS_CONF;
	mRoot = new Ogre::Root(pluginsPath, OGRE_CONF, LOGFILE_NAME);
	mOverlaySystem = new Ogre::OverlaySystem();

	// XML setup
	mConfigFile = new tinyxml2::XMLDocument();
	res = mConfigFile->LoadFile(SYSTEM_CONF);
	assert(res == tinyxml2::XML_NO_ERROR);
	mConfig = mConfigFile->FirstChildElement("document");
	assert(mConfig != NULL);
	
	// Resources
	cf.load(RESOURCES_CONF);
	Ogre::ConfigFile::SectionIterator seci = cf.getSectionIterator();
	while (seci.hasMoreElements())
	{
		secName = seci.peekNextKey();
		Ogre::ConfigFile::SettingsMultiMap::iterator i;
		Ogre::ConfigFile::SettingsMultiMap *settings = seci.getNext();
		for (i = settings->begin(); i != settings->end(); ++i)
		{
			typeName = i->first;
			archName = i->second;
			Ogre::ResourceGroupManager::getSingleton().addResourceLocation(archName, typeName, secName);
		}
	}
}


bool Game::setupSystem(const String desiredRenderer)
{
    bool bRes = false;
	Ogre::RenderSystemList rdrs;
    Ogre::RenderSystem* renderSystem;
	
	// Render systems list
    rdrs = mRoot->getAvailableRenderers();
    if (rdrs.empty())
	{
        return false;
	}

	// Search a correct render system
    for (Ogre::RenderSystemList::iterator it = rdrs.begin(); it != rdrs.end(); it++)
    {
        renderSystem = (*it);
		Ogre::Log(renderSystem->getName());
        if (StringUtil::startsWith(renderSystem->getName(), desiredRenderer, false))
        {
            bRes = true;
            break;
        }
    }
    if (!bRes)
	{
        renderSystem = *(rdrs.begin());
    }
    mRoot->setRenderSystem(renderSystem);

	// Render options
	tinyxml2::XMLElement* renderConf = mConfig->FirstChildElement("rendersystem");
	assert(renderConf != NULL);
	tinyxml2::XMLElement* i = renderConf->FirstChildElement("option");
	while (i != NULL)
	{
		renderSystem->setConfigOption(i->Attribute("name"), i->Attribute("value"));
		i = i->NextSiblingElement("option");
	}

	// Window
	mWindow = mRoot->initialise(true, "Soyouz");
	mScene = mRoot->createSceneManager(Ogre::ST_GENERIC, "GameScene");
    return true;
}


void Game::setupRender(bool bShowPostProcess)
{
	// Render resources
	Ogre::ResourceGroupManager::getSingleton().initialiseResourceGroup("Core");
	Ogre::ResourceGroupManager::getSingleton().initialiseResourceGroup("Debug");
	Ogre::ResourceGroupManager::getSingleton().initialiseResourceGroup("Game");

	// Player
	if (mOverlaySystem)
	{
			mScene->addRenderQueueListener(mOverlaySystem);
	}
	setupPlayer();
	Ogre::Camera* cam = mPlayer->getCamera();

	// Window
	Ogre::Viewport* vp = mWindow->addViewport(cam);
	mPlayer->setCameraRatio(Real(vp->getActualWidth()) / Real(vp->getActualHeight()));
	
	// IO manager
	mIOManager = new IOManager(mWindow, mPlayer, this);
	mRoot->addFrameListener(this);

	// Deferred rendering setup
	mRenderer = new Renderer(mWindow->getViewport(0), mScene, mConfig);
	mRenderer->setMode(Renderer::DSM_SHOWLIT);
}


void Game::setupPhysics(Vector3 gravity, bool bDrawDebug)
{
	mPhysCollisionConfiguration = new btDefaultCollisionConfiguration();
	mPhysDispatcher = new btCollisionDispatcher(mPhysCollisionConfiguration);
	mPhysBroadphase = new btDbvtBroadphase();
	mPhysSequentialImpulseConstraintSolver = new btSequentialImpulseConstraintSolver;

	mPhysWorld = new btDiscreteDynamicsWorld(
		mPhysDispatcher,
		mPhysBroadphase,
		mPhysSequentialImpulseConstraintSolver,
		mPhysCollisionConfiguration);

	mPhysWorld->setGravity(btVector3(gravity[0], gravity[1], gravity[2]));

	mPhysDrawer = new DebugDrawer(mScene, mScene->getRootSceneNode(), mPhysWorld);
	mPhysDrawer->setDebugMode(bDrawDebug ? 1:0);
	mPhysWorld->setDebugDrawer(mPhysDrawer);
}


void Game::setupPlayer()
{
	mPlayer = new Player(this, "LocalPlayer");
}


void Game::dumpNodes(std::stringstream &ss, Ogre::Node* n, int level)
{
	for (int i = 0; i < level; i++)
	{
		ss << " ";
	}
	ss << "SceneNode: " << n->getName() << std::endl;
 
	Ogre::SceneNode::ObjectIterator object_it = ((Ogre::SceneNode *)n)->getAttachedObjectIterator();
	Ogre::Node::ChildNodeIterator node_it = n->getChildIterator();
	Ogre::MovableObject *m;

	while (object_it.hasMoreElements())
	{
		for (int i = 0; i < level + 2; i++)
		{
			ss << " ";
		}
		m = object_it.getNext();
		ss << m->getMovableType() << ": " << m->getName() << std::endl;
	}
	while (node_it.hasMoreElements())
	{
		dumpNodes(ss, node_it.getNext(), level + 2);
	}
}

