#ifndef WM_H
#define WM_H

#include <vector>
#include <string>
#include <gstreamermm.h>
#include <map>

#include "chatterer.hpp"
#include "wm_widget.hpp"
#include "wm_widget_video.hpp"
#include "wm_treeview.hpp"
#include "metadata.hpp"
#include "pad.hpp"

namespace Ats {

    class Wm : public Chatterer, public Logger {

        typedef std::pair<uint,uint> resolution_t;

    public:
        Wm() : Chatterer ("wm") {}
        Wm(Wm&&) = delete;
        Wm(const Wm&) = delete;
        virtual ~Wm() = default;

        void reset();
        void add_to_pipe (const Glib::RefPtr<Gst::Bin>);
        void plug (std::shared_ptr<Pad>); // plug source
        void plug (Glib::RefPtr<Gst::Pad>); // plug sink

        // Chatterer
        string to_string() const;
        json   serialize() const;
        void   deserialize(const json&);

    private:
        pair<uint,uint> _resolution = make_pair(1280, 720);
        std::map<std::string,std::shared_ptr<Wm_widget> > _widgets;
        Wm_treeview                _treeview;
        Glib::RefPtr<Gst::Bin>     _bin;
        Glib::RefPtr<Gst::Element> _background;
        Glib::RefPtr<Gst::Pad>     _background_pad;
        Glib::RefPtr<Gst::Element> _mixer;

        void   on_remove_widget (std::string);

        void   set_resolution(const resolution_t);
        void   apply_resolution();
    };

    template<class T>
    void to_json(json& j, const pair<std::string,shared_ptr<T>> p) { j = {p.first,p.second}; }
    void to_json(json& j, const shared_ptr<const Wm_widget>);
    void to_json(json& j, const Wm_container&);
    void to_json(json& j, const Wm_position&);
}

#endif /* WM_H */
