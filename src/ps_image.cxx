//
// "$Id: image.cxx 4324 2005-05-09 21:47:22Z rokan $"
//
// Postscript image drawing implementation for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2015 by Bill Spitzak and others.
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file.  If this
// file is missing or damaged, see the license at:
//
//     http://www.fltk.org/COPYING.php
//
// Please report all bugs and problems on the following page:
//
//     http://www.fltk.org/str.php
//

#ifndef FL_DOXYGEN

#include <stdio.h>
#include <math.h>
#include <string.h>

#include <FL/Fl_PostScript.H>
#include <FL/Fl.H>
#include <FL/Fl_Pixmap.H>
#include <FL/Fl_Bitmap.H>


//
// Implementation of the /ASCII85Encode PostScript filter
// as described in "PostScript LANGUAGE REFERENCE third edition" p. 131
//
struct struct85 {
  uchar bytes4[4]; // holds up to 4 input bytes
  int l4;          // # of unencoded input bytes
  int blocks;      // counter to insert newlines after 80 output characters
  uchar chars5[5]; // holds 5 output characters
};


void *Fl_PostScript_Graphics_Driver::prepare85() // prepare to produce ASCII85-encoded output
{
  struct85 *big = new struct85;
  big->l4 = 0;
  big->blocks = 0;
  return big;
}

// ASCII85-encodes 4 input bytes from bytes4 into chars5 array
// returns # of output chars
static int convert85(const uchar *bytes4, uchar *chars5)
{
  if (bytes4[0] == 0 && bytes4[1] == 0 && bytes4[2] == 0 && bytes4[3] == 0) {
    chars5[0] = 'z';
    return 1;
  }
  unsigned val = bytes4[0]*(256*256*256) + bytes4[1]*(256*256) + bytes4[2]*256 + bytes4[3];
  chars5[0] = val / 52200625 + 33; // 52200625 = 85 to the 4th
  val = val % 52200625;
  chars5[1] = val / 614125 + 33;   // 614125 = 85 cube
  val = val % 614125;
  chars5[2] = val / 7225 + 33;     // 7225 = 85 squared
  val = val % 7225;
  chars5[3] = val / 85 + 33;
  chars5[4] = val % 85 + 33;
  return 5;
}


void Fl_PostScript_Graphics_Driver::write85(void *data, const uchar *p, int len) // sends len input bytes for ASCII85 encoding
{
  struct85 *big = (struct85 *)data;
  const uchar *last = p + len;
  while (p < last) {
    int c = 4 - big->l4;
    if (last-p < c) c = last-p;
    memcpy(big->bytes4 + big->l4, p, c);
    p += c;
    big->l4 += c;
    if (big->l4 == 4) {
      c = convert85(big->bytes4, big->chars5);
      fwrite(big->chars5, c, 1, output);
      big->l4 = 0;
      if (++big->blocks >= 16) { fputc('\n', output); big->blocks = 0; }
    }
  }
}


void Fl_PostScript_Graphics_Driver::close85(void *data)  // stops ASCII85-encoding after processing remaining unencoded input bytes, if any
{
  struct85 *big = (struct85 *)data;
  int l;
  if (big->l4) { // # of remaining unencoded input bytes
    l = big->l4;
    while (l < 4) big->bytes4[l++] = 0; // complete them with 0s
    l = convert85(big->bytes4, big->chars5); // encode them
    if (l == 1) memset(big->chars5, '!', 5);
    fwrite(big->chars5, big->l4 + 1, 1, output);
  }
  fputs("~>", output); // write EOD mark
  delete big;
}

//
// End of implementation of the /ASCII85Encode PostScript filter
//

//
// Implementation of the /RunLengthEncode + /ASCII85Encode PostScript filter
// as described in "PostScript LANGUAGE REFERENCE third edition" p. 142
//

struct struct_rle85 {
  struct85 *data85;  // aux data for ASCII85 encoding
  uchar buffer[128]; // holds non-run data
  int count;  // current buffer length
  int run_length; // current length of run
};

void *Fl_PostScript_Graphics_Driver::prepare_rle85() // prepare to produce RLE+ASCII85-encoded output
{
  struct_rle85 *rle = new struct_rle85;
  rle->count = 0;
  rle->run_length = 0;
  rle->data85 = (struct85*)prepare85();
  return rle;
}


