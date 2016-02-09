#ifndef ANALYSIS_H
#define ANALYSIS_H

#include "videoanalysis_api.h"
#include <stdlib.h>

//#define ALG

inline VideoParams
analyse_buffer(guint8* data,
	       guint8* data_prev,
	       guint stride,
	       guint width,
	       guint height,
	       guint black_bnd,
	       guint freez_bnd)
{
  VideoParams rval;
  rval.avg_bright = .0;
  rval.avg_diff = .0;
  rval.blocks = .0;
  rval.black_pix = .0;
  rval.frozen_pix = .0;

  #ifdef ALG

  float Shblock = 0;
  float Shnonblock = 0;
  for (guint i = 1; i < width/4; i++) {
    for (guint j = 0; j < height; j++) {
      float sum, sub, subNext, subPrev;
      int ind = i*4 + j*stride;
      sub =     (float)abs(data[ind - 1] - data[ind]);
      subNext = (float)abs(data[ind] - data[ind + 1]); 
      subPrev = (float)abs(data[ind - 2] - data[ind - 1]);
      sum = subNext + subPrev;
      if (sum == 0) sum = 1; 
      else sum = sum / 2;
      if (i%2) {
	Shnonblock += sub;
	Shblock += sum;
      }
      else {
	Shnonblock += sum;
	Shblock += sub;
      }
    }
  }
  if (!Shnonblock) Shnonblock = 4;
  rval.blocks = Shnonblock/Shblock;

  long brightness = 0;
  long difference = 0;
  guint black = 0;
  guint frozen = 0;
  for (int i = 0; i < width; i++) {
    for (int j = 0; j < height; j++) {
      guint8 current = data[i + j*stride];

      brightness += current;
      black += (current <= black_bnd) ? 1 : 0;

      if(data_prev != NULL){
	guint8 diff;
	guint8 current_prev = data_prev[i + j*stride];
	diff = abs(current - current_prev);
	difference += abs(current - current_prev);
	frozen += (diff <= freez_bnd) ? 1 : 0;
	data_prev[i + j*stride] = current;
      }
    }
  }
  rval.avg_bright = (float)brightness / (height*width);
  rval.black_pix = (black/(height*width))*100.0;
  rval.avg_diff = (float)difference / (height*width);
  rval.frozen_pix = (frozen/(height*width))*100.0;

  #else
  
  long brightness = 0;
  long difference = 0;
  guint black = 0;
  guint frozen = 0;
  float Shblock = 0;
  float Shnonblock = 0;
  
  for (guint j = 0; j < height; j++) {
    for (guint i = 1; i < width; i++) {
      int ind = i + j*stride;
      guint8 current = data[ind];
      guint8 diff;
      if (!(i % 4)) {
	float sum, sub, subNext, subPrev;
	int ind = i + j*stride;
	sub = (float)abs(data[ind - 1] - current);
	subNext = (float)abs(current - data[ind + 1]); 
	subPrev = (float)abs(data[ind - 2] - data[ind - 1]);
	sum = subNext + subPrev;
	if (sum == 0) sum = 1;
	else sum = sum / 2; 
	if (!(i % 8)){
	  Shnonblock += sub;
	  Shblock += sum;
	}
	else {
	  Shblock += sub;
	  Shnonblock += sum;
	}
      }
      brightness += current;
      black += (current <= black_bnd) ? 1 : 0;
      if (data_prev != NULL){
	guint8 current_prev = data_prev[ind];
	diff = abs(current - current_prev);
	difference += diff;
	frozen += (diff <= freez_bnd) ? 1 : 0;
	data_prev[ind] = current;
      }
    }
  }
  if (!Shnonblock) Shnonblock = 4;
  
  rval.blocks = Shnonblock/Shblock;
  rval.avg_bright = (float)brightness / (height*width);
  rval.black_pix = (black/(height*width))*100.0;
  rval.avg_diff = (float)difference / (height*width);
  rval.frozen_pix = (frozen/(height*width))*100.0;
  #endif // ALG
  return rval;
}

#endif /* ANALYSIS_H */
