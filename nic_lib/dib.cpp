/******************************************************************************\
*       This is a part of the Microsoft Source Code Samples. 
*       Copyright 1993 - 1998 Microsoft Corporation.
*       All rights reserved. 
*       This source code is only intended as a supplement to 
*       Microsoft Development Tools and/or WinHelp documentation.
*       See these sources for detailed information regarding the 
*       Microsoft samples programs.
*
*       Modified on 9/25/98 by Nick Codignotto where applicable
*
\******************************************************************************/

#include "PreComp.h"
#include "dib.h"
#include "showdib.h"


/****************************************************************************
 *                                                                          *
 *  FUNCTION   : WriteDIB(LPSTR szFile,HANDLE hdib)                         *
 *                                                                          *
 *  PURPOSE    : Write a global handle in CF_DIB format to a file.          *
 *                                                                          *
 *  RETURNS    : TRUE  - if successful.                                     *
 *               FALSE - otherwise                                          *
 *                                                                          *
 ****************************************************************************/
BOOL WriteDIB ( IStream* pStream, HANDLE hdib )
{
    BITMAPFILEHEADER    hdr;
    LPBITMAPINFOHEADER  lpbi;

    if (!hdib)
        return FALSE;


    lpbi = (LPBITMAPINFOHEADER)GlobalLock (hdib);

    // Fill in the fields of the file header
    hdr.bfType          = BFT_BITMAP;
    hdr.bfSize          = GlobalSize (hdib) + SIZEOF_BITMAPFILEHEADER_PACKED;
    hdr.bfReserved1     = 0;
    hdr.bfReserved2     = 0;
    hdr.bfOffBits       = (DWORD) (SIZEOF_BITMAPFILEHEADER_PACKED + lpbi->biSize +
                          PaletteSize(lpbi));

    WriteMapFileHeaderandConvertFromDwordAlignToPacked(pStream, &hdr);

   /* this struct already DWORD aligned!*/
   /* Write the DIB header and the bits */
   pStream->Write ( (LPSTR)lpbi, GlobalSize (hdib), 0 );

   GlobalUnlock (hdib);
   return TRUE;
}

/****************************************************************************
 *                                                                          *
 *  FUNCTION   : DibInfo(HANDLE hbi,LPBITMAPINFOHEADER lpbi)                *
 *                                                                          *
 *  PURPOSE    : Retrieves the DIB info associated with a CF_DIB            *
 *               format memory block.                                       *
 *                                                                          *
 *  RETURNS    : TRUE  - if successful.                                     *
 *               FALSE - otherwise                                          *
 *                                                                          *
 ****************************************************************************/
BOOL DibInfo ( HANDLE hbi, LPBITMAPINFOHEADER lpbi )
{
   if (hbi)
   {
      *lpbi = *(LPBITMAPINFOHEADER)GlobalLock (hbi);

      // fill in the default fields
      if (lpbi->biSize != sizeof (BITMAPCOREHEADER))
      {
         if (lpbi->biSizeImage == 0L)
         lpbi->biSizeImage = WIDTHBYTES(lpbi->biWidth*lpbi->biBitCount) * lpbi->biHeight;

         if (lpbi->biClrUsed == 0L)
         lpbi->biClrUsed = DibNumColors (lpbi);
      }

      GlobalUnlock (hbi);
      return TRUE;
   }

   return FALSE;
}

/****************************************************************************
 *                                                                          *
 *  FUNCTION   : CreateBIPalette(LPBITMAPINFOHEADER lpbi)                   *
 *                                                                          *
 *  PURPOSE    : Given a Pointer to a BITMAPINFO struct will create a       *
 *               a GDI palette object from the color table.                 *
 *                                                                          *
 *  RETURNS    : A handle to the palette.                                   *
 *                                                                          *
 ****************************************************************************/
