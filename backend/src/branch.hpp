#ifndef BRANCH_H
#define BRANCH_H

#include "settings.hpp"
#include "metadata.hpp"
#include "errexpn.hpp"
#include "pad.hpp"
#include "video_data.hpp"
#include "audio_data.hpp"

#include <gstreamermm.h>
#include <string>
#include <vector>

namespace Ats {
    
    class Branch {
    public:
	enum class Type { Video, Audio };

	Branch();
	Branch(Branch&) = delete;
	Branch(const Branch&&) = delete;
	virtual ~Branch();

        bool operator==(const std::tuple<uint,uint,uint>& t)
            { return (get<0>(t) == _stream) && (get<1>(t) == _channel) && (get<2>(t) == _pid); }

	virtual Type                                type() = 0;
	std::vector< std::shared_ptr<Pad> > pads() { return _pads; }
	void    add_to_pipe ( const Glib::RefPtr<Gst::Bin>& bin ) { bin->add(_bin); _bin->sync_state_with_parent(); }
        void    plug ( const Glib::RefPtr<Gst::Pad>& p );
	
        template <class PropertyType >
	void    set_property(const std::string& p, const PropertyType& v) {
	    if (_analyser) _analyser->set_property(p, v);
	    else throw Error_expn("Branch: set_property - no analyser exists");
	}
	template <class PropertyType >
	void    get_property(const std::string& p, PropertyType& v) {
	    if (_analyser) _analyser->get_property(p, v);
	    else throw Error_expn("Branch: get_property - no analyser exists");
	}

        virtual void apply (const Settings&) = 0;
        
	static std::unique_ptr<Branch> create(std::string, uint, uint, uint,
                                              std::shared_ptr<Video_data>, std::shared_ptr<Audio_data> ad);

	sigc::signal <void,std::shared_ptr <Pad> > signal_pad_added() { return _pad_added; }
	sigc::signal <void,const uint,const uint,const uint,Meta_pid::Pid_type> signal_set_pid() { return _set_pid; }
	
    protected:

	uint _stream;
	uint _channel;
	uint _pid;
	std::vector< std::shared_ptr<Pad> > _pads;
	Glib::RefPtr<Gst::Element> _analyser;
	Glib::RefPtr<Gst::Element> _decoder;
	Glib::RefPtr<Gst::Bin> _bin;
	sigc::signal <void,std::shared_ptr <Pad> > _pad_added;
	sigc::signal<void,const uint,const uint,const uint,Meta_pid::Pid_type>  _set_pid;
    };

    class Video_branch : public Branch {
    public:
	Video_branch() : Branch () {}
	Video_branch(uint, uint, uint, std::shared_ptr<Video_data>);
        virtual ~Video_branch() {}

        void parse_data_msg(int64_t, Glib::RefPtr<Gst::Buffer>,int64_t, Glib::RefPtr<Gst::Buffer>);
	virtual Type   type() { return Branch::Type::Video; }
        virtual void   apply (const Settings&);

    private:
        std::shared_ptr<Video_data> _video_sender;
	void set_video (const Glib::RefPtr<Gst::Pad>);
    };

    class Audio_branch : public Branch {
    public:
        Audio_branch() : Branch () {}
	Audio_branch(uint, uint, uint, std::shared_ptr<Audio_data> as);
        virtual ~Audio_branch() {}

	virtual Type   type() { return Branch::Type::Audio; }
        virtual void   apply (const Settings&);
	std::shared_ptr<Pad> get_audio_pad() { return _audio_pad; }
	sigc::signal <void,std::shared_ptr <Pad> > signal_audio_pad_added() { return _audio_pad_added; }
	
    private:
        std::shared_ptr<Audio_data> _audio_sender;
	std::shared_ptr<Pad>        _audio_pad;
	sigc::signal <void,std::shared_ptr <Pad> > _audio_pad_added;
    };
}

#endif /* BRANCH_H */
