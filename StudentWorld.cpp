#include "StudentWorld.h"
#include <string>
#include <queue>
using namespace std;

GameWorld* createStudentWorld(string assetDir)
{
	return new StudentWorld(assetDir);
}

StudentWorld::StudentWorld(std::string assetDir):GameWorld(assetDir){}

StudentWorld::~StudentWorld(){
    //destructing IceArray
    for(int i = 0; i != 64; i++){
        for(int j = 0; j != 64; j++){
            if(IceArray[i][j] != nullptr){
                delete IceArray[i][j];
                IceArray[i][j] = nullptr;
            }
        }
    }
    
    //destructing Iceman object
    delete hero;
    
    //destructing all actors
    while(ActorList.size() != 0){
        delete ActorList[0];
        ActorList.erase(ActorList.begin());
    }
}

int StudentWorld::init(){
    
    //set number of ticks to spawn new protesters
    int tt = 200 - getLevel();
    ticksPerNewP = max(25 ,tt);
    //set maximum number of protesters
    int pt = 2 + getLevel()*1.5;
    maxP = min(15,pt);
    //set probability for hardcore protester
    int ht = getLevel()*10 + 30;
    chanceForHardcore = min(90, ht);
    //set counter for new protesters
    tickCounter = ticksPerNewP + 1;

    //constructing Iceman
    hero = new IceMan(this, 30, 60);
    
    int bX;
    int bY;
    //constructing boulders
    int bt = getLevel()/2 + 2;
    int numBoulders = min(bt, 9);
    for(int q = 0; q != numBoulders; q++){
        //generate points suitable for boulder spawn
        generateBoulderPoints(bX,bY);
        Boulder *n = new Boulder(this,bX,bY);
        ActorList.push_back(n);
    }
    
    
    //constructing nuggets
    int gt = 5-getLevel()/2;
    int nuggetsLeft = max(gt, 2);
    for(int q = 0; q != nuggetsLeft; q++){
        //generate points suitable for gold spawn
        generateHiddenPoints(bX,bY);
        GoldNugget *k = new GoldNugget(this,bX,bY,false);
        ActorList.push_back(k);
    }
    
    //set number of total barrels
    int lt = 2+getLevel();
    barrelsLeft = min(lt, 21);
    //constructing barrels
    for(int q = 0; q != barrelsLeft; q++){
        //generate points suitable for barrel spawn
        generateHiddenPoints(bX,bY);
        OilBarrel *r = new OilBarrel(this, bX, bY);
        ActorList.push_back(r);
    }
    
    //constructing ice field
    for(int i = 0; i != 64; i++){
        for(int j = 0; j != 64; j++){
            bool isBoulderSquare = false;
            if(j>=60){
                //no ice above 60
                IceArray[i][j] = nullptr;
                continue;
            }
            else if(i <= 33 && i >= 30 && j>=4 && j<=59){
                //no ice in ice shaft
                IceArray[i][j] = nullptr;
                continue;
            }
            else{
                for(int t = 0; t != ActorList.size(); t++){
                    if(!ActorList[t]->canActorsPassThroughMe()){
                        //check if Boulder overlaps the coordinate
                        int sqAbove = j-ActorList[t]->getY();
                        int sqRight = i-ActorList[t]->getX();
                        if(sqRight<=3 && sqRight>=0 && sqAbove>=0 && sqAbove<=3){
                            isBoulderSquare = true;
                            IceArray[i][j] = nullptr;
                        }
                    }
                }
                if(!isBoulderSquare){
                    //if not occupied by boulders, insert ice
                    IceArray[i][j] = new Ice(this,i,j);
                }
            }
        }
    }
    
    Actor *f;
 //   constructing first Protester. uses "pick-a-number" algorithm to determine whether it should be hardcore
    if(rand()%(chanceForHardcore) == 0)
        f = new HardcoreProtester(this,60,60);
    else
        f = new HardcoreProtester(this,60,60);
    
    ActorList.push_back(f);
    tickCounter = ticksPerNewP;
    setMaze(f);
    return GWSTATUS_CONTINUE_GAME;
}


