#include "Player.h"
#include "Board.h"
#include "Game.h"
#include "globals.h"
#include "utility.h"
#include <iostream>
#include <algorithm>
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

    // Human player makes decisions, no body necessary
    virtual void recordAttackResult(Point p, bool validShot, bool shotHit, bool shipDestroyed, int shipId) { }
    virtual void recordAttackByOpponent(Point p) { }
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
    virtual void recordAttackByOpponent(Point p) { } // Ignores attack by opponent

private:
    bool recursivePlace(Board& b, int shipId);
    bool pointNotChosen(Point& p);

    // Stores the Move State (1 or 2)
    int m_moveState;

    // Stores the point that transitions from State 1 to 2
    Point transitionPoint;

    // Stores previous attacks to not target again
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
    virtual bool placeShips(Board& b);
    virtual Point recommendAttack();
    virtual void recordAttackResult(Point p, bool validShot, bool shotHit,
        bool shipDestroyed, int shipId);
    virtual void recordAttackByOpponent(Point p) { } // Ignores attack by opponent

private:
    bool validPoint(Point p);
    bool validPlace(Point p, int shipId, Direction dir);
    bool recursivePlace(Board& b, int shipId);
    void resetProbArray();
    void printProbArray();
    void huntProb();
    void targetProb();

    enum AttackMode
    {
        HUNT,
        TARGET
    };

    // Stores missed shots (or already destroyed ship points)
    vector<Point> m_missed;

    // Stores hits that aren't fully destroyed ships
    vector<Point> m_destroyed;

    // Stores the enemy's undestroyed ships
    vector<ShipType> shipsAlive;

    // Attacking mode for recommending a point
    // HUNT or TARGET
    AttackMode m_attackMode;

    // Stores probability density array for every point on enemy's board
    int probArray[MAXROWS][MAXCOLS];
};

//##################
// Prints out probability density
// of all points on enemy's board
// 
// USES CERR FOR DEBUGGING
//##################
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

//#####################
// GoodPlayer starts out in HUNT mode
//#####################
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
    // End case (all ships placed)
    if (shipId == game().nShips())
        return true;

    // Try to place ships randomly up to max 50 times
    for (int i = 0; i < 50; i++)
    {
        int shipLength = game().shipLength(shipId);

        // Random point
        Point p(randInt(game().rows()) - shipLength + 1, randInt(game().cols()) - shipLength + 1);

        // Random direction;
        int dirChoice = randInt(2);
        Direction dir = dirChoice == 0 ? VERTICAL : HORIZONTAL;

        // Try to place ship at random position and direction
        if (b.placeShip(p, shipId, dir))
        {
            // Try to place next ship randomly
            if (recursivePlace(b, shipId + 1))
                return true;
            // Unplace current ship if cannot place next ship
            else
                b.unplaceShip(p, shipId, dir);
        }
    }

    // Unable to place ship on board
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
    // Point is out of bounds
    if (!game().isValid(p))
        return false;

    // Checks if passed Point pos matches Point p
    auto matchingPos = [&p](Point pos)
    {
        if (p.r == pos.r && p.c == pos.c)
            return true;
        return false;
    };

    // If Point p matches any Point in m_missed, it is invalid
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
    // TopLeft point is invalid
    if (!validPoint(p))
        return false;

    if (dir == VERTICAL)
    {
        // Validate Points along vertical direction from Point p
        for (int r = p.r; r < p.r + shipLength; r++)
        {
            if (!validPoint(Point(r, p.c)))
                return false;
        }
        return true;
    }

    if (dir == HORIZONTAL)
    {
        // Validate Points along horizontal direction from Point p
        for (int c = p.c; c < p.c + shipLength; c++)
        {
            if (!validPoint(Point(p.r, c)))
                return false;
        }
        return true;
    }

    // Invalid direction
    return false;
}

//#######################
// Sets the probability of all points to zero
//#######################
void GoodPlayer::resetProbArray()
{
    for (int r = 0; r < MAXROWS; r++)
        for (int c = 0; c < MAXCOLS; c++)
            probArray[r][c] = 0;
}

//########################
// Hunting Mode: No ships currently targeted
// 
// Produces a probability density array based
// on number of possibly ship configurations
// at every single point on the board
// 
// Points in the crosshair of a missed shot
// have reduced probability
//########################
void GoodPlayer::huntProb()
{
    resetProbArray();

    // Add probability density for each ship
    for (const ShipType& st : shipsAlive)
    {
        // Loop through all points on the board
        for (int r = 0; r < game().rows(); r++)
            for (int c = 0; c < game().cols(); c++)
            {
                // Validate placements along vertical crosshair centered at point
                for (int i = r - st.length + 1; i <= r; i++)
                    // Add to probability if able to place particular ship configuration
                    if (validPlace(Point(i, c), st.length, VERTICAL))
                        probArray[r][c]++;
                
                // Validate placements along horizontal crosshair centered at point
                for (int i = c - st.length + 1; i <= c; i++)
                    // Add to probability if able to place particular ship configuration
                    if (validPlace(Point(r, i), st.length, HORIZONTAL))
                        probArray[r][c]++;
            }
    }

    // Parity Strategy
    // Keep every other N (smallest ship length) positions, set others to 0 probability

    // Find smallest ship length
    int smallestLength = shipsAlive[0].length;
    for (const ShipType& st : shipsAlive)
    {
        if (st.length < smallestLength)
            smallestLength = st.length;
    }
    
    // Set probability of ships not on parity grid to zero
    for (int r = 0; r < game().rows(); r++)
    {
        for (int c = 0; c < game().cols(); c++)
        {
            if (r % smallestLength != c % smallestLength)
                probArray[r][c] = 0;
        }
    }
}

