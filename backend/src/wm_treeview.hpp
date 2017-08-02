#ifndef WM_TREEVIEW_H
#define WM_TREEVIEW_H

#include "wm_container.hpp"

#include <functional>
#include <map>

namespace Ats {

    class Wm_treeview {
    public:
        class Window_not_found  : public std::exception {};
        class Widget_not_found  : public std::exception {};
        class Window_wrong_type : public std::exception {};
        class Widget_wrong_type : public std::exception {};

        Wm_treeview () {}
        // Wm_treeview (const std::map<std::string,std::shared_ptr<Wm_window>>& _windows,
        //              const std::map<std::string,std::shared_ptr<Wm_widget>>& _widgets,
        //              json& j);
        Wm_treeview (Wm_treeview&) = delete;
        Wm_treeview (const Wm_treeview&&) = delete;
	
        void add_window (std::string, shared_ptr<Wm_window>);
        void add_widget (std::string, std::string, shared_ptr<Wm_widget>);
        void remove_window (std::string);
        void remove_widget (std::string, std::string);

        void validate () {}

        void for_each (std::function<void(const std::string&,Wm_container&)>&);
        void for_each (std::function<void(const std::string&,const Wm_container&)>&) const;

        // Wm_container*       find_container (std::string uid);
        const Wm_container* find_container (std::string uid) const;

    private:
        std::map<std::string,std::shared_ptr<Wm_container>> _containers;
    };
};

#endif /* WM_TREEVIEW_H */
