#include <Ogre.h>
#include <OgreApplicationContext.h>
#include <OgreInput.h>
#include <OgreRTShaderSystem.h>

class TutorialApplication : public OgreBites::ApplicationContext, public OgreBites::InputListener
{
public:
    TutorialApplication() : OgreBites::ApplicationContext("OgreApp")
    {
    }

    void setup() override
    {
        OgreBites::ApplicationContext::setup();
        addInputListener(this);

        Ogre::Root* root = getRoot();
        Ogre::SceneManager* scnMgr = root->createSceneManager();

        Ogre::RTShader::ShaderGenerator* shadergen = Ogre::RTShader::ShaderGenerator::getSingletonPtr();
        shadergen->addSceneManager(scnMgr);

        Ogre::Light* light = scnMgr->createLight("MainLight");
        Ogre::SceneNode* lightNode = scnMgr->getRootSceneNode()->createChildSceneNode();
        lightNode->attachObject(light);
        lightNode->setPosition(20, 80, 50);

        Ogre::SceneNode* camNode = scnMgr->getRootSceneNode()->createChildSceneNode();
        camNode->setPosition(0, 0, 15);
        camNode->lookAt(Ogre::Vector3(0, 0, -1), Ogre::Node::TS_PARENT);

        Ogre::Camera* cam = scnMgr->createCamera("myCam");
        cam->setNearClipDistance(5);
        cam->setAutoAspectRatio(true);
        camNode->attachObject(cam);
        getRenderWindow()->addViewport(cam);

        Ogre::Entity* ogreEntity = scnMgr->createEntity("ogrehead.mesh");
        Ogre::SceneNode* ogreNode = scnMgr->getRootSceneNode()->createChildSceneNode();
        ogreNode->attachObject(ogreEntity);
    }

    bool keyPressed(const OgreBites::KeyboardEvent& evt) override
    {
        if (evt.keysym.sym == OgreBites::SDLK_ESCAPE)
        {
            getRoot()->queueEndRendering();
        }
        return true;
    }
};

int main()
{
    TutorialApplication app;
    app.initApp();
    app.getRoot()->startRendering();
    app.closeApp();
    return 0;
}
