#include "Board.h"
#include "Game.h"
#include "globals.h"
#include <vector>
#include <iostream>
#include <algorithm>

using namespace std;

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

  private:
    struct ShipInstance
    {
        int shipId;
        Point topOrLeft;
        Direction dir;
        ShipInstance(int shipId, Point topOrLeft, Direction dir) : shipId(shipId), topOrLeft(topOrLeft), dir(dir) { }
    };

    bool shipInstanceDestroyed(const ShipInstance& instance) const;

    const Game& m_game;

    // Stores the display grid
    char m_grid[MAXROWS][MAXCOLS];

    // Stores list of ship IDs, their topOrLeft positions, and placement direction
    vector<ShipInstance> m_shipInstances;
};

BoardImpl::BoardImpl(const Game& g) : m_game(g)
{
    // Initialize grid with '.'
    clear();
}

// #############
// Set all points on the 
// game board array to '.'
// #############
void BoardImpl::clear()
{
    // Reset grid with '.'s
    for (int r = 0; r < m_game.rows(); r++)
        for (int c = 0; c < m_game.cols(); c++)
            m_grid[r][c] = '.';
}

// ##############
// Blocks 50% of the board
// at random positions
// ##############
void BoardImpl::block()
{
    // Stores previously blocked Points
    vector<Point> previouslyBlocked;
    // Number of blocked cells is half of total cells
    int blockCount = (m_game.rows() * m_game.cols()) / 2;

    while (blockCount > 0)
    {
        // Random row and column
        int r = randInt(m_game.rows());
        int c = randInt(m_game.cols());

        // If already blocked, skip
        if (m_grid[r][c] == 'X')
            continue;

        // Block out array at Point
        m_grid[r][c] = 'X';
        blockCount--;
    }
}

// ###################
// Unblocks previously blocked
// positions from BoardImpl::block()
// ###################
void BoardImpl::unblock()
{
    for (int r = 0; r < m_game.rows(); r++)
        for (int c = 0; c < m_game.cols(); c++)
        {
            // Unblock 'X' to '.'
            if (m_grid[r][c] == 'X') m_grid[r][c] = '.';
        }
}

// #####################
// Attempt to place a ship on the board
// at a position and in a direction
// #####################
bool BoardImpl::placeShip(Point topOrLeft, int shipId, Direction dir)
{
    // Invalid ID
    if (shipId < 0 || shipId >= m_game.nShips())
        return false;
    
    // Check if Ship with ID already exists on Board
    for (const ShipInstance& sI : m_shipInstances)
    {
        if (shipId == sI.shipId)
            return false;
    }

    // Start position validation at topOrLeft
    Point current = topOrLeft;

    // Number of positions to loop through
    int shipLength = m_game.shipLength(shipId);

    // Position validation along intended ship space
    for (int i = 0; i < shipLength; i++)
    {
        // Ship goes out of bounds
        if (!m_game.isValid(current))
            return false;

        // Position is already occupied
        if (m_grid[current.r][current.c] != '.')
            return false;

        // Increment column if Direction is HORIZONTAL
        if (dir == HORIZONTAL)
            current.c++;
        // Increment row if Direction is VERTICAL
        if (dir == VERTICAL)
            current.r++;
    }

    // Store ShipInstance as a part of the Board now
    m_shipInstances.push_back(ShipInstance(shipId, topOrLeft, dir));

    // Place shipSymbols in allocated space
    current = topOrLeft;
    int shipSymbol = m_game.shipSymbol(shipId);
    for (int i = 0; i < shipLength; i++)
    {
        m_grid[current.r][current.c] = shipSymbol;

        // Increment column if Direction is HORIZONTAL
        if (dir == HORIZONTAL)
            current.c++;
        // Increment row if Direction is VERTICAL
        if (dir == VERTICAL)
            current.r++;
    }
    return true;
}

