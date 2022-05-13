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
//  GoodPlayer
//*********************************************************************
namespace Player1
{
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
}

namespace Player2

{
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
}

//*********************************************************************
//  createPlayer
//*********************************************************************

Player* createPlayer(string type, string nm, const Game& g)
{
    static string types[] = {
        "good1", "good2"
    };

    int pos;
    for (pos = 0; pos != sizeof(types) / sizeof(types[0]) &&
        type != types[pos]; pos++)
        ;
    switch (pos)
    {
    case 0:  return new Player1::GoodPlayer(nm, g);
    case 1:  return new Player2::GoodPlayer(nm, g);
    default: return nullptr;
    }
}
