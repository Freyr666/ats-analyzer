#ifndef OPTIONS_H
#define OPTIONS_H

#include <vector>
#include <gstreamermm.h>

#include "metadata.hpp"
#include "probe.hpp"
#include "address.hpp"

using namespace std;

namespace Ats {

  struct Options {

    struct Qoe_settings {
      /* loss */
      float vloss;
      float aloss;
      /* black frame */
      bool black_cont_en;
      float black_cont;
      bool black_peak_en;
      float black_peak;
      bool luma_cont_en;
      float luma_cont;
      bool luma_peak_en;
      float luma_peak;
      float black_time;
      int black_pixel;
      /* freeze */
      bool freeze_cont_en;
      float freeze_cont;
      bool freeze_peak_en;
      float freeze_peak;
      bool diff_cont_en;
      float diff_cont;
      bool diff_peak_en;
      float diff_peak;
      float freeze_time;
      int pixel_diff;
      /* blockiness */
      bool blocky_cont_en;
      float blocky_cont;
      bool blocky_peak_en;
      float blocky_peak;
      float blocky_time;
      bool mark_blocks;
      /* silence */
      bool silence_cont_en;
      float silence_cont;
      bool silence_peak_en;
      float silence_peak;
      float silence_time;
      /* loudness */
      bool loudness_cont_en;
      float loudness_cont;
      bool loudness_peak_en;
      float loudness_peak;
      float loudness_time;
      /* adv loudness */
      float adv_diff;
      int adv_buf;
    };

    /* prog list */
    vector<Metadata> data;

    /* qoe analysis settings */
    Qoe_settings qoe_settings;

    /* mosaic settings */
    pair<int,int> resolution;
    string ch_name_fmt;
    string ch_name_overlay;
    string ch_name_size;
    string aud_meter_pos;
    bool aud_meter_overlay;

    /* output stream settings */
    Address output_sink;

    sigc::signal<void,const Options&> updated;
    
    Options() {}
    ~Options() {}

    void   connect(Probe& p) { p.updated.connect(
	    sigc::mem_fun(this, &Options::set_data));
    }

    void   set_data(const Metadata&);
    string to_string() const;
    string to_json()   const;
    void   of_json(const string&);

    void operator=(const Metadata& m) { set_data(m); }
    void operator=(const string& js) { of_json(js); }
  };

};

#endif /* OPTIONS_H */
