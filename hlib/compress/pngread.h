/*
 * pngread.h
 * 
 * Header containing a PNG image file reader. 
 * 
 * Copyright (c) 2003--2004 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. (See COPYING.GPL for details.)
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#ifndef _HLIB_PNGReader_H_
#define _HLIB_PNGReader_H_ 1

#include <hlib/prototypes.h>

#if HAVE_PNG_H
#  include <png.h>
#endif

/*
This information can be stored in an PNG file. Hope I got them all. 
png_get_IHDR
png_get_bKGD
png_get_cHRM
png_get_cHRM_fixed
png_get_gAMA
png_get_gAMA_fixed
png_get_hIST
png_get_iCCP
png_get_oFFs
png_get_pCAL
png_get_pHYs
png_get_PLTE
png_get_sBIT
png_get_sCAL
png_get_sPLT
png_get_sRGB
png_get_text
png_get_tIME
png_get_tRNS
*/

class PNGReader
{
	public:
		enum  // flags for SetBasicBitOps(): 
		{
			PNGR_Palette2RGB=    0x0001,
			PNGR_GrayExpand=     0x0002,
			PNGR_AddAlphaIfAvail=0x0004,
			PNGR_Strip16=        0x0008,
			PNGR_StripAlpha=     0x0010,
			PNGR_InvertAlpha=    0x0020,
			PNGR_Unpack124=      0x0040,
			PNGR_ColorBGR=       0x0080,
			PNGR_SwapAlpha=      0x0100,
			PNGR_Gray2RBG=       0x0200,
			PNGR_InvertGray=     0x0400,
			PNGR_Swap16=         0x0800,
			PNGR_PackSwap=       0x1000,
		};
		
	private:
		FILE *fp;
		
		png_structp png_ptr;
		png_infop info_ptr;
		//png_infop end_info;
		
		enum ReadState
		{
			RS_None=0,        // no file opened
			RS_Opened,        // opened and header read
			RS_Reading,       // reading file (after StartReading())
			RS_Done           // reading done; info at end read
		} rstate;
		
		// This is only valid in state RS_Reading. 
		int interlacing_passes;   // 1 -> no interlacing. 
		
		// Current error string: 
		const char *curr_error;
		
		static void error_func(png_structp,png_const_charp str);
		static void warning_func(png_structp,png_const_charp str);
	public:  _CPP_OPERATORS_FF
		PNGReader(int * /*failflag*/=NULL);
		~PNGReader();
		
		// You have to call the functions in a special order. 
		// There are several states (rstate); the state advances 
		// while the file is opened, read (hdr and body), and 
		// closed. During a state, functions which are available 
		// in that state can be called. A state is marked by 
		// the ----<STATE>---- comment. 
		// You may call close() and the destructor at any time. 
		
		// Common return values: 
		//   0 -> OK
		//  -3 -> libpng error
		//        Use GetError() to query the last error. 
		//  -4 -> cannot be called now
		
		const char *GetError() const
			{  return(curr_error);  }
		
		//-------------------------------<None>------------------------------
		
		// First, you have to open the file which you want to read: 
		// This function opens the file and reads the header. 
		// Additional return values: 
		//  -1 -> alloc failure
		//  -2 -> see errno
		// -10 -> not a PNG file (wrong signature/magic)
		int Open(const char *name);
		
		//------------------------------<Opened>------------------------------
		
		// Get file descriptor of opened file (e.g to use fstat() on it, 
		// etc.) Handle with care. 
		int fileno() const
			{  return(fp ? ::fileno(fp) : -4);  }
		
		// (Can also be called in reading state.)
		// You probably first want to query the most basic image 
		// properties like size and color type: 
		// color_type: describes which color/alpha channels are present. 
		//    PNG_COLOR_TYPE_GRAY        (bit depths 1, 2, 4, 8, 16)
		//    PNG_COLOR_TYPE_GRAY_ALPHA  (bit depths 8, 16)
		//    PNG_COLOR_TYPE_PALETTE     (bit depths 1, 2, 4, 8)
		//    PNG_COLOR_TYPE_RGB         (bit_depths 8, 16)
		//    PNG_COLOR_TYPE_RGB_ALPHA   (bit_depths 8, 16)
		//    Masks: 
		//      PNG_COLOR_MASK_PALETTE
		//      PNG_COLOR_MASK_COLOR
		//      PNG_COLOR_MASK_ALPHA
		// Additional return values: 
		//  -10 -> not supported image properties found
		int GetSize(unsigned int *width,unsigned int *height,
			int *bit_depth=NULL,int *color_type=NULL);
		