HPALETTE CreateBIPalette ( LPBITMAPINFOHEADER lpbi )
{
    LOGPALETTE          *pPal;
    HPALETTE            hpal = NULL;
    WORD                nNumColors;
    BYTE                red;
    BYTE                green;
    BYTE                blue;
    WORD                i;
    RGBQUAD        FAR *pRgb;

    if (!lpbi)
        return NULL;

    if (lpbi->biSize != sizeof(BITMAPINFOHEADER))
        return NULL;

    /* Get a pointer to the color table and the number of colors in it */
    pRgb = (RGBQUAD FAR *)((LPSTR)lpbi + (WORD)lpbi->biSize);
    nNumColors = DibNumColors(lpbi);

    if (nNumColors){
        /* Allocate for the logical palette structure */
        pPal = (LOGPALETTE*)LocalAlloc(LPTR,sizeof(LOGPALETTE) + nNumColors * sizeof(PALETTEENTRY));
        if (!pPal)
            return NULL;

        pPal->palNumEntries = nNumColors;
        pPal->palVersion    = PALVERSION;

        /* Fill in the palette entries from the DIB color table and
         * create a logical color palette.
         */
        for (i = 0; i < nNumColors; i++){
            pPal->palPalEntry[i].peRed   = pRgb[i].rgbRed;
            pPal->palPalEntry[i].peGreen = pRgb[i].rgbGreen;
            pPal->palPalEntry[i].peBlue  = pRgb[i].rgbBlue;
            pPal->palPalEntry[i].peFlags = (BYTE)0;
        }
        hpal = CreatePalette(pPal);
        LocalFree((HANDLE)pPal);
    }
    else if (lpbi->biBitCount == 24){
        /* A 24 bitcount DIB has no color table entries so, set the number of
         * to the maximum value (256).
         */
        nNumColors = MAXPALETTE;
        pPal = (LOGPALETTE*)LocalAlloc(LPTR,sizeof(LOGPALETTE) + nNumColors * sizeof(PALETTEENTRY));
        if (!pPal)
            return NULL;

        pPal->palNumEntries = nNumColors;
        pPal->palVersion    = PALVERSION;

        red = green = blue = 0;

        /* Generate 256 (= 8*8*4) RGB combinations to fill the palette
         * entries.
         */
        for (i = 0; i < pPal->palNumEntries; i++){
            pPal->palPalEntry[i].peRed   = red;
            pPal->palPalEntry[i].peGreen = green;
            pPal->palPalEntry[i].peBlue  = blue;
            pPal->palPalEntry[i].peFlags = (BYTE)0;

            if (!(red += 32))
                if (!(green += 32))
                    blue += 64;
        }
        hpal = CreatePalette(pPal);
        LocalFree((HANDLE)pPal);
    }
    return hpal;
}

/****************************************************************************
 *                                                                          *
 *  FUNCTION   : CreateDibPalette(HANDLE hbi)                               *
 *                                                                          *
 *  PURPOSE    : Given a Global HANDLE to a BITMAPINFO Struct               *
 *               will create a GDI palette object from the color table.     *
 *               (BITMAPINFOHEADER format DIBs only)                        *
 *                                                                          *
 *  RETURNS    : A handle to the palette.                                   *
 *                                                                          *
 ****************************************************************************/
HPALETTE CreateDibPalette ( HANDLE hbi )
{
    HPALETTE hpal;

    if (!hbi)
        return NULL;
    hpal = CreateBIPalette((LPBITMAPINFOHEADER)GlobalLock(hbi));
    GlobalUnlock(hbi);
    return hpal;
}

/****************************************************************************
 *                                                                          *
 *  FUNCTION   :  PaletteSize(VOID FAR * pv)                                *
 *                                                                          *
 *  PURPOSE    :  Calculates the palette size in bytes. If the info. block  *
 *                is of the BITMAPCOREHEADER type, the number of colors is  *
 *                multiplied by 3 to give the palette size, otherwise the   *
 *                number of colors is multiplied by 4.                      *
 *                                                                          *
 *  RETURNS    :  Palette size in number of bytes.                          *
 *                                                                          *
 ****************************************************************************/
WORD PaletteSize ( VOID FAR * pv )
{
    LPBITMAPINFOHEADER lpbi;
    WORD               NumColors;

    lpbi      = (LPBITMAPINFOHEADER)pv;
    NumColors = DibNumColors(lpbi);

    if (lpbi->biSize == sizeof(BITMAPCOREHEADER))
        return (WORD)(NumColors * sizeof(RGBTRIPLE));
    else
        return (WORD)(NumColors * sizeof(RGBQUAD));
}

/****************************************************************************
 *                                                                          *
 *  FUNCTION   : DibNumColors(VOID FAR * pv)                                *
 *                                                                          *
 *  PURPOSE    : Determines the number of colors in the DIB by looking at   *
 *               the BitCount filed in the info block.                      *
 *                                                                          *
 *  RETURNS    : The number of colors in the DIB.                           *
 *                                                                          *
 ****************************************************************************/
