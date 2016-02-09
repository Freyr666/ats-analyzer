#include "analysis.h"
#include <stdlib.h>
#include <stdio.h>

inline VideoParams
analyse_buffer(guint8* data,
	       guint stride,
	       guint width,
	       guint height,
	       guint black_bnd)
{
  VideoParams rval;
  rval.avg_bright = .0;
  rval.avg_diff = .0;
  rval.blocks = .0;
  rval.black_pix = .0;
  rval.frozen_pix = .0;

  long brightness = 0;
  guint black = 0;
  float Shblock = 0;
  float Shnonblock = 0;
  for (guint i = 0; i < height*stride; i++) {
    guint rowpos = (i%stride);
    guint8 current = data[i];
    if (rowpos > width)
      continue;
    if (!(rowpos % 4))
      if (i > 1){
	float sum, sub, subNext, subPrev;
	sub = (float)abs(data[i - 1] - current);
	subNext = (float)abs(current - data[i + 1]); 
	subPrev = (float)abs(data[i - 2] - data[i - 1]);
	sum = subNext + subPrev;
	if (sum == 0) sum = 1;
	else sum = sum / 2; 
	if (!(rowpos % 8)){
	  Shnonblock += sub;
	  Shblock += sum;
	}
	else {
	  Shblock += sub;
	  Shnonblock += sum;
	}
      }
    brightness += current;
    black += (current < black_bnd) ? 1 : 0;
  }
  if (!Shnonblock) Shnonblock = 4;
  rval.blocks = Shnonblock/Shblock;
  rval.avg_bright = (float)brightness / (height*width);
  rval.black_pix = (black/(height*width))*100.0;
  return rval;
}

inline VideoParams
analyse_buffer_with_prev(guint8* data,
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

  long brightness = 0;
  long difference = 0;
  guint black = 0;
  guint frozen = 0;
  float Shblock = 0;
  float Shnonblock = 0;

  for (guint i = 0; i < height*stride; i++) {
    guint rowpos = (i%stride);
    if (rowpos > width)
      continue;
    guint8 current = data[i];
    guint8 current_prev = data_prev[i];
    guint8 diff;
    if (!(rowpos % 4))
      if (i > 1){
	float sum, sub, subNext, subPrev;
	sub = (float)abs(data[i - 1] - current);
	subNext = (float)abs(current - data[i + 1]); 
	subPrev = (float)abs(data[i - 2] - data[i - 1]);
	sum = subNext + subPrev;
	if (sum == 0) sum = 1;
	else sum = sum / 2; 
	if (!(rowpos % 8)){
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
    diff = abs(current - current_prev);
    difference += diff;
    frozen += (diff <= freez_bnd) ? 1 : 0;
  }
  if (!Shnonblock) Shnonblock = 4;
  
  rval.blocks = Shnonblock/Shblock;
  rval.avg_bright = (float)brightness / (height*width);
  rval.black_pix = (black/(height*width))*100.0;
  rval.avg_diff = (float)difference / (height*width);
  rval.frozen_pix = (frozen/(height*width))*100.0;
  return rval;
}
