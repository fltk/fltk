/*
 * "$Id:  $"
 *
 * Unicode to UTF-8 conversion functions.
 *
 *      Copyright (c) 2000,2001 by O'ksi'D.
 *                      All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *      Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *
 *      Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *
 *      Neither the name of O'ksi'D nor the names of its contributors
 *      may be used to endorse or promote products derived from this software
 *      without specific prior written permission.
 *
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER 
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *
 *  Author: Jean-Marc Lienher ( http://oksid.ch )
 */

#if !defined(WIN32) && !defined(__APPLE__)

#include "../../FL/Xutf8.h"

/*** NOTE : all functions are LIMITED to 24 bits Unicode values !!! ***/

/* 
 * Converts the first char of the UTF-8 string to an Unicode value 
 * Returns the byte length of the converted UTF-8 char 
 * Returns -1 if the UTF-8 string is not valid 
 */
int
XConvertUtf8ToUcs(
        const unsigned char     *buf,
        int                     len,
        unsigned int          	*ucs)
{
      if (buf[0] & 0x80) {
	if (buf[0] & 0x40) {
	  if (buf[0] & 0x20) {
	    if (buf[0] & 0x10) {
	      if (buf[0] & 0x08) {
		if (buf[0] & 0x04) {
		  if (buf[0] & 0x02) {
			/* bad UTF-8 string */
		  } else {
			/* 0x04000000 - 0x7FFFFFFF */
		  }	
		} else if (len > 4 
				&& (buf[1] & 0xC0) == 0x80
				&& (buf[2] & 0xC0) == 0x80
				&& (buf[3] & 0xC0) == 0x80
				&& (buf[4] & 0xC0) == 0x80) 
		{
		  /* 0x00200000 - 0x03FFFFFF */
                  *ucs =  ((buf[0] & ~0xF8) << 24) +
                          ((buf[1] & ~0x80) << 18) +
                          ((buf[2] & ~0x80) << 12) +
                          ((buf[3] & ~0x80) << 6) +
                           (buf[4] & ~0x80);
		  if (*ucs > 0x001FFFFF && *ucs < 0x01000000) return 5;
		}
              } else if (len > 3 
				&& (buf[1] & 0xC0) == 0x80
				&& (buf[2] & 0xC0) == 0x80
				&& (buf[3] & 0xC0) == 0x80) 
	      {
		/* 0x00010000 - 0x001FFFFF */
                *ucs =  ((buf[0] & ~0xF0) << 18) +
                        ((buf[1] & ~0x80) << 12) +
                        ((buf[2] & ~0x80) << 6) +
                         (buf[3] & ~0x80);
	        if (*ucs > 0x0000FFFF) return 4;
              }
	    } else if (len > 2 && 
			(buf[1] & 0xC0) == 0x80 && 
			(buf[2] & 0xC0) == 0x80) 
	    {
	      /* 0x00000800 - 0x0000FFFF */
              *ucs =  ((buf[0] & ~0xE0) << 12) +
               	      ((buf[1] & ~0x80) << 6) +
                       (buf[2] & ~0x80);
              if (*ucs > 0x000007FF) return 3;
	    }	
	  } else if (len > 1 && (buf[1] & 0xC0) == 0x80) {
	    /* 0x00000080 - 0x000007FF */
	    *ucs = ((buf[0] & ~0xC0) << 6) +
		    (buf[1] & ~0x80);
	    if (*ucs > 0x0000007F) return 2;
	  }
	}
      } else if (len > 0) {
	/* 0x00000000 - 0x0000007F */
	*ucs = buf[0];
	return 1;
      } 

      *ucs = (unsigned int) '?'; /* bad utf-8 string */
      return -1;
}

/* 
 * Converts an Unicode value to an UTF-8 string 
 * NOTE : the buffer (buf) must be at least 5 bytes long !!!  
 */
