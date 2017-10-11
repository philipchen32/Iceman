// A possible interface for StudentWorld.  You may use all, some, or none
// of this, as you wish.

#ifndef STUDENTWORLD_H_
#define STUDENTWORLD_H_

#include "GraphObject.h"
#include "GameWorld.h"
#include "Actor.h"
#include <string>
#include <vector>

struct Coord{
    int x;
    int y;
    int steps;
    Coord(int ax, int ay, int s){
        x = ax;
        y = ay;
        steps = s;
    }
};

class Actor;
class Ice;
class IceMan;
class StudentWorld : public GameWorld
{
private:
    IceMan* hero;
    Ice* IceArray[64][64];
    int barrelsLeft;
    int ticksPerNewP;
    int maxP;
    int chanceForHardcore;
    int tickCounter;
    std::vector<Actor*> ActorList;
    char maze[61][61];
    int trackSteps[61][61];
    
    int trackIceMan[61][61];
    
    void generateBoulderPoints(int& x, int& y);
    void generateHiddenPoints(int &x, int &y);
    void generateOpenPoints(int &x, int &y);
    bool checkRadius(Actor* a, int x, int y, int radius) const;
    
    void setDisplayText();
    void setMaze(Actor *a);
public:
    StudentWorld(std::string assetDir);
    virtual ~StudentWorld();
    
    virtual int init();
    virtual int move();
    virtual void cleanUp();
    
    // Add an actor to the world.
    void addActor(Actor* a){ActorList.push_back(a);}
    
    // Clear a 4x4 region of Ice.
    void clearIce(int x, int y);
    
    // Can actor move to x,y?
    bool canActorMoveTo(Actor* a, int x, int y) const;
    
    // Annoy all other actors within radius of annoyer, returning the
    // number of actors annoyed.
    int annoyAllNearbyActors(Actor* annoyer, int points, int radius);
    
    // Reveal all objects within radius of x,y.
    void revealAllNearbyObjects(int x, int y, int radius);
    
    // If the IceMan is within radius of a, return a pointer to the
    // IceMan, otherwise null.
    Actor* findNearbyIceMan(Actor* a, int radius) const;
    
    // If at least one actor that can pick things up is within radius of a,
    // return a pointer to one of them, otherwise null.
    Actor* findNearbyPickerUpper(Actor* a, int radius) const;
    
    // Annoy the IceMan.
    void annoyIceMan(int points);
    
    // Give IceMan some gold nuggets.
    void giveIceManGold(){hero->addGold();}
    
    // Give IceMan some sonar charges.
    void giveIceManSonar(){hero->addSonar();}

    // Give IceMan some water.
    void giveIceManWater(){hero->addWater();}
    
    // Is the Actor a facing toward the IceMan?
    bool facingTowardIceMan(Actor* a) const;
    
    // If the Actor a has a clear line of sight to the IceMan, return
    // the direction to the IceMan, otherwise GraphObject::none.
    GraphObject::Direction lineOfSightToIceMan(Actor* a) const;
    
    // Return whether the Actor a is within radius of IceMan.
    bool isNearIceMan(Actor* a, int radius) const;
    
    // Determine the direction of the first move a quitting protester
    // makes to leave the oil field.
    GraphObject::Direction determineFirstMoveToExit(int x, int y);
    
    // Determine the direction of the first move a hardcore protester
    // makes to approach the IceMan.
    GraphObject::Direction determineFirstMoveToIceMan(Actor *a, int x, int y);
    
    //setGridToLeave every time a protester dies.
    //setMaze everytime determineFirstMoveToIceMan
    void setGridToLeave(Actor *a);
};

#endif // STUDENTWORLD_H_
