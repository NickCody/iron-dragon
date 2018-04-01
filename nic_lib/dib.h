// =---------------------------------------------------------------------------
//
// Declarations of functions used in dib.c module           
//                                                         
// Modified on 9/25/98 by Nick Codignotto, where appropriate
//

BOOL        WriteDIB (IStream* pStream,HANDLE hdib);
BOOL        DibInfo (HANDLE hdib,LPBITMAPINFOHEADER lpbi);
HPALETTE    CreateBIPalette (LPBITMAPINFOHEADER lpbi);
HPALETTE    CreateDibPalette (HANDLE hdib);
WORD        PaletteSize (VOID FAR * pv);
WORD        DibNumColors (VOID FAR * pv);
HANDLE      DibFromBitmap (HBITMAP hbm, DWORD biStyle, WORD biBits, HPALETTE hpal);
HBITMAP     BitmapFromDib (HANDLE hdib, HPALETTE hpal);
BOOL        DrawBitmap (HDC hdc, INT x, INT y, HBITMAP hbm, DWORD rop);
BOOL        DibBlt (HDC hdc, INT x0, INT y0, INT dx, INT dy, HANDLE hdib, INT x1, INT y1, LONG rop);
BOOL        StretchDibBlt (HDC hdc, INT x0, INT y0, INT dx, INT dy, HANDLE hdib, INT x1, INT y1, INT dx1, INT dy1, LONG rop);
VOID        WriteMapFileHeaderandConvertFromDwordAlignToPacked(IStream* pStream, LPBITMAPFILEHEADER pbf);
HANDLE      ReadDIBFile(IStream* pStream );

