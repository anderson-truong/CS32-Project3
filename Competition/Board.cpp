#include "Board.h"
#include "Game.h"
#include "globals.h"
#include <vector>
#include <iostream>
#include <algorithm>

using namespace std;

struct ShipInstance
{
    int shipId;
    Point topOrLeft;
    Direction dir;
    ShipInstance(int shipId, Point topOrLeft, Direction dir): shipId(shipId), topOrLeft(topOrLeft), dir(dir){ }
};

class BoardImpl
{
  public:
    BoardImpl(const Game& g);
    void clear();
    void block();
    void unblock();
    bool placeShip(Point topOrLeft, int shipId, Direction dir);
    bool unplaceShip(Point topOrLeft, int shipId, Direction dir);
    void display(bool shotsOnly) const;
    bool attack(Point p, bool& shotHit, bool& shipDestroyed, int& shipId);
    bool allShipsDestroyed() const;

    bool shipInstanceDestroyed(const ShipInstance& instance) const;

  private:
    const Game& m_game;
    // Stores the display grid
    char m_grid[MAXROWS][MAXCOLS];
    // Stores list of ship IDs and their topOrLeft positions
    vector<ShipInstance> m_shipInstances;
};

// Check if ShipInstance is destroyed (through iterator)
bool BoardImpl::shipInstanceDestroyed(const ShipInstance& instance) const
{
    // Check if damaged ship is destroyed
    Point current = instance.topOrLeft;
    int shipLength = m_game.shipLength(instance.shipId);
    char shipSymbol = m_game.shipSymbol(instance.shipId);

    for (int i = 0; i < shipLength; i++)
    {
        // If still has symbol along allocated space, ship is not destroyed
        if (m_grid[current.r][current.c] == shipSymbol)
        {
            return false;
        }
        // Increment column
        if (instance.dir == HORIZONTAL)
            current.c++;
        // Increment row
        if (instance.dir == VERTICAL)
            current.r++;
    }
    return true;
}

BoardImpl::BoardImpl(const Game& g)
 : m_game(g)
{
    // Initialize grid with '.'s
    clear();
}

void BoardImpl::clear()
{
    // Reset grid with '.'s
    for (int r = 0; r < m_game.rows(); r++)
        for (int c = 0; c < m_game.cols(); c++)
            m_grid[r][c] = '.';
}

void BoardImpl::block()
{
    // Stores previously blocked Points
    vector<Point> previouslyBlocked;
    // Number of blocked cells is half of total cells
    int blockCount = (m_game.rows() * m_game.cols()) / 2;

    while (blockCount > 0)
    {
        // New Point with random row/col coordinates
        Point p(randInt(m_game.rows()), randInt(m_game.cols()));

        // If Point already exists, continue and find new random Point
        bool pointBlocked = false;
        for (Point prev : previouslyBlocked)
        {
            if (p.r == prev.r && p.c == prev.c)
            {
                pointBlocked = true;
                break;
            }
        }
        if (pointBlocked)
            continue;

        // Block out array at Point
        m_grid[p.r][p.c] = 'X';
        previouslyBlocked.push_back(p);
        blockCount--;
    }
}

void BoardImpl::unblock()
{
    for (int r = 0; r < m_game.rows(); r++)
        for (int c = 0; c < m_game.cols(); c++)
        {
            // Unblock 'X' to '.'
            if (m_grid[r][c] == 'X') m_grid[r][c] = '.';
        }
}

bool BoardImpl::placeShip(Point topOrLeft, int shipId, Direction dir)
{
    // Invalid ID
    if (shipId < 0 || shipId >= m_game.nShips())
        return false;
    
    // Check if Ship with ID already exists on Board
    auto idMatches = [&shipId](ShipInstance sp)
    {
        if (shipId == sp.shipId)
            return true;
        return false;
    };

    // If any of the existing ships on the board has ID that matches shipId
    if (any_of(m_shipInstances.begin(), m_shipInstances.end(), idMatches))
        return false;

    // Current position of ship block for validation
    Point current = topOrLeft;

    // Validate allocated ship space
    int shipLength = m_game.shipLength(shipId);

    // Loop to validate each block of the ship
    for (int i = 0; i < shipLength; i++)
    {
        // Check Point goes off the board
        if (current.r < 0 || current.r >= m_game.rows() || current.c < 0 || current.c >= m_game.cols())
            return false;
        // Check if Point is already occupied
        if (m_grid[current.r][current.c] != '.')
            return false;

        // Column varies, row constant
        if (dir == HORIZONTAL)
            current.c++;
        // Row varies, column constant
        if (dir == VERTICAL)
            current.r++;
    }

    // Log ship's ID
    m_shipInstances.push_back(ShipInstance(shipId, topOrLeft, dir));

    // Reset current Position to starting
    current = topOrLeft;

    // Place shipSymbols in allocated space
    int shipSymbol = m_game.shipSymbol(shipId);
    for (int i = 0; i < shipLength; i++)
    {
        m_grid[current.r][current.c] = shipSymbol;
        // Column varies, row constant
        if (dir == HORIZONTAL)
            current.c++;
        // Row varies, column constant
        if (dir == VERTICAL)
            current.r++;
    }
    return true;
}

