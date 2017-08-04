#ifndef WM_TREEVIEW_H
#define WM_TREEVIEW_H

#include <functional>
#include <map>

#include "json.hpp"
#include "wm_container.hpp"

namespace Ats {

    using json = nlohmann::json;

    class Wm_treeview_template {

        struct Wm_window_template {
            std::string           uid;
            Wm_window::Type       type;
            Wm_position           position;
            shared_ptr<Wm_window> window;
        };

        struct Wm_widget_template {
            std::string           uid;
            Wm_widget::Type       type;
            Wm_position           position;
            shared_ptr<Wm_widget> widget;
        };

        class Wm_container_template {

        public:
            Wm_container_template(std::string uid,
                                  Wm_window::Type type,
                                  Wm_position pos,
                                  shared_ptr<Wm_window> w) : _window{uid,type,pos,w} {};

            void add_widget (std::string,Wm_widget::Type,Wm_position&,shared_ptr<Wm_widget>);

            const std::vector<Wm_widget_template>& get_widgets() const { return _widgets; }
            const Wm_window_template&              get_window()  const { return _window; }

            void  validate() const;

        private:
            const Wm_window_template        _window;
            std::vector<Wm_widget_template> _widgets;
        };

    public:
        static std::unique_ptr<Wm_treeview_template> create(
            const json& j,
            const std::map<std::string,shared_ptr<Wm_window>> _windows,
            const std::map<std::string,shared_ptr<Wm_widget>> _widgets);
        const std::vector<Wm_container_template>&    get_containers() const { return _containers; }

        void validate(pair<uint,uint> res) const;

    private:
        static Wm_position parse_position    (const json&);

        void add_window (std::string, Wm_window::Type,Wm_position&, shared_ptr<Wm_window>);
        void add_widget (std::string, std::string, Wm_widget::Type, Wm_position&, shared_ptr<Wm_widget>);

        static std::string elt_not_present   (std::string, std::string);
        static std::string elt_wrong_type    (std::string, std::string, std::string);
        static std::string elt_wrong_type    (std::string, std::string, std::string, std::string);
        static std::string elt_already_added (std::string, std::string, std::string wnd_uid = "");

        Wm_treeview_template () {};
        std::vector<Wm_container_template> _containers;
    };

    class Wm_treeview {
    public:
        Wm_treeview () {}
        Wm_treeview (Wm_treeview&) = delete;
        Wm_treeview (const Wm_treeview&&) = delete;

        void reset();
        void reset_from_template(Wm_treeview_template*);
        void add_window (std::string,shared_ptr<Wm_window>);
        void add_widget (std::string,std::string, shared_ptr<Wm_widget>);
        void remove_window (std::string);
        void remove_widget (std::string, std::string);

        void validate (std::pair<uint,uint>);

        void for_each (std::function<void(const std::string&,Wm_container&)>&);
        void for_each (std::function<void(const std::string&,const Wm_container&)>&) const;

        // Wm_container*       find_container (std::string uid);
        const Wm_container* find_container (std::string uid) const;

    private:
        std::map<std::string,std::shared_ptr<Wm_container>> _containers;
    };
};

#endif /* WM_TREEVIEW_H */
