/*
 * pngwrite.h
 * 
 * Header containing a PNG image file writer. 
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

#ifndef _HLIB_PNGWriter_H_
#define _HLIB_PNGWriter_H_ 1

#include <hlib/prototypes.h>

#if HAVE_PNG_H
#  include <png.h>
#endif


class PNGWriter
{
	public:
		enum  // flags for SetBasicBitOps(): 
		{
			PNGW_Swap16=     0x01,
			PNGW_InvertAlpha=0x02,
			PNGW_ColorBGR=   0x04,
			PNGW_PackSwap=   0x08,
			PMGW_InvertMono= 0x10,
		};
		
	private:
		FILE *fp;
		
		png_structp png_ptr;
		png_infop info_ptr;
		
		// PNG Text ptrs to be written next time: 
		int n_text_chunks;
		png_text *text_array;
		
		enum WriteState
		{
			WS_None=0,        // no file opened
			WS_Opened,        // fresh opened
			WS_HdrWritten,    // header has been written
			WS_TailWritten    // tail has been written
		} wstate;
		
		// Current error string: 
		const char *curr_error;
		
		static void error_func(png_structp,png_const_charp str);
		static void warning_func(png_structp,png_const_charp str);
		
		void _ClearTextChunks();
	public:  _CPP_OPERATORS_FF
		PNGWriter(int * /*failflag*/=NULL);
		~PNGWriter();
		
		// You have to call the functions in a special order. 
		// There are several states (wstate); the state advances 
		// while the file is opened, written, finished off and 
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
		
		//------------------------------<None>------------------------------
		
		// First, you have to open the file which you want to write: 
		// You can also supply a compression level. 
		// Additional return values: 
		//  -1 -> alloc failure
		//  -2 -> see errno
		int Open(const char *name,int compress_level=-999);
		
		//---------------------------<Opened>---------------------------
		
		// Get file descriptor of opened file (e.g to use fstat() on it, 
		// etc.) Handle with care. 
		int fileno() const
			{  return(fp ? ::fileno(fp) : -4);  }
		
		// Now, first set the basic info: 
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
		//   none
		int SetSize(unsigned int width,unsigned int height,
			int bit_depth,int color_type=PNG_COLOR_TYPE_RGB);
		
		// Turn on or off filtering, and/or choose specific filters. 
		// You can use either a single PNG_FILTER_VALUE_NAME or the 
		// bitwise OR of one or more PNG_FILTER_NAME masks: 
		//   PNG_FILTER_NONE  | PNG_FILTER_VALUE_NONE  |
		//   PNG_FILTER_SUB   | PNG_FILTER_VALUE_SUB   |
		//   PNG_FILTER_UP    | PNG_FILTER_VALUE_UP    |
		//   PNG_FILTER_AVG   | PNG_FILTER_VALUE_AVG   |
		//   PNG_FILTER_PAETH | PNG_FILTER_VALUE_PAETH |
		//   PNG_ALL_FILTERS
		int SetFilter(int filter);
		
		// Store gamma value of image. 
		// If not specified, no gamma entry will be saved. 
		int SetGamma(double gamma);
		
		// When called in Opened state, the text chunks are added at the 
		// beginning before the actual image data; if called in HdrWritten 
		// state, the text is added after the image data by WriteTail(). 
		//  key:  keyword for comment; must contain 1-79 characters. 
		//  text: text comment, can be NULL or empty
		//  text_len: length of above text; strlen() is used if <0. 
		//  do_compress: 1 -> compress text; 0 -> do not; 
		//              -1 -> compress if longer than 512 bytes
		// Note: All strings are copied. 
		//       It is probably not worth compressing strings <1000 bytes. 
		//       Any number of text chunks may be added. 
		// Additional return values: 
		//  -1 -> alloc failure
		int AddTextChunk(const char *key, const char *text,
			ssize_t text_len=-1,int do_compress=-1);
		
		// Write the file header. This has to be done before feeding 
		// data. 
		// Additional return values: 
		//   none
		int WriteHeader();
		
		//---------------------------<HdrWritten>---------------------------
		
		// Get the size of an image row (in bytes): 
		// (Can also be called in "opened" state.) 
		// Additional return values: 
		//   none
		ssize_t GetRowSize();
		
		// Set various bit and byte swap stuff: 
		// flags: Bitwise OR of the following: 
		//   PNGW_Swap16:
		//      Needed for bit_depth 16: Convert to network order 
		//      (big-endian, ie. most significant bits first): 
		//   PNGW_InvertAlpha:
		//      In PNG files, the alpha channel in an image is the 
		//      level of opacity. If your data is supplied as a level 
		//      of transparency, you can invert the alpha channel 
		//      before you write it, so that 0 is fully transparent 
		//      and 255 (in 8-bit or paletted images) or 65535 (in 
		//      16-bit images) is fully opaque. 
		//   PNGW_ColorBGR:
		//      PNG files store 3 color pixels in red, green, blue 
		//      order. This can be be used if they are supplied 
		//      as blue, green, red. 
		//   PNGW_PackSwap:
		//      If you are using packed-pixel images (1, 2, or 4 
		//      bits/pixel), and you need to change the order the 
		//      pixels are packed into bytes. 
		//   PMGW_InvertMono:
		//      PNG files describe monochrome as black being zero 
		//      and white being one. This can be used if the pixels 
		//      are supplied with this reversed (black being one and 
		//      white being zero). 
		int SetBasicBitOps(int flags);
		
		// Feed the data into the file: 
		// Additional return values: 
		//   none
		int WriteRow(const char *row_buf,int do_flush=0);
		int WriteRows(const char **row_pointers,unsigned int nrows,
			int do_flush=0);
		
		// Before closing: finish off writing; write tail. 
		// Additional return values: 
		//   none
		int WriteTail();
		
		//---------------------------<TailWritten>--------------------------
		
		// Close file. Simply close and free; use WriteTail() 
		// to correctly terminate file before closing. 
		// Return value: 
		//   0 -> OK
		//  -3 -> libpng error
		int Close();
};

#endif  /* _HLIB_PNGWriter_H_ */
