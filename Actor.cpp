#include "Actor.h"
#include "StudentWorld.h"

Actor::Actor(StudentWorld* world, int startX, int startY, Direction startDir, bool visible, int imageID, double size, int depth) :GraphObject(imageID, startX, startY, startDir, size, depth){
    setDirection(startDir);
    isActive = true;
    sw = world;
    if(visible)
        setVisible(true);
}

bool Actor::moveToIfPossible(int x, int y){
    if(getWorld()->canActorMoveTo(this,x,y)){
        moveTo(x,y);
        return true;
    }
    return false;
}

bool Actor::moveToIfPossible(Direction dir){
    //move in direction if you can. If you can't return false.
    switch(dir){
        case right:
            if(getWorld()->canActorMoveTo(this,getX()+1,getY())){
                moveTo(getX()+1,getY());
                return true;
            }
            return false;
        case left:
            if(getWorld()->canActorMoveTo(this,getX()-1,getY())){
                moveTo(getX()-1,getY());
                return true;
            }
            return false;
        case up:
            if(getWorld()->canActorMoveTo(this,getX(),getY()+1)){
                moveTo(getX(),getY()+1);
                return true;
            }
            return false;
        case down:
            if(getWorld()->canActorMoveTo(this,getX(),getY()-1)){
                moveTo(getX(),getY()-1);
                return true;
            }
            return false;
        default:
            return true;
    }
}

void Actor::moveByDirection(){
    //just using math of grid
    switch(getDirection()){
        case right:
            moveTo(getX()+1,getY());
            break;
        case left:
            moveTo(getX()-1,getY());
            break;
        case up:
            moveTo(getX(),getY()+1);
            break;
        case down:
            moveTo(getX(),getY()-1);
            break;
        default:
            break;
    }
}


Agent::Agent(StudentWorld* world, int startX, int startY, Direction startDir, int imageID, unsigned int hitPoints):Actor(world,startX,startY, startDir, true, imageID, 1.0,0){
    hp = hitPoints;
}

bool Agent::annoy(unsigned int amount){
    hp = hp - amount;
    return true;
}

IceMan::IceMan(StudentWorld* world, int startX, int startY):Agent(world, startX, startY, right, IID_PLAYER, 10){
    waterStock = 5;
    sonarStock = 1;
    goldStock = 0;
}

void IceMan::move(){
    int x = getX();
    int y = getY();
    int ch;
    //take user input
    if(getWorld()->getKey(ch))
    {
        switch(ch)
        {
            case KEY_PRESS_ESCAPE:
                setDead();
                break;
            case KEY_PRESS_SPACE:
                //use squirt
                if(waterStock > 0){
                    spurtWater();
                }
                break;
            case KEY_PRESS_UP:
                if(getDirection() != up){
                    setDirection(up);
                    return;
                }
                if(moveToIfPossible(x,y+1))
                    getWorld()->clearIce(x,y+1);
                else
                    moveTo(x,y);
                break;
            case KEY_PRESS_DOWN:
                if(getDirection() != down){
                    setDirection(down);
                    return;
                }
                if(moveToIfPossible(x,y-1))
                    getWorld()->clearIce(x, y-1);
                else
                    moveTo(x,y);
                break;
            case KEY_PRESS_LEFT:
                if(getDirection() != left){
                    setDirection(left);
                    return;
                }
                if(moveToIfPossible(x-1,y))
                    getWorld()->clearIce(x-1,y);
                else
                    moveTo(x,y);
                break;
            case KEY_PRESS_RIGHT:
                if(getDirection() != right){
                    setDirection(right);
                    return;
                }
                if(moveToIfPossible(x+1,y))
                    getWorld()->clearIce(x+1,y);
                else
                    moveTo(x,y);
                break;
            case KEY_PRESS_TAB:
                //release gold
                if(goldStock >0){
                    GoldNugget* n = new GoldNugget(getWorld(), x, y, true);
                    getWorld()->addActor(n);
                    goldStock--;
                }
                break;
            case 'Z':
            case 'z':
                //use sonarcharge
                if(sonarStock > 0){
                    getWorld()->revealAllNearbyObjects(x,y,12);
                    sonarStock--;
                }
                break;
            default:
                break;
        }
    }
}