// ###############
// Attempts to unplace a ship
// from a point in a direction
// ###############
bool BoardImpl::unplaceShip(Point topOrLeft, int shipId, Direction dir)
{
    // Tries to match arguments to a ShipInstance
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

    // Try to match to existing ShipInstances
    vector<ShipInstance>::iterator matchedSP = find_if(m_shipInstances.begin(), m_shipInstances.end(), match);

    // Could not find matching ShipInstance
    if (matchedSP == m_shipInstances.end())
        return false;

    // Replace Ship's symbols with '.'
    Point current = topOrLeft;
    int shipLength = m_game.shipLength(shipId);
    for (int i = 0; i < shipLength; i++)
    {
        m_grid[current.r][current.c] = '.';

        // Increment column if Direction is HORIZONTAL
        if (dir == HORIZONTAL)
            current.c++;
        // Increment row if Direction is VERTICAL
        if (dir == VERTICAL)
            current.r++;
    }

    // Remove ShipInstance
    m_shipInstances.erase(matchedSP);

    return true;
}

// #################
// Displays the game board
// 
// Can show both ships and shots
// Or just shots only
// #################
void BoardImpl::display(bool shotsOnly) const
{
    // Print column indices
    cout << "  ";
    for (int c = 0; c < m_game.cols(); c++) cout << c;
    cout << endl;
    
    for (int r = 0; r < m_game.rows(); r++)
    {
        // Print row index
        cout << r << " ";

        // Print ship and shot symbols
        for (int c = 0; c < m_game.cols(); c++)
        {
            // If shots only, print '.' if point is a ship symbol (i.e. not 'X' or 'o')
            if (shotsOnly && m_grid[r][c] != 'X' && m_grid[r][c] != 'o')
                cout << '.';
            // Otherwise, just print normally
            else
                cout << m_grid[r][c];
        }
        cout << endl;
    }
}

// ############################
// Check if a particular ShipInstance is destroyed
// 
// Checks the game board for ShipInstance's symbol 
// from topOrLeft position towards Direction
// for shipLength number of positions
// 
// If no symbols left along path, return true
// ############################
bool BoardImpl::shipInstanceDestroyed(const ShipInstance& instance) const
{
    // Start position at topOrLeft
    Point current = instance.topOrLeft;
    int shipLength = m_game.shipLength(instance.shipId);
    char shipSymbol = m_game.shipSymbol(instance.shipId);

    // Loop up to shipLength number of positions
    for (int i = 0; i < shipLength; i++)
    {
        // If symbol is along path, ShipInstance is still alive
        if (m_grid[current.r][current.c] == shipSymbol)
        {
            return false;
        }

        // Increment column if Direction is HORIZONTAL
        if (instance.dir == HORIZONTAL)
            current.c++;
        // Increment row if Direction is VERTICAL
        if (instance.dir == VERTICAL)
            current.r++;
    }
    return true;
}

// #########################
// Attack a point on a board
//  
// Tells if the attack was valid, hit a ship, 
// and if it destroyed a ship (and the ship's id)
// #########################
bool BoardImpl::attack(Point p, bool& shotHit, bool& shipDestroyed, int& shipId)
{
    // Point is outside board or is already attacked
    if (!m_game.isValid(p) || m_grid[p.r][p.c] == 'X' || m_grid[p.r][p.c] == 'o')
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

    // Find ship by matching symbol at the Point to a shipId
    vector<ShipInstance>::iterator matchedShipPos;
    for (matchedShipPos = m_shipInstances.begin(); matchedShipPos != m_shipInstances.end(); matchedShipPos++)
    {
        // Symbol at Point matches a ship's symbol
        if (m_grid[p.r][p.c] == m_game.shipSymbol(matchedShipPos->shipId))
            break;
    }

    // Mark board position as a hit
    m_grid[p.r][p.c] = 'X';

    // Check if damaged ship is destroyed
    shipDestroyed = shipInstanceDestroyed(*matchedShipPos);
    shotHit = true;
    shipId = matchedShipPos->shipId;
    return true;
}

// ########################
// Checks if all ShipInstances are destroyed
// ########################
bool BoardImpl::allShipsDestroyed() const
{
    // Loop through all ship instances
    for (const ShipInstance& sp : m_shipInstances)
    {
        // If ship is not destroyed
        if (!shipInstanceDestroyed(sp))
            return false;
    }

    // All ships destroyed
    return true;
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
