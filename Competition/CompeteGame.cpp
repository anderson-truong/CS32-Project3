#include "Game.h"
#include "Board.h"
#include "Player.h"
#include "globals.h"
#include "utility.h"
#include <iostream>
#include <string>
#include <cstdlib>
#include <cctype>
#include <vector>
#include <algorithm>

using namespace std;

class GameImpl
{
public:
    GameImpl(int nRows, int nCols);
    int rows() const;
    int cols() const;
    bool isValid(Point p) const;
    Point randomPoint() const;
    bool addShip(int length, char symbol, string name);
    int nShips() const;
    int shipLength(int shipId) const;
    char shipSymbol(int shipId) const;
    string shipName(int shipId) const;
    Player* play(Player* p1, Player* p2, Board& b1, Board& b2, bool shouldPause);

    bool playerAttack(Player* attacker, Player* attacked, Board& attackedBoard, bool shouldPause);
private:
    int m_rows;
    int m_cols;
    vector<ShipType> shipTypes;
};

void waitForEnter()
{
    cout << "Press enter to continue: ";
    cin.ignore(10000, '\n');
}

GameImpl::GameImpl(int nRows, int nCols) : m_rows(nRows), m_cols(nCols)
{
}

int GameImpl::rows() const
{
    return m_rows;
}

int GameImpl::cols() const
{
    return m_cols;
}

bool GameImpl::isValid(Point p) const
{
    // 0 <= Row < # of rows
    // 0 <= Column < # of columns
    return p.r >= 0 && p.r < rows() && p.c >= 0 && p.c < cols();
}

Point GameImpl::randomPoint() const
{
    return Point(randInt(rows()), randInt(cols()));
}

bool GameImpl::addShip(int length, char symbol, string name)
{
    // Length is at least 1 and cannot exceed row/column size
    if (length < 1 || length > MAXROWS || length > MAXCOLS)
        return false;

    // Checks if new ship has same name
    auto sameName = [&name, &symbol](ShipType ship)
    {
        // Same name
        if (name == ship.name)
            return true;

        return false;
    };

    // If new ship name matches old, cannot add new ship
    if (find_if(shipTypes.begin(), shipTypes.end(), sameName) != shipTypes.end())
        return false;

    shipTypes.push_back(ShipType(length, symbol, name));
    return true;
}

int GameImpl::nShips() const
{
    return shipTypes.size();
}

int GameImpl::shipLength(int shipId) const
{
    return shipTypes[shipId].length;
}

char GameImpl::shipSymbol(int shipId) const
{
    return shipTypes[shipId].symbol;
}

string GameImpl::shipName(int shipId) const
{
    return shipTypes[shipId].name;
}

bool GameImpl::playerAttack(Player* attacker, Player* attacked, Board& attackedBoard, bool shouldPause)
{
    Point attackPos = attacker->recommendAttack();
    bool shotHit;
    bool shipDestroyed;
    int shipIdAttacked;
    bool boardAttack = attackedBoard.attack(attackPos, shotHit, shipDestroyed, shipIdAttacked);

    attacker->recordAttackResult(attackPos, boardAttack, shotHit, shipDestroyed, shipIdAttacked);
    attacked->recordAttackByOpponent(attackPos);
    if (attackedBoard.allShipsDestroyed())
        return true;

    if (shouldPause)
        waitForEnter();
    return false;
}

Player* GameImpl::play(Player* p1, Player* p2, Board& b1, Board& b2, bool shouldPause)
{
    if (!p1->placeShips(b1) || !p2->placeShips(b2))
        return nullptr;
    while (true)
    {
        if (playerAttack(p1, p2, b2, shouldPause))
        {
            cout << p1->name() << " wins!" << endl;
            return p1;
        }
        if (playerAttack(p2, p1, b1, shouldPause))
        {
            cout << p2->name() << " wins!" << endl;
            return p2;
        }
    }
    return nullptr;
}

//******************** Game functions *******************************

// These functions for the most part simply delegate to GameImpl's functions.
// You probably don't want to change any of the code from this point down.

Game::Game(int nRows, int nCols)
{
    if (nRows < 1 || nRows > MAXROWS)
    {
        cout << "Number of rows must be >= 1 and <= " << MAXROWS << endl;
        exit(1);
    }
    if (nCols < 1 || nCols > MAXCOLS)
    {
        cout << "Number of columns must be >= 1 and <= " << MAXCOLS << endl;
        exit(1);
    }
    m_impl = new GameImpl(nRows, nCols);
}

Game::~Game()
{
    delete m_impl;
}

int Game::rows() const
{
    return m_impl->rows();
}

int Game::cols() const
{
    return m_impl->cols();
}

bool Game::isValid(Point p) const
{
    return m_impl->isValid(p);
}

Point Game::randomPoint() const
{
    return m_impl->randomPoint();
}

bool Game::addShip(int length, char symbol, string name)
{
    if (length < 1)
    {
        cout << "Bad ship length " << length << "; it must be >= 1" << endl;
        return false;
    }
    if (length > rows() && length > cols())
    {
        cout << "Bad ship length " << length << "; it won't fit on the board"
            << endl;
        return false;
    }
    if (!isascii(symbol) || !isprint(symbol))
    {
        cout << "Unprintable character with decimal value " << symbol
            << " must not be used as a ship symbol" << endl;
        return false;
    }
    if (symbol == 'X' || symbol == '.' || symbol == 'o')
    {
        cout << "Character " << symbol << " must not be used as a ship symbol"
            << endl;
        return false;
    }
    int totalOfLengths = 0;
    for (int s = 0; s < nShips(); s++)
    {
        totalOfLengths += shipLength(s);
        if (shipSymbol(s) == symbol)
        {
            cout << "Ship symbol " << symbol
                << " must not be used for more than one ship" << endl;
            return false;
        }
    }
    if (totalOfLengths + length > rows() * cols())
    {
        cout << "Board is too small to fit all ships" << endl;
        return false;
    }
    return m_impl->addShip(length, symbol, name);
}

int Game::nShips() const
{
    return m_impl->nShips();
}

int Game::shipLength(int shipId) const
{
    assert(shipId >= 0 && shipId < nShips());
    return m_impl->shipLength(shipId);
}

char Game::shipSymbol(int shipId) const
{
    assert(shipId >= 0 && shipId < nShips());
    return m_impl->shipSymbol(shipId);
}

string Game::shipName(int shipId) const
{
    assert(shipId >= 0 && shipId < nShips());
    return m_impl->shipName(shipId);
}

Player* Game::play(Player* p1, Player* p2, bool shouldPause)
{
    if (p1 == nullptr || p2 == nullptr || nShips() == 0)
        return nullptr;
    Board b1(*this);
    Board b2(*this);
    return m_impl->play(p1, p2, b1, b2, shouldPause);
}

