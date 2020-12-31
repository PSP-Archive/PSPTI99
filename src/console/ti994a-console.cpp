//----------------------------------------------------------------------------
//
// File:        ti994a-console.cpp
// Date:        23-Feb-1998
// Programmer:  Marc Rousseau
//
// Description:
//
// Copyright (c) 1998-2003 Marc Rousseau, All Rights Reserved.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307, USA.
//
// Revision History:
//
//----------------------------------------------------------------------------

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.hpp"
#include "compress.hpp"
#include "tms9900.hpp"
#include "tms9918a.hpp"
#include "cartridge.hpp"
#include "ti994a.hpp"
#include "ti994a-console.hpp"
#include "tms9918a-console.hpp"
#include "screenio.hpp"
#include "support.hpp"

#define KEY_BACKSPACE   0x00000008
#define KEY_CR          0x0000000D
#define KEY_LF          0x0000000A
#define KEY_DELETE      0x0000007F
#define KEY_ENTER       0x0000000A
#define KEY_ESCAPE      0x0000001B
#define KEY_UP          0x00415B1B
#define KEY_DOWN        0x00425B1B
#define KEY_RIGHT       0x00435B1B
#define KEY_LEFT        0x00445B1B

extern "C" UCHAR  CpuMemory [ 0x10000 ];

extern USHORT DisassembleASM ( USHORT, UCHAR *, char * );
extern USHORT DisassembleGPL ( USHORT, UCHAR *, char * );

#if ! defined ( PM )

cConsoleTI994A::cConsoleTI994A ( cCartridge *ctg, cTMS9918A *vdp ) :
    cTI994A ( ctg, vdp ),
    m_CapsLock ( false ),
    m_ColumnSelect ( 0 ),
    m_KeyHead ( 0 ),
    m_KeyTail ( 0 )
{
    memset ( m_KeyBuffer, 0, sizeof ( m_KeyBuffer ));
}

cConsoleTI994A::~cConsoleTI994A ()
{
}

void cConsoleTI994A::KeyPressed ( int ch )
{
    switch ( ch ) {

        case '~'  : ch = FCTN_KEY | 'W';  break;
        case '['  : ch = FCTN_KEY | 'R';  break;
        case ']'  : ch = FCTN_KEY | 'T';  break;
        case '_'  : ch = FCTN_KEY | 'U';  break;
        case '?'  : ch = FCTN_KEY | 'I';  break;
        case '\'' : ch = FCTN_KEY | 'O';  break;
        case '"'  : ch = FCTN_KEY | 'P';  break;
        case '|'  : ch = FCTN_KEY | 'A';  break;
        case '{'  : ch = FCTN_KEY | 'F';  break;
        case '}'  : ch = FCTN_KEY | 'G';  break;
        case '\\' : ch = FCTN_KEY | 'Z';  break;
        case '`'  : ch = FCTN_KEY | 'C';  break;

        case '!'  : ch = SHIFT_KEY | '1';  break;
        case '@'  : ch = SHIFT_KEY | '2';  break;
        case '#'  : ch = SHIFT_KEY | '3';  break;
        case '$'  : ch = SHIFT_KEY | '4';  break;
        case '%'  : ch = SHIFT_KEY | '5';  break;
        case '^'  : ch = SHIFT_KEY | '6';  break;
        case '&'  : ch = SHIFT_KEY | '7';  break;
        case '*'  : ch = SHIFT_KEY | '8';  break;
        case '('  : ch = SHIFT_KEY | '9';  break;
        case ')'  : ch = SHIFT_KEY | '0';  break;
        case '+'  : ch = SHIFT_KEY | '=';  break;
        case '-'  : ch = SHIFT_KEY | '/';  break;
        case ':'  : ch = SHIFT_KEY | ';';  break;
        case '<'  : ch = SHIFT_KEY | ',';  break;
        case '>'  : ch = SHIFT_KEY | '.';  break;

        case 0x0D : ch = 0x0A;  break;

        case KEY_BACKSPACE :
        case KEY_DELETE   :
        case KEY_LEFT     : ch = FCTN_KEY | 'S';  break;
        case KEY_RIGHT    : ch = FCTN_KEY | 'D';  break;
        case KEY_UP       : ch = FCTN_KEY | 'E';  break;
        case KEY_DOWN     : ch = FCTN_KEY | 'X';  break;

        default :
            if ( ch < 0x1B ) {
                ch = CTRL_KEY | ( ch + 'a' );
            }
            if (( ch & 0xFFFF00FF ) == KEY_ESCAPE ) {
                ch = FCTN_KEY | (( ch >> 8 ) & 0xFF );
            } else {
                ch = isupper ( ch ) ? SHIFT_KEY | ch : toupper ( ch );
            }
            break;
    }

    m_KeyBuffer [ m_KeyHead ] = ch;
    m_KeyHead = ( m_KeyHead + 1 ) % SIZE ( m_KeyBuffer );
}

