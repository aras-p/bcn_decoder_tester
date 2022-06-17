#ifndef __BLOCKDATA_HPP__
#define __BLOCKDATA_HPP__

#include <stdint.h>

void etcpak_BlockData_DecodeDxt1(const void* m_data, int m_sizex, int m_sizey, void* ret_Data);
void etcpak_BlockData_DecodeDxt5(const void* m_data, int m_sizex, int m_sizey, void* ret_Data);


#endif
