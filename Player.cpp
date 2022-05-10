#include "Player.h"
#include "Board.h"
#include "Game.h"
#include "globals.h"
#include "utility.h"
#include <iostream>
#include <string>
#include <iomanip>

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
    
    // Perform input validation routine for each ship
    for (int i = 0; i < game().nShips(); i++)
    {
        b.display(false);

        //======================
        // Validate direction input
        //======================
        char dirChar = ' ';

        // Loop until player enters in valid direction
        while (dirChar != 'h' && dirChar != 'v')
        {
            cout << "Enter h or v for direction of " << game().shipName(i) << " (length " << game().shipLength(i) << "): ";
            cin >> dirChar;
            cin.ignore(10000, '\n');

            // Break out of loop of valid character 'h' or 'v'
            if (dirChar == 'h' || dirChar == 'v')
                break;

            cout << "Direction must be h or v." << endl;
        }
        // Convert character to Direction type
        Direction dir = dirChar == 'h' ? HORIZONTAL : VERTICAL;


        //=========================
        // Validate ship position input
        //=========================
        int r, c;

        // Loop until player enters in valid coordinates
        while (true)
        {
            cout << "Enter row and column of leftmost cell (e.g., 3 5): ";

            // Invalid input type
            if (!getLineWithTwoIntegers(r, c))
            {
                cout << "You must enter two integers." << endl;
                continue;
            }

            // Invalid point (out of bounds) or cannot place ship (space goes out of bounds/is occupied)
            if (!game().isValid(Point(r, c)) || !b.placeShip(Point(r, c), i, dir))
            {
                cout << "The ship can not be placed there." << endl;
                continue;
            }
            
            // If input is valid, break out of loop
            break;
        }
    }
    return true;
}

Point HumanPlayer::recommendAttack()
{
    int r, c;

    // Loop until Human enters in valid coordinates
    while (true)
    {
        cout << "Enter the row and column to attack (e.g., 3 5): ";

        // Invalid input type
        if (!getLineWithTwoIntegers(r, c))
        {
            cout << "You must enter two integers." << endl;
            continue;
        }

        // If input is valid, break out of loop
        break;
    }
    return Point(r, c);
}

void HumanPlayer::recordAttackResult(Point p, bool validShot, bool shotHit,
    bool shipDestroyed, int shipId) 
{
    // No code necessary, user makes decisiond
}

void HumanPlayer::recordAttackByOpponent(Point p)
{
    // No code necessary, user makes decisions
}

//*********************************************************************
//  MediocrePlayer
//*********************************************************************

class MediocrePlayer : public Player
{
public:
    MediocrePlayer(string nm, const Game& g) : Player(nm, g), m_moveState(1), transitionPoint(5, 5) { }
    virtual bool placeShips(Board& b);
    bool prevAttacksContains(Point& p);
    virtual Point recommendAttack();
    virtual void recordAttackResult(Point p, bool validShot, bool shotHit,
        bool shipDestroyed, int shipId);
    virtual void recordAttackByOpponent(Point p) { return; }

    bool recursivePlace(Board& b, Point p, int shipId);
private:
    int m_moveState;
    Point transitionPoint;
    vector<Point> prevAttacks;
};
// Remember that Mediocre::placeShips(Board& b) must start by calling
// b.block(), and must call b.unblock() just before returning.
            
bool MediocrePlayer::recursivePlace(Board& b, Point p, int shipId)
{
    // Store ship direction (to potentially unplace later)
    Direction chosenDir;

    // Tries to place ship horizontally first
    if (b.placeShip(p, shipId, HORIZONTAL))
        chosenDir = HORIZONTAL;

    // Can't place horizontally, try vertically
    else if (b.placeShip(p, shipId, VERTICAL))
        chosenDir = VERTICAL;

    // Can't place in any direction, try different location
    else
    {
        // Move one column over
        Point newPoint(p.r, p.c + 1);

        // If new column exceeds column size, move to next row
        if (newPoint.c >= game().cols())
        {
            newPoint.r++;
            newPoint.c = 0;
        }

        // If row exceeds row size, unable to fit ship
        if (newPoint.r >= game().rows())
            return false;

        // Try to place ship at new position
        if (recursivePlace(b, newPoint, shipId))
            return true;

        // Cannot place ship at any position
        return false;
    }

    // Finished placing (no more ships to place)
    if (shipId + 1 >= game().nShips())
        return true;

    // Begin recursively placing with the next ship ID
    if (recursivePlace(b, Point(0, 0), shipId + 1))
        return true;
    // Able to place, but branch fails
    b.unplaceShip(p, shipId, chosenDir);

    // Move one column over
    Point newPoint(p.r, p.c + 1);
    //cerr << "Branch failed, moving to (" << newPoint.r << "," << newPoint.c << ")" << endl;
    // Move to new row
    if (newPoint.c >= game().cols())
    {
        newPoint.r++;
        newPoint.c = 0;
    }
    // Cannot fit ship
    if (newPoint.r >= game().rows())
        return false;
    if (recursivePlace(b, newPoint, shipId))
        return true;
    return false;
}

