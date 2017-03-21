#ifndef CHATTERER_H
#define CHATTERER_H

#include <glibmm.h>
#include <string>

namespace Ats {

    class Chatterer {
    public:
	sigc::signal<void,const Chatterer&> send;

	void talk() { send.emit(*this); }
	
	virtual std::string to_string() const = 0;	
	virtual std::string to_json()   const = 0;
	virtual void   of_json(const std::string&) = 0;
	virtual std::string to_msgpack()   const = 0;
	virtual void   of_msgpack(const std::string&) = 0;
    };

}

#endif /* CHATTERER_H */