int cConsoleTI994A::ReadCRU ( ADDRESS address )
{
    static UCHAR Keys[8][8] = {
        { '=',    '.', ',', 'M', 'N', '/' },		// Row 7
        { ' ',    'L', 'K', 'J', 'H', ';' },		// Row 6
        { '\x0A', 'O', 'I', 'U', 'Y', 'P' },		// Row 5
        { '\x00', '9', '8', '7', '6', '0' },		// Row 4
        { '\x00', '2', '3', '4', '5', '1' },		// Row 3
        { '\x00', 'S', 'D', 'F', 'G', 'A' },		// Row 2
        { '\x00', 'W', 'E', 'R', 'T', 'Q' },		// Row 1
        { '\x00', 'X', 'C', 'V', 'B', 'Z' } 		// Row 0
    };

    int retVal = 1;

    if (( address >= 3 ) && ( address <= 10 )) {

        if (( m_CapsLock == false ) && ( address == 7 )) {
            return 0;
        }

        if ( m_KeyHead == m_KeyTail ) return 1;
        int key = m_KeyBuffer [ m_KeyTail ];
        if ( m_ColumnSelect == 0 ) {
            if ( address > 5 ) {
                switch ( address ) {
                    case  7 : retVal = ( key & FCTN_KEY )  ? 0 : 1;	break;
                    case  8 : retVal = ( key & SHIFT_KEY ) ? 0 : 1;	break;
                    case  9 : retVal = ( key & CTRL_KEY )  ? 0 : 1;	break;
                }
                return retVal;
            } else if ( address == 3 ) {
                m_KeyTail = ( m_KeyTail + 1 ) % SIZE ( m_KeyBuffer );
            }
        }
        if ( m_ColumnSelect < 6 ) {
            UCHAR *row;
            switch ( address ) {
                case  3 : row = Keys [ 0 ];		break;
                case  4 : row = Keys [ 1 ];		break;
                case  5 : row = Keys [ 2 ];		break;
                case  6 : row = Keys [ 3 ];		break;
                case  7 : row = Keys [ 4 ];		break;
                case  8 : row = Keys [ 5 ];		break;
                case  9 : row = Keys [ 6 ];		break;
                case 10 : row = Keys [ 7 ];		break;
                default : fprintf ( stderr, "Invalid address (%d) for ColumSelect (%d)\n", address, m_ColumnSelect );
                          return 0;
            }
            if ( row [ m_ColumnSelect ] == ( key & 0xFF )) retVal = 0;
        }
    } else {
        retVal = cTI994A::ReadCRU ( address );
    }

    return retVal;
}

void cConsoleTI994A::WriteCRU ( ADDRESS address, USHORT data )
{
    if (( address >= 18 ) && ( address <= 20 )) {
        int shift = address - 18;
        m_ColumnSelect &= ~ ( 1 << shift );
        m_ColumnSelect |= ( data & 1 ) << shift;
    } else if ( address == 21 ) {
        m_CapsLock = ( data != 0 ) ? true : false;
    } else {
        cTI994A::WriteCRU ( address, data );
    }
}
#else
void cConsoleTI994A::KeyPressed ( int ) {}
int cConsoleTI994A::ReadCRU ( ADDRESS ) { return 1; }
void cConsoleTI994A::WriteCRU ( ADDRESS, int ) {}
#endif

#if ! defined ( PM )

