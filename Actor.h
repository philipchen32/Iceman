// Possible interfaces for actors.  You may use all, some, or none
// of this, as you wish.

#ifndef ACTOR_H_
#define ACTOR_H_

#include "GraphObject.h"

class StudentWorld;

class Actor : public GraphObject
{
private:
    bool isActive;
    StudentWorld* sw;
public:
    Actor(StudentWorld* world, int startX, int startY, Direction startDir, bool visible, int imageID, double size, int depth);
    
    // Action to perform each tick.
    virtual void move() = 0;
    
    // Is this actor alive?
    bool isAlive() const{return isActive;}
    
    // Mark this actor as dead.
    void setDead(){isActive = false;}
    
    // Annoy this actor.
    virtual bool annoy(unsigned int amt){return false;}
    
    // Get this actor's world
    StudentWorld* getWorld() const{return sw;}
    
    virtual void addGold(){}
    // Can other actors pass through this actor?
    // Determines if it's a boulder
    virtual bool canActorsPassThroughMe() const{return true;}
    
    // Can this actor dig through Ice?
    virtual bool canDigThroughIce() const{return false;}
    
    // Can this actor pick items up?
    virtual bool canPickThingsUp() const{return false;}
    
    // Does this actor hunt the IceMan?
    virtual bool huntsIceMan() const{return false;}
    
    virtual void makeVisible(){setVisible(true);}
    virtual bool canBribe() const{return false;}
    // Can this actor need to be picked up to finish the level?
    // Determines if it's an oil barrel
    virtual bool needsToBePickedUpToFinishLevel() const{return false;}
    
    // Move this actor to x,y if possible, and return true; otherwise,
    // return false without moving.
    bool moveToIfPossible(int x, int y);
    bool moveToIfPossible(Direction dir);
    void moveByDirection();
};

class Agent : public Actor
{
private:
    int hp;
public:
    Agent(StudentWorld* world, int startX, int startY, Direction startDir,
          int imageID, unsigned int hitPoints);
    // Pick up a gold nugget.
    virtual void addGold() = 0;
    
    // How many hit points does this actor have left?
    int getHitPoints() const{return hp;}
    virtual bool annoy(unsigned int amount);
    virtual bool canPickThingsUp() const{return true;}
};

class IceMan : public Agent
{
private:
    int waterStock;
    int sonarStock;
    int goldStock;
    void spurtWater();
public:
    IceMan(StudentWorld* world, int startX, int startY);
    virtual void move();
    virtual bool annoy(unsigned int amount);
    virtual void addGold(){goldStock++;}
    virtual bool canDigThroughIce() const{return true;}
    
    // Pick up a sonar kit.
    void addSonar(){sonarStock++;}
    
    // Pick up water.
    void addWater(){waterStock+=5;}
    
    // Get amount of gold
    unsigned int getGold() const{return goldStock;}
    
    // Get amount of sonar charges
    unsigned int getSonar() const{return sonarStock;}
    
    // Get amount of water
    unsigned int getWater() const{return waterStock;}
};

class Protester : public Agent
{
private:
    int ticksPerMove;
    bool leaveField;
    int tickCounter;
    int sqToMove;
    int shoutTicks;
    int squirtScore;
    int perpTicks;
    GraphObject::Direction generateDirection();
    GraphObject::Direction checkPerpendicularMove();
public:
    Protester(StudentWorld* world, int startX, int startY, int imageID, unsigned int hitPoints, unsigned int score);
    virtual void move();
    virtual bool annoy(unsigned int amount);
    virtual void addGold();
    virtual bool huntsIceMan() const{return true;}
    virtual bool detectsIceMan() const{return false;}
    // Set state to having given up protest
    void setMustLeaveOilField();
    // Set number of ticks until next move
    void setMoreTicks(int moreTicks){tickCounter = moreTicks;}
};

class RegularProtester : public Protester
{
public:
    RegularProtester(StudentWorld* world, int startX, int startY);
    virtual void addGold();
};

class HardcoreProtester : public Protester
{
public:
    HardcoreProtester(StudentWorld* world, int startX, int startY);
    virtual bool detectsIceMan() const{return true;}
    virtual void addGold();
};

class Ice : public Actor
{
public:
    Ice(StudentWorld* world, int startX, int startY) : Actor(world,startX, startY, right, true, IID_ICE, 0.25, 3){}
    virtual void move(){}
};

class Boulder : public Actor
{
private:
    int currState;
    int waitingTicks;
public:
    Boulder(StudentWorld* world, int startX, int startY);
    virtual void move();
    virtual bool canActorsPassThroughMe() const{return false;}
};

class Squirt : public Actor
{
private:
    int distanceLeft;
public:
    Squirt(StudentWorld* world, int startX, int startY, Direction startDir):Actor(world, startX, startY, startDir, true, IID_WATER_SPURT, 1.0, 1){    distanceLeft = 4;}
    virtual void move();
};

class ActivatingObject : public Actor
{
private:
    int pickupSound;
    bool forIceMan;
    bool forProtester;
    bool startsActive;
    int ticksToLive;
public:
    ActivatingObject(StudentWorld* world, int startX, int startY, int imageID, int sound, bool activateOnPlayer, bool activateOnProtester, bool initiallyActive);
    virtual void move();
    virtual void addToIceMan(){}
    virtual void makeVisible(){
        setVisible(true);
        startsActive = true;
    }
    // Set number of ticks until this object dies
    void setTicksToLive();
};

class OilBarrel : public ActivatingObject
{
public:
    OilBarrel(StudentWorld* world, int startX, int startY);
    virtual void addToIceMan();
    virtual bool needsToBePickedUpToFinishLevel() const{return true;}
};

class GoldNugget : public ActivatingObject
{
private:
    bool temp;
public:
    GoldNugget(StudentWorld* world, int startX, int startY, bool temporary);
    virtual void move();
    virtual void addToIceMan();
    virtual bool canBribe() const{return true;}
};

class SonarKit : public ActivatingObject
{
private:
    int ticksToLive;
public:
    SonarKit(StudentWorld* world, int startX, int startY);
    virtual void addToIceMan();
};

class WaterPool : public ActivatingObject
{
private:
    int ticksToLive;
public:
    WaterPool(StudentWorld* world, int startX, int startY);
    virtual void addToIceMan();
};

#endif // ACTOR_H_