void Fl_PostScript_Graphics_Driver::write_rle85(uchar b, void *data) // sends one input byte to RLE+ASCII85 encoding
{
  struct_rle85 *rle = (struct_rle85 *)data;
  uchar c;
  if (rle->run_length > 0) { // if within a run
    if (b == rle->buffer[0] &&  rle->run_length < 128) { // the run can be extended
      rle->run_length++;
      return;
    } else { // output the run
      c = (uchar)(257 - rle->run_length);
      write85(rle->data85, &c, 1); // the run-length info
      write85(rle->data85, rle->buffer, 1); // the byte of the run
      rle->run_length = 0;
    }
  }
  if (rle->count >= 2 && b == rle->buffer[rle->count-1] && b == rle->buffer[rle->count-2]) {
    // about to begin a run
    if (rle->count > 2) { // there is non-run data before the run in the buffer
      c = (uchar)(rle->count-2 - 1);
      write85(rle->data85, &c, 1); // length of non-run data
      write85(rle->data85, rle->buffer, rle->count-2); // non-run data
    }
    rle->run_length = 3;
    rle->buffer[0] = b;
    rle->count = 0;
    return;
  }
  if (rle->count >= 128) { // the non-run buffer is full, output it
    c = (uchar)(rle->count - 1);
    write85(rle->data85, &c, 1); // length of non-run data
    write85(rle->data85, rle->buffer, rle->count); // non-run data
    rle->count = 0;
  }
  rle->buffer[rle->count++] = b; // add byte to end of non-run buffer
}


void Fl_PostScript_Graphics_Driver::close_rle85(void *data) // stop doing RLE+ASCII85 encoding
{
  struct_rle85 *rle = (struct_rle85 *)data;
  uchar c;
  if (rle->run_length > 0) { // if within a run, output it
    c = (uchar)(257 - rle->run_length);
    write85(rle->data85, &c, 1);
    write85(rle->data85, rle->buffer, 1);
  } else if (rle->count) { // output the non-run buffer, if not empty
    c = (uchar)(rle->count - 1);
    write85(rle->data85, &c, 1);
    write85(rle->data85, rle->buffer, rle->count);
  }
  c = (uchar)128;
  write85(rle->data85, &c, 1); // output EOD mark
  close85(rle->data85); // close ASCII85 encoding process
  delete rle;
}

//
// End of implementation of the /RunLengthEncode + /ASCII85Encode PostScript filter
//

 
int Fl_PostScript_Graphics_Driver::alpha_mask(const uchar * data, int w, int h, int D, int LD){

  mask = 0;
  if ((D/2)*2 != D){ //no mask info
    return 0;
  }
  int xx;
  int i,j, k, l;
  LD += w*D;
  int V255=0;
  int V0 =0;
  int V_=0;
  for (j=0;j<h;j++){
    for (i=0;i<w;i++)
      switch(data[j*LD+D*i+D-1]){
        case 255: V255 = 1; break;
        case 0: V0 = 1; break;
        default: V_= 1;
      }
    if (V_) break;
  };
  if (!V_){
    if (V0)
      if (V255){// not true alpha, only masking
        xx = (w+7)/8;
        mask = new uchar[h * xx];
        for (i=0;i<h * xx;i++) mask[i]=0;
        for (j=0;j<h;j++)
          for (i=0;i<w;i++)
            if (data[j*LD+D*i+D-1])
              mask[j*xx+i/8] |= 1 << (i % 8);
        mx = w;
        my = h; //mask imensions
        return 0;
      } else {
        mask=0;
        return 1; //everything masked
      }
    else
      return 0;
  }



  /////   Alpha dither, generating (4*w) * 4 mask area       /////
  /////         with Floyd-Steinberg error diffusion         /////

  mask = new uchar[((w+1)/2) * h * 4];

  for (i = 0; i<((w+1)/2) * h * 4; i++) mask[i] = 0; //cleaning



  mx= w*4;
  my=h*4; // mask dimensions

  xx = (w+1)/2;                //  mask line width in bytes

  short * errors1 = new short [w*4+2]; //  two rows of dither errors
  short * errors2 = new short [w*4+2]; //  two rows of dither errors

  for (i=0; i<w*4+2; i++) errors2[i] = 0; // cleaning,after first swap will become current
  for (i=0; i<w*4+2; i++) errors1[i] = 0; // cleaning,after first swap will become current

  short * current = errors1;
  short * next = errors2;
  short * swap;

  for (j=0; j<h; j++){
    for (l=0; l<4; ){           // generating 4 rows of mask lines for 1 RGB line
      int jj = j*4+l;

      /// mask row index
      swap = next;
      next = current;
      current = swap;
      *(next+1) = 0;          // must clean the first cell, next are overriden by *1
      for (i=0; i<w; i++){
        for (k=0; k<4; k++){   // generating 4 x-pixels for 1 RGB
          short error, o1, o2, o3;
          int ii = i*4+k;   // mask cell index
          short val = data[j*LD+D*i+D-1] + current[1+ii];
          if (val>127){
            mask[jj*xx+ii/8]  |= 1 << (ii % 8); //set mask bit
            error =  val-255;
          }else
            error = val;

          ////// error spreading /////
          if (error >0){
            next[ii] +=  o1 = (error * 3 + 8)/16;
            current[ii+2] += o2 = (error * 7 + 8)/16;
            next[ii+2] = o3 =(error + 8)/16;  // *1 - ok replacing (cleaning)
          } else {
            next[ii] += o1 = (error * 3 - 8)/16;
            current[ii+2] += o2 = (error * 7 - 8)/16;
            next[ii+2] = o3 = (error - 8)/16;
          }
          next[1+ii] += error - o1 - o2 - o3;
        }
      }
      l++;

      ////// backward

      jj = j*4+l;
      swap = next;
      next = current;
      current = swap;
      *(next+1) = 0;          // must clean the first cell, next are overriden by *1

      for (i = w-1; i >= 0; i--){

        for (k=3; k>=0; k--){   // generating 4 x-pixels for 1 RGB
          short error, o1, o2, o3;

          int ii = i*4+k;   // mask cell index
          short val = data[j*LD+D*i+D-1] + current[1+ii];
          if (val>127){

            mask[jj*xx+ii/8]  |= 1 << (ii % 8); //set mask bit
            error =  val-255;
          } else
            error = val;

          ////// error spreading /////
          if (error >0){
            next[ii+2] +=  o1 = (error * 3 + 8)/16;
            current[ii] += o2 = (error * 7 + 8)/16;
            next[ii] = o3 =(error + 8)/16;  // *1 - ok replacing (cleaning)
          } else {
            next[ii+2] += o1 = (error * 3 - 8)/16;

            current[ii] += o2 = (error * 7 - 8)/16;
            next[ii] = o3 = (error - 8)/16;
          }
          next[1+ii] += error - o1 - o2 - o3;
        }
      }
      l++;
    }
  }
  delete[] errors1;
  delete[] errors2;
  return 0;
}

