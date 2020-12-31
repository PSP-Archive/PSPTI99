/*
 *  Copyright (C) 2006 Ludovic Jacomme (ludovic.jacomme@gmail.com)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <stdio.h>
#include <zlib.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>

#include <zlib.h>
#include <sys/stat.h>
#include <pspuser.h>
#include "global.h"
#include "psp_fmgr.h"
#include "psp_kbd.h"
#include "psp_ti99.h"

//LUDO:
  TI99_t TI99;

void
ti99_default_settings()
{
  TI99.ti99_snd_enable     = 1;
  TI99.ti99_render_mode    = TI99_RENDER_FIT_HEIGHT;
  TI99.psp_reverse_analog  = 0;
  TI99.psp_cpu_clock       = 222;
  TI99.psp_display_lr      = 0;
  TI99.ti99_speed_limiter  = 50;
  TI99.ti99_view_fps       = 0;
  TI99.psp_screenshot_id   = 0;
  TI99.ti99_vsync          = 0;
  TI99.ti99_auto_fire = 0;
  TI99.ti99_auto_fire_period = 6;
  TI99.ti99_auto_fire_pressed = 0;

  scePowerSetClockFrequency(TI99.psp_cpu_clock, TI99.psp_cpu_clock, TI99.psp_cpu_clock/2);
}

int
loc_ti99_save_settings(char *chFileName)
{
  FILE* FileDesc;
  int   error = 0;

  FileDesc = fopen(chFileName, "w");
  if (FileDesc != (FILE *)0 ) {

    fprintf(FileDesc, "psp_cpu_clock=%d\n"       , TI99.psp_cpu_clock);
    fprintf(FileDesc, "psp_reverse_analog=%d\n"  , TI99.psp_reverse_analog);
    fprintf(FileDesc, "psp_skip_max_frame=%d\n"  , TI99.psp_skip_max_frame);
    fprintf(FileDesc, "psp_display_lr=%d\n"      , TI99.psp_display_lr);
    fprintf(FileDesc, "ti99_snd_enable=%d\n"     , TI99.ti99_snd_enable);
    fprintf(FileDesc, "ti99_render_mode=%d\n"    , TI99.ti99_render_mode);
    fprintf(FileDesc, "ti99_speed_limiter=%d\n"  , TI99.ti99_speed_limiter);
    fprintf(FileDesc, "ti99_view_fps=%d\n"       , TI99.ti99_view_fps);
    fprintf(FileDesc, "ti99_vsync=%d\n"        , TI99.ti99_vsync);
    fprintf(FileDesc, "ti99_auto_fire_period=%d\n", TI99.ti99_auto_fire_period);

    fclose(FileDesc);

  } else {
    error = 1;
  }

  return error;
}

int
ti99_save_settings(void)
{
  char  FileName[MAX_PATH+1];
  int   error;

  error = 1;

  snprintf(FileName, MAX_PATH, "%s/set/%s.set", TI99.ti99_home_dir, TI99.ti99_save_name);
  error = loc_ti99_save_settings(FileName);

  return error;
}

static int
loc_ti99_load_settings(char *chFileName)
{
  char  Buffer[512];
  char *Scan;
  unsigned int Value;
  FILE* FileDesc;

  FileDesc = fopen(chFileName, "r");
  if (FileDesc == (FILE *)0 ) return 0;

  while (fgets(Buffer,512, FileDesc) != (char *)0) {

    Scan = strchr(Buffer,'\n');
    if (Scan) *Scan = '\0';
    /* For this #@$% of windows ! */
    Scan = strchr(Buffer,'\r');
    if (Scan) *Scan = '\0';
    if (Buffer[0] == '#') continue;

    Scan = strchr(Buffer,'=');
    if (! Scan) continue;

    *Scan = '\0';
    Value = atoi(Scan+1);

    if (!strcasecmp(Buffer,"psp_cpu_clock"))      TI99.psp_cpu_clock = Value;
    else
    if (!strcasecmp(Buffer,"psp_reverse_analog")) TI99.psp_reverse_analog = Value;
    else
    if (!strcasecmp(Buffer,"psp_skip_max_frame")) TI99.psp_skip_max_frame = Value;
    else
    if (!strcasecmp(Buffer,"psp_display_lr"))     TI99.psp_display_lr = Value;
    else
    if (!strcasecmp(Buffer,"ti99_snd_enable"))     TI99.ti99_snd_enable = Value;
    else
    if (!strcasecmp(Buffer,"ti99_render_mode"))    TI99.ti99_render_mode = Value;
    else
    if (!strcasecmp(Buffer,"ti99_speed_limiter"))  TI99.ti99_speed_limiter = Value;
    else
    if (!strcasecmp(Buffer,"ti99_view_fps"))  TI99.ti99_view_fps = Value;
    else
    if (!strcasecmp(Buffer,"ti99_vsync"))  TI99.ti99_vsync = Value;
    else
    if (!strcasecmp(Buffer,"ti99_auto_fire_period")) TI99.ti99_auto_fire_period = Value;
  }

  fclose(FileDesc);

  scePowerSetClockFrequency(TI99.psp_cpu_clock, TI99.psp_cpu_clock, TI99.psp_cpu_clock/2);

  return 0;
}