bool MediocrePlayer::placeShips(Board& b)
{
    if (game().nShips() <= 0)
        return false;
    b.block();
    Point start(0, 0);
    bool canPlace = recursivePlace(b, start, 0);
   // b.display(false);
    b.unblock();
   // b.display(false);
    return canPlace;
}

bool MediocrePlayer::prevAttacksContains(Point& p)
{
    auto it = std::find_if(prevAttacks.begin(), prevAttacks.end(), [&p](Point prev) { return p.r == prev.r && p.c == prev.c; });
    // If equal to end(), then cannot find and return true
    // If found, return false
    return it != prevAttacks.end();
}

Point MediocrePlayer::recommendAttack()
{ 
    if (m_moveState == 1)
    {
        Point randomPoint;
        do
        {
            randomPoint = game().randomPoint();
        } while (prevAttacksContains(randomPoint));
        prevAttacks.push_back(randomPoint);
        return randomPoint;
    }
    if (m_moveState == 2)
    {
        // Game has ships of length 6 or more
        for (int i = 0; i < game().nShips(); i++)
            if (game().shipLength(i) >= 6)
            {
                m_moveState = 1;
                return recommendAttack();
            }

        vector<Point> validPoints;
        for (int i = transitionPoint.r - 4; i <= transitionPoint.r + 4; i++)
        {
            Point newValidPoint(i, transitionPoint.c);
            if (game().isValid(newValidPoint))
                validPoints.push_back(newValidPoint);
        }
        for (int i = transitionPoint.c - 4; i <= transitionPoint.c + 4; i++)
        {
            Point newValidPoint(transitionPoint.r, i);
            if (game().isValid(newValidPoint))
                validPoints.push_back(newValidPoint);
        }
        //for (Point p : validPoints)
        //    cerr << p.r << ", " << p.c << endl;
        
        // While randomPoint is not a previous attack
        Point randomPoint(transitionPoint);
        do
        {
            randomPoint = validPoints[randInt(validPoints.size())];
        } while (prevAttacksContains(randomPoint));
        prevAttacks.push_back(randomPoint);
        return randomPoint;
    }
    return Point();
}

void MediocrePlayer::recordAttackResult(Point p, bool validShot, bool shotHit, bool shipDestroyed, int shipId) 
{
    // Hits ship but does not destroy
    if (m_moveState == 1 && shotHit && !shipDestroyed)
    {
        transitionPoint = p;
        m_moveState = 2;
    }
    if (m_moveState == 2 && shipDestroyed)
    {
        m_moveState = 1;
    }
}

//*********************************************************************
//  GoodPlayer
//*********************************************************************

class GoodPlayer : public Player
{
public:
    GoodPlayer(string nm, const Game& g);
    bool recursivePlace(Board& b, int shipId);
    virtual bool placeShips(Board& b);
    virtual Point recommendAttack();
    virtual void recordAttackResult(Point p, bool validShot, bool shotHit,
        bool shipDestroyed, int shipId);
    virtual void recordAttackByOpponent(Point p) { return; }

    bool validPoint(Point p);
    bool validPlace(Point p, int shipId, Direction dir);
    void huntProb();
    void targetProb();
    void resetProbArray();

    void printProbArray();
private:
    vector<Point> m_missed;
    vector<Point> m_destroyed;
    vector<ShipType> shipsAlive;
    int huntOrTarget; // 0 = Hunt, 1 = Target
    int probArray[MAXROWS][MAXCOLS];
};

void GoodPlayer::printProbArray()
{
    for (int r = 0; r < game().rows(); r++)
    {
        for (int c = 0; c < game().cols(); c++)
        {
            cerr << setw(2) << probArray[r][c];
            if (c != game().cols() - 1)
                cerr << ", ";
        }
        cerr << endl;
    }
}

GoodPlayer::GoodPlayer(string nm, const Game& g) : Player(nm, g), huntOrTarget(0)
{ 
    resetProbArray();
    for (int n = 0; n < g.nShips(); n++)
        shipsAlive.push_back(ShipType(g.shipLength(n), g.shipSymbol(n), g.shipName(n)));
}

bool GoodPlayer::recursivePlace(Board& b, int shipId)
{
    // Random point
    Point p(randInt(game().rows()), randInt(game().cols()));
    // Random direction;
    int dirChoice = randInt(2);
    Direction dir = dirChoice == 0 ? VERTICAL : HORIZONTAL;
    if (b.placeShip(p, shipId, dir))
    {
        // Try placing next ship for max 50 times
        for (int i = 0; i < 50; i++)
        {
            if (shipId + 1 == game().nShips())
                return true;
            if (recursivePlace(b, shipId+1))
                return true;
        }
    }
    return false;
}

