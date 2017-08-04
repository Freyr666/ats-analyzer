#ifndef WM_POSITION_H
#define WM_POSITION_H

#include <string>

namespace Ats {

    class Wm_position {
    public:
        class Wrong_position : public std::exception {};
            
        Wm_position (std::pair<int,int> luc, std::pair<int,int> rlc) {
            // invariants
            if   (luc.first > rlc.first || luc.second > rlc.second) throw Wrong_position {};
            else { _luc = luc; _rlc = rlc; }
        }

        std::pair<int,int> get_luc () const { return _luc; }
        std::pair<int,int> get_rlc () const { return _rlc; }
        void set_luc (std::pair<int,int> luc) { 
            if (luc.first >= _rlc.first || luc.second >= _rlc.second) throw Wrong_position {};
            else _luc = luc;
        }
        void set_rlc (std::pair<int,int> rlc) { 
            if (_luc.first >= rlc.first || _luc.second >= rlc.second) throw Wrong_position {};
            else _rlc = rlc;
        }

        int get_x      () const { return _luc.first; }
        int get_y      () const { return _luc.second; }
        int get_height () const { return _rlc.second - _luc.second; }
        int get_width  () const { return _rlc.first - _luc.first; }

        int get_left   () const { return get_x(); }
        int get_top    () const { return get_y(); }
        int get_right  () const { return _rlc.first; }
        int get_bottom () const { return _rlc.second; }

        operator bool () const { return ! (_rlc == _luc && _rlc == std::make_pair(0,0)); }
        bool operator== (const Wm_position& p) const { return (_rlc == p._rlc) && (_luc == p._luc); }
        bool operator!= (const Wm_position& p) const { return (_rlc != p._rlc) || (_luc != p._luc); }
        bool is_overlap (const Wm_position& p) const {
            return (_luc.first < p._rlc.first && _rlc.first > p._luc.first &&
                    _luc.second > p._rlc.second && _rlc.second < p._luc.second);
        }
    private:
        std::pair<int,int> _luc; // left-upper corner
        std::pair<int,int> _rlc; // right-lower corner
    };
}

#endif /* WM_POSITION_H */