// bitwise inversion of all 4-bit quantities
static const unsigned char swapped[16] = {0,8,4,12,2,10,6,14,1,9,5,13,3,11,7,15};

// bitwise inversion of a byte
static inline uchar swap_byte(const uchar b) {
  return (swapped[b & 0xF] << 4) | swapped[b >> 4];
}


extern uchar **fl_mask_bitmap;

struct callback_data {
  const uchar *data;
  int D, LD;
};


static void draw_image_cb(void *data, int x, int y, int w, uchar *buf) {
  struct callback_data *cb_data;
  const uchar *curdata;

  cb_data = (struct callback_data*)data;
  curdata = cb_data->data + x*cb_data->D + y*cb_data->LD;

  memcpy(buf, curdata, w*cb_data->D);
}


void Fl_PostScript_Graphics_Driver::draw_image(const uchar *data, int ix, int iy, int iw, int ih, int D, int LD) {
  if (D<3){ //mono
    draw_image_mono(data, ix, iy, iw, ih, D, LD);
    return;
  }

  struct callback_data cb_data;

  if (!LD) LD = iw*D;

  cb_data.data = data;
  cb_data.D = D;
  cb_data.LD = LD;

  draw_image(draw_image_cb, &cb_data, ix, iy, iw, ih, D);
}

