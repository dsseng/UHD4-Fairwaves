//
// Copyright 2010 Ettus Research LLC
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#include <usrp_uhd/wax.hpp>
#include <stdexcept>
#include <boost/bind.hpp>
#include <boost/format.hpp>
#include <boost/function.hpp>

/***********************************************************************
 * Proxy Contents
 **********************************************************************/
struct proxy_args_t{
    boost::function<void(wax::obj &)>       get;
    boost::function<void(const wax::obj &)> set;
};

/***********************************************************************
 * WAX Object
 **********************************************************************/
wax::obj::obj(void){
    /* NOP */
}

wax::obj::obj(const obj &o){
    _contents = o._contents;
}

wax::obj::~obj(void){
    /* NOP */
}

wax::obj wax::obj::operator[](const obj &key){
    if (_contents.type() == typeid(proxy_args_t)){
        obj val = resolve();
        //check if its a regular pointer and call
        if (val.type() == typeid(obj::ptr)){
            return (*cast<obj::ptr>(val))[key];
        }
        //check if its a smart pointer and call
        if (val.type() == typeid(obj::sptr)){
            return (*cast<obj::sptr>(val))[key];
        }
        //unknown obj
        throw std::runtime_error("cannot use [] on non wax::obj pointer");
    }
    else{
        proxy_args_t proxy_args;
        proxy_args.get = boost::bind(&obj::get, this, key, _1);
        proxy_args.set = boost::bind(&obj::set, this, key, _1);
        return wax::obj(proxy_args);
    }
}

wax::obj & wax::obj::operator=(const obj &val){
    if (_contents.type() == typeid(proxy_args_t)){
        boost::any_cast<proxy_args_t>(_contents).set(val);
    }
    else{
        _contents = val._contents;
    }
    return *this;
}

wax::obj wax::obj::get_link(void) const{
    return ptr(this);
}

const std::type_info & wax::obj::type(void) const{
    return _contents.type();
}

boost::any wax::obj::resolve(void) const{
    if (_contents.type() == typeid(proxy_args_t)){
        obj val;
        boost::any_cast<proxy_args_t>(_contents).get(val);
        return val.resolve();
    }
    else{
        return _contents;
    }
}

std::ostream& operator<<(std::ostream &os, const wax::obj &x){
    os << boost::format("WAX obj (%s)") % x.type().name();
    return os;
}

void wax::obj::get(const obj &, obj &){
    throw std::runtime_error("Cannot call get on wax obj base class");
}

void wax::obj::set(const obj &, const obj &){
    throw std::runtime_error("Cannot call set on wax obj base class");
}