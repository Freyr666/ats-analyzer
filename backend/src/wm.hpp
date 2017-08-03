#ifndef WM_H
#define WM_H

#include <vector>
#include <string>
#include <gstreamermm.h>
#include <map>

#include "chatterer.hpp"
#include "wm_widget.hpp"
#include "wm_window.hpp"
#include "wm_treeview.hpp"
#include "position.hpp"
#include "metadata.hpp"
#include "pad.hpp"

namespace Ats {

    class Wm : public Chatterer, public Logger {

        typedef std::pair<uint,uint> resolution_t;

    public:
        Wm() : Chatterer ("WM") {}
        Wm(Wm&&) = delete;
        Wm(const Wm&) = delete;
        virtual ~Wm() {}

        void reset();
        void add_to_pipe (const Glib::RefPtr<Gst::Bin>);
        void plug (std::shared_ptr<Pad>); // plug source
        void plug (Glib::RefPtr<Gst::Pad>); // plug sink

        // Chatterer
        string to_string() const;
        json   serialize() const;
        void   deserialize(const json&);

        shared_ptr<Wm_window>       find_window (const std::string uid);
        const shared_ptr<Wm_window> find_window (const std::string uid) const;
        shared_ptr<Wm_widget>       find_widget (const std::string uid);
        const shared_ptr<Wm_widget> find_widget (const std::string uid) const;

    private:
        pair<uint,uint> _resolution = make_pair(1920, 1080);
        std::map<std::string,std::shared_ptr<Wm_window> > _windows;
        std::map<std::string,std::shared_ptr<Wm_widget> > _widgets;
        Wm_treeview                _treeview;
        Glib::RefPtr<Gst::Bin>     _bin;
        Glib::RefPtr<Gst::Element> _background;
        Glib::RefPtr<Gst::Pad>     _background_pad;
        Glib::RefPtr<Gst::Element> _mixer;

        void   on_remove_window (std::string);

        void   set_resolution(const resolution_t);
        void   apply_resolution();
    };

    template<class T>
    void to_json(json& j, const pair<std::string,shared_ptr<T>> p) { j = {p.first,p.second}; }
    void to_json(json& j, const shared_ptr<const Wm_window>);
    void to_json(json& j, const shared_ptr<const Wm_widget>);
    void to_json(json& j, const shared_ptr<const Wm_container>);
    void to_json(json& j, const Wm_position&);
}

#endif /* WM_H */
