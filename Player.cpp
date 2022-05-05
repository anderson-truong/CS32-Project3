#include "Player.h"
#include "Board.h"
#include "Game.h"
#include "globals.h"
#include <iostream>
#include <string>
#include <cctype>

using namespace std;

//*********************************************************************
//  AwfulPlayer
//*********************************************************************

class AwfulPlayer : public Player
{
  public:
    AwfulPlayer(string nm, const Game& g);
    virtual bool placeShips(Board& b);
    virtual Point recommendAttack();
    virtual void recordAttackResult(Point p, bool validShot, bool shotHit,
                                                bool shipDestroyed, int shipId);
    virtual void recordAttackByOpponent(Point p);
  private:
    Point m_lastCellAttacked;
};

AwfulPlayer::AwfulPlayer(string nm, const Game& g)
 : Player(nm, g), m_lastCellAttacked(0, 0)
{}

bool AwfulPlayer::placeShips(Board& b)
{
      // Clustering ships is bad strategy
    for (int k = 0; k < game().nShips(); k++)
        if ( ! b.placeShip(Point(k,0), k, HORIZONTAL))
            return false;
    return true;
}

Point AwfulPlayer::recommendAttack()
{
    if (m_lastCellAttacked.c > 0)
        m_lastCellAttacked.c--;
    else
    {
        m_lastCellAttacked.c = game().cols() - 1;
        if (m_lastCellAttacked.r > 0)
            m_lastCellAttacked.r--;
        else
            m_lastCellAttacked.r = game().rows() - 1;
    }
    return m_lastCellAttacked;
}

void AwfulPlayer::recordAttackResult(Point /* p */, bool /* validShot */,
                                     bool /* shotHit */, bool /* shipDestroyed */,
                                     int /* shipId */)
{
      // AwfulPlayer completely ignores the result of any attack
}

void AwfulPlayer::recordAttackByOpponent(Point /* p */)
{
      // AwfulPlayer completely ignores what the opponent does
}

//*********************************************************************
//  HumanPlayer
//*********************************************************************

bool getLineWithTwoIntegers(int& r, int& c)
{
    bool result(cin >> r >> c);
    if (!result)
        cin.clear();  // clear error state so can do more input operations
    cin.ignore(10000, '\n');
    return result;
}

// TODO:  You need to replace this with a real class declaration and
//        implementation.
class HumanPlayer : public Player
{
public: 
    HumanPlayer(string nm, const Game& g) : Player(nm, g) { }
    virtual ~HumanPlayer() {}
    virtual bool isHuman() const { return true;  }
    virtual bool placeShips(Board& b);
    virtual Point recommendAttack();
    virtual void recordAttackResult(Point p, bool validShot, bool shotHit,
        bool shipDestroyed, int shipId);
    virtual void recordAttackByOpponent(Point p);
};

bool HumanPlayer::placeShips(Board& b)
{
    cout << name() << " must place " << game().nShips() << " ships" << endl;
    
    for (int i = 0; i < game().nShips(); i++)
    {
        b.display(false);
        // Check valid direction
        char dirChar;
        while (true)
        {
            cout << "Enter h or v for direction of " << game().shipName(i) << " (length " << game().shipLength(i) << "): ";
            cin >> dirChar;
            cin.ignore(10000, '\n');

            // Valid direction
            if (dirChar == 'h' || dirChar == 'v')
                break;

            cout << "Direction must be h or v." << endl;
        }

        // Convert direction character to Direction
        Direction dir;
        if (dirChar == 'h')
            dir = HORIZONTAL;
        if (dirChar == 'v')
            dir = VERTICAL;

        // Check valid position
        int r, c;
        while (true)
        {
            cout << "Enter row and column of leftmost cell (e.g., 3 5): ";
            cin >> r >> c;

            // Incorrect input type
            if (!cin)
            {
                cin.clear();
                cin.ignore(10000, '\n');
                cout << "You must enter two integers." << endl;
                continue;
            }

            // Out of bounds or occupied position
            if (r < 0 || r >= game().rows() || c < 0 || c >= game().cols() || 
                !b.placeShip(Point(r, c), i, dir))
            {
                cin.ignore(10000, '\n');
                cout << "The ship can not be placed there." << endl;
                continue;
            }
            cin.ignore(10000, '\n');
            break;
        }
    }
    return true;
}

Point HumanPlayer::recommendAttack()
{
    int r, c;
    while (true)
    {
        cout << "Enter the row and column to attack (e.g., 3 5): ";
        cin >> r >> c;
        if (cin)
        {
            cin.ignore(10000, '\n');
            break;
        }
        cin.clear();
        cin.ignore(10000, '\n');
        cout << "You must enter two integers." << endl;
    }
    return Point(r, c);
}

void HumanPlayer::recordAttackResult(Point p, bool validShot, bool shotHit,
    bool shipDestroyed, int shipId) {
    return;
}

void HumanPlayer::recordAttackByOpponent(Point p)
{
    return;
}

//typedef AwfulPlayer HumanPlayer;

//*********************************************************************
//  MediocrePlayer
//*********************************************************************

// TODO:  You need to replace this with a real class declaration and
//        implementation.
typedef AwfulPlayer MediocrePlayer;
// Remember that Mediocre::placeShips(Board& b) must start by calling
// b.block(), and must call b.unblock() just before returning.

//*********************************************************************
//  GoodPlayer
//*********************************************************************

// TODO:  You need to replace this with a real class declaration and
//        implementation.
typedef AwfulPlayer GoodPlayer;

//*********************************************************************
//  createPlayer
//*********************************************************************

Player* createPlayer(string type, string nm, const Game& g)
{
    static string types[] = {
        "human", "awful", "mediocre", "good"
    };
    
    int pos;
    for (pos = 0; pos != sizeof(types)/sizeof(types[0])  &&
                                                     type != types[pos]; pos++)
        ;
    switch (pos)
    {
      case 0:  return new HumanPlayer(nm, g);
      case 1:  return new AwfulPlayer(nm, g);
      case 2:  return new MediocrePlayer(nm, g);
      case 3:  return new GoodPlayer(nm, g);
      default: return nullptr;
    }
}