int StudentWorld::move(){
    //Delete any dead actors
    for(int t = 0; t!= ActorList.size(); t++){
        if(!ActorList[t]->isAlive()){
            if(ActorList[t]->needsToBePickedUpToFinishLevel())
                barrelsLeft--;
            delete ActorList[t];
            ActorList.erase(ActorList.begin() + t);
            t--;
        }
    }
    
    //case: Iceman died
    if(!hero->isAlive()){
        playSound(SOUND_PLAYER_GIVE_UP);
        return GWSTATUS_PLAYER_DIED;
    }
    
    //case: no barrels left
    if(barrelsLeft == 0)
        return GWSTATUS_FINISHED_LEVEL;

    hero->move();


   setDisplayText();
    for(int t = 0; t!= ActorList.size(); t++){
        ActorList[t]->move();
    }
    

    //constructing protesters
    if(tickCounter <= 0 && maxP > 0){
        //uses "pick-a-number" strategy to determine whether it should be hardcore
        Actor *n;
        if(rand()%chanceForHardcore == 0)
            n = new HardcoreProtester(this, 60,60);
        else
            n = new RegularProtester(this,60,60);
        
        ActorList.push_back(n);
        tickCounter = ticksPerNewP + 1;
        maxP--;
    }
    tickCounter--; //counts ticks until next protester spawn

    //Constructing Water or Sonar
    int G = getLevel()*25+300;
    //1 in G chance that Goodie will be added during tick
    if(rand()%G == 0){
        //1 in 5 chance for SonarKit
        if(rand()%5 == 0){
            SonarKit *n = new SonarKit(this, 0, 60);
            ActorList.push_back(n);
        }
        else{
            int x, y;
            generateOpenPoints(x,y); //generate points for water spawn
            WaterPool *n = new WaterPool(this,x,y);
            ActorList.push_back(n);
        }
    }
    
    return GWSTATUS_CONTINUE_GAME;
}

void StudentWorld::cleanUp(){
    for(int i = 0; i != 64; i++){
        for(int j = 0; j != 64; j++){
            if(IceArray[i][j] != nullptr){
                delete IceArray[i][j];
                IceArray[i][j] = nullptr;
            }
        }
    }
    
    delete hero;
    
    while(ActorList.size() != 0){
        delete ActorList[0];
        ActorList.erase(ActorList.begin());
    }
}

void StudentWorld::setDisplayText(){
    //remember to take into account spaces in front of number, so use num/10 to figure out number of digits
    string text = "Lvl: ";
    if(getLevel()/10 == 0)
        text += " ";
    text += to_string(getLevel()) + "  Lives: " + to_string(getLives()) + "  Hlth: ";
    if((hero->getHitPoints()*10)/100 == 0)
        text += " ";
    if((hero->getHitPoints()*10)/10 == 0)
        text += " ";
    text += to_string(hero->getHitPoints()*10) + "%  Wtr: ";
    if(hero->getWater()/10 == 0)
        text += " ";
    text += to_string(hero->getWater()) + "  Gld: ";
    if(hero->getGold()/10 == 0)
        text += " ";
    text += to_string(hero->getGold()) + "  Oil Left: ";
    if(barrelsLeft/10 == 0)
        text += " ";
    text += to_string(barrelsLeft) + "  Sonar: ";
    if(hero->getSonar()/10 == 0)
        text += " ";
    text += to_string(hero->getSonar()) + "  Scr: ";
    if(getScore()/100000 == 0)
        text += to_string(0);
    if(getScore()/10000 == 0)
        text += to_string(0);
    if(getScore()/1000 == 0)
        text += to_string(0);
    if(getScore()/100 == 0)
        text += to_string(0);
    if(getScore()/10 == 0)
        text += to_string(0);
    text += to_string(getScore());
    
    setGameStatText(text);
}

