#ifndef WM_WIDGET_H
#define WM_WIDGET_H

#include <gstreamermm.h>
#include <numeric>
#include "pad.hpp"
#include "wm_position.hpp"
//#include <gst/gst.h>
#include <gst/video/video.h>

namespace Ats {

    class Wm_widget {
    public:
        class Not_plugged : public std::exception {};

        std::pair<uint,uint> aspect;

        virtual void add_to_pipe (const Glib::RefPtr<Gst::Bin>) = 0;
        virtual void plug(shared_ptr<Pad>) = 0; // plug source
        virtual void plug(Glib::RefPtr<Gst::Pad>) = 0; // plug sink
        virtual std::string gen_uid() = 0;
        virtual std::string get_type_string() const = 0;
        virtual bool is_enabled() const = 0;
        virtual void enable() = 0;
        virtual void disable() = 0;
        virtual std::string description () const = 0;
        virtual void set_position(const Wm_position&) = 0;
        virtual const Wm_position& get_position() const = 0;
        virtual void set_layer(const uint) = 0;
        virtual uint get_layer() const = 0;
        sigc::signal <void > signal_unlinked() { return _unlinked; }
        sigc::signal <void > signal_linked() { return _linked; }

        virtual std::string to_string() const = 0;

        void retrieve_aspect (const Glib::RefPtr<Gst::Pad> p) {
            if (! p) return;
            
            auto vi = gst_video_info_new();
            auto pcaps = p->get_current_caps();

            if (! gst_video_info_from_caps(vi, pcaps->gobj())) {
                gst_video_info_free(vi);
                return;
            }

            auto x    = vi->width * vi->par_n;
            auto y    = vi->height * vi->par_d;
            auto gcd  = std::gcd(x, y);

            aspect = std::make_pair(x/gcd, y/gcd);
                                                                               
            gst_video_info_free(vi);
            _linked.emit();
        }
        
    protected:
        sigc::signal <void > _unlinked;
        sigc::signal <void > _linked;
    };
    
}

#endif /* WM_WIDGET_H */
