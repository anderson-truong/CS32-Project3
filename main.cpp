#include "Game.h"
#include "Player.h"
#include "Board.h"
#include <iostream>
#include <iomanip>
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

////========================================================================
//// Timer t;                 // create a timer and start it
//// t.start();               // start the timer
//// double d = t.elapsed();  // milliseconds since timer was last started
////========================================================================
//
//#include <chrono>
//
//class Timer
//{
//public:
//    Timer()
//    {
//        start();
//    }
//    void start()
//    {
//        m_time = std::chrono::high_resolution_clock::now();
//    }
//    double elapsed() const
//    {
//        std::chrono::duration<double, std::milli> diff =
//            std::chrono::high_resolution_clock::now() - m_time;
//        return diff.count();
//    }
//private:
//    std::chrono::high_resolution_clock::time_point m_time;
//};
//
//int main()
//{
//    Game g(10, 10);
//    g.addShip(5, 'A', "aircraft carrier");
//    g.addShip(4, 'B', "battleship");
//    g.addShip(3, 'D', "destroyer");
//    g.addShip(3, 'S', "submarine");
//    g.addShip(2, 'P', "patrol boat");
//    Board b(g);
//    bool shotHit = false;
//    bool destroyed = false;
//    int id = -1;
//    Player* p = createPlayer("good", "bob", g);
//    p->placeShips(b);
//    for (int i = 0; i < 100; i++)
//    {
//        Timer timer;
//        timer.start();
//        Point a = p->recommendAttack();
//        bool valid = b.attack(a, shotHit, destroyed, id);
//        p->recordAttackResult(a, valid, shotHit, destroyed, id);
//        p->recordAttackByOpponent(a);
//        cout  << timer.elapsed() << endl;
//    }
//    
//}

int main()
{
    const int NTRIALS = 1;

    cout << "Select one of these choices for an example of the game:" << endl;
    cout << "  1.  A mini-game between two mediocre players" << endl;
    cout << "  2.  A mediocre player against a human player" << endl;
    cout << "  3.  A " << NTRIALS << "-game match between a mediocre and an awful player, with no pauses" << endl;
    cout << "Enter your choice: ";
    string line;
    getline(cin, line);
    if (line.empty())
    {
        cout << "You did not enter a choice" << endl;
    }
    else if (line[0] == '1')
    {
        Game g(2, 3);
        g.addShip(2, 'R', "rowboat");
        Player* p1 = createPlayer("mediocre", "Popeye", g);
        Player* p2 = createPlayer("mediocre", "Bluto", g);
        cout << "This mini-game has one ship, a 2-segment rowboat." << endl;
        g.play(p1, p2);
        delete p1;
        delete p2;
    }
    else if (line[0] == '2')
    {
        Game g(10, 10);
        addStandardShips(g);
        Player* p1 = createPlayer("mediocre", "Mediocre Midori", g);
        Player* p2 = createPlayer("human", "Shuman the Human", g);
        g.play(p1, p2);
        delete p1;
        delete p2;
    }
    else if (line[0] == '3')
    {
        int nMediocreWins = 0;

        for (int k = 1; k <= NTRIALS; k++)
        {
            cout << "============================= Game " << k
                << " =============================" << endl;
            Game g(10, 10);
            addStandardShips(g);
            Player* p1 = createPlayer("awful", "Awful Audrey", g);
            Player* p2 = createPlayer("mediocre", "Mediocre Mimi", g);
            Player* winner = (k % 2 == 1 ?
                g.play(p1, p2, false) : g.play(p2, p1, false));
            if (winner == p2)
                nMediocreWins++;
            delete p1;
            delete p2;
        }
        cout << "The mediocre player won " << nMediocreWins << " out of "
            << NTRIALS << " games." << endl;
        // We'd expect a mediocre player to win most of the games against
        // an awful player.  Similarly, a good player should outperform
        // a mediocre player.
    }
    else if (line[0] == '4')
    {
        int nMediocreWins = 0;

        for (int k = 1; k <= NTRIALS; k++)
        {
            cout << "============================= Game " << k
                << " =============================" << endl;
            Game g(10, 10);
            addStandardShips(g);
            Player* p1 = createPlayer("mediocre", "smol brain", g);
            Player* p2 = createPlayer("good", "MEGAMIND", g);
            Player* winner = (k % 2 == 1 ?
                g.play(p1, p2, false) : g.play(p2, p1, false));
            if (winner == p2)
                nMediocreWins++;
            delete p1;
            delete p2;
        }
        cout << "MEGAMIND won " << nMediocreWins << " out of "
            << NTRIALS << " games." << endl;
        // We'd expect a mediocre player to win most of the games against
        // an awful player.  Similarly, a good player should outperform
        // a mediocre player.
    }
    else if (line[0] == '5')
    {
        string name;
        cout << "WHAT IS YOUR NAME WORTHY CHALLENGER? ";
        getline(cin, name);
        Game g(10, 10);
        addStandardShips(g);
        Player* p1 = createPlayer("human", name, g);
        Player* p2 = createPlayer("good", "MEGAMIND", g);
        g.play(p1, p2);
        delete p1;
        delete p2;
    }
    else
    {
        cout << "That's not one of the choices." << endl;
    }
}