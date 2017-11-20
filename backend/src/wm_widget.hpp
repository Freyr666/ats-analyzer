#ifndef WM_WIDGET_H
#define WM_WIDGET_H

#include <gstreamermm.h>
#include "pad.hpp"
#include "wm_position.hpp"

namespace Ats {

    class Wm_widget {
    public:
        class Not_plugged : public std::exception {};

        virtual void add_to_pipe (const Glib::RefPtr<Gst::Bin>) = 0;
        virtual void plug(shared_ptr<Pad>) = 0; // plug source
        virtual void plug(Glib::RefPtr<Gst::Pad>) = 0; // plug sink
        virtual std::string gen_uid() = 0;
        virtual std::string get_type_string() const = 0;
        virtual bool is_enabled() const = 0;
        virtual void enable() = 0;
        virtual void disable() = 0;
        virtual void set_position(const Wm_position&) = 0;
        virtual const Wm_position& get_position() const = 0;
        sigc::signal <void > signal_unlinked() { return _unlinked; }

        virtual std::string to_string() const = 0;
        
    protected:
        sigc::signal <void > _unlinked;
    };
    
}

#endif /* WM_WIDGET_H */