int
ti99_load_settings()
{
  char  FileName[MAX_PATH+1];
  int   error;

  error = 1;

  snprintf(FileName, MAX_PATH, "%s/set/%s.set", TI99.ti99_home_dir, TI99.ti99_save_name);
  error = loc_ti99_load_settings(FileName);

  return error;
}

int
ti99_load_file_settings(char *FileName)
{
  return loc_ti99_load_settings(FileName);
}

static int 
loc_load_ctg(char *TmpName)
{
  int error;
  error = ti99_load_cartridge(TmpName);
  return error;
}

void
update_save_name(char *Name)
{
  char        TmpFileName[MAX_PATH];
#ifdef LINUX_MODE
  struct stat aStat;
#else
  SceIoStat   aStat;
# endif
  int         index;
  char       *SaveName;
  char       *Scan1;
  char       *Scan2;

  SaveName = strrchr(Name,'/');
  if (SaveName != (char *)0) SaveName++;
  else                       SaveName = Name;

  if (!strncasecmp(SaveName, "sav_", 4)) {
    Scan1 = SaveName + 4;
    Scan2 = strrchr(Scan1, '_');
    if (Scan2 && (Scan2[1] >= '0') && (Scan2[1] <= '5')) {
      strncpy(TI99.ti99_save_name, Scan1, MAX_PATH);
      TI99.ti99_save_name[Scan2 - Scan1] = '\0';
    } else {
      strncpy(TI99.ti99_save_name, SaveName, MAX_PATH);
    }
  } else {
    strncpy(TI99.ti99_save_name, SaveName, MAX_PATH);
  }

  if (TI99.ti99_save_name[0] == '\0') {
    strcpy(TI99.ti99_save_name,"default");
  }

  for (index = 0; index < TI99_MAX_SAVE_STATE; index++) {
    TI99.ti99_save_state[index].used  = 0;
    memset(&TI99.ti99_save_state[index].date, 0, sizeof(ScePspDateTime));
    TI99.ti99_save_state[index].thumb = 0;

    snprintf(TmpFileName, MAX_PATH, "%s/save/sav_%s_%d.img", TI99.ti99_home_dir, TI99.ti99_save_name, index);
# ifdef LINUX_MODE
    if (! stat(TmpFileName, &aStat)) 
# else
    if (! sceIoGetstat(TmpFileName, &aStat))
# endif
    {
      TI99.ti99_save_state[index].used = 1;
      TI99.ti99_save_state[index].date = aStat.st_mtime;
      snprintf(TmpFileName, MAX_PATH, "%s/save/sav_%s_%d.png", TI99.ti99_home_dir, TI99.ti99_save_name, index);
# ifdef LINUX_MODE
      if (! stat(TmpFileName, &aStat)) 
# else
      if (! sceIoGetstat(TmpFileName, &aStat))
# endif
      {
        if (psp_sdl_load_thumb_png(TI99.ti99_save_state[index].surface, TmpFileName)) {
          TI99.ti99_save_state[index].thumb = 1;
        }
      }
    }
  }
}

