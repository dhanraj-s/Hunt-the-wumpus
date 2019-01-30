// The MIT License (MIT)

// Copyright (c) 2019

//  Permission is hereby granted, free of charge, to any person obtaining a
//  copy of this software and associated documentation files (the "Software"),
//  to deal in the Software without restriction, including without limitation
//  the rights to use, copy, modify, merge, publish, distribute, sublicense,
//  and/or sell copies of the Software, and to permit persons to whom the
//  Software is furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in
//  all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
//  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
//  DEALINGS IN THE SOFTWARE.

/* code */

#include<iostream>
#include<vector>
#include<algorithm>
#include<limits>

constexpr int ROOMS = 20;
constexpr int BATS = 3;
constexpr int PITS = 3;

constexpr int END_GAME = -1;

struct Room
{
    std::vector<int>adjRooms{std::vector<int>(3)};
    bool player{false};
    bool bat{false};
    bool wump{false};
    bool pit{false};
};

class Player
{
    std::vector<int> adjRooms{std::vector<int>(3)};
    int currRoom;
    void setAdjRooms();
public:
    void setCurrRoom(int r){currRoom = r; setAdjRooms();}

    int room() const {return currRoom;}
    int getAdj(int i) const {return adjRooms[i];}
};

void Player::setAdjRooms()
{
    int t = 2+2*(currRoom&1);
    adjRooms[0] = ROOMS-1-currRoom;
    adjRooms[1] = (currRoom+t)%ROOMS;
    adjRooms[2] = (currRoom-t+20)%ROOMS;
}

class Map
{
    std::vector<Room> cave{std::vector<Room>(20)};
    std::vector<int> vacant; //vector to keep track of empty rooms

    Player p;

    void addWump();
    void addBats();
    void addPits();
    void addPlayer();

    void reportState();
    int input();

    int movePlayer(int);
    int shoot(int target);
    void batEncounter();
    int moveWump();
public:
    void init();
    void play();
    void printState(); //Only for debugging. Not part of the game.
};

void Map::addPlayer()
//spawn player
{
    int r = rand()%vacant.size();
    cave[vacant[r]].player = true;
    p.setCurrRoom(vacant[r]);

    //std::cout<<"Player in room "<<vacant[r]<<std::endl;
    vacant.erase(vacant.begin()+r);

    //no enemies should spawn adjacent to player
    for(int i = 0; i < 3; ++i)
        vacant.erase(std::find(vacant.begin(),vacant.end(),p.getAdj(i)));
}

void Map::addWump()
//spawns the wumpus in a random room
{
    int r = rand()%vacant.size();
    cave[vacant[r]].wump = true;
    //std::cout<<"Wumpus in room "<<vacant[r]<<std::endl;
    vacant.erase(vacant.begin()+r); //remove vacancy
}

void Map::addBats()
//spawns bats
{
    for(int i = 0; i < BATS; ++i){
        int r = rand()%vacant.size();
        cave[vacant[r]].bat = true;
        //std::cout<<"Bat in room "<<vacant[r]<<std::endl;
        vacant.erase(vacant.begin()+r);
    }
}

void Map::addPits()
//place pits
{
    for(int i = 0; i < PITS; ++i){
        int r = rand()%vacant.size();
        cave[vacant[r]].pit = true;
        //std::cout<<"Pit in room "<<vacant[r]<<std::endl;
        vacant.erase(vacant.begin()+r);
    }
}

void Map::printState()
//for debugging
{
    for(int i = 0; i < ROOMS; ++i){
        std::cout << "Room #" << i << ":" << std::endl;
        std::cout << "\tWumpus -> " << ((cave[i].wump)?"yes":"no") << std::endl;
        std::cout << "\tBat -> " << ((cave[i].bat)?"yes":"no") << std::endl;
        std::cout << "\tPit -> " << ((cave[i].pit)?"yes":"no") << std::endl;
        std::cout << "\tPlayer -> " << ((cave[i].player)?"yes":"no") << std::endl;
        std::cout << "\tAdjacent Rooms -> " <<(cave[i].adjRooms[0])<<", "
                  <<(cave[i].adjRooms[1])<<", "<<cave[i].adjRooms[2]<<std::endl;
        std::cout << std::endl;
    }
}

