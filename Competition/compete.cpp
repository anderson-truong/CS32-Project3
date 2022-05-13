#include "Game.h"
#include "Player.h"
#include "Board.h"
#include <iostream>
#include <string>

using namespace std;

bool addStandardShips(Game& g)
{
    return g.addShip(5, 'A', "aircraft carrier")  &&
           g.addShip(4, 'B', "battleship")  &&
           g.addShip(3, 'D', "destroyer")  &&
           g.addShip(3, 'S', "submarine")  &&
           g.addShip(2, 'P', "patrol boat");
}

int main()
{
    const int NTRIALS = 10;
    string name1 = "BOB";
    string name2 = "MEGAMIND";
    cout << "COMPETITION BETWEEN " << name1 << " AND " << name2 << endl;
    cin.ignore();
    int p1Wins = 0;
    int p2Wins = 0;

    for (int k = 1; k <= NTRIALS; k++)
    {
        cout << "============================= Game " << k
            << " =============================" << endl;
        Game g(10, 10);
        addStandardShips(g);
        Player* p1 = createPlayer("good1", name1, g);
        Player* p2 = createPlayer("good2", name2, g);
        Player* winner = (k % 2 == 1 ?
            g.play(p1, p2, false) : g.play(p2, p1, false));
        if (winner == p1)
            p1Wins++;
        if (winner == p2)
            p2Wins++;
        delete p1;
        delete p2;
    }
    
    if (p1Wins == p2Wins)
        cout << "DRAW!" << endl;
    else
    {
        string winner = p1Wins > p2Wins ? name1 : name2;
        cout << "WINNER IS " << winner << "!" << endl;
    }
    cout << name1 << " won " << p1Wins << " out of " << NTRIALS << " games." << endl;
    cout << name2 << " won " << p2Wins << " out of " << NTRIALS << " games." << endl;
}