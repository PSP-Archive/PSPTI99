//----------------------------------------------------------------------------
//
// File:        ti-disk.hpp
// Date:        27-Mar-1998
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

#ifndef TIDISK_HPP_
#define TIDISK_HPP_

#if ! defined ( DEVICE_HPP_ )
    #error You must include device.hpp before ti-disk.hpp
#endif

#if ! defined ( DISKIO_HPP_ )
    #error You must include diskio.hpp before ti-disk.hpp
#endif

#define REG_STATUS		0x5FF0
#define REG_RD_TRACK		0x5FF2
#define REG_RD_DATA		0x5FF6
#define REG_COMMAND		0x5FF8
#define REG_WR_TRACK		0x5FFA
#define REG_WR_SECTOR		0x5FFC
#define REG_WR_DATA		0x5FFE

// Not Ready
// Seek Error ( track not found )
// Record Not Found ( track/sector/side not found )
// Lost Data
// CRC Error
// Write protect

#define STATUS_NOT_READY	0x80		// No Diskette/Drive or Bad H/S
#define STATUS_WRITE_PROTECTED	0x40		// Write Protected Disk
#define STATUS_HEAD_LOADED      0x20
#define STATUS_WRITE_FAULT      0x20
#define STATUS_SEEK_ERROR	0x10		// Seek error or Bad H/S
#define STATUS_RECORD_NOT_FOUND 0x10            
#define STATUS_CRC_ERROR	0x08		// CRC Error on a read
#define STATUS_LOST_DATA	0x04		// Lost data on a read
#define STATUS_TRACK_0		0x04		// Head positioned to track 0
#define STATUS_INDEX_PULSE	0x02
#define STATUS_DATA_REQUEST     0x02
#define STATUS_BUSY     	0x01

#define STATUS_NOT_FOUND	( STATUS_CRC_ERROR | STATUS_LOST_DATA )

class cDiskDevice : public cDevice {

    enum TRAP_TYPE_E {
        TRAP_DISK
    };

    enum CMD_STATE_E {
        CMD_NONE,
        CMD_READ_ADDRESS,
        CMD_READ_TRACK,
        CMD_READ_SECTOR,
        CMD_WRITE_TRACK,
        CMD_WRITE_SECTOR
    };

    int            m_StepDirection;
    ULONG          m_ClocksPerRev;
    ULONG          m_ClockStart;

    // CRU bits
    int            m_HardwareBits;
    UCHAR          m_DriveSelect;
    UCHAR          m_HeadSelect;
    UCHAR          m_TrackSelect;           // Actual position of the hardware

    bool           m_IsFD1771;
    bool           m_TransferEnabled;

    cDiskMedia    *m_DiskMedia [3];
    cDiskMedia    *m_CurDisk;
    const sSector *m_CurSector;

    // Hardware registers
    UCHAR          m_StatusRegister;
    UCHAR          m_TrackRegister;
    UCHAR          m_SectorRegister;

    // The following are used to virtualize the Data Register
    UCHAR          m_LastData;
    int            m_BytesExpected;
    int            m_BytesLeft;
    UCHAR         *m_DataPtr;
    UCHAR          m_DataBuffer [MAX_TRACK_SIZE];

    CMD_STATE_E    m_CmdInProgress;
    UCHAR          m_TrapIndex;

    static USHORT TrapFunction ( void *, int, bool, const ADDRESS, USHORT );

    void FindSector ();

    void CompleteCommand ();

    UCHAR ReadByte ();
    void  WriteByte ( UCHAR );

    void Restore ( UCHAR );
    void Seek ( UCHAR );
    void Step ( UCHAR );
    void StepIn ( UCHAR );
    void StepOut ( UCHAR );
    void ReadSector ( UCHAR );
    void WriteSector ( UCHAR );
    void ReadAddress ( UCHAR );
    void ReadTrack ( UCHAR );
    void WriteTrack ( UCHAR );
    void ForceInterrupt ( UCHAR );
    void WriteTrackComplete ();

    void HandleCommand ( UCHAR );

    UCHAR WriteMemory ( const ADDRESS, UCHAR );
    UCHAR ReadMemory ( const ADDRESS, UCHAR );

public:

    cDiskDevice ( const char * );
    ~cDiskDevice ();

    void LoadDisk ( int, const char * );
    void UnLoadDisk ( int );

    //
    // cDevice methods
    //
    void Activate ();
    void DeActivate ();

    void WriteCRU ( ADDRESS, int );
    int  ReadCRU ( ADDRESS );

    void SaveImage ( FILE * );
    void LoadImage ( FILE * );

};

#endif
