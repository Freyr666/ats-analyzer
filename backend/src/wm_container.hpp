#ifndef WM_CONTAINER_H
#define WM_CONTAINER_H

#include "wm_widget.hpp"
#include "wm_window.hpp"

#include <map>

namespace Ats {

    class Wm_container {
    public:
	Wm_container (std::shared_ptr<Wm_window> w) : _stream(w->stream()), _pid(w->pid()), _window(w) { _window->enable(); }
	Wm_container (Wm_container&) = delete;
	Wm_container (const Wm_container&&) = delete;
	~Wm_container ();
	
	void add_widget (shared_ptr<Wm_widget>);
	void remove_widget (std::tuple<uint,uint>);
    private:
	uint _stream;
	uint _pid;
	std::shared_ptr<Wm_window>                                 _window;
	std::map<std::tuple<uint,uint>,std::shared_ptr<Wm_widget>> _widgets;
    };
	
}

#endif /* WM_CONTAINER_H */