void Map::reportState()
{
    std::cout << "You are in room " << p.room() << std::endl;
    std::cout << "Adjacent rooms are " << p.getAdj(0) <<", "<<p.getAdj(1)
              <<", "<<p.getAdj(2)<<std::endl;

    if(cave[p.getAdj(0)].bat || cave[p.getAdj(1)].bat || cave[p.getAdj(2)].bat)
        std::cout << "I hear a bat." << std::endl;

    if(cave[p.getAdj(0)].pit || cave[p.getAdj(1)].pit || cave[p.getAdj(2)].pit)
        std::cout << "I feel a draft." << std::endl;

    if(cave[p.getAdj(0)].wump || cave[p.getAdj(1)].wump || cave[p.getAdj(2)].wump)
        std::cout << "I smell the wumpus." << std::endl;
}

int Map::movePlayer(int pos)
{
    if(pos != p.getAdj(0) && pos != p.getAdj(1) && pos != p.getAdj(2)){
        std::cout << "Invalid choice. Please move to an ADJACENT room." << std::endl;
        return 0;
    }
    cave[p.room()].player = false;
    cave[pos].player = true;
    vacant.push_back(p.room());
    p.setCurrRoom(pos);

    if(cave[p.room()].wump){
        std::cout << "The Wumpus got you! YOU LOSE." << std::endl;
        return END_GAME;
    }
    if(cave[p.room()].pit){
        std::cout << "You fell into a bottomless pit! YOU LOSE." << std::endl;
        return END_GAME;
    }
    if(cave[p.room()].bat){
        std::cout << "A giant bat takes you to another room!" << std::endl;
        batEncounter();
        return 0;
    }
}

int Map::moveWump()
//move wumpus to a random adjacent room
{
    int r = rand()%3;
    int pos = 0;
    for(; !(cave[pos].wump); ++pos); //get the room that contains the wumpus
    cave[pos].wump = false;
    if((cave[pos].wump && !(cave[pos].bat)) || (cave[pos].wump && !(cave[pos].pit)))
        vacant.push_back(pos);
    cave[cave[pos].adjRooms[r]].wump = true;
    if(cave[cave[pos].adjRooms[r]].player){
        std::cout << "The Wumpus got you! YOU LOSE." << std::endl;
        return END_GAME;
    }
    return 0;
}

int Map::shoot(int target)
{
    if(target != p.getAdj(0) && target != p.getAdj(1) && target != p.getAdj(2)){
        std::cout << "Invalid choice. Please target an ADJACENT room." << std::endl;
        return 0;
    }
    if(cave[target].wump){
        std::cout << "You killed the Wumpus! YOU WIN!" << std::endl;
        return END_GAME;
    }
    else if(cave[p.getAdj(0)].wump || cave[p.getAdj(1)].wump || cave[p.getAdj(2)].wump)
        return moveWump();
}

void Map::batEncounter()
{
    int r = rand()%vacant.size();
    cave[p.room()].player = false;
    vacant.push_back(p.room());
    cave[vacant[r]].player = true;
    p.setCurrRoom(vacant[r]);
    vacant.erase(vacant.begin()+r);
}

void Map::init()
//set up map
//place player, bats, pits and the wumpus
{
    //generate the dodecahedral cave
    for(int i = 0; i < ROOMS; ++i){
        int t = 2+2*(i&1);
        cave[i].adjRooms[0] = ROOMS-1-i;
        cave[i].adjRooms[1] = (i+t)%ROOMS;
        cave[i].adjRooms[2] = (i-t+20)%ROOMS;
        vacant.push_back(i);
    }

    //add entities
    addPlayer();
    addWump();
    addBats();
    addPits();

    //restore vacant rooms adjacent to player
    for(int i = 0; i < 3; ++i)
        vacant.push_back(p.getAdj(i));
}

void Map::play()
{
    reportState();
    while(input() != END_GAME){
        reportState();
    }
}

int Map::input()
{
    char c = 0;
    int r = -1;
    std::cout << "Type mXX(sXX) to move(shoot) to(at) room XX." << std::endl;
    while(1){
        std::cout << "Enter command: ";
        if(std::cin >> c >> r) { break; }
        else if(std::cin.fail() || (c != 'm' && c != 's')){
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<int>::max(),'\n');
            std::cout << "Invalid input. Type mXX(sXX) to move(shoot) to(at) room XX." << std::endl;
        }
    }
    return (c == 'm') ? movePlayer(r) : shoot(r);
}

int main()
{
    srand(unsigned(time(0)));
    Map game;
    game.init();
    game.play();
    //game.printState();
}