void Fl_PostScript_Graphics_Driver::draw_image(Fl_Draw_Image_Cb call, void *data, int ix, int iy, int iw, int ih, int D) {
  double x = ix, y = iy, w = iw, h = ih;

  int level2_mask = 0;
  fprintf(output,"save\n");
  int i,j,k;
  const char * interpol;
  if (lang_level_ > 1) {
    if (interpolate_) interpol="true";
    else interpol="false";
    if (mask && lang_level_ > 2) {
      fprintf(output, "%g %g %g %g %i %i %i %i %s CIM\n", x , y+h , w , -h , iw , ih, mx, my, interpol);
    }
    else if (mask && lang_level_ == 2) {
      level2_mask = 1; // use method for drawing masked color image with PostScript level 2
      fprintf(output, " %g %g %g %g %d %d pixmap_plot\n", x, y, w, h, iw, ih);
    }
    else {
      fprintf(output, "%g %g %g %g %i %i %s CII\n", x , y+h , w , -h , iw , ih, interpol);
    }
  } else {
    fprintf(output , "%g %g %g %g %i %i CI", x , y+h , w , -h , iw , ih);
  }
  
  int LD=iw*D;
  uchar *rgbdata=new uchar[LD];
  uchar *curmask=mask;
  void *big = prepare_rle85();
  
  if (level2_mask) {
    for (j = ih - 1; j >= 0; j--) { // output full image data
      call(data, 0, j, iw, rgbdata);
      uchar *curdata = rgbdata;
      for (i=0 ; i<iw ; i++) {
        write_rle85(curdata[0], big); write_rle85(curdata[1], big); write_rle85(curdata[2], big);
        curdata += D;
      }
    }
    close_rle85(big); fputc('\n', output);
    big = prepare_rle85();
    for (j = ih - 1; j >= 0; j--) { // output mask data
      curmask = mask + j * (my/ih) * ((mx+7)/8);
      for (k=0; k < my/ih; k++) {
        for (i=0; i < ((mx+7)/8); i++) {
          write_rle85(swap_byte(*curmask), big);
          curmask++;
        }
      }
    }
  }
  else {
    for (j=0; j<ih;j++) {
      if (mask && lang_level_ > 2) {  // InterleaveType 2 mask data
        for (k=0; k<my/ih;k++) { //for alpha pseudo-masking
          for (i=0; i<((mx+7)/8);i++) {
            write_rle85(swap_byte(*curmask), big);
            curmask++;
          }
        }
      }
      call(data,0,j,iw,rgbdata);
      uchar *curdata=rgbdata;
      for (i=0 ; i<iw ; i++) {
        uchar r = curdata[0];
        uchar g =  curdata[1];
        uchar b =  curdata[2];
        
        if (lang_level_<3 && D>3) { //can do  mixing using bg_* colors)
          unsigned int a2 = curdata[3]; //must be int
          unsigned int a = 255-a2;
          r = (a2 * r + bg_r * a)/255;
          g = (a2 * g + bg_g * a)/255;
          b = (a2 * b + bg_b * a)/255;
        }
        
        write_rle85(r, big); write_rle85(g, big); write_rle85(b, big);
        curdata +=D;
      }
      
    }
  }
  close_rle85(big);
  fprintf(output,"\nrestore\n");
  delete[] rgbdata;
}

void Fl_PostScript_Graphics_Driver::draw_image_mono(const uchar *data, int ix, int iy, int iw, int ih, int D, int LD) {
  double x = ix, y = iy, w = iw, h = ih;

  fprintf(output,"save\n");

  int i,j, k;

  const char * interpol;
  if (lang_level_>1){
    if (interpolate_)
      interpol="true";
    else
      interpol="false";
    if (mask && lang_level_>2)
      fprintf(output, "%g %g %g %g %i %i %i %i %s GIM\n", x , y+h , w , -h , iw , ih, mx, my, interpol);
    else
      fprintf(output, "%g %g %g %g %i %i %s GII\n", x , y+h , w , -h , iw , ih, interpol);
  }else
    fprintf(output , "%g %g %g %g %i %i GI", x , y+h , w , -h , iw , ih);


  if (!LD) LD = iw*D;


  int bg = (bg_r + bg_g + bg_b)/3;

  uchar *curmask=mask;
  void *big = prepare_rle85();
  for (j=0; j<ih;j++){
    if (mask){
      for (k=0;k<my/ih;k++){
        for (i=0; i<((mx+7)/8);i++){
          write_rle85(swap_byte(*curmask), big);
          curmask++;
        }
      }
    }
    const uchar *curdata=data+j*LD;
    for (i=0 ; i<iw ; i++) {
      uchar r = curdata[0];
      if (lang_level_<3 && D>1) { //can do  mixing

        unsigned int a2 = curdata[1]; //must be int
        unsigned int a = 255-a2;
        r = (a2 * r + bg * a)/255;
      }
      write_rle85(r, big);
      curdata +=D;
    }

  }
  close_rle85(big);
  fprintf(output,"restore\n");
}



