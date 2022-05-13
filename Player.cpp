#include "Player.h"
#include "Board.h"
#include "Game.h"
#include "globals.h"
#include "utility.h"
#include <iostream>
#include <string>
#include <iomanip>
#include <list>

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

void AwfulPlayer::recordAttackResult(Point p, bool validShot, bool shotHit, bool shipDestroyed, int shipId)
{
      // AwfulPlayer completely ignores the result of any attack
}

void AwfulPlayer::recordAttackByOpponent(Point p)
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
    
    // Validate input for each ship placement
    for (int i = 0; i < game().nShips(); i++)
    {
        b.display(false);

        //======================
        // Validate direction input
        //======================

        // Loop until player enters in valid direction
        char dirChar;
        while (true)
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
        Direction dir = (dirChar == 'h') ? HORIZONTAL : VERTICAL;

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
            
            // Valid input
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

        // Valid input, break out
        break;
    }
    return Point(r, c);
}

void HumanPlayer::recordAttackResult(Point p, bool validShot, bool shotHit, bool shipDestroyed, int shipId) 
{
    // No code necessary, user makes decisions
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
    virtual Point recommendAttack();
    virtual void recordAttackResult(Point p, bool validShot, bool shotHit,
        bool shipDestroyed, int shipId);
    virtual void recordAttackByOpponent(Point p) { return; }

    bool recursivePlace(Board& b, int shipId);
private:
    bool pointNotChosen(Point& p);

    int m_moveState;
    Point transitionPoint;
    list<Point> prevAttacks;
};

//#########################
// Recursively places ships starting from shipId
// to maximum number of ships
//#########################
bool MediocrePlayer::recursivePlace(Board& b, int shipId)
{
    // If all ships placed, return true
    if (shipId == game().nShips())
        return true;
    
    // Loop through all possible positions, attempting to place at each
    for (int r = 0; r < game().rows(); r++)
        for (int c = 0; c < game().cols(); c++)
        {
            // Try to place ship horizontal or vertical
            if (b.placeShip(Point(r, c), shipId, HORIZONTAL) || b.placeShip(Point(r, c), shipId, VERTICAL))
            {
                // Try placing next ship
                if (recursivePlace(b, shipId + 1))
                    return true;
                // Can't place next ship, unplace current ship
                else
                {
                    // Unplace both directions
                    b.unplaceShip(Point(r, c), shipId, HORIZONTAL);
                    b.unplaceShip(Point(r, c), shipId, VERTICAL);
                }
            }
        }
    
    // Cannot place ships on board
    return false;
}

//#################
// Places all ships with 50% of 
// positions randomly blocked out
// 
// Tries 50 times before failing
//#################
bool MediocrePlayer::placeShips(Board& b)
{
    // Try on up to 50 different blocked boards
    for (int i = 0; i < 50; i++)
    {
        b.block();

        // Able to place all ships
        if (recursivePlace(b, 0))
        {
            b.unblock();
            return true;
        }

        b.unblock();
    }
    // Cannot place all ships
    return false;
}

//###########################
// Checks if Point was already chosen as an attack
// Returns true if unchosen, false if already chosen
//###########################
bool MediocrePlayer::pointNotChosen(Point& p)
{
    for (Point& prev : prevAttacks)
    {
        // Point matches chosen point
        if (prev.r == p.r && prev.c == p.c)
            return false;
    }
    // Point is unchosen
    return true;
}