void
reset_save_name()
{
  update_save_name("");
}

void
ti99_kbd_load(void)
{
  char        TmpFileName[MAX_PATH + 1];
  struct stat aStat;

  snprintf(TmpFileName, MAX_PATH, "%s/kbd/%s.kbd", TI99.ti99_home_dir, TI99.ti99_save_name );
  if (! stat(TmpFileName, &aStat)) {
    psp_kbd_load_mapping(TmpFileName);
  }
}

int
ti99_kbd_save(void)
{
  char TmpFileName[MAX_PATH + 1];
  snprintf(TmpFileName, MAX_PATH, "%s/kbd/%s.kbd", TI99.ti99_home_dir, TI99.ti99_save_name );
  return( psp_kbd_save_mapping(TmpFileName) );
}


//Load Functions

typedef struct {
   char *pchZipFile;
   char *pchExtension;
   char *pchFileNames;
   char *pchSelection;
   int iFiles;
   unsigned int dwOffset;
} t_zip_info;

t_zip_info zip_info;

typedef unsigned int    dword;
typedef unsigned short  word;
typedef unsigned char   byte;

#define ERR_FILE_NOT_FOUND       13
#define ERR_FILE_BAD_ZIP         14
#define ERR_FILE_EMPTY_ZIP       15
#define ERR_FILE_UNZIP_FAILED    16

FILE *pfileObject;
char *pbGPBuffer = NULL;

static dword
loc_get_dword(byte *buff)
{
  return ( (((dword)buff[3]) << 24) |
           (((dword)buff[2]) << 16) |
           (((dword)buff[1]) <<  8) |
           (((dword)buff[0]) <<  0) );
}

static void
loc_set_dword(byte *buff, dword value)
{
  buff[3] = (value >> 24) & 0xff;
  buff[2] = (value >> 16) & 0xff;
  buff[1] = (value >>  8) & 0xff;
  buff[0] = (value >>  0) & 0xff;
}

static word
loc_get_word(byte *buff)
{
  return( (((word)buff[1]) <<  8) |
          (((word)buff[0]) <<  0) );
}