// Place ships randomly
bool GoodPlayer::placeShips(Board& b)
{
    for (int i = 0; i < 50; i++)
        if (recursivePlace(b, 0))
            return true;
    return true;
}

// Checks if Point is in bounds and has not been moved on yet
bool GoodPlayer::validPoint(Point p)
{
    if (!game().isValid(p))
        return false;
    auto matchingPos = [&p](Point pos)
    {
        if (p.r == pos.r && p.c == pos.c)
            return true;
        return false;
    };
    // Point has already been moved on
    if (find_if(m_missed.begin(), m_missed.end(), matchingPos) != m_missed.end())
        return false;
    return true;
}

// Checks if placing a ship at a point in a direction is valid
bool GoodPlayer::validPlace(Point p, int shipLength, Direction dir)
{
    if (!validPoint(p))
        return false;
    if (dir == VERTICAL)
    {
        // Validate Points along vertical path
        for (int r = p.r; r < p.r + shipLength; r++)
        {
            if (!validPoint(Point(r, p.c)))
                return false;
        }
        return true;
    }
    if (dir == HORIZONTAL)
    {
        // Validate Points along horizontal path
        for (int c = p.c; c < p.c + shipLength; c++)
        {
            if (!validPoint(Point(p.r, c)))
                return false;
        }
        return true;
    }
    return false;
}

void GoodPlayer::huntProb()
{
    resetProbArray();
    for (const ShipType& st : shipsAlive)
    {
        for (int r = 0; r < game().rows(); r++)
            for (int c = 0; c < game().cols(); c++)
            {
                // Check placements along vertical crosshair
                for (int i = r - st.length + 1; i <= r; i++)
                    if (validPlace(Point(i, c), st.length, VERTICAL))
                        probArray[r][c]++;
                
                // Check placements along horizontal crosshair
                for (int i = c - st.length + 1; i <= c; i++)
                    if (validPlace(Point(r, i), st.length, HORIZONTAL))
                        probArray[r][c]++;
            }
    }
}

void GoodPlayer::targetProb()
{
    resetProbArray();
    for (const ShipType& st : shipsAlive)
    {
        for (const Point& p : m_destroyed)
        {
            // For each possible placement starting position in vertical crosshair
            for (int i = p.r - st.length + 1; i <= p.r; i++)
                // For each position along ship placement path
                for (int r = i; r < i + st.length; r++)
                    // If valid position, add to probability
                    if (validPoint(Point(r, p.c)))
                        probArray[r][p.c]++;   

            // For each possible placement starting position in horizontal crosshair
            for (int i = p.c - st.length + 1; i <= p.c; i++)
                // For each position along ship placement path
                for (int c = i; c < i + st.length; c++)
                    // If valid position, add to probability
                    if (validPoint(Point(p.r, c)))
                        probArray[p.r][c]++;
        }
    }
    // Set destroyed spot to 0 probability
    for (const Point& p : m_destroyed)
        probArray[p.r][p.c] = 0;
}

Point GoodPlayer::recommendAttack()
{
    if (huntOrTarget == 0)
        huntProb();
    if (huntOrTarget == 1)
        targetProb();
   //printProbArray();
    int maxProb = 0;
    vector<Point> maxPoints;
    for (int r = 0; r < game().rows(); r++)
    {
        for (int c = 0; c < game().cols(); c++)
        {
            // Probability of a ship at a point
            int prob = probArray[r][c];
            // Add to list of best points to recommend
            if (prob == maxProb)
                maxPoints.push_back(Point(r, c));

            // Override previous best points
            if (prob > maxProb)
            {
                maxProb = prob;
                maxPoints.clear();
                maxPoints.push_back(Point(r, c));
            }
        }
    }

    // Return random Point from best points list
    return maxPoints[randInt(maxPoints.size())];
}

void GoodPlayer::recordAttackResult(Point p, bool validShot, bool shotHit, bool shipDestroyed, int shipId)
{
    // If hit, switch to targeting mode, add Point to list of destroyed
    if (shotHit)
    {
        huntOrTarget = 1;
        m_destroyed.push_back(p);
    }
    // If not, stay in same mode, add Point to list of missed
    else
        m_missed.push_back(p);

    // If destroyed ship, switch to hunting mode
    // Move all destroyed Points to missed
    // Reset list of destroyed
    if (shipDestroyed)
    {
        huntOrTarget = 0;
        for (Point p : m_destroyed)
            m_missed.push_back(p);
        m_destroyed.clear();
    }
}

void GoodPlayer::resetProbArray()
{
    for (int r = 0; r < MAXROWS; r++)
        for (int c = 0; c < MAXCOLS; c++)
            probArray[r][c] = 0;
}

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