		// (Can also be called in reading state.)
		// Get number of channels: 
		// valid values are 
		//   1  (GRAY, PALETTE)
		//   2  (GRAY_ALPHA)
		//   3  (RGB)
		//   4  (RGB_ALPHA or RGB + filler byte))
		int GetNChannels();
		
		// (Can also be called in reading state.)
		// Get the size of an image row (in bytes): 
		// NOTE: This may change when applying transformations 
		//       and calling StartReading(). 
		// Additional return values: 
		//   none
		ssize_t GetRowSize();
		
		// (Can also be called in reading state.)
		// Get the interlace type: 
		//   PNG_INTERLACE_NONE, PNG_INTERLACE_ADAM7
		// You can use this early to reject interlaced images if 
		// your application needs to read in the image data 
		// progessively row by row. 
		int GetInterlaceType();
		
		// Get the gamma value of the image. 
		// Additional return values: 
		//   1 -> no gamma info in file; value unknown; returned 
		//        gamma value is 0.454545.. = 1/2.2
		int GetGamma(double *gamma);
		
		// You may apply the following transformations. 
		// ** The order is important to take care. It's best to **
		// ** stick to the order as represented in the docu for **
		// ** SetBasicBitOps() right below.                     **
		
		// Set various bit and byte swap stuff: 
		// It's best to set it all at the same time or keep the order 
		// in which they are listed here. There are comments when to 
		// call some of the other functions. 
		// sane: 
		//    0 -> always request the passed transformation even if it 
		//         does not make sense (e.g. PNGR_InvertMono on RGB image)
		//    1 -> silently ignore the requested flag if it does not 
		//         make sense 
		// flags: Bitwise OR of the following: 
		//   PNGR_Palette2RGB:
		//      Change paletted image to RGB. 
		//   PNGR_GrayExpand:
		//      Transforms grayscale images of less than 8 to 8 bits. 
		//   PNGR_AddAlphaIfAvail:
		//      Add a full alpha channel if there is transparency 
		//      information in a tRNS chunk (independent from 'sane'). 
		//   PNGR_Strip16:
		//      PNG can have files with 16 bits per channel. If you only 
		//      can handle 8 bits per channel, this will strip the pixels 
		//      down to 8 bit.
		//   PNGR_StripAlpha:
		//      If, for  some reason, you don't need the alpha channel 
		//      on an image, and you want to remove it rather than 
		//      combining it with the background (but the image author 
		//      certainly had in mind  that you *would* combine it with 
		//      the background, so that's what you should probably do). 
		//   PNGR_InvertAlpha:
		//      In PNG files, the alpha channel in an image is the 
		//      level of opacity. If you need the alpha channel in an 
		//      image to be the level of transparency instead of opacity, 
		//      you can invert the alpha channel (or the tRNS chunk data) 
		//      after it's read, so that 0 is fully opaque and 255 (in 
		//      8-bit or paletted images) or 65535 (in 16-bit images) is 
		//      fully transparent. 
		//   PNGR_Unpack124:
		//      PNG files pack pixels of bit depths 1, 2, and 4 into bytes 
		//      as small as they can, resulting in, for example, 8 pixels 
		//      per byte for 1 bit files. This code expands to 1 pixel per 
		//      byte without changing the values of the pixels. 
		//   PNGR_ColorBGR:
		//      PNG files store 3 color pixels in red, green, blue order. 
		//      This can be be used if you need them as blue, green, red. 
		// --<call SetInsertFiller() here>--
		//   PNGR_SwapAlpha:
		//      If you are reading an image with an alpha channel, and you 
		//      need the data as ARGB instead  of the normal PNG format RGBA. 
		//   PNGR_Gray2RBG:
		//      For some uses, you may want a grayscale image to be 
		//      represented as RGB...
		// --<call SetRBG2GrayConversion() here>--
		// --<set background (if needed) here>--
		// --<call gamma correction stuff here>--
		//   PNGR_InvertGray:
		//      PNG files describe monochrome as black being zero 
		//      and white being one. This can be used if the pixels 
		//      are supplied with this reversed (black being one and 
		//      white being zero). 
		//      This can also be used to invert grayscale and 
		//      gray-alpha images. 
		//   PNGR_Swap16:
		//      Needed for bit_depth 16: Convert to network order 
		//      (big-endian, ie. most significant bits first): 
		//   PNGR_PackSwap:
		//      If you are using packed-pixel images (1, 2, or 4 
		//      bits/pixel), and you need to change the order the 
		//      pixels are packed into bytes. 
		// Additional return values: 
		//   none
		int SetBasicBitOps(int flags,int sane);
		