int 
zip_dir(t_zip_info *zi)
{
   int n, iFileCount;
   long lFilePosition;
   dword dwCentralDirPosition, dwNextEntry;
   word wCentralDirEntries, wCentralDirSize, wFilenameLength;
   byte *pbPtr;
   char *pchStrPtr;
   dword dwOffset;

   iFileCount = 0;
   if ((pfileObject = fopen(zi->pchZipFile, "rb")) == NULL) {
      return ERR_FILE_NOT_FOUND;
   }

   if (pbGPBuffer == (char *)0) {
     pbGPBuffer = (char *)malloc( sizeof(byte) * 128*1024); 
   }

   wCentralDirEntries = 0;
   wCentralDirSize = 0;
   dwCentralDirPosition = 0;
   lFilePosition = -256;
   do {
      fseek(pfileObject, lFilePosition, SEEK_END);
      if (fread(pbGPBuffer, 256, 1, pfileObject) == 0) {
         fclose(pfileObject);
         return ERR_FILE_BAD_ZIP; // exit if loading of data chunck failed
      }
      pbPtr = (byte*)(pbGPBuffer + (256 - 22)); // pointer to end of central directory (under ideal conditions)
      while (pbPtr != (byte *)pbGPBuffer) {
         if (loc_get_dword(pbPtr) == 0x06054b50) { // check for end of central directory signature
            wCentralDirEntries = loc_get_word(pbPtr + 10);
            wCentralDirSize = loc_get_word(pbPtr + 12);
            dwCentralDirPosition = loc_get_dword(pbPtr + 16);
            break;
         }
         pbPtr--; // move backwards through buffer
      }
      lFilePosition -= 256; // move backwards through ZIP file
   } while (wCentralDirEntries == 0);
   if (wCentralDirSize == 0) {
      fclose(pfileObject);
      return ERR_FILE_BAD_ZIP; // exit if no central directory was found
   }
   fseek(pfileObject, dwCentralDirPosition, SEEK_SET);
   if (fread(pbGPBuffer, wCentralDirSize, 1, pfileObject) == 0) {
      fclose(pfileObject);
      return ERR_FILE_BAD_ZIP; // exit if loading of data chunck failed
   }

   pbPtr = (byte *)pbGPBuffer;
   if (zi->pchFileNames) {
      free(zi->pchFileNames); // dealloc old string table
   }
   zi->pchFileNames = (char *)malloc(wCentralDirSize); // approximate space needed by using the central directory size
   pchStrPtr = zi->pchFileNames;

   for (n = wCentralDirEntries; n; n--) {
      wFilenameLength = loc_get_word(pbPtr + 28);
      dwOffset = loc_get_dword(pbPtr + 42);
      dwNextEntry = wFilenameLength + loc_get_word(pbPtr + 30) + loc_get_word(pbPtr + 32);
      pbPtr += 46;
      char *pchThisExtension = zi->pchExtension;
      while (*pchThisExtension != '\0') { // loop for all extensions to be checked
         if (strncasecmp((char *)pbPtr + (wFilenameLength - 4), pchThisExtension, 4) == 0) {
            strncpy(pchStrPtr, (char *)pbPtr, wFilenameLength); // copy filename from zip directory
            pchStrPtr[wFilenameLength] = 0; // zero terminate string
            pchStrPtr += wFilenameLength+1;
            loc_set_dword((byte*)pchStrPtr, dwOffset);
            pchStrPtr += 4;
            iFileCount++;
            break;
         }
         pchThisExtension += 4; // advance to next extension
      }
      pbPtr += dwNextEntry;
   }
   fclose(pfileObject);

   if (iFileCount == 0) { // no files found?
      return ERR_FILE_EMPTY_ZIP;
   }

   zi->iFiles = iFileCount;
   return 0; // operation completed successfully
}

