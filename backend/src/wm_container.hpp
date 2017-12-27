#ifndef WM_CONTAINER_H
#define WM_CONTAINER_H

#include "wm_position.hpp"
#include "wm_widget.hpp"

#include <functional>
#include <map>

namespace Ats {

    class Wm_container {
    public:
        Wm_container (std::string name, Wm_position pos) : _name(name), _position(pos) {}
        Wm_container (Wm_container&) = delete;
        Wm_container (const Wm_container&&) = delete;
        ~Wm_container ();

        std::string get_uid() const { return _name; }
        Wm_position get_position () const { return _position; }
        void add_widget (std::string, const shared_ptr<Wm_widget>&);
        void remove_widget (std::string);
        bool empty () { return _widgets.empty(); }

        void for_each (std::function<void(const std::string&,const std::shared_ptr<Wm_widget>&)>);
        void for_each (std::function<void(const std::string&,const std::shared_ptr<const Wm_widget>&)>) const;
    private:
        std::string _name;
        Wm_position _position;
        std::map<std::string,std::shared_ptr<Wm_widget>> _widgets;
    };
	
}

#endif /* WM_CONTAINER_H */