WORD DibNumColors (VOID FAR * pv)
{
    INT                 bits;
    LPBITMAPINFOHEADER  lpbi;
    LPBITMAPCOREHEADER  lpbc;

    lpbi = ((LPBITMAPINFOHEADER)pv);
    lpbc = ((LPBITMAPCOREHEADER)pv);

    /*  With the BITMAPINFO format headers, the size of the palette
     *  is in biClrUsed, whereas in the BITMAPCORE - style headers, it
     *  is dependent on the bits per pixel ( = 2 raised to the power of
     *  bits/pixel).
     */
    if (lpbi->biSize != sizeof(BITMAPCOREHEADER)){
        if (lpbi->biClrUsed != 0)
            return (WORD)lpbi->biClrUsed;
        bits = lpbi->biBitCount;
    }
    else
        bits = lpbc->bcBitCount;

    switch (bits)
    {
        case 1:
                return 2;
        case 4:
                return 16;
        case 8:
                return 256;
        default:
                /* A 24 bitcount DIB has no color table */
                return 0;
    }
}

/****************************************************************************
 *                                                                          *
 *  FUNCTION   : DibFromBitmap()                                            *
 *                                                                          *
 *  PURPOSE    : Will create a global memory block in DIB format that       *
 *               represents the Device-dependent bitmap (DDB) passed in.    *
 *                                                                          *
 *  RETURNS    : A handle to the DIB                                        *
 *                                                                          *
 ****************************************************************************/
HANDLE DibFromBitmap ( HBITMAP hbm, DWORD biStyle, WORD biBits, HPALETTE hpal )
{
    BITMAP               bm;
    BITMAPINFOHEADER     bi;
    BITMAPINFOHEADER FAR *lpbi;
    DWORD                dwLen;
    HANDLE               hdib;
    HANDLE               h;
    HDC                  hdc;

    if (!hbm)
        return NULL;

    if (hpal == NULL)
        hpal = (HPALETTE)GetStockObject(DEFAULT_PALETTE);

    GetObject(hbm,sizeof(bm),(LPSTR)&bm);

    if (biBits == 0)
        biBits =  bm.bmPlanes * bm.bmBitsPixel;

    bi.biSize               = sizeof(BITMAPINFOHEADER);
    bi.biWidth              = bm.bmWidth;
    bi.biHeight             = bm.bmHeight;
    bi.biPlanes             = 1;
    bi.biBitCount           = biBits;
    bi.biCompression        = biStyle;
    bi.biSizeImage          = 0;
    bi.biXPelsPerMeter      = 0;
    bi.biYPelsPerMeter      = 0;
    bi.biClrUsed            = 0;
    bi.biClrImportant       = 0;

    dwLen  = bi.biSize + PaletteSize(&bi);

    hdc = GetDC(GetDesktopWindow());
    hpal = SelectPalette(hdc,hpal,FALSE);
         RealizePalette(hdc);

    hdib = GlobalAlloc(GHND,dwLen);

    if (!hdib){
        SelectPalette(hdc,hpal,FALSE);
        ReleaseDC(NULL,hdc);
        return NULL;
    }

    lpbi = (LPBITMAPINFOHEADER)GlobalLock(hdib);

    *lpbi = bi;

    /*  call GetDIBits with a NULL lpBits param, so it will calculate the
     *  biSizeImage field for us
     */
    GetDIBits(hdc, hbm, 0L, (DWORD)bi.biHeight,
        (LPBYTE)NULL, (LPBITMAPINFO)lpbi, (DWORD)DIB_RGB_COLORS);

    bi = *lpbi;
    GlobalUnlock(hdib);

    /* If the driver did not fill in the biSizeImage field, make one up */
    if (bi.biSizeImage == 0){
        bi.biSizeImage = WIDTHBYTES((DWORD)bm.bmWidth * biBits) * bm.bmHeight;

        if (biStyle != BI_RGB)
            bi.biSizeImage = (bi.biSizeImage * 3) / 2;
    }

    /*  realloc the buffer big enough to hold all the bits */
    dwLen = bi.biSize + PaletteSize(&bi) + bi.biSizeImage;
    if (h = GlobalReAlloc(hdib,dwLen,0))
        hdib = h;
    else{
        GlobalFree(hdib);
        hdib = NULL;

        SelectPalette(hdc,hpal,FALSE);
        ReleaseDC(NULL,hdc);
        return hdib;
    }

    /*  call GetDIBits with a NON-NULL lpBits param, and actualy get the
     *  bits this time
     */
    lpbi = (LPBITMAPINFOHEADER)GlobalLock(hdib);

    uint_08* p_bits = (((uint_08*)lpbi) + sizeof(BITMAPINFOHEADER) + 256*sizeof(RGBQUAD));

    if (GetDIBits( hdc,
                   hbm,
                   0L,
                   (DWORD)bi.biHeight,
                   (LPBYTE)lpbi + (WORD)lpbi->biSize + PaletteSize(lpbi),
                   (LPBITMAPINFO)lpbi, (DWORD)DIB_RGB_COLORS) == 0)
    {
         GlobalUnlock(hdib);
         hdib = NULL;
         SelectPalette(hdc,hpal,FALSE);
         ReleaseDC(NULL,hdc);
         return NULL;
    }

    bi = *lpbi;
    GlobalUnlock(hdib);

    SelectPalette(hdc,hpal,FALSE);
    ReleaseDC(NULL,hdc);
    return hdib;
}