int 
zip_extract(char *pchZipFile, char *pchFileName, dword dwOffset, char *ext)
{
   int iStatus, iCount;
   dword dwSize;
   byte *pbInputBuffer, *pbOutputBuffer;
   FILE *pfileOut, *pfileIn;
   z_stream z;

   strcpy(pchFileName, TI99.ti99_home_dir);
   strcat(pchFileName, "/unzip.");
   strcat(pchFileName, ext);

   if (!(pfileOut = fopen(pchFileName, "wb"))) {
      return ERR_FILE_UNZIP_FAILED; // couldn't create output file
   }
   if (pbGPBuffer == (char *)0) {
     pbGPBuffer = (char *)malloc( sizeof(byte) * 128*1024); 
   }
   pfileIn = fopen(pchZipFile, "rb"); // open ZIP file for reading
   fseek(pfileIn, dwOffset, SEEK_SET); // move file pointer to beginning of data block
   fread(pbGPBuffer, 30, 1, pfileIn); // read local header
   dwSize = loc_get_dword((byte *)(pbGPBuffer + 18)); // length of compressed data
   dwOffset += 30 + loc_get_word((byte *)(pbGPBuffer + 26)) + loc_get_word((byte *)(pbGPBuffer + 28));
   fseek(pfileIn, dwOffset, SEEK_SET); // move file pointer to start of compressed data

   pbInputBuffer = (byte *)pbGPBuffer; // space for compressed data chunck
   pbOutputBuffer = pbInputBuffer + 16384; // space for uncompressed data chunck
   z.zalloc = (alloc_func)0;
   z.zfree = (free_func)0;
   z.opaque = (voidpf)0;
   iStatus = inflateInit2(&z, -MAX_WBITS); // init zlib stream (no header)
   do {
      z.next_in = pbInputBuffer;
      if (dwSize > 16384) { // limit input size to max 16K or remaining bytes
         z.avail_in = 16384;
      } else {
         z.avail_in = dwSize;
      }
      z.avail_in = fread(pbInputBuffer, 1, z.avail_in, pfileIn); // load compressed data chunck from ZIP file
      while ((z.avail_in) && (iStatus == Z_OK)) { // loop until all data has been processed
         z.next_out = pbOutputBuffer;
         z.avail_out = 16384;
         iStatus = inflate(&z, Z_NO_FLUSH); // decompress data
         iCount = 16384 - z.avail_out;
         if (iCount) { // save data to file if output buffer is full
            fwrite(pbOutputBuffer, 1, iCount, pfileOut);
         }
      }
      dwSize -= 16384; // advance to next chunck
   } while ((dwSize > 0) && (iStatus == Z_OK)) ; // loop until done
   if (iStatus != Z_STREAM_END) {
      return ERR_FILE_UNZIP_FAILED; // abort on error
   }
   iStatus = inflateEnd(&z); // clean up
   fclose(pfileIn);
   fclose(pfileOut);

   return 0; // data was successfully decompressed
}

int
ti99_load_ctg(char *FileName)
{
  char *scan;
  char  SaveName[MAX_PATH+1];
  int   error;

  error = 1;

  strncpy(SaveName,FileName,MAX_PATH);
  scan = strrchr(SaveName,'.');
  if (scan) *scan = '\0';
  update_save_name(SaveName);
  error = loc_load_ctg(FileName);

  if (! error ) {
    ti99_kbd_load();
    ti99_load_settings();
  }

  return error;
}

static int
loc_load_state(char *filename)
{
  int error;
  error = ! ti99_load_img(filename);
  return error;
}

int
ti99_load_state(char *FileName)
{
  char *scan;
  char  SaveName[MAX_PATH+1];
  int   error;

  error = 1;

  strncpy(SaveName,FileName,MAX_PATH);
  scan = strrchr(SaveName,'.');
  if (scan) *scan = '\0';
  update_save_name(SaveName);
  error = ti99_load_img(FileName);

  if (! error ) {
    ti99_kbd_load();
    ti99_load_settings();
  }

  return error;
}

int
ti99_snapshot_save_slot(int save_id)
{
  char  FileName[MAX_PATH+1];
#ifdef LINUX_MODE
  struct stat aStat;
#else
  SceIoStat   aStat;
# endif
  int   error;

  error = 1;

  if (save_id < TI99_MAX_SAVE_STATE) {
    snprintf(FileName, MAX_PATH, "%s/save/sav_%s_%d.img", TI99.ti99_home_dir, TI99.ti99_save_name, save_id);
    error = ti99_save_img(FileName);
    if (! error) {
# ifdef LINUX_MODE
      if (! stat(FileName, &aStat)) 
# else
      if (! sceIoGetstat(FileName, &aStat))
# endif
      {
        TI99.ti99_save_state[save_id].used  = 1;
        TI99.ti99_save_state[save_id].thumb = 0;
        TI99.ti99_save_state[save_id].date  = aStat.st_mtime;
        snprintf(FileName, MAX_PATH, "%s/save/sav_%s_%d.png", TI99.ti99_home_dir, TI99.ti99_save_name, save_id);
        if (psp_sdl_save_thumb_png(TI99.ti99_save_state[save_id].surface, FileName)) {
          TI99.ti99_save_state[save_id].thumb = 1;
        }
      }
    }
  }

  return error;
}

