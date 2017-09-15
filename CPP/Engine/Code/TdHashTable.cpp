#include <assert.h>
#include <memory.h>
#include "TdHashTable.h"

namespace eng {

#define FIRST_CELL(hash) (m_cells + ((hash) & (m_arraySize - 1)))
#define CIRCULAR_NEXT(c) ((c) + 1 != m_cells + m_arraySize ? (c) + 1 : m_cells)
#define CIRCULAR_OFFSET(a, b) ((b) >= (a) ? (b) - (a) : m_arraySize + (b) - (a))

TdHashTable::TdHashTable(size_t initialSize)
{
    m_arraySize = initialSize;
    assert((m_arraySize & (m_arraySize - 1)) == 0);   // Must be a power of 2
    m_cells = new Cell[m_arraySize];
    memset(m_cells, 0, sizeof(Cell) * m_arraySize);
    m_population = 0;

    m_zeroUsed = 0;
    m_zeroCell.key = 0;
    m_zeroCell.value = 0;
}

TdHashTable::~TdHashTable()
{
    delete[] m_cells;
}

TdHashTable::Cell* TdHashTable::Lookup(size_t key)
{
    if (key)
    {
        for (Cell* cell = FIRST_CELL(tdIntegerHash(key));; cell = CIRCULAR_NEXT(cell))
        {
            if (cell->key == key)
                return cell;
            if (!cell->key)
                return nullptr;
        }
    }
    else
    {
        if (m_zeroUsed)
            return &m_zeroCell;
        return nullptr;
    }
};

TdHashTable::Cell* TdHashTable::Insert(size_t key)
{
    if (key)
    {
        for (;;)
        {
            for (Cell* cell = FIRST_CELL(tdIntegerHash(key));; cell = CIRCULAR_NEXT(cell))
            {
                if (cell->key == key)
                    return cell;        // Found
                if (cell->key == 0)
                {
                    // Insert here
                    if ((m_population + 1) * 4 >= m_arraySize * 3)
                    {
                        // Time to resize
                        Repopulate(m_arraySize * 2);
                        break;
                    }
                    ++m_population;
                    cell->key = key;
                    return cell;
                }
            }
        }
    }
    else
    {
        // Check zero cell
        if (!m_zeroUsed)
        {
            // Insert here
            m_zeroUsed = true;
            if (++m_population * 4 >= m_arraySize * 3)
			{
				// Even though we didn't use a regular slot, let's keep the sizing rules consistent
                Repopulate(m_arraySize * 2);
			}
        }
        return &m_zeroCell;
    }
}

void TdHashTable::Delete(Cell* cell)
{
    if (cell != &m_zeroCell)
    {
        // Delete from regular cells
        assert(cell >= m_cells && (size_t)(cell - m_cells) < m_arraySize);
        assert(cell->key);

        // Remove this cell by shuffling neighboring cells so there are no gaps in anyone's probe chain
        for (Cell* neighbor = CIRCULAR_NEXT(cell);; neighbor = CIRCULAR_NEXT(neighbor))
        {
            if (!neighbor->key)
            {
                // There's nobody to swap with. Go ahead and clear this cell, then return
                cell->key = 0;
                cell->value = 0;
                m_population--;
                return;
            }
            Cell* ideal = FIRST_CELL(tdIntegerHash(neighbor->key));
            if (CIRCULAR_OFFSET(ideal, cell) < CIRCULAR_OFFSET(ideal, neighbor))
            {
                // Swap with neighbor, then make neighbor the new cell to remove.
                *cell = *neighbor;
                cell = neighbor;
            }
        }
    }
    else
    {
        // Delete zero cell
        assert(m_zeroUsed);
        m_zeroUsed = false;
        cell->value = 0;
        m_population--;
        return;
    }
}

void TdHashTable::Clear()
{
    // (Does not resize the array)
    // Clear regular cells
    memset(m_cells, 0, sizeof(Cell) * m_arraySize);
    m_population = 0;
    // Clear zero cell
    m_zeroUsed = false;
    m_zeroCell.value = 0;
}

void TdHashTable::Compact()
{
    Repopulate(tdUpperPowerOfTwo((m_population * 4 + 3) / 3));
}

void TdHashTable::Repopulate(size_t desiredSize)
{
    assert((desiredSize & (desiredSize - 1)) == 0);   // Must be a power of 2
    assert(m_population * 4  <= desiredSize * 3);

    // Get start/end pointers of old array
    Cell* oldCells = m_cells;
    Cell* end = m_cells + m_arraySize;

    // Allocate new array
    m_arraySize = desiredSize;
    m_cells = new Cell[m_arraySize];
    memset(m_cells, 0, sizeof(Cell) * m_arraySize);

    // Iterate through old array
    for (Cell* c = oldCells; c != end; c++)
    {
        if (c->key)
        {
            // Insert this element into new array
            for (Cell* cell = FIRST_CELL(tdIntegerHash(c->key));; cell = CIRCULAR_NEXT(cell))
            {
                if (!cell->key)
                {
                    // Insert here
                    *cell = *c;
                    break;
                }
            }
        }
    }

    // Delete old array
    delete[] oldCells;
}

TdHashTable::Iterator::Iterator(TdHashTable &table) : m_table(table)
{
    m_cur = &m_table.m_zeroCell;
    if (!m_table.m_zeroUsed)
        Next();
}

TdHashTable::Cell* TdHashTable::Iterator::Next()
{
    // Already finished?
    if (!m_cur)
        return m_cur;

    // Iterate past zero cell
    if (m_cur == &m_table.m_zeroCell)
        m_cur = &m_table.m_cells[-1];

    // Iterate through the regular cells
    Cell* end = m_table.m_cells + m_table.m_arraySize;
    while (++m_cur != end)
    {
        if (m_cur->key)
            return m_cur;
    }

    // Finished
    return m_cur = nullptr;
}
}