#ifndef GRAPH_H
#define GRAPH_H

#include <string>
#include <gstreamermm.h>
#include <glibmm.h>
#include <functional>
#include <map>
#include <boost/variant.hpp>

#include "chatterer.hpp"
#include "metadata.hpp"

using namespace std;
using namespace Glib;

namespace Ats {

    class Options;
    class Settings;
    
    class Graph : public Chatterer, public Logger {
	
    public:

        /* --------- Json schema ----------------- */
        static constexpr const char* JSON_SCHEMA = R"({
            "comment":"JSON schema for Graph class",
            "type":"object",
            "properties":{
                "state":{"type":"string",
                         "enum":["null","pause","play","stop"]}
            }
        })";

        Graph(const std::string& n) : Chatterer(n) {}
        Graph(const Graph&) = delete;
        Graph(Graph&&) = delete;
        virtual ~Graph() {}
	
        void       set(const Options&);
        void       reset();
        void       apply_options(const Options&);
        void       apply_settings(const Settings&);
	
        void       set_state(Gst::State);
        Gst::State get_state() const;

        void   set_resolution(const pair<uint,uint>);
        void   set_position(uint, uint, uint, const Position&);
        void   set_settings(const Settings&);

        void   connect(Options& o);
        void   connect(Settings& o);

        sigc::signal<void,const uint,const uint,const uint,Meta_pid::Pid_type>   set_pid;
	
        // Chatterer
        string to_string() const;
        json   serialize() const;
        void   deserialize(const json&);
	
    private:
        struct Node {
            string               type;
            RefPtr<Gst::Element> analysis;
            RefPtr<Gst::Pad>     connected;
        };
        class Tree {
            map<tuple<uint,uint,uint>,Node> _tree;
        public:
            bool  empty() {return _tree.empty();}
            void  reset();
            void  add(uint, uint, uint, Node);
            Node* get(uint, uint, uint);
        };
	
        Tree                  elms;
        RefPtr<Gst::Element>  bg;
        RefPtr<Gst::Pad>      bg_pad;
	
        RefPtr<Gst::Pipeline> pipe;
        RefPtr<Gst::Bus>      bus;
	
        RefPtr<Gst::Bin> create_root(const Metadata&);
        RefPtr<Gst::Bin> create_branch(const uint,
                                       const uint,
                                       const uint,
                                       const Metadata&);

        bool             on_bus_message(const Glib::RefPtr<Gst::Bus>&,
                                        const Glib::RefPtr<Gst::Message>&);
    };

};

#endif /* GRAPH_H */
