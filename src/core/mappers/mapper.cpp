#include "mapper.h"

#include "../cartridge.h"

Mapper::Mapper(Cartridge &cart)
    : m_cartridge(cart)
{
}