/****************************************************************************
 *                                                                          *
 *  FUNCTION   : BitmapFromDib(HANDLE hdib, HPALETTE hpal)                  *
 *                                                                          *
 *  PURPOSE    : Will create a DDB (Device Dependent Bitmap) given a global *
 *               handle to a memory block in CF_DIB format                  *
 *                                                                          *
 *  RETURNS    : A handle to the DDB.                                       *
 *                                                                          *
 ****************************************************************************/
HBITMAP BitmapFromDib ( HANDLE hdib, HPALETTE hpal )
{
    LPBITMAPINFOHEADER  lpbi;
    HPALETTE            hpalT;
    HDC                 hdc;
    HBITMAP             hbm;

    //StartWait();

    if (!hdib)
        return NULL;

    lpbi = (LPBITMAPINFOHEADER)GlobalLock(hdib);

    if (!lpbi)
        return NULL;

    hdc = GetDC(GetDesktopWindow());

    if (hpal){
        hpalT = SelectPalette(hdc,hpal,FALSE);
        RealizePalette(hdc);     // GDI Bug...????
    }

    hbm = CreateDIBitmap(hdc,
                (LPBITMAPINFOHEADER)lpbi,
                (LONG)CBM_INIT,
                (LPSTR)lpbi + lpbi->biSize + PaletteSize(lpbi),
                (LPBITMAPINFO)lpbi,
                DIB_RGB_COLORS );

    if (hpal)
        SelectPalette(hdc,hpalT,FALSE);

    ReleaseDC(NULL,hdc);
    GlobalUnlock(hdib);

    //EndWait();

    return hbm;
}
/****************************************************************************
 *                                                                          *
 *  FUNCTION   : DrawBitmap(HDC hdc, int x, int y, HBITMAP hbm, DWORD rop)  *
 *                                                                          *
 *  PURPOSE    : Draws bitmap <hbm> at the specifed position in DC <hdc>    *
 *                                                                          *
 *  RETURNS    : Return value of BitBlt()                                   *
 *                                                                          *
 ****************************************************************************/