//########################
// Targeting Mode: One ship being targeted
// 
// Produces probability density crosshair
// centered at the point when the mode 
// switched from HUNT to TARGET
// 
// If another point is a hit, points along the
// path from that hit to the switching point
// is heavily weighted to produce a bias
// along that particular direction
//########################
void GoodPlayer::targetProb()
{
    resetProbArray();

    // Find Point that triggered TARGET mode (first Point in m_destroyed)
    Point target = m_destroyed.front();
    int row = target.r;
    int col = target.c;

    // Calculate probability of each ship along crosshair centered at Point
    for (ShipType& st : shipsAlive)
    {
            // For each possible ship placement position in vertical crosshair
            for (int i = row - st.length + 1; i <= row; i++)
                // If able to place a ship vertically
                // Add 1 to all points along ship placement path
                if (validPlace(Point(i, col), st.length, VERTICAL))
                {
                    for (int r = i; r < i + st.length; r++)
                    {
                        probArray[r][col]++;
                    }
                }

            // For each possible ship placement position in horizontal crosshair
            for (int i = col - st.length + 1; i <= col; i++)
                // If able to place a ship horizontally
                // Add 1 to all points along ship placement path
                if (validPlace(Point(row, i), st.length, HORIZONTAL))
                {
                    for (int c = i; c < i + st.length; c++)
                    {
                        probArray[row][c]++;
                    }
                }
    }

    // If there are at least 2 hit points (forming a line)
    // increase weights for the points on the line
    if (m_destroyed.size() >= 2)
    {
        // Both points are on same row
        if (target.r == m_destroyed[1].r)
        {
            // Loop through all points on same row
            for (int i = 0; i < game().cols(); i++)
            {
                // Double weights on point
                probArray[target.r][i] *= 2;

                // Increase weights based on proximity to target point
                if (i != target.c)
                    probArray[target.r][i] *= 10 / abs(i - target.c);
            }
        }

        // Both points are on same column
        if (target.c == m_destroyed[1].c)
        {
            // Loop through all points on same column
            for (int i = 0; i < game().rows(); i++)
            {
                // Double weights on point
                probArray[i][target.c] *= 2;

                // Increase weights based on proximity to target point
                if (i != target.r)
                    probArray[i][target.c] *= 10 / abs(i - target.r);
            }
        }
    }
    // Set destroyed spot to 0 probability
    for (const Point& p : m_destroyed)
        probArray[p.r][p.c] = 0;
}

//#############################
// Calculates probability density based on attack mode
// Returns point with highest probability in the array
//#############################
Point GoodPlayer::recommendAttack()
{
    // No ships left
    if (shipsAlive.empty())
        return Point();

    // Calculate HUNT probabilities
    if (m_attackMode == HUNT)
        huntProb();

    // Calculate TARGET probabilities
    if (m_attackMode == TARGET)
        targetProb();

    // Return point with highest value (probability) in probArray
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

//#############################
// Calculates probability density based on attack mode
// Returns point with highest probability in the array
//#############################
void GoodPlayer::recordAttackResult(Point p, bool validShot, bool shotHit, bool shipDestroyed, int shipId)
{
    if (shipsAlive.empty())
        return;
        
    // If hit, switch to targeting mode, add Point to list of destroyed
    if (shotHit)
    {
        m_destroyed.push_back(p);
        m_attackMode = TARGET;
    }
    // If not, stay in same mode, add Point to list of missed
    else
        m_missed.push_back(p);

    // If ship was destroyed at Point p:
    // -------------------------------------
    // 1. Remove ship from vector of remaining ships
    // 2. Deduce which positions the ship was located on
    // 3. Move those positions from m_destroyed to m_missed
    // 4. If there are still positions in m_destroyed, stay in TARGET mode
    // 5. If m_destroyed is empty, switch to HUNT mode
    if (shipDestroyed)
    {
        // Remove destroyed ship from vector
        for (vector<ShipType>::iterator it = shipsAlive.begin(); it != shipsAlive.end(); )
        {
            if (it->symbol == game().shipSymbol(shipId))
                it = shipsAlive.erase(it);
            else
                it++;
        }

        // Determine the space where the ship was located
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