int EditNumber ( int x, int y, USHORT *value, int max, int &pos )
{
    static int hex [] = { '0', '1', '2', '3', '4', '5', '6', '7',
                   '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
    int ch;

    ShowCursor ();

    do {
        GotoXY ( x + pos, y );
        ch = GetKey ();
        switch ( ch ) {
            case KEY_CR        :
            case KEY_LF        : ch = KEY_DOWN; break;
            case KEY_BACKSPACE :
            case KEY_DELETE    :
            case KEY_LEFT      : pos--; ch = 0; break;
            case KEY_RIGHT     : pos++; ch = 0; break;
            default :
                ch = ( ch < 256 ) ? toupper ( ch ) : ch;
                for ( unsigned i = 0; i < SIZE ( hex ); i++ ) {
                    if ( ch == hex [i] ) {
                        int byteIndex   = pos / 2;
                        GotoXY ( x + byteIndex * 2, y );
                        UCHAR *ptr = ( UCHAR * ) value + byteIndex;
                        *ptr = ( UCHAR ) (( pos % 2 == 0 ) ? (( *ptr & 0x0F ) | ( i << 4 )) : (( *ptr & 0xF0 ) | i ));
                        outByte ( *ptr );
                        pos++;
                    }
                }
                break;
        }
        if ( pos >= max ) ch = KEY_RIGHT;
        if ( pos < 0 ) ch = KEY_LEFT;
    } while ((( ch & 0xFF00 ) == 0 ) && ( ch != KEY_ESCAPE ));

    HideCursor ();

    return ch;
}

void cConsoleTI994A::EditRegisters ()
{
    USHORT *currReg = ( USHORT * ) &CpuMemory [ m_CPU->GetWP () ];
    int ch, pos = 0, regIndex = 0;
    do {
        ch = EditNumber ( 48 + 9 * ( regIndex >> 2 ), 4 + ( regIndex & 0x03 ), currReg + regIndex, 4, pos );
        switch ( ch ) {
            case KEY_UP    : if ( regIndex > 0 ) regIndex--;
                             break;
            case KEY_DOWN  : if ( regIndex < 15 ) regIndex++;
                             break;
            case KEY_RIGHT : if ( regIndex < 15 ) {
                                 regIndex += ( regIndex < 12 ) ? 4 : -11;
                                 pos = 0;
                             } else {
                                 pos = 3;
                             }
                             break;
            case KEY_LEFT  : if ( regIndex > 0 ) {
                                 regIndex -= ( regIndex > 3 ) ? 4 : -11;
                                 pos = 3;
                             } else {
                                 pos = 0;
                             }
                             break;
        }
    } while ( ch != KEY_ESCAPE );
}

void cConsoleTI994A::Run ()
{
    static ULONG lastClock = 0;

    Refresh ( true );
    enum { STEP, RUN };
    ADDRESS bkptPC = 0x0070;
    int bkptCount = -1;
    int mode = STEP;
    int update = 0;
    bool done = false;
    cConsoleTMS9918A *vdp = ( cConsoleTMS9918A * ) m_VDP;

    do {
        USHORT PC = m_CPU->GetPC ();

        bool refresh = false;
        if (( int ) m_CPU->GetCounter () == bkptCount ) mode = STEP;
        if ( PC == bkptPC ) mode = STEP;
        if ( PC == 0x0070 ) refresh = true;
        if ( update++ == 32 ) refresh = true;
        if ( mode == STEP ) refresh = true;
        if ( refresh ) { Refresh ( false ); update = 0; }

        if ( mode == STEP ) {
            int ch;
            do {
                ch = GetKey ();
                ch = ( ch < 256 ) ? toupper ( ch ) : ch;
                switch ( ch ) {
                    case 'C' : bkptPC = 0xFFFF;                         break;
                    case 'G' : mode = RUN;                              break;
                    case 'R' : EditRegisters ();                        break;
                    case 'B' : vdp->SetBias ( 0x60 );                   break;
                    case 'N' : vdp->SetBias ( 0x00 );                   break;
                    case 'L' : LoadImage ( "ti-994a.img" );             break;
                    case 'S' : SaveImage ( "ti-994a.img" );             break;
                }
            } while (( ch != 'G' ) && ( ch != ' ' ) && ( ch != 'Q' ));
            if ( ch == 'Q' ) break;
        }

        Step ();

        ULONG clockCycles = m_CPU->GetClocks ();

        // Simulate a 50/60Hz VDP interrupt
        if ( clockCycles - lastClock > m_RefreshInterval ) {
            lastClock = clockCycles;
            vdp->Retrace ();
        }

        if (( mode == RUN ) && ::KeyPressed ()) {
            int ch = GetKey ();
            if ( ch == KEY_ESCAPE ) {
                mode = STEP;
            } else {
                KeyPressed ( ch );
            }
        }

    } while ( ! done );
}
#endif

#if defined ( LOG_GROM )
static FILE *gplFile = fopen ( "GPL.HST", "wt" );
static int level = 0;
#endif
bool cConsoleTI994A::Step ()
{
    m_CPU->Step ();
    if ( m_CPU->GetPC () == 0x0070 ) {
        m_GromCounter++;
#if defined ( LOG_GROM )
        char buffer [256];
        DisassembleGPL ( m_GromAddress, m_GromPtr, buffer );
        fprintf ( gplFile, "%*.*s%s\n", level, level, "", buffer );
        if ( *m_GromPtr == 0x06 ) level += 2;
        else if (( level > 0 ) && (( *m_GromPtr == 0x00 ) || ( *m_GromPtr == 0x01 ))) level -= 2;
#endif
    }
    return false;
}

void cConsoleTI994A::SaveImage ( const char *filename )
{
    char buffer [256];

    sprintf ( buffer, "%s%c%s", HOME_PATH, FILE_SEPERATOR, filename );

    cTI994A::SaveImage ( buffer );
}

bool cConsoleTI994A::LoadImage ( const char *filename )
{
    char buffer [256];

    sprintf ( buffer, "%s%c%s", HOME_PATH, FILE_SEPERATOR, filename );

    return cTI994A::LoadImage ( buffer );
}

USHORT cConsoleTI994A::VideoReadBreakPoint ( const ADDRESS address, USHORT data )
{
    switch ( address ) {
        case 0x8800 : data = m_VDP->ReadData ();
                      GotoXY ( 74, 0 );  outWord ( m_VDP->GetAddress ());
                      GotoXY ( 75, 1 );  outByte (( UCHAR ) data );
                      break;
        case 0x8802 : data = m_VDP->ReadStatus ();
                      break;
    }
    return data;
}

USHORT cConsoleTI994A::VideoWriteBreakPoint ( const ADDRESS address, USHORT data )
{
    switch ( address ) {
        case 0x8C00 : m_VDP->WriteData (( UCHAR ) data );
                      GotoXY ( 75, 1 );  outByte (( UCHAR ) data );
                      break;
        case 0x8C02 : m_VDP->WriteAddress (( UCHAR ) data );
                      break;
    }
    GotoXY ( 74, 0 );  outWord ( m_VDP->GetAddress ());
    return data;
}

USHORT cConsoleTI994A::GromReadBreakPoint ( const ADDRESS address, USHORT data )
{
    switch ( address ) {
        // Grom/Gram Read Byte Port
        case 0x9800 : data = *m_GromPtr++;
                      m_GromAddress++;
                      GotoXY ( 62, 0 );  outWord ( m_GromAddress );
                      GotoXY ( 63, 1 );  outByte (( UCHAR ) data );
                      break;
        // Grom/Gram Read Address Port
        case 0x9802 : data = ( USHORT ) ((( m_GromAddress + 1 ) >> m_GromReadShift ) & 0x00FF );
                      m_GromReadShift = 8 - m_GromReadShift;
                      m_GromPtr = &m_GromMemory [ m_GromAddress ];
                      m_GromWriteShift = 8;
                      break;
    }
    return data;
}

USHORT cConsoleTI994A::GromWriteBreakPoint ( const ADDRESS address, USHORT data )
{
    sMemoryRegion *memory;
    switch ( address ) {
        // Grom/Gram Write Byte Port
        case 0x9C00 : memory = m_GromMemoryInfo [ m_GromAddress >> 13 ];
                      if ( memory && ( memory->CurBank->Type != MEMORY_ROM )) *m_GromPtr = ( UCHAR ) data;
                      m_GromPtr++;
                      m_GromAddress++;
                      GotoXY ( 63, 1 );  outByte (( UCHAR ) data );
                      break;
        // Grom/Gram Write (set) Address Port
        case 0x9C02 : m_GromAddress &= ( USHORT ) ( 0xFF00 >> m_GromWriteShift );
                      m_GromAddress |= ( USHORT ) ( data << m_GromWriteShift );
                      m_GromWriteShift = 8 - m_GromWriteShift;
                      m_GromPtr = &m_GromMemory [ m_GromAddress ];
                      m_GromReadShift = 8;
                      break;
    }
    GotoXY ( 62, 0 );  outWord ( m_GromAddress );
    return data;
}

void cConsoleTI994A::Refresh ( bool complete )
{
    static ADDRESS lastWP = 0;
    static UCHAR   vdpReg [8];
    static USHORT  lastST = 0, lastReg [16];
    char buffer [ 256 ];

    if ( complete ) {
        PutXY ( 45,  0, "WP: ", 4 );
        PutXY ( 45,  1, "PC: ", 4 );
        PutXY ( 45,  2, "ST: ", 4 );

        PutXY ( 56,  0, "GROM: ", 6 );  outWord ( m_GromAddress );
        PutXY ( 56,  1, "Byte:  --", 9 );

        PutXY ( 68,  0, " VDP: ", 6 );  outWord ( m_VDP->GetAddress ());
        PutXY ( 68,  1, "Byte:  --", 9 );

        PutXY ( 45,  4, "R0 0000  R4 0000  R8 0000 R12 0000", 34 );
        PutXY ( 45,  5, "R1 0000  R5 0000  R9 0000 R13 0000", 34 );
        PutXY ( 45,  6, "R2 0000  R6 0000 R10 0000 R14 0000", 34 );
        PutXY ( 45,  7, "R3 0000  R7 0000 R11 0000 R15 0000", 34 );

        PutXY ( 45,  9, "V0 00    V1 00    V2 00    V3 00", 32 );
        PutXY ( 45, 10, "V4 00    V5 00    V6 00    V7 00", 32 );

        PutXY ( 45, 12, "GPL:", 4 );
        PutXY ( 45, 19, "CPU:", 4 );

        lastWP = ( ADDRESS ) -1;
        lastST = ( USHORT ) -1;
        memset ( lastReg, -1, sizeof ( lastReg ));
        memset ( vdpReg, -1, sizeof ( vdpReg ));
    }

    char *spaces = "                                 ";
    USHORT PC = m_CPU->GetPC ();
    if ( complete || ( PC == 0x0070 )) {
        if ( PC == 0x0070 ) m_GromLastInstruction = m_GromAddress;
        UCHAR *gromPtr = m_GromPtr;
        for ( int i = 0; i < 5; i++ ) {
            USHORT gromAddress = ( USHORT ) ( m_GromAddress + ( gromPtr  - m_GromPtr ));
            gromPtr += DisassembleGPL ( gromAddress, gromPtr, buffer );
            int len = strlen ( buffer );
            if ( len > 35 ) len = 35;
            PutXY ( 45, 13 + i, buffer, len );
            if ( len != 35 ) PutXY ( 45 + len, 13 + i, spaces, 35 - len );
        }
        GotoXY ( 71, 12 );    outLong ( m_GromCounter );
    }
    if ( complete || ( m_CPU->GetWP () != lastWP )) {
        GotoXY ( 49, 0 );    outWord ( lastWP = m_CPU->GetWP ());
    }
    GotoXY ( 49, 1 );  outWord ( PC );
    if ( complete || ( m_CPU->GetST () != lastST )) {
        GotoXY ( 49, 2 );  outWord ( lastST = m_CPU->GetST ());
    }

    GotoXY ( 61, 19 );    outLong ( m_CPU->GetClocks ());
    GotoXY ( 71, 19 );    outLong ( m_CPU->GetCounter ());
    USHORT *currReg = ( USHORT * ) &CpuMemory [ lastWP ];

    for ( int i = 0; i < 16; i++ ) {
        if ( complete || ( lastReg[i] != currReg [i] )) {
            GotoXY ( 48 + 9 * ( i >> 2 ), 4 + ( i & 0x03 ));
            lastReg[i] = currReg [i];
            USHORT value = ( USHORT ) (( CpuMemory [ lastWP + i * 2 ] << 8 ) | CpuMemory [ lastWP + i * 2 + 1 ] );
            outWord ( value );
        }
    }

    for ( int i = 0; i < 8; i++ ) {
        if ( complete || ( vdpReg[i] != m_VDP->ReadRegister ( i ))) {
            GotoXY ( 48 + ( i % 4 ) * 9, ( i < 4 ) ? 9 : 10 );
            outByte (( UCHAR ) ( vdpReg[i] = m_VDP->ReadRegister ( i )));
        }
    }

    for ( int i = 0; i < 5; i++ ) {
        PC = DisassembleASM ( PC, &CpuMemory [ PC ], buffer );
        int len = strlen ( buffer );
        if ( len > 33 ) len = 33;
        PutXY ( 45, 20 + i, buffer, len );
        if ( len != 33 ) PutXY ( 45 + len, 20 + i, spaces, 33 - len );
    }
}