BOOL DrawBitmap ( HDC hdc, INT x, INT y, HBITMAP hbm, DWORD rop )
{
    HDC       hdcBits;
    BITMAP    bm;
    BOOL      f;

    if (!hdc || !hbm)
        return FALSE;

    hdcBits = CreateCompatibleDC(hdc);
    GetObject(hbm,sizeof(BITMAP),(LPSTR)&bm);
    SelectObject(hdcBits,hbm);
    f = BitBlt(hdc,0,0,bm.bmWidth,bm.bmHeight,hdcBits,0,0,rop);
    DeleteDC(hdcBits);

    return f;
        UNREFERENCED_PARAMETER(y);
        UNREFERENCED_PARAMETER(x);
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
BOOL DibBlt ( HDC hdc, INT x0, INT y0, INT dx, INT dy, HANDLE hdib, 
               INT x1,INT y1, LONG rop )
{
    LPBITMAPINFOHEADER   lpbi;
    LPSTR                pBuf;

    if (!hdib)
        return PatBlt(hdc,x0,y0,dx,dy,rop);

    lpbi = (LPBITMAPINFOHEADER)GlobalLock(hdib);

    if (!lpbi)
        return FALSE;

    pBuf = (LPSTR)lpbi + (WORD)lpbi->biSize + PaletteSize(lpbi);
    SetDIBitsToDevice (hdc, x0, y0, dx, dy,
                       x1,y1,
                       x1,
                       dy,
                       pBuf, (LPBITMAPINFO)lpbi,
                       DIB_RGB_COLORS );

    GlobalUnlock(hdib);
    return TRUE;
}
/****************************************************************************
 *                                                                          *
 *  FUNCTION   : StretchDibBlt( HDC hdc,                                    *
 *                              int x, int y,                               *
 *                              int dx, int dy,                             *
 *                              HANDLE hdib,                                *
 *                              int x0, int y0,                             *
 *                              int dx0, int dy0,                           *
 *                              LONG rop)                                   *
 *                                                                          *
 *  PURPOSE    : Draws a bitmap in CF_DIB format, using StretchDIBits()     *
 *               taking the same parameters as StretchBlt().                *
 *                                                                          *
 *  RETURNS    : TRUE  - if function succeeds.                              *
 *               FALSE - otherwise.                                         *
 *                                                                          *
 ****************************************************************************/
BOOL StretchDibBlt ( HDC hdc, INT x, INT y, INT dx, INT dy, HANDLE hdib, 
                    INT x0, INT y0, INT dx0, INT dy0, LONG rop )
{
    LPBITMAPINFOHEADER lpbi;
    LPSTR        pBuf;
    BOOL         f;

    if (!hdib)
        return PatBlt(hdc,x,y,dx,dy,rop);

    lpbi = (LPBITMAPINFOHEADER)GlobalLock(hdib);

    if (!lpbi)
        return FALSE;

    pBuf = (LPSTR)lpbi + (WORD)lpbi->biSize + PaletteSize(lpbi);

    f = StretchDIBits ( hdc,
                        x, y,
                        dx, dy,
                        x0, y0,
                        dx0, dy0,
                        pBuf, (LPBITMAPINFO)lpbi,
                        DIB_RGB_COLORS,
                        rop);

    GlobalUnlock(hdib);
    return f;
}


/****************************************************************************
 *                                                                          *
 *  FUNCTION   : WriteMapFileHeaderandConvertFromDwordAlignToPacked(HFILE fh, LPBITMAPFILEHEADER pbf)
 *                                                                          *
 *  PURPOSE    : write header structure (which NOT packed) and write it PACKED
 *                                                                          *
 *  RETURNS    : VOID
 *                                                                          *
 ****************************************************************************/

VOID WriteMapFileHeaderandConvertFromDwordAlignToPacked (
   IStream* pStream, LPBITMAPFILEHEADER pbf )
{

   /* write bfType*/
   pStream->Write ( (LPSTR)&pbf->bfType, (UINT)sizeof (WORD), 0 );

   /* now pass over extra word, and only write next 3 DWORDS!*/
   pStream->Write ( (LPSTR)&pbf->bfSize, sizeof(DWORD) * 3, 0 );
}


/*************************************************************************
 *
 * Function:  ReadDIBFile (int)
 *
 *  Purpose:  Reads in the specified DIB file into a global chunk of
 *            memory.
 *
 *  Returns:  A handle to a dib (hDIB) if successful.
 *            NULL if an error occurs.
 *
 * Comments:  BITMAPFILEHEADER is stripped off of the DIB.  Everything
 *            from the end of the BITMAPFILEHEADER structure on is
 *            returned in the global memory handle.
 *
 *
 * NOTE: The DIB API were not written to handle OS/2 DIBs, so this
 * function will reject any file that is not a Windows DIB.
 *
 *************************************************************************/

HANDLE ReadDIBFile(IStream* pStream )
{
    BITMAPFILEHEADER    bmfHeader;
    UINT                nNumColors;   // Number of colors in table
    HANDLE              hDIB;        
    HANDLE              hDIBtmp;      // Used for GlobalRealloc() //MPB
    LPBITMAPINFOHEADER  lpbi;
    DWORD               offBits;
    DWORD               dwRead;

    // get length of DIB in bytes for use when reading

    // Allocate memory for header & color table. We'll enlarge this
    // memory as needed.

    hDIB = GlobalAlloc(GMEM_MOVEABLE, (DWORD)(sizeof(BITMAPINFOHEADER) +
            256 * sizeof(RGBQUAD)));

    if (!hDIB)
        return NULL;

    lpbi = (LPBITMAPINFOHEADER)GlobalLock(hDIB);

    if (!lpbi) 
    {
        GlobalFree(hDIB);
        return NULL;
    }

    // read the BITMAPFILEHEADER from our file

    //if (!ReadFile(hFile, (LPSTR)&bmfHeader, sizeof (BITMAPFILEHEADER),
    //        &dwRead, NULL))
    //    goto ErrExit;
    pStream->Read( (LPSTR)&bmfHeader, sizeof (BITMAPFILEHEADER), &dwRead );

    if (sizeof (BITMAPFILEHEADER) != dwRead)
        goto ErrExit;

    if (bmfHeader.bfType != 0x4d42)  // 'BM'
        goto ErrExit;

    // read the BITMAPINFOHEADER

    //if (!ReadFile(hFile, (LPSTR)lpbi, sizeof(BITMAPINFOHEADER), &dwRead,
    //        NULL))
    //    goto ErrExit;
    pStream->Read( (LPSTR)lpbi, sizeof(BITMAPINFOHEADER), &dwRead );

    if (sizeof(BITMAPINFOHEADER) != dwRead)
        goto ErrExit;

    // Check to see that it's a Windows DIB -- an OS/2 DIB would cause
    // strange problems with the rest of the DIB API since the fields
    // in the header are different and the color table entries are
    // smaller.
    //
    // If it's not a Windows DIB (e.g. if biSize is wrong), return NULL.

    if (lpbi->biSize == sizeof(BITMAPCOREHEADER))
        goto ErrExit;

    // Now determine the size of the color table and read it.  Since the
    // bitmap bits are offset in the file by bfOffBits, we need to do some
    // special processing here to make sure the bits directly follow
    // the color table (because that's the format we are susposed to pass
    // back)

    if (!(nNumColors = (UINT)lpbi->biClrUsed))
    {
        // no color table for 24-bit, default size otherwise

        if (lpbi->biBitCount != 24)
            nNumColors = 1 << lpbi->biBitCount; // standard size table
    }

    // fill in some default values if they are zero

    if (lpbi->biClrUsed == 0)
        lpbi->biClrUsed = nNumColors;

    if (lpbi->biSizeImage == 0)
    {
        lpbi->biSizeImage = ((((lpbi->biWidth * (DWORD)lpbi->biBitCount) +
                31) & ~31) >> 3) * lpbi->biHeight;
    }

    // get a proper-sized buffer for header, color table and bits

    GlobalUnlock(hDIB);
    hDIBtmp = GlobalReAlloc(hDIB, lpbi->biSize + nNumColors *
            sizeof(RGBQUAD) + lpbi->biSizeImage, 0);

    if (!hDIBtmp) // can't resize buffer for loading
        goto ErrExitNoUnlock; //MPB
    else
        hDIB = hDIBtmp;

    lpbi = (LPBITMAPINFOHEADER)GlobalLock(hDIB);

    // read the color table

    //ReadFile (hFile, (LPSTR)(lpbi) + lpbi->biSize,
    //        nNumColors * sizeof(RGBQUAD), &dwRead, NULL);
    pStream->Read( (LPSTR)(lpbi) + lpbi->biSize, nNumColors * sizeof(RGBQUAD), &dwRead );

    // offset to the bits from start of DIB header

    offBits = lpbi->biSize + nNumColors * sizeof(RGBQUAD);

    // If the bfOffBits field is non-zero, then the bits might *not* be
    // directly following the color table in the file.  Use the value in
    // bfOffBits to seek the bits.

    if (bmfHeader.bfOffBits != 0L)
    {

        //SetFilePointer(hFile, bmfHeader.bfOffBits, NULL, FILE_BEGIN);
    }

    //if (ReadFile(hFile, (LPSTR)lpbi + offBits, lpbi->biSizeImage, &dwRead,
    //        NULL))
    //    goto OKExit;
    pStream->Read( (LPSTR)lpbi + offBits, lpbi->biSizeImage, &dwRead );
    goto OKExit;


ErrExit:
    GlobalUnlock(hDIB);    

ErrExitNoUnlock:    
    GlobalFree(hDIB);
    return NULL;

OKExit:
    GlobalUnlock(hDIB);
    return hDIB;
}
