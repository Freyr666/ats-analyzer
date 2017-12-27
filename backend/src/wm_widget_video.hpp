#ifndef WM_WIDGET_VIDEO_H
#define WM_WIDGET_VIDEO_H

#include "wm_widget.hpp"

namespace Ats {

    class Wm_widget_video : public Wm_widget {
    public:
        Wm_widget_video ();
        Wm_widget_video (const Wm_widget_video&) = delete;
        Wm_widget_video (Wm_widget_video&&) = delete;
        virtual ~Wm_widget_video ();

        // Wm_element
        virtual void add_to_pipe (const Glib::RefPtr<Gst::Bin>);
        virtual void plug(shared_ptr<Pad>); // plug source
        virtual void plug(Glib::RefPtr<Gst::Pad>); // plug sink
        virtual std::string gen_uid();
        virtual bool is_enabled() const;
        virtual void enable();
        virtual void disable();
        virtual std::string description () const { return "video widget"; }
        virtual void set_position(const Wm_position&);
        virtual const Wm_position& get_position() const;
        virtual void set_layer(const uint);
        virtual uint get_layer() const;

        std::pair<uint,uint> get_aspect() const;

        uint stream()  const { if (_plugged) return _stream; else throw Not_plugged {}; }
        uint channel() const { if (_plugged) return _channel; else throw Not_plugged {}; }
        uint pid()     const { if (_plugged) return _pid; else throw Not_plugged {}; }

        virtual std::string get_type_string() const { return "video"; }
        std::string to_string() const;
        
    private:
        bool _plugged = false;
        uint _stream;
        uint _channel;
        uint _pid;
        bool _enabled = false;
        Wm_position _position;
        
        Glib::RefPtr<Gst::Pad> _mixer_pad;
        Glib::RefPtr<Gst::Pad> _input_pad;
        Glib::RefPtr<Gst::Element> _scale;
        Glib::RefPtr<Gst::Element> _caps;

        void apply_position ();
    };
    
}

#endif /* WM_WIDGET_VIDEO_H */