		// PNG files store RGB pixels packed into 3 or 6 bytes. This 
		// expands them into 4 or 8 bytes for windowing systems that 
		// need them in this format. 
		// This transformation does not affect images that already have 
		// full alpha channels. To add an opaque alpha channel, use 
		// filler=0xff or 0xffff and location=+1 which will generate 
		// RGBA pixels.
		//   location:  -1 -> before; +1 ->after the RBG
		//   filler_value: value to fill with (
		// Additional return values: 
		//   none
		int SetInsertFiller(u_int16_t filler_value,int location=+1);
		
		// You can convert an RGB or RGBA image to grayscale or 
		// grayscale with alpha. 
		//   red_weight, green_weight: weight of component times 100000
		//     or -1 for default. 
		// Note: I've seen conversion weights 30000,59000 commonly; these, 
		//       however, differ from those used by libpng as default. 
		// Note: If the file has a gamma value set, it is respected. 
		//       If you want true 1:1 linear combination without any gamma 
		//       stuff, use SetGamma(1.0,1.0) immediately after calling 
		//       this function. 
		// Additional return values: 
		//   none
		int SetRBG2GrayConversion(int red_weight=-1,int green_weight=-1);
		
		// Do gamma correction for input data. 
		// If file_gamma is <0, the value from GetGamma() is used for it. 
		// To force getting unaltered values from the file, use 
		// SetGamma(1.0, 1.0). 
		// Additional return values: 
		//   none
		int SetGamma(double screen_gamma,double file_gamma=-1.0);
		
		// Call this to start reading after all transformations were 
		// specified. Switches states Opened -> Reading. 
		// NOTE that this may modify the stored header info and 
		//      especially the size of an image row, i.e. the value 
		//      returned by GetRowSize(). 
		//      [Unless you set do_update_header=0 which is discouraged.] 
		// Additional return values: 
		//   none
		int StartReading(int do_update_header=1);
		
		//------------------------------<Reading>------------------------------
		
		// Read one more image row from the file. 
		// Additional return values: 
		//   -10 -> image is interlaced; must use ReadImage(). 
		int ReadRow(char *row_buf);
		
		// Read several image rows at once. 
		// Additional return values: 
		//   -10 -> image is interlaced and nrows!=image_height
		//          (interlaced images must be read in completely in 
		//          one step)
		int ReadImage(char **row_pointers,unsigned int nrows);
		
		// Read image tail to get info attached after the image data 
		// such as text comments. Should be called after having read 
		// data. Switches states Reading -> Done. 
		// NOTE: Will change the info entries. 
		// Additional return values: 
		//   none
		int ReadTail();
		
		//-------------------------------<Done>-------------------------------
		
		// This can also be called when in Opened mode. 
		// Note that text can be attached before and after the image data. 
		// array returns pointer to text structure array of size nchunks. 
		// The png_text struct contains the following important members: 
		//   char *key:   NUL-terminated key, 1..79 chars
		//   char *text:  actual text and...
		//   text_length: ...its length
		// WHO THE HELL HAS TO FREE THE DATA? (Assume libpng takes care of it.)
		// Additional return values: 
		//   none
		int GetTextChunksRaw(png_text **array,int *nchunks);
		
		// Close file. Can be called at any time. 
		// Return value: 
		//   0 -> OK
		//  -3 -> libpng error
		int Close();
};

#endif  /* _HLIB_PNGReader_H_ */
