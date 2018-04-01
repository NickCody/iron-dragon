// =---------------------------------------------------------------------------
// u t i l . c p p
// 
//   (C) 1999 - Martin R. Szinger, Nicholas Codignotto.
//
//
//   DESCRIPTION
//   -----------
//
//
//   WIN32 IMPLEMENTATION NOTES
//   --------------------------
//
//
//   DATE        WHO   REVISION
//   ---------   ---   --------------------------------------------------------
//   99 Apr 03   nic   Created.
//

#include "PreComp.h"

#include <objidl.h>
#include <SHLOBJ.H>

// =---------------------------------------------------------------------------
// Process_Command_Line 
//
// ALGORITHM: 
//
// Initial State:
//    argc = 0
//    argc = array of strig pointers, each initialize to null
//    raw_cl = "E:\Program Files\NMRF\res_util.exe" "c:\test\test.nmrf"
//
//    char* arg_start, arg_end = raw_cl
//
//    In a loop, arg_end is incremented until a space (not enclosed in quotes)
//    is encountered, wherupon the string between arg_start and arg_end is cropped
//    the process repeats until arg_end actually encounters a real null terminator
//
void Process_Command_Line ( int& argc, char**& argv, const int max_args )
{

   // Initialize our in-out params
   //
   argc = 0;
   argv = new char*[max_args];
   for ( int i=0; i < max_args; i++ )
      argv[i] = 0;

   char* raw_cl    = GetCommandLine();
   char* arg_start = raw_cl;
   char* arg_end   = raw_cl;

   bool count_space = true;
   while ( 1 )
   {
      if ( *arg_end == '\"' )
      {
         count_space = !count_space;
         arg_end++;
      }
      else if ( count_space && (*arg_end == ' ' || *arg_end == 0) )
      {
         char old_arg_end = *arg_end;
         *arg_end = 0;

         if ( strlen(arg_start) > 0 )
         {
            argv[argc] = new char[strlen(arg_start)+1];
            strcpy ( argv[argc], arg_start );
            *arg_end = old_arg_end;

            argc++;
         }

         if ( old_arg_end == 0 )
            break;

         arg_end++;
         arg_start = arg_end;

      }
      else
      {
         arg_end++;
      }
   }  // while
}


// =---------------------------------------------------------------------------
// Destroy_Command_Line
//
//
void Destroy_Command_Line ( int& argc, char**& argv, const int max_args )
{
   if ( !argv )
      return;

   for ( int i=0; i < max_args; i++ )
   {
      delete [] argv[i];
   }

   delete [] argv;
   
   argv   = 0;
   argc = 0;
}

// =---------------------------------------------------------------------------
// Translate_Shortcut
//
// If the argument filename is a .lnk file, it is resolved to a real filename
// Argument buffer is rewritten. Should be MAX_PATH in size (255 or so)
// 

bool Translate_Shortcut( char* file_name )
{
   char* file_extension = strrchr( file_name, '.' );
   if ( file_extension ) file_extension++;

   if ( file_extension  && strcmpi( file_extension, "lnk" ) == 0 )
   {
      // we will actually modify the contents of the argument buffer

      HRESULT hres;     
      IShellLink* psl;     
      char szGotPath[MAX_PATH]; 
      char szDescription[MAX_PATH];     
      WIN32_FIND_DATA wfd;  

      // Get a pointer to the IShellLink interface. 
      //
      hres = CoCreateInstance( CLSID_ShellLink, NULL, 
            CLSCTX_INPROC_SERVER, IID_IShellLink, (void**)&psl ); 

      if ( SUCCEEDED(hres) ) 
      {         
         IPersistFile* ppf;  

         // Get a pointer to the IPersistFile interface. 
         //
         hres = psl->QueryInterface( IID_IPersistFile, (void**)&ppf );
         if (SUCCEEDED(hres) ) 
         { 
            // Ensure that the string is Unicode. 
            //
            WORD wsz[MAX_PATH];  
            MultiByteToWideChar( CP_ACP, 0, file_name, -1, wsz, MAX_PATH );
            
            // Load the shortcut. 
            //
            hres = ppf->Load( wsz, STGM_READ ); 
            if (SUCCEEDED(hres)) 
            {                  
               // Resolve the link. 
               //
               hres = psl->Resolve( 0 /*desktop*/, SLR_ANY_MATCH); 
               if (SUCCEEDED(hres)) 
               {  
                  // Get the path to the link target. 
                  //
                    hres = psl->GetPath( szGotPath, MAX_PATH, 
                       (WIN32_FIND_DATA *)&wfd, SLGP_UNCPRIORITY ); 

                    if ( !SUCCEEDED(hres) )
                    {
                        //TODO: handle
                    }

                    // Get the description of the target. 
                    //
                    hres = psl->GetDescription( szDescription, MAX_PATH ); 

                    if ( !SUCCEEDED(hres) ) 
                    {
                        //TODO: handle
                    }

                    lstrcpy( file_name, szGotPath);
               } 
            
            }

            // Release the pointer to the IPersistFile interface. 
            //
            ppf->Release();

         }

         // Release the pointer to the IShellLink interface. 
         //
         psl->Release();     

      }

      return true;

   }

   return false;

}

// =---------------------------------------------------------------------------
// Strip_Character
//
// ALGORITHM:
//    Characters are stripped in-place
//    1. We find first occurrence of the character to strip in the string
//    2. We pull the rest of the string one character left, effectively erasing 
//       the first found strip character
//    3. We repeat this
// 
void Strip_Character ( char* str, char strip )
{
   char* c = str;
   while ( 0 != (c=strchr(c,strip)) )
   {
      char* k = c;
      while ( *k != 0 )
      {
         *k = *(k+1);
         k++;
      }
   }
}

/****************************************************************************
 *                                                                          *
 *  FUNCTION   : DibBlt( HDC hdc,                                           *
 *                       int x0, int y0,                                    *
 *                       int dx, int dy,                                    *
 *                       HANDLE hdib,                                       *
 *                       int x1, int y1,                                    *
 *                       LONG rop)                                          *
 *                                                                          *
 *  PURPOSE    : Draws a bitmap in CF_DIB format, using SetDIBits to device.*
 *               taking the same parameters as BitBlt().                    *
 *                                                                          *
 *  RETURNS    : TRUE  - if function succeeds.                              *
 *               FALSE - otherwise.                                         *
 *                                                                          *
 ****************************************************************************/
bool DibBlt ( HDC hdc, sint_32 x0, sint_32 y0, sint_32 dx, sint_32 dy, 
               HANDLE hdib, sint_32 x1, sint_32 y1, sint_32 rop )
{
    LPBITMAPINFOHEADER   lpbi;
    LPSTR                pBuf;

    if (!hdib)
        return PatBlt(hdc,x0,y0,dx,dy,rop) != 0;

    lpbi = (LPBITMAPINFOHEADER)GlobalLock(hdib);

    if (!lpbi)
        return false;

    pBuf = (LPSTR)lpbi + (WORD)lpbi->biSize + 256*sizeof(RGBQUAD);
    SetDIBitsToDevice (hdc, x0, y0, dx, dy,
                       x1,y1,
                       x1,
                       dy,
                       pBuf, (LPBITMAPINFO)lpbi,
                       DIB_RGB_COLORS );

    GlobalUnlock(hdib);
    return true;
}