void IceMan::spurtWater(){
    int x = getX();
    int y = getY();
    Squirt *n;
    //put it 4 away from the iceman
    switch(getDirection()){
        case right:
            n = new Squirt(getWorld(), x+4, y, getDirection());
            break;
        case left:
            n = new Squirt(getWorld(), x-4, y, getDirection());
            break;
        case up:
            n = new Squirt(getWorld(), x, y+4, getDirection());
            break;
        case down:
            n = new Squirt(getWorld(), x, y-4, getDirection());
            break;
        default:
            break;
    }
    waterStock--;
    getWorld()->playSound(SOUND_PLAYER_SQUIRT);
    if(!getWorld()->canActorMoveTo(n, n->getX(), n->getY()))
        delete n; //if it can't move there, then just delete. no need to place in studentworld
    else
        getWorld()->addActor(n);
}

bool IceMan::annoy(unsigned int amount){
    Agent::annoy(amount);
    if(getHitPoints() <= 0){
        setDead();
    }
    return true;
}

Protester::Protester(StudentWorld* world, int startX, int startY, int imageID, unsigned int hitPoints, unsigned int score):Agent(world, startX, startY, left, imageID, hitPoints){
    int t = 3 - getWorld()->getLevel()/4;
    ticksPerMove = std::max(0,t);
    tickCounter = ticksPerMove;
    sqToMove = rand()%53 +8;
    squirtScore = score;
    shoutTicks = 0;
    perpTicks = 0;
    leaveField = false;
}

void Protester::move(){
    //count the ticks
    tickCounter--;
    if(tickCounter >=0)
        return;
    shoutTicks--;
    perpTicks--;
    tickCounter = ticksPerMove; //after it gone to 0, then reset counter
    
    int x = getX();
    int y = getY();
    
    //quitting state
    if(leaveField == true){
        //achieved exit
        if(x == 60 && y == 60){
            setDead();
            return;
        }
        else{
            Direction exitDir = getWorld()->determineFirstMoveToExit(x, y);//find next move
            setDirection(exitDir);
            moveByDirection();
            return;
        }
    }
    //shouting at IceMan
    Actor* hero = getWorld()->findNearbyIceMan(this,4);
    if(shoutTicks<=0 && hero != nullptr && getWorld()->facingTowardIceMan(this)){
        getWorld()->playSound(SOUND_PROTESTER_YELL);
        hero->annoy(2);
        shoutTicks = 15;
        return;
    }
    
    
    //search for Iceman.
    if(hero == nullptr && detectsIceMan()){
        Direction dir = getWorld()->determineFirstMoveToIceMan(this, x, y);
        if(dir!=none){
            setDirection(dir);
            moveByDirection();
            return;
        }
    }
    //if Iceman is within lineofsight
    Direction toIceMan = getWorld()->lineOfSightToIceMan(this);
    if(hero == nullptr && toIceMan != none){
        setDirection(toIceMan);
        moveByDirection();
        sqToMove = 0;
        return;
    }
    if(hero == nullptr && toIceMan == none){
        sqToMove--;
        if(sqToMove<=0){
            Direction newDir = generateDirection();
            setDirection(newDir);
            sqToMove = rand()%53 +8;
        }
        else{
            //case: move perpendicularly.
            Direction perpDir = checkPerpendicularMove();
            if(perpTicks<=0 && perpDir != none){
                setDirection(perpDir);
                sqToMove = rand()%53 +8;
                perpTicks = 200;
            }
        }
    }
    if(!moveToIfPossible(getDirection()))
        sqToMove = 0;
}

