#ifndef BRANCH_H
#define BRANCH_H

#include <gstreamermm.h>
#include <string>
#include <vector>

namespace Ats {

    class Pad {
    public:
	enum class Type { Video, Audio, Graph_volume, Unknown };
	
	Pad(Glib::RefPtr<Gst::Pad>&);
	Pad(const Pad&&) = delete;
	Pad(Pad&) = delete;

	Type                   type() { return _t; }
	Glib::RefPtr<Gst::Pad> pad()  { return _pad; }
    private:
	Type _t;
	Glib::RefPtr<Gst::Pad> _pad;
    };
    
    class Branch {
    public:
	enum class Type { Video, Audio };

	Branch();
	Branch(Branch&) = delete;
	Branch(const Branch&&) = delete;
	virtual ~Branch() {}

	virtual Type                                type() = 0;
	std::vector< std::shared_ptr<Pad> > pads() { return _pads; }
	Glib::RefPtr<Gst::Bin>              bin()  { return _bin; }
	
	template <class PropertyType >
	void    set_property(const std::string& p, const PropertyType& v) {
	    _analyser->set_property(p, v);
	}
	template <class PropertyType >
	void    get_property(const std::string& p, PropertyType& v) {
	    _analyser->get_property(p, v);
	}
	
	static std::unique_ptr<Branch> create(std::string);
    protected:
	std::vector< std::shared_ptr<Pad> > _pads;
	Glib::RefPtr<Gst::Element> _analyser;
	Glib::RefPtr<Gst::Bin> _bin;
    };

    class Video_branch : Branch {
    public:
	Video_branch(uint, uint, uint);

	virtual Type   type() { return Branch::Type::Video; }
    };

    class Audio_branch : Branch {
    public:
	Audio_branch(uint, uint, uint);

	virtual Type   type() { return Branch::Type::Audio; }
    };
}

#endif /* BRANCH_H */
