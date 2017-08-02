#ifndef WM_WINDOW_H
#define WM_WINDOW_H

#include <gstreamermm.h>

#include "wm_element.hpp"

namespace Ats {
    
    class Wm_window : public Wm_element {
    public:
        enum class Type { Video, Background };
        // Wm_element
        virtual Type type() = 0;
    };

    class Wm_window_video : public Wm_window {
    public:
        Wm_window_video ();
        Wm_window_video (const Wm_window_video&) = delete;
        Wm_window_video (Wm_window_video&&) = delete;
        virtual ~Wm_window_video ();

        // Wm_element
        virtual void add_to_pipe (Glib::RefPtr<Gst::Bin>);
        virtual void plug(shared_ptr<Pad>); // plug source
        virtual void plug(Glib::RefPtr<Gst::Pad>); // plug sink
        virtual std::string gen_name();
        virtual bool is_enabled();
        virtual void enable();
        virtual void disable();
        virtual void set_position(const Wm_position&);
        virtual Wm_position get_position();

        uint stream()  { if (_plugged) return _stream; else throw Not_plugged {}; }
        uint channel() { if (_plugged) return _channel; else throw Not_plugged {}; }
        uint pid()     { if (_plugged) return _pid; else throw Not_plugged {}; }

        virtual Type type() { return Type::Video; }
        
    private:
        bool _plugged = false;
        uint _stream;
        uint _channel;
        uint _pid;
        bool _enabled = false;
        Wm_position _position;
        
        Glib::RefPtr<Gst::Pad> _mixer_pad;

        Glib::RefPtr<Gst::Element> _scale;
        Glib::RefPtr<Gst::Element> _caps;

        void apply_position ();
    };

}
    
#endif /* WM_WINDOW_H */