void StudentWorld::generateBoulderPoints(int& x, int& y){
    //x between 0 and 60, y between 20 and 60
    x = rand()%61;
    y = rand()%37+20;//20 to 56 rand()%(max-min+1) + min
    //can't be at the ice shaft
    if(x>=27 && x <= 33){
        generateBoulderPoints(x,y);
        return;
    }
    for(int i = 0; i != ActorList.size(); i++){
        //check if point generated has been occupied by boulders already
        if(!ActorList[i]->canActorsPassThroughMe()){
            if(checkRadius(ActorList[i],x,y,6)){
                generateBoulderPoints(x, y);
                return;
            }
        }
    }
}


void StudentWorld::generateHiddenPoints(int &x, int &y){
    //x between 0 and 60, y between 0 and 56
    x = rand()%61;
    y = rand()%57;
    
    //can't be at the ice shaft
    if(x>=27 && x <= 33){
        generateHiddenPoints(x,y);
        return;
    }
    
    for(int i = 0; i != ActorList.size(); i++){
        //check if point generated has been occupied by other points
        if(checkRadius(ActorList[i],x,y,6)){
            generateHiddenPoints(x, y);
            return;
        }
    }
}

void StudentWorld::generateOpenPoints(int &x, int&y){
    //x between 0 and 60, y between 0 and 60
    x = rand()%61;
    y = rand()%61;
    
    for(int i = 0; i != 4; i++){
        for(int j = 0; j!=4; j++){
            //requirement is points can't be where ice is
            if(IceArray[x+i][y+j] != nullptr){
                generateOpenPoints(x,y);
                return;
            }
        }
    }
}

bool StudentWorld::checkRadius(Actor* a, int x, int y, int radius) const{
    //use euclidean distance formula
    int distance = sqrt(pow(a->getX() - x,2)+pow(a->getY() - y,2));
    return (distance <= radius);
}

void StudentWorld::clearIce(int x, int y){
    //isDug tracks whether Ice was dug, so it would make the sound accordingly
    bool isDug = false;
    //delete the ice
    for(int i = 0; i != 4; i++){
        for(int j = 0; j != 4; j++){
            Ice* q = IceArray[x+i][y+j];
            if(q != nullptr){
                delete q;
                IceArray[x+i][y+j]= nullptr;
                isDug = true;
            }
        }
    }
    if(isDug)
        playSound(SOUND_DIG);
}

bool StudentWorld::canActorMoveTo(Actor* a, int x, int y) const{
    //out of bounds
    if(y > 60 || y < 0 || x > 60 || x <0)
        return false;
    
    //check boulders
    for(int i = 0; i != ActorList.size(); i++){
        if(!ActorList[i]->canActorsPassThroughMe()){
            if(a == ActorList[i])
                continue;
            if(checkRadius(ActorList[i],x,y,3))
                return false;
        }
    }
    
    //case: a is IceMan
    if(a->canDigThroughIce())
        return true;
    //case: a isn't, so check IceBlock
    for(int i = 0; i != 4; i++){
        for(int j = 0; j!=4; j++){
            if(IceArray[x+i][y+j] != nullptr)
                return false;
        }
    }
    return true;
}

int StudentWorld::annoyAllNearbyActors(Actor* annoyer, int points, int radius){
    int numAnnoyed = 0;
    for(int i = 0; i != ActorList.size(); i++){
        //check protesters within radius and annoy them
        if(ActorList[i]->huntsIceMan() && checkRadius(ActorList[i], annoyer->getX(), annoyer->getY(), radius)){
            ActorList[i]->annoy(points);
            numAnnoyed++;
        }
    }
    return numAnnoyed;
}


void StudentWorld::revealAllNearbyObjects(int x, int y, int radius){
    playSound(SOUND_SONAR);
    for(int i = 0; i != ActorList.size(); i++){
        //check barrels and gold
        if(ActorList[i]->needsToBePickedUpToFinishLevel() || ActorList[i]->canBribe()){
            //make them visible if they're within 12 radius
            if(isNearIceMan(ActorList[i], 12)){
                ActorList[i]->makeVisible();
            }
        }
    }
}

