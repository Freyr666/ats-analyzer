#ifndef WM_TREEVIEW_H
#define WM_TREEVIEW_H

#include "wm_container.hpp"

#include <map>

namespace Ats {

    class Wm_treeview {
    public:
        Wm_treeview () {}
        Wm_treeview (Wm_treeview&) = delete;
        Wm_treeview (const Wm_treeview&&) = delete;
	
        void add_window (shared_ptr<Wm_window>);
        void add_widget (std::string, shared_ptr<Wm_widget>);
        void remove_window (std::string);
        void remove_widget (std::string, std::string);

        json serialize ();
    private:
        std::map<std::string,std::shared_ptr<Wm_container>> _containers;
    };
};

#endif /* WM_TREEVIEW_H */
