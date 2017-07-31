#ifndef WM_WIDGET_H
#define WM_WIDGET_H

#include "wm_element.hpp"

namespace Ats {

    class Wm_widget : public Wm_element {
	enum class Type {Error};
    };

    class Wm_error_widget : public Wm_widget {};
    
}

#endif /* WM_WIDGET_H */