Actor* StudentWorld::findNearbyIceMan(Actor* a, int radius) const{
    if(isNearIceMan(a,radius))
        return hero;
    return nullptr;
}

void StudentWorld::annoyIceMan(int points){
    hero->annoy(points);
}

bool StudentWorld::facingTowardIceMan(Actor* a) const{
    //used IceMan's location to check whether it is in the direction of a
    switch(a->getDirection()){
        case GraphObject::left:
            if(hero->getX() < a->getX())
                return true;
            break;
        case GraphObject::right:
            if(hero->getX() > a->getX())
                return true;
            break;
        case GraphObject::up:
            if(hero->getY() > a->getY())
                return true;
            break;
        case GraphObject::down:
            if(hero->getY() < a->getY())
                return true;
            break;
        default:
            break;
    }
    return false;

}

bool StudentWorld::isNearIceMan(Actor* a, int radius) const{
    if (checkRadius(a, hero->getX(), hero->getY(), radius))
        return true;
    return false;
}

GraphObject::Direction StudentWorld::lineOfSightToIceMan(Actor* a) const{
    int x = a->getX();
    int y = a->getY();
    //case of vertical
    if(abs(hero->getX() - x) <= 3){
        //above
        if(hero->getY()>y){
            for(int i = 0; i != (hero->getY()-y); i++){
                //ice or boulder in the way
                if(!canActorMoveTo(a, x,y+i))
                    return GraphObject::none;
            }
            return GraphObject::up;
        }
        //below
        else{
            for(int i = (y-hero->getY()); i != 0; i--){
                //ice or boulder in the way
                if(!canActorMoveTo(a, x,y-i))
                    return GraphObject::none;
            }
            return GraphObject::down;
        }
    }
    //case of horizontal
    else if(abs(hero->getY()-y) <= 3){
        //right
        if(hero->getX()>x){
            for(int i = 0; i != (hero->getX()-x); i++){
                //ice or boulder in the way
                if(!canActorMoveTo(a, x+i,y))
                    return GraphObject::none;
            }
            return GraphObject::right;
        }
        //left
        else{
            for(int i = (x-hero->getX()); i != 0; i--){
                //ice or boulder in the way
                if(!canActorMoveTo(a, x-i,y))
                    return GraphObject::none;
            }
            return GraphObject::left;
        }
    }
    return GraphObject::none;
}

void StudentWorld::setMaze(Actor *a){
    //'.' for spaces one can move to. X for spaces one can't move to
    for(int i = 0; i != 61; i++){
        for(int j = 0; j!= 61; j++){
            if(canActorMoveTo(a, i, j))
                maze[i][j] = '.';
            else
                maze[i][j] = 'X';
        }
    }
}

void StudentWorld::setGridToLeave(Actor *a){
    setMaze(a);
    //set trackSteps to a large number to make every space unmoveable. We'll change the moveable ones to lower numbers so we can use comparison to decide which direction to go
    for(int i = 0; i != 61; i++){
        for(int j = 0; j!= 61; j++){
            trackSteps[i][j] = 300;
        }
    }
    
    //breadth first search
    queue<Coord> s;
    s.push(Coord(60,60,0));
    maze[60][60] = '*';
    trackSteps[60][60] = 0;
    while(!s.empty()){
        int ax = s.front().x;
        int ay = s.front().y;
        int steps = s.front().steps; // steps away from exit
        s.pop();
        
        //'*' to represent spaces passed through. record steps into trackSteps
        if(ay > 0 && maze[ax][ay-1] == '.'){
            s.push(Coord(ax,ay-1, steps+1));
            maze[ax][ay-1] = '*';
            trackSteps[ax][ay-1] = steps+1;
        }
        if(ay < 60 && maze[ax][ay+1] == '.'){
            s.push(Coord(ax,ay+1, steps+1));
            maze[ax][ay+1] = '*';
            trackSteps[ax][ay+1] = steps+1;
        }
        if(ax > 0 && maze[ax-1][ay]=='.'){
            s.push(Coord(ax-1,ay, steps+1));
            maze[ax-1][ay] = '*';
            trackSteps[ax-1][ay] = steps+1;
        }
        if(ax < 60 && maze[ax+1][ay]=='.'){
            s.push(Coord(ax+1,ay,steps+1));
            maze[ax+1][ay] = '*';
            trackSteps[ax+1][ay] = steps+1;
        }
    }
}