int
ti99_snapshot_load_slot(int load_id)
{
  char  FileName[MAX_PATH+1];
  int   error;

  error = 1;

  if (load_id < TI99_MAX_SAVE_STATE) {
    snprintf(FileName, MAX_PATH, "%s/save/sav_%s_%d.img", TI99.ti99_home_dir, TI99.ti99_save_name, load_id);
    error = loc_load_state(FileName);
  }
  return error;
}

int
ti99_snapshot_del_slot(int save_id)
{
  char  FileName[MAX_PATH+1];
# ifdef LINUX_MODE
  struct stat aStat;
# else
  SceIoStat   aStat;
# endif
  int   error;

  error = 1;

  if (save_id < TI99_MAX_SAVE_STATE) {
    snprintf(FileName, MAX_PATH, "%s/save/sav_%s_%d.img", TI99.ti99_home_dir, TI99.ti99_save_name, save_id);
    error = remove(FileName);
    if (! error) {
      TI99.ti99_save_state[save_id].used  = 0;
      TI99.ti99_save_state[save_id].thumb = 0;
      memset(&TI99.ti99_save_state[save_id].date, 0, sizeof(ScePspDateTime));

      snprintf(FileName, MAX_PATH, "%s/save/sav_%s_%d.png", TI99.ti99_home_dir, TI99.ti99_save_name, save_id);
# ifdef LINUX_MODE
      if (! stat(FileName, &aStat)) 
# else
      if (! sceIoGetstat(FileName, &aStat))
# endif
      {
        remove(FileName);
      }
    }
  }

  return error;
}

void
ti99_audio_pause()
{
  if (TI99.ti99_snd_enable) {
    SDL_PauseAudio(1);
  }
}

void
ti99_audio_resume()
{
  if (TI99.ti99_snd_enable) {
    SDL_PauseAudio(0);
  }
}

void
ti99_global_init()
{
  memset(&TI99, 0, sizeof(TI99_t));
  getcwd(TI99.ti99_home_dir,MAX_PATH);

  psp_sdl_init();

  ti99_default_settings();
  update_save_name("");
  ti99_load_settings();

  scePowerSetClockFrequency(TI99.psp_cpu_clock, TI99.psp_cpu_clock, TI99.psp_cpu_clock/2);
}

void
ti99_treat_command_key(int ti99_idx)
{
  int new_render;

  switch (ti99_idx) 
  {
    case TI99_C_FPS: TI99.ti99_view_fps = ! TI99.ti99_view_fps;
    break;
    case TI99_C_JOY: TI99.psp_reverse_analog = ! TI99.psp_reverse_analog;
    break;
    case TI99_C_RENDER: 
      psp_sdl_black_screen();
      new_render = TI99.ti99_render_mode + 1;
      if (new_render > TI99_LAST_RENDER) new_render = 0;
      TI99.ti99_render_mode = new_render;
    break;
    case TI99_C_LOAD: psp_main_menu_load_current();
    break;
    case TI99_C_SAVE: psp_main_menu_save_current(); 
    break;
    case TI99_C_RESET: 
       psp_sdl_black_screen();
       ti99_reset_computer();
       reset_save_name();
    break;
    case TI99_C_AUTOFIRE: 
       kbd_change_auto_fire(! TI99.ti99_auto_fire);
    break;
    case TI99_C_DECFIRE: 
      if (TI99.ti99_auto_fire_period > 0) TI99.ti99_auto_fire_period--;
    break;
    case TI99_C_INCFIRE: 
      if (TI99.ti99_auto_fire_period < 19) TI99.ti99_auto_fire_period++;
    break;
    case TI99_C_SCREEN: psp_screenshot_mode = 10;
    break;
  }
}
