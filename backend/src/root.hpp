#ifndef ROOT_H
#define ROOT_H

#include <vector>
#include <gstreamermm.h>
#include <thread>
#include <mutex>

#include "settings.hpp"
#include "metadata.hpp"
#include "branch.hpp"
#include "video_data.hpp"
#include "audio_data.hpp"

namespace Ats {

    class Root {
    public:
	Root(Root&) = delete;
	Root(const Root&&) = delete;
        ~Root();

        void apply(const Settings&);
        Glib::RefPtr<Gst::Element> src() { return _src; }

	static unique_ptr<Root> create (const Glib::RefPtr<Gst::Bin>,
                                        const Metadata&, const Settings&,
                                        std::shared_ptr<Video_data>, std::shared_ptr<Audio_data>);

	sigc::signal <void,std::shared_ptr <Pad> > signal_pad_added() { return _pad_added; }
	sigc::signal <void,std::shared_ptr <Pad> > signal_audio_pad_added() { return _audio_pad_added; }

    private:
	Root(const Glib::RefPtr<Gst::Bin>, const Metadata&, const Settings&,
             std::shared_ptr<Video_data>, std::shared_ptr<Audio_data>);
	Glib::RefPtr<Gst::Bin> _bin;
        Glib::RefPtr<Gst::Element> _src;
	Glib::RefPtr<Gst::Element> _tee;
	Glib::RefPtr<Gst::Element> _demux;
	std::vector<std::unique_ptr<Branch>> _branches;
        Settings                   _settings;
        std::shared_ptr<Video_data> _video_sender;
        std::shared_ptr<Audio_data> _audio_sender;
	
	void build_cb (const uint stream, const Meta_channel&);
	void build_branch (const uint, const Meta_channel, const Glib::RefPtr<Gst::Pad>&);
        void destroy_branch (const uint, const uint, const Glib::RefPtr<Gst::Pad>&);
	
	sigc::signal <void,std::shared_ptr <Pad> > _pad_added;
	sigc::signal <void,std::shared_ptr <Pad> > _audio_pad_added;
        /* protect audio pads */
        std::mutex      _mutex_pad;
        std::mutex      _mutex_audio_pad;
    };

}

#endif /* ROOT_H */