GraphObject::Direction StudentWorld::determineFirstMoveToExit(int x, int y){
    //check the spot 1 step away that moves closer to the exit
    int pos = trackSteps[x][y];
    if(x<60 && trackSteps[x+1][y] < pos)
        return GraphObject::right;
    if(y<60 && trackSteps[x][y+1] < pos)
        return GraphObject::up;
    if(x>0 && trackSteps[x-1][y] < pos)
        return GraphObject::left;
    if(y>0 && trackSteps[x][y-1] < pos)
        return GraphObject::down;
    
    return GraphObject::none;
}

GraphObject::Direction StudentWorld::determineFirstMoveToIceMan(Actor *a, int x, int y){

    //determine max steps between IceMan and protester to use cell service surveillance
    int M = 16 + getLevel()*2;
    if(!checkRadius(hero, x, y, M))
        return GraphObject::none;
    
    
    int hx = hero->getX();
    int hy = hero->getY();
    
    //reset the maze
    setMaze(a);
    //clear represents the spots we can move to
    char clear = maze[hx][hy];
    
    //make trackIceMan array all large numbers so we can edit it with smaller numbers to determine fastest way out
    for(int i = 0; i != 61; i++){
        for(int j = 0; j!= 61; j++){
            trackIceMan[i][j] = 300;
        }
    }
    
    //breadth first search!
    queue<Coord> t;
    t.push(Coord(hx,hy,0));
    maze[hx][hy] = '@'; //represents spots we've moved to
    trackIceMan[hx][hy] = 0;
    while(!t.empty()){
        int bx = t.front().x;
        int by = t.front().y;
        int s = t.front().steps;
        t.pop();
        
        if(by > 0 && maze[bx][by-1] == clear){
            t.push(Coord(bx,by-1, s+1));
            maze[bx][by-1] = '@';
            trackIceMan[bx][by-1] = s+1;
        }
        if(by < 60 && maze[bx][by+1] == clear){
            t.push(Coord(bx,by+1,s+1));
            maze[bx][by+1] = '@';
            trackIceMan[bx][by+1] = s+1;
        }
        if(bx > 0 && maze[bx-1][by]==clear){
            t.push(Coord(bx-1,by,s+1));
            maze[bx-1][by] = '@';
            trackIceMan[bx-1][by] = s+1;
        }
        if(bx < 60 && maze[bx+1][by]==clear){
            t.push(Coord(bx+1,by,s+1));
            maze[bx+1][by] = '@';
            trackIceMan[bx+1][by] = s+1;
        }
    }
    
    //uses comparisons to find less number of steps to get out
    int movesToIceMan = trackIceMan[x][y];
    if(movesToIceMan > M)
        return GraphObject::none;
    if(x<60 && trackIceMan[x+1][y] < movesToIceMan)
        return GraphObject::right;
    if(y<60 && trackIceMan[x][y+1] < movesToIceMan)
        return GraphObject::up;
    if(x>0 && trackIceMan[x-1][y] < movesToIceMan)
        return GraphObject::left;
    if(y>0 && trackIceMan[x][y-1] < movesToIceMan)
        return GraphObject::down;
    
    return GraphObject::none;
}

Actor* StudentWorld::findNearbyPickerUpper(Actor *a, int radius) const{
    
    //look for Agent that can pick things up within the radius and return it 
    for(int i = 0; i != ActorList.size(); i++){
        if(ActorList[i]->canPickThingsUp() && checkRadius(ActorList[i],a->getX(),a->getY(), radius)){
            return ActorList[i];
        }
    }
    return nullptr;
}
