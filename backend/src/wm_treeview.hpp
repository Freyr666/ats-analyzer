#ifndef WM_TREEVIEW_H
#define WM_TREEVIEW_H

#include <functional>
#include <map>

#include "json.hpp"
#include "wm_container.hpp"
#include "wm_position.hpp"

namespace Ats {

    using json = nlohmann::json;

    class Wm_treeview_template {

        struct Wm_widget_template {
            std::string           uid;
            Wm_position           position;
            uint                  layer;
            shared_ptr<Wm_widget> widget;
        };

        class Wm_container_template {

        public:
            Wm_container_template(std::string uid,
                                  Wm_position pos) : name(uid), position(pos) {};

            void add_widget (std::string,Wm_position&,uint,const shared_ptr<Wm_widget>&);

            const std::vector<Wm_widget_template>& get_widgets() const { return _widgets; }

            void  validate() const;

            const std::string name;
            const Wm_position position;

        private:
            std::vector<Wm_widget_template> _widgets;
        };

    public:
        static std::unique_ptr<Wm_treeview_template> create(
            const json& j,
            const std::map<std::string,const shared_ptr<Wm_widget>> _widgets);
        const std::vector<Wm_container_template>&    get_containers() const { return _containers; }

        void validate(pair<uint,uint> res) const;

    private:
        static Wm_position parse_position    (const json&);

        void add_container (std::string, Wm_position&);
        void add_widget    (std::string, std::string, Wm_position&, uint, const shared_ptr<Wm_widget>&);

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
        void add_container    (std::string, const shared_ptr<Wm_container>&);
        void add_widget       (std::string, std::string, const shared_ptr<Wm_widget>&);
        void remove_container (std::string);
        void remove_widget    (std::string, std::string);
        void remove_widget    (std::string);

        void for_each (std::function<void(const std::string&,const std::shared_ptr<Wm_container>&)>);
        void for_each (std::function<void(const std::string&,const std::shared_ptr<const Wm_container>&)>) const;

        // Wm_container*       find_container (std::string uid);
        const Wm_container* find_container (std::string uid) const;

    private:
        std::map<std::string,std::shared_ptr<Wm_container>> _containers;
    };
};

#endif /* WM_TREEVIEW_H */
