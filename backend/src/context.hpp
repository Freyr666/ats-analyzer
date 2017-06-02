#ifndef CONTEXT_H
#define CONTEXT_H

#include <glibmm.h>
#include <gstreamermm.h>
#include <exception>
#include <vector>

#include "control.hpp"
#include "chatterer.hpp"
#include "probe.hpp"
#include "options.hpp"
#include "settings.hpp"
#include "graph.hpp"
#include "msgtype.hpp"

using namespace std;
using namespace Glib;

namespace Ats {

    class Context : public Chatterer_proxy, public Logger {
    public:

        struct Size_error : std::exception {};
	
    private:
        Graph         graph;
        vector< unique_ptr<Probe> > probes;
        Control       control;
        Options       options;
        Settings      settings;

        const json    j_schema;
	
        RefPtr<MainLoop> main_loop;
  	
    public:
        Context(Initial);
        Context(const Context&) = delete;
        Context(Context&&) = delete;
        virtual ~Context() {}

        Msg_type msg_type = Msg_type::Debug;

        void run() { main_loop->run(); }

        // Chatterer_proxy
        void forward_talk(const Chatterer&);
        void forward_error(const std::string&);
        void dispatch(const std::string&);
    };

};

#endif /* CONTEXT_H */