int 
XConvertUcsToUtf8(
	unsigned int 	ucs, 
	char 		*buf)
{
	if (ucs < 0x000080) {
		buf[0] = ucs;
		return 1;
	} else if (ucs < 0x000800) {
		buf[0] = 0xC0 | (ucs >> 6);
		buf[1] = 0x80 | (ucs & 0x3F);
		return 2;
	} else if (ucs < 0x010000) { 
		buf[0] = 0xE0 | (ucs >> 12);
		buf[1] = 0x80 | ((ucs >> 6) & 0x3F);
		buf[2] = 0x80 | (ucs & 0x3F);
		return 3;
	} else if (ucs < 0x00200000) {
		buf[0] = 0xF0 | (ucs >> 18);
		buf[1] = 0x80 | ((ucs >> 12) & 0x3F);
		buf[2] = 0x80 | ((ucs >> 6) & 0x3F);
		buf[3] = 0x80 | (ucs & 0x3F);
		return 4;
	} else if (ucs < 0x01000000) {
		buf[0] = 0xF8 | (ucs >> 24);
		buf[1] = 0x80 | ((ucs >> 18) & 0x3F);
		buf[2] = 0x80 | ((ucs >> 12) & 0x3F);
		buf[3] = 0x80 | ((ucs >> 6) & 0x3F);
		buf[4] = 0x80 | (ucs & 0x3F);
		return 5;
	}
	buf[0] = '?';
	return -1;
}

/* 
 * returns the byte length of the first UTF-8 char 
 * (returns -1 if not valid) 
 */
int
XUtf8CharByteLen(
        const unsigned char     *buf,
        int                     len)
{
	unsigned int ucs;
	return XConvertUtf8ToUcs(buf, len, &ucs);
}

/*
 * returns the quantity of Unicode chars in the UTF-8 string 
 */
int 
XCountUtf8Char(
	const unsigned char 	*buf, 
	int 			len)
{
	int i = 0;
	int nbc = 0;
	while (i < len) {
		int cl = XUtf8CharByteLen(buf + i, len - i);
		if (cl < 1) cl = 1;
		nbc++;
		i += cl;
	}
	return nbc;
}

/* 
 * Same as XConvertUtf8ToUcs but no sanity check is done.
 */
int
XFastConvertUtf8ToUcs(
        const unsigned char     *buf,
        int                     len,
        unsigned int          	*ucs)
{
      if (buf[0] & 0x80) {
	if (buf[0] & 0x40) {
	  if (buf[0] & 0x20) {
	    if (buf[0] & 0x10) {
	      if (buf[0] & 0x08) {
		if (buf[0] & 0x04) {
		  if (buf[0] & 0x02) {
			/* bad UTF-8 string */
		  } else {
			/* 0x04000000 - 0x7FFFFFFF */
		  }	
		} else if (len > 4) {
		  /* 0x00200000 - 0x03FFFFFF */
                  *ucs =  ((buf[0] & ~0xF8) << 24) +
                          ((buf[1] & ~0x80) << 18) +
                          ((buf[2] & ~0x80) << 12) +
                          ((buf[3] & ~0x80) << 6) +
                           (buf[4] & ~0x80);
		  return 5;
		}
              } else if (len > 3) {
		/* 0x00010000 - 0x001FFFFF */
                *ucs =  ((buf[0] & ~0xF0) << 18) +
                        ((buf[1] & ~0x80) << 12) +
                        ((buf[2] & ~0x80) << 6) +
                         (buf[3] & ~0x80);
	        return 4;
              }
	    } else if (len > 2) {
	      /* 0x00000800 - 0x0000FFFF */
              *ucs =  ((buf[0] & ~0xE0) << 12) +
               	      ((buf[1] & ~0x80) << 6) +
                       (buf[2] & ~0x80);
              return 3;
	    }	
	  } else if (len > 1) {
	    /* 0x00000080 - 0x000007FF */
	    *ucs = ((buf[0] & ~0xC0) << 6) +
		    (buf[1] & ~0x80);
	    return 2;
	  }
	}
      } else if (len > 0) {
	/* 0x00000000 - 0x0000007F */
	*ucs = buf[0];
	return 1;
      } 

      *ucs = (unsigned int) '?'; /* bad utf-8 string */
      return -1;
}

#endif // X11 only

/*
 * End of "$Id: $".
 */