bool BoardImpl::unplaceShip(Point topOrLeft, int shipId, Direction dir)
{
    // Matches arguments to existing ShipInstance
    // Returns true if matches, false otherwise
    auto match = [&topOrLeft, &shipId, &dir](ShipInstance sp)
    {
        if (topOrLeft.r == sp.topOrLeft.r && 
            topOrLeft.c == sp.topOrLeft.c && 
            shipId == sp.shipId &&
            dir == sp.dir)
            return true;
        return false;
    };

    // Try to match to existing ShipInstance
    vector<ShipInstance>::iterator matchedSP = find_if(m_shipInstances.begin(), m_shipInstances.end(), match);

    // Could not find matching ShipInstance
    if (matchedSP == m_shipInstances.end())
        return false;

    // Replace Ship's symbols with '.'
    int shipLength = m_game.shipLength(shipId);
    Point current = topOrLeft;
    for (int i = 0; i < shipLength; i++)
    {
        m_grid[current.r][current.c] = '.';
        // Increment column
        if (dir == HORIZONTAL)
            current.c++;
        if (dir == VERTICAL)
            current.r++;
    }

    // Remove ShipInstance from existing list
    m_shipInstances.erase(matchedSP);

    return true;
}

void BoardImpl::display(bool shotsOnly) const
{
    // Print column indices
    cout << "  ";
    for (int c = 0; c < m_game.cols(); c++) cout << c;
    cout << endl;
    
    // Print rows
    for (int r = 0; r < m_game.rows(); r++)
    {
        cout << r << " ";
        for (int c = 0; c < m_game.cols(); c++)
        {
            // To only show shots,
            // print '.' if point is a ship symbol (i.e. not 'X' or 'o')
            if (shotsOnly && m_grid[r][c] != 'X' && m_grid[r][c] != 'o')
                cout << '.';
            else
                cout << m_grid[r][c];
        }
        cout << endl;
    }
}

bool BoardImpl::attack(Point p, bool& shotHit, bool& shipDestroyed, int& shipId)
{
    // Point is outside board
    if (p.r < 0 || p.r >= m_game.rows() || p.c < 0 || p.c >= m_game.cols())
    {
        shotHit = false;
        shipDestroyed = false;
        shipId = -1;
        return false;
    }

    // Already attacked position
    if (m_grid[p.r][p.c] == 'X' || m_grid[p.r][p.c] == 'o')
    {
        shotHit = false;
        shipDestroyed = false;
        shipId = -1;
        return false;
    }

    // If doesn't hit
    if (m_grid[p.r][p.c] == '.')
    {
        m_grid[p.r][p.c] = 'o';

        shotHit = false;
        shipDestroyed = false;
        shipId = -1;
        return true;
    }

    // Match symbol at position to ShipInstance
    vector<ShipInstance>::iterator matchedShipPos;
    for (matchedShipPos = m_shipInstances.begin(); matchedShipPos != m_shipInstances.end(); matchedShipPos++)
    {
        if (m_grid[p.r][p.c] == m_game.shipSymbol(matchedShipPos->shipId))
            break;
    }

    // Mark ship section as damaged
    m_grid[p.r][p.c] = 'X';

    // Check if damaged ship is destroyed
    shipDestroyed = shipInstanceDestroyed(*matchedShipPos);
    shotHit = true;
    shipId = matchedShipPos->shipId;
    return true;
}

bool BoardImpl::allShipsDestroyed() const
{
    // Check if all of ShipInstances are destroyed
    return all_of(m_shipInstances.begin(), m_shipInstances.end(), 
        [this](const ShipInstance& sp) { return shipInstanceDestroyed(sp);  }
    );
}

//******************** Board functions ********************************

// These functions simply delegate to BoardImpl's functions.
// You probably don't want to change any of this code.

Board::Board(const Game& g)
{
    m_impl = new BoardImpl(g);
}

Board::~Board()
{
    delete m_impl;
}

void Board::clear()
{
    m_impl->clear();
}

void Board::block()
{
    return m_impl->block();
}

void Board::unblock()
{
    return m_impl->unblock();
}

bool Board::placeShip(Point topOrLeft, int shipId, Direction dir)
{
    return m_impl->placeShip(topOrLeft, shipId, dir);
}

bool Board::unplaceShip(Point topOrLeft, int shipId, Direction dir)
{
    return m_impl->unplaceShip(topOrLeft, shipId, dir);
}

void Board::display(bool shotsOnly) const
{
    m_impl->display(shotsOnly);
}

bool Board::attack(Point p, bool& shotHit, bool& shipDestroyed, int& shipId)
{
    return m_impl->attack(p, shotHit, shipDestroyed, shipId);
}

bool Board::allShipsDestroyed() const
{
    return m_impl->allShipsDestroyed();
}
