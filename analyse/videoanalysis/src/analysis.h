#ifndef ANALYSIS_H
#define ANALYSIS_H

#include "videoanalysis_api.h"

VideoParams analyse_buffer(guint8* data, guint stride, guint width, guint height, guint black_bnd);

VideoParams analyse_buffer_with_prev(guint8* data, guint8* data_prev, guint stride, guint width, guint height, guint black_bnd, guint freez_bnd);

#endif /* ANALYSIS_H */