GraphObject::Direction Protester::generateDirection(){
    //use "pick-a-number" to pick a direction and if you can't move there then recurse it until it does!
    int n = rand()%4+1;
    int x = getX();
    int y = getY();
    switch(n){
        case 1:
            if(getWorld()->canActorMoveTo(this,x,y+1))
                return up;
            return generateDirection();
        case 2:
            if(getWorld()->canActorMoveTo(this,x,y-1))
                return down;
            return generateDirection();
        case 3:
            if(getWorld()->canActorMoveTo(this,x-1,y))            return left;
            return generateDirection();
        case 4:
            if(getWorld()->canActorMoveTo(this,x+1,y))
                return right;
            return generateDirection();
    }
    return none;
}

GraphObject::Direction Protester::checkPerpendicularMove(){
    //check perpendicular direcitons from current directions.
    if(getDirection() == left || getDirection() == right){
        bool u, d = false;
        if(getWorld()->canActorMoveTo(this, getX(), getY()+1))
           u = true;
        if(getWorld()->canActorMoveTo(this, getX(), getY()-1))
            d = true;
        if(!u && !d)
            return none;
        else if(!u && d)
            return down;
        else if(u && !d)
            return up;
        else{
            if(rand()%2 == 0)
                return up;
            else
                return down;
        }
    }
    else{
        bool l, r = false;
        if(getWorld()->canActorMoveTo(this, getX()-1, getY()))
            l = true;
        if(getWorld()->canActorMoveTo(this, getX()+1, getY()))
            r = true;
        if(!l && !r)
            return none;
        else if(!l && r)
            return right;
        else if(l && !r)
            return left;
        else{
            if(rand()%2 == 0)
                return right;
            else
                return left;
        }
    }
}

bool Protester::annoy(unsigned int amount){
    if(leaveField == true)
        return false;
    Agent::annoy(amount);
    //turn it into leaveField state
    if(getHitPoints() <= 0){
        setMustLeaveOilField();
        getWorld()->playSound(SOUND_PROTESTER_GIVE_UP);
        tickCounter = 0;
        if(amount == 2)
            getWorld()->increaseScore(squirtScore);
        else if(amount == 100)
            getWorld()->increaseScore(500);
        return true;
    }
    //not leaveField state yet
    getWorld()->playSound(SOUND_PROTESTER_ANNOYED);
    int t = 100-getWorld()->getLevel()*10;
    tickCounter = std::max(50,t);
    return true;
}

void Protester::addGold(){
    getWorld()->playSound(SOUND_PROTESTER_FOUND_GOLD);
}

void Protester::setMustLeaveOilField(){
    leaveField = true;
    getWorld()->setGridToLeave(this);
}

RegularProtester::RegularProtester(StudentWorld* world, int startX, int startY):Protester(world, startX, startY, IID_PROTESTER, 5, 100){}

void RegularProtester::addGold(){
    Protester::addGold();
    setMustLeaveOilField();
    getWorld()->increaseScore(25);
}

HardcoreProtester::HardcoreProtester(StudentWorld* world, int startX, int startY):Protester(world, startX, startY, IID_HARD_CORE_PROTESTER, 20, 250){}

void HardcoreProtester::addGold(){
    Protester::addGold();
    getWorld()->increaseScore(50);
    int tt = 100 - getWorld()->getLevel()*10;
    setMoreTicks(std::max(50, tt));
}

Boulder::Boulder(StudentWorld* world, int startX, int startY):Actor(world, startX, startY, down, true, IID_BOULDER, 1.0, 1){
    currState = 0;
    waitingTicks = 30;
}