void Fl_PostScript_Graphics_Driver::draw_image_mono(Fl_Draw_Image_Cb call, void *data, int ix, int iy, int iw, int ih, int D) {
  double x = ix, y = iy, w = iw, h = ih;

  fprintf(output,"save\n");
  int i,j,k;
  const char * interpol;
  if (lang_level_>1){
    if (interpolate_) interpol="true";
    else interpol="false";
    if (mask && lang_level_>2)
      fprintf(output, "%g %g %g %g %i %i %i %i %s GIM\n", x , y+h , w , -h , iw , ih, mx, my, interpol);
    else
      fprintf(output, "%g %g %g %g %i %i %s GII\n", x , y+h , w , -h , iw , ih, interpol);
  } else
    fprintf(output , "%g %g %g %g %i %i GI", x , y+h , w , -h , iw , ih);

  int LD=iw*D;
  uchar *rgbdata=new uchar[LD];
  uchar *curmask=mask;
  void *big = prepare_rle85();
  for (j=0; j<ih;j++){

    if (mask && lang_level_>2){  // InterleaveType 2 mask data
      for (k=0; k<my/ih;k++){ //for alpha pseudo-masking
        for (i=0; i<((mx+7)/8);i++){
          write_rle85(swap_byte(*curmask), big);
          curmask++;
        }
      }
    }
    call(data,0,j,iw,rgbdata);
    uchar *curdata=rgbdata;
    for (i=0 ; i<iw ; i++) {
      write_rle85(curdata[0], big);
      curdata +=D;
    }
  }
  close_rle85(big);
  fprintf(output,"restore\n");
  delete[] rgbdata;
}


////////////////////////////// Image classes //////////////////////


void Fl_PostScript_Graphics_Driver::draw(Fl_Pixmap * pxm,int XP, int YP, int WP, int HP, int cx, int cy){
  const char * const * di =pxm->data();
  int w,h;
  if (!fl_measure_pixmap(di, w, h)) return;
  mask=0;
  fl_mask_bitmap=&mask;
  mx = WP;
  my = HP;
  push_clip(XP, YP, WP, HP);
  fl_draw_pixmap(di,XP -cx, YP -cy, FL_BLACK );
  pop_clip();
  delete[] mask;
  mask=0;
  fl_mask_bitmap=0;
}

void Fl_PostScript_Graphics_Driver::draw(Fl_RGB_Image * rgb,int XP, int YP, int WP, int HP, int cx, int cy){
  const uchar  * di = rgb->array;
  int w = rgb->w();
  int h = rgb->h();
  mask=0;
  if (lang_level_>2) //when not true, not making alphamask, mixing colors instead...
  if (alpha_mask(di, w, h, rgb->d(),rgb->ld())) return; //everthing masked, no need for painting!
  push_clip(XP, YP, WP, HP);
  draw_image(di, XP + cx, YP + cy, w, h, rgb->d(), rgb->ld());
  pop_clip();
  delete[]mask;
  mask=0;
}

int Fl_PostScript_Graphics_Driver::draw_scaled(Fl_Image *img, int XP, int YP, int WP, int HP){
  int X, Y, W, H;
  clip_box(XP,YP,WP,HP,X,Y,W,H); // X,Y,W,H will give the unclipped area of XP,YP,WP,HP
  if (W == 0 || H == 0) return 1;
  push_no_clip(); // remove the FLTK clip that can't be rescaled
  clocale_printf("%d %d %i %i CL\n", X, Y, W, H);
  clocale_printf("GS %d %d TR  %f %f SC GS\n", XP, YP, float(WP)/img->w(), float(HP)/img->h());
  img->draw(0, 0, img->w(), img->h(), 0, 0);
  clocale_printf("GR GR\n");
  pop_clip(); // restore FLTK's clip
  return 1;
}

void Fl_PostScript_Graphics_Driver::draw(Fl_Bitmap * bitmap,int XP, int YP, int WP, int HP, int cx, int cy){
  const uchar  * di = bitmap->array;
  int w,h;
  int LD=(bitmap->w()+7)/8;
  int xx;

  if (WP> bitmap->w() - cx){// to assure that it does not go out of bounds;
     w = bitmap->w() - cx;
     xx = (bitmap->w()+7)/8 - cx/8; //length of mask in bytes
  }else{
    w =WP;
    xx = (w+7)/8 - cx/8;
  }
  if ( HP > bitmap->h()-cy)
    h = bitmap->h() - cy;
  else
    h = HP;

  di += cy*LD + cx/8;
  int si = cx % 8; // small shift to be clipped, it is simpler than shifting whole mask

  int i,j;
  push_clip(XP, YP, WP, HP);
  fprintf(output , "%i %i %i %i %i %i MI\n", XP - si, YP + HP , WP , -HP , w , h);

  void *rle85 = prepare_rle85();
  for (j=0; j<HP; j++){
    for (i=0; i<xx; i++){
      write_rle85(swap_byte(*di), rle85);
      di++;
    }
  }
  close_rle85(rle85); fputc('\n', output);
  pop_clip();
}

#endif // FL_DOXYGEN

//
// End of "$Id: image.cxx 4324 2005-05-09 21:47:22Z rokan $"
//










  







