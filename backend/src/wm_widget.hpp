#ifndef WM_WIDGET_H
#define WM_WIDGET_H

#include "wm_element.hpp"

namespace Ats {

    class Wm_widget : public Wm_element {
    public:
        enum class Type {Error};

        virtual Type        type() const = 0;
    };

    class Wm_error_widget : public Wm_widget {};
    
}

#endif /* WM_WIDGET_H */