void Boulder::move(){
    int x = getX();
    int y = getY();
    //stable state
    if(currState == 0){
        if(!getWorld()->canActorMoveTo(this,x,y-1))
            return;
        else{
            currState = 1;
            waitingTicks = 30;
        }
    }
    //waiting state
    else if(currState == 1){
        if(waitingTicks <= 0){
            currState = 2;
            getWorld()->playSound(SOUND_FALLING_ROCK);
        }
        waitingTicks--;
    }
    //falling state
    else{
        //hit IceMan if you can
        if(getWorld()->isNearIceMan(this, 3))
            getWorld()->annoyIceMan(100);
        
        getWorld()->annoyAllNearbyActors(this, 100, 3);
        if(!moveToIfPossible(x, y-1)){
            setDead();
            return;
        }
    }
}

void Squirt::move(){
    if(getWorld()->annoyAllNearbyActors(this, 2, 3) > 0){
        setDead();
        return;
    }
    else if(distanceLeft <= 0){
        setDead();
        return;
    }
    else if(!moveToIfPossible(getDirection())){
        setDead();
        return;
    }
    distanceLeft--;
}

ActivatingObject::ActivatingObject(StudentWorld* world, int startX, int startY, int imageID, int sound, bool activateOnPlayer, bool activateOnProtester, bool initiallyActive):Actor(world, startX, startY, right, false, imageID, 1.0, 2){
    pickupSound = sound;
    forIceMan = activateOnPlayer;
    forProtester = activateOnProtester;
    startsActive = initiallyActive;
    ticksToLive = -1;
    if(startsActive)
        setVisible(true);
}

void ActivatingObject::move(){
    ticksToLive--;
    if(forIceMan){
        //if it's not visible, and it's 4 away then make it visible
        if(!startsActive&& getWorld()->isNearIceMan(this, 4)){
            setVisible(true);
            startsActive = true;
            return;
        }
        //if it's within 3 away just go dead. you got picked up by iceman
        Actor *hero = getWorld()->findNearbyIceMan(this, 3);
        if(hero!=nullptr){
            setDead();
            getWorld()->playSound(pickupSound);
            addToIceMan();
            return;
        }
    }
    if(ticksToLive==0)
        setDead();
}

void ActivatingObject::setTicksToLive(){
    //formula given to me to set number of ticks
    int t = 300-10*getWorld()->getLevel();
    ticksToLive = std::max(100,t);
}

OilBarrel::OilBarrel(StudentWorld* world, int startX, int startY):ActivatingObject(world, startX, startY, IID_BARREL,SOUND_FOUND_OIL, true, false, false){
}

void OilBarrel::addToIceMan(){
    getWorld()->increaseScore(1000);
}

GoldNugget::GoldNugget(StudentWorld* world, int startX, int startY, bool temporary):ActivatingObject(world, startX, startY, IID_GOLD, SOUND_GOT_GOODIE, !temporary, temporary, temporary){
    temp = temporary;
    if(temp)
        setTicksToLive();
}

void GoldNugget::move(){
    ActivatingObject::move();
    if(temp){
        Actor *enemy = getWorld()->findNearbyPickerUpper(this, 3);
        if(enemy != nullptr){
            setDead();
            enemy->addGold();
            getWorld()->increaseScore(25);
        }
    }
}

void GoldNugget::addToIceMan(){
    getWorld()->increaseScore(10);
    getWorld()->giveIceManGold();
}

SonarKit::SonarKit(StudentWorld* world, int startX, int startY):ActivatingObject(world, startX, startY, IID_SONAR, SOUND_GOT_GOODIE, true, false, true){
    setTicksToLive();
}

void SonarKit::addToIceMan(){
    getWorld()->increaseScore(75);
    getWorld()->giveIceManSonar();
}

WaterPool::WaterPool(StudentWorld* world, int startX, int startY):ActivatingObject(world,startX, startY, IID_WATER_POOL, SOUND_GOT_GOODIE, true, false, true){
    setTicksToLive();
}


void WaterPool::addToIceMan(){
    getWorld()->giveIceManWater();
    getWorld()->increaseScore(100);
}