//#######################
// Recommends an unchosen point to attack
// based on MediocrePlayer's Move State
//#######################
Point MediocrePlayer::recommendAttack()
{ 
    //=============================
    // Move State 1:
    // Returns random unchosen point
    //=============================
    if (m_moveState == 1)
    {
        // Keep trying to find unchosen point
        while (true)
        {
            Point randomPoint = game().randomPoint();

            // Return point if unchosen
            if (pointNotChosen(randomPoint))
            {
                prevAttacks.push_back(randomPoint);
                return randomPoint;
            }
        }
    }

    //===========================================
    // Move State 2:
    // Targets random point in crosshair of the point 
    // that triggered the transition to Move State 2
    //===========================================
    if (m_moveState == 2)
    {
        // Check if game has ship lengths of 6+
        for (int i = 0; i < game().nShips(); i++)
            if (game().shipLength(i) >= 6)
            {
                // Switch to Move State 1
                m_moveState = 1;
                return recommendAttack();
            }

        // Find all possible points in crosshair (up to 4 steps away)
        vector<Point> crosshairPoints;
        for (int i = - 4; i <= 4; i++)
        {
            // Point along vertical crosshair
            Point verticalPoint(transitionPoint.r + i, transitionPoint.c);
            // Store if valid and unchosen
            if (game().isValid(verticalPoint) && pointNotChosen(verticalPoint))
                crosshairPoints.push_back(verticalPoint);

            // Point along horizontal crosshair
            Point horizontalPoint(transitionPoint.r, transitionPoint.c + i);
            // Store if valid and unchosen
            if (game().isValid(horizontalPoint) && pointNotChosen(horizontalPoint))
                crosshairPoints.push_back(horizontalPoint);
        }

        // Return random point from valid crosshair
        Point randomPoint = crosshairPoints[randInt(crosshairPoints.size())];
        prevAttacks.push_back(randomPoint);
        return randomPoint;
    }
    return Point();
}

//##################
// Controls Move State transitions
// if a ship is hit or destroyed
//##################
void MediocrePlayer::recordAttackResult(Point p, bool validShot, bool shotHit, bool shipDestroyed, int shipId) 
{
    // Hits ship but does not destroy
    if (m_moveState == 1 && shotHit && !shipDestroyed)
    {
        // Switch to Move State 2 and store point
        m_moveState = 2;
        transitionPoint = p;
    }
    // Destroys ship in Move State 2
    if (m_moveState == 2 && shipDestroyed)
    {
        // Switch to Move State 1
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
    virtual void recordAttackByOpponent(Point p) { }

    bool validPoint(Point p);
    bool validPlace(Point p, int shipId, Direction dir);
    void huntProb();
    void targetProb();
    void resetProbArray();

    void printProbArray();
private:
    enum AttackMode
    {
        HUNT,
        TARGET
    };
    vector<Point> m_missed;
    vector<Point> m_destroyed;
    vector<ShipType> shipsAlive;
    AttackMode m_attackMode; // HUNT or TARGET
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

GoodPlayer::GoodPlayer(string nm, const Game& g) : Player(nm, g), m_attackMode(HUNT)
{ 
    // Sets probability array to all 0s
    resetProbArray();

    // Store starting ship types
    for (int n = 0; n < g.nShips(); n++)
        shipsAlive.push_back(ShipType(g.shipLength(n), g.shipSymbol(n), g.shipName(n)));
}

//##################
// Recursively places random ships
//##################
bool GoodPlayer::recursivePlace(Board& b, int shipId)
{
    if (shipId == game().nShips())
        return true;

    for (int i = 0; i < 50; i++)
    {
        int shipLength = game().shipLength(shipId);
        // Make sure no ships adjacent to ship space
        
        // Random point
        Point p(randInt(game().rows()) - shipLength + 1, randInt(game().cols()) - shipLength + 1);
        // Random direction;
        int dirChoice = randInt(2);
        Direction dir = dirChoice == 0 ? VERTICAL : HORIZONTAL;

        if (b.placeShip(p, shipId, dir))
        {
            if (recursivePlace(b, shipId + 1))
                return true;
            else
                b.unplaceShip(p, shipId, dir);
        }
    }

    return false;
}

//#############
// Randomly places ships
//#############
bool GoodPlayer::placeShips(Board& b)
{
    return recursivePlace(b, 0);
}

//################
// Checks if Point is in bounds
// and was not attacked before
//################
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

//#################
// Checks if able to place a ship
// length at a point in a direction
//#################
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

    // Parity: only keep every other N positions
    // Set other points to 0 probability
    // N: smallest length of ship
    int smallestLength = shipsAlive[0].length;
    for (const ShipType& st : shipsAlive)
    {
        if (st.length < smallestLength)
            smallestLength = st.length;
    }
    for (int r = 0; r < game().rows(); r++)
    {
        for (int c = 0; c < game().cols(); c++)
        {
            if (r % smallestLength != c % smallestLength)
                probArray[r][c] = 0;
        }
    }
}

void GoodPlayer::targetProb()
{
    resetProbArray();
    Point target = m_destroyed.front();
    int row = target.r;
    int col = target.c;
    for (const ShipType& st : shipsAlive)
    {
            // For each possible placement starting position in vertical crosshair
            for (int i = row - st.length + 1; i <= row; i++)
                if (validPlace(Point(i, col), st.length, VERTICAL))
                {
                    for (int r = i; r < i + st.length; r++)
                    {
                        probArray[r][col]++;
                    }
                }
            // For each possible placement starting position in horizontal crosshair
            for (int i = col - st.length + 1; i <= col; i++)
                if (validPlace(Point(row, i), st.length, HORIZONTAL))
                {
                    for (int c = i; c < i + st.length; c++)
                    {
                        probArray[row][c]++;
                    }
                }
    }

    // If hits twice, hone in one single line in crosshair
    if (m_destroyed.size() >= 2)
    {
        // Same row
        if (m_destroyed[0].r == m_destroyed[1].r)
        {
            for (int i = 0; i < game().cols(); i++)
                probArray[m_destroyed[0].r][i] *= 2;
        }
        // Same column
        if (m_destroyed[0].c == m_destroyed[1].c)
        {
            for (int i = 0; i < game().rows(); i++)
                probArray[i][m_destroyed[0].c] *= 2;
        }
    }
    // Set destroyed spot to 0 probability
    for (const Point& p : m_destroyed)
        probArray[p.r][p.c] = 0;
}

Point GoodPlayer::recommendAttack()
{
    if (m_attackMode == HUNT)
        huntProb();
    if (m_attackMode == TARGET)
        targetProb();

    int maxProb = 0;
    Point best;
    for (int r = 0; r < game().rows(); r++)
    {
        for (int c = 0; c < game().cols(); c++)
        {
            if (probArray[r][c] > maxProb)
            {
                maxProb = probArray[r][c];
                best.r = r;
                best.c = c;
            }
        }
    }

    return best;
}

void GoodPlayer::recordAttackResult(Point p, bool validShot, bool shotHit, bool shipDestroyed, int shipId)
{
    // If hit, switch to targeting mode, add Point to list of destroyed
    if (shotHit)
    {
        m_destroyed.push_back(p);
        m_attackMode = TARGET;
    }
    // If not, stay in same mode, add Point to list of missed
    else
        m_missed.push_back(p);

    // If ship was destroyed at Point P
    // 1. Deduce which positions the ship was located on
    // 2. Move those positions to m_missed positions
    // 3. If there are still positions in m_destroyed, Keep targeting those positions
    // 4. If m_destroyed is empty, switch to HUNT mode
    if (shipDestroyed)
    {
        Point target = m_destroyed.front();
        int destroyedShipLength = game().shipLength(shipId);
        int start = 0;
        int end = 0;
        // Point p is topmost
        if (p.r < target.r)
        {
            start = p.r;
            end = p.r + destroyedShipLength;
        }
        // Point p is bottommost
        else if (p.r > target.r)
        {
            start = p.r - destroyedShipLength + 1;
            end = p.r + 1;
        }
        // Point p is leftmost
        else if (p.c < target.c)
        {
            start = p.c;
            end = p.c + destroyedShipLength;
        }
        // Point p is rightmost
        else if (p.c > target.c)
        {
            start = p.c - destroyedShipLength + 1;
            end = p.c + 1;
        }

        // Loop from start to end positions
        for (int i = start; i < end; i++)
        {
            int r, c;

            // Ship was vertical
            if (p.c == target.c)
            {
                r = i;
                c = p.c;
            }
            // Ship was horizontal
            else
            {
                r = p.r;
                c = i;
            }

            // Store destroyed position as a missed position
            m_missed.push_back(Point(r, c));

            // Remove destroyed position from vector
            for (vector<Point>::iterator it = m_destroyed.begin(); it != m_destroyed.end();)
            {
                if ((*it).r == r && (*it).c == c)
                    it = m_destroyed.erase(it);
                else
                    it++;
            }
        }

        // Switch to HUNT if no positions left in m_destroyed
        if (m_destroyed.empty())
            m_attackMode = HUNT;
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
