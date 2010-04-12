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

#include "adf4360_regs.hpp"
#include <uhd/usrp/subdev_props.hpp>
#include <uhd/types/ranges.hpp>
#include <uhd/utils/assert.hpp>
#include <uhd/utils/static.hpp>
#include <uhd/usrp/dboard_base.hpp>
#include <uhd/usrp/dboard_manager.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/format.hpp>

using namespace uhd;
using namespace uhd::usrp;
using namespace boost::assign;

/***********************************************************************
 * The RFX series of dboards
 **********************************************************************/
class rfx_xcvr : public xcvr_dboard_base{
public:
    rfx_xcvr(ctor_args_t const& args, const freq_range_t &freq_range);
    ~rfx_xcvr(void);

    void rx_get(const wax::obj &key, wax::obj &val);
    void rx_set(const wax::obj &key, const wax::obj &val);

    void tx_get(const wax::obj &key, wax::obj &val);
    void tx_set(const wax::obj &key, const wax::obj &val);

private:
    freq_range_t _freq_range;
    double       _lo_freq;
    std::string  _rx_ant;
    float        _rx_pga0_gain;
    adf4360_regs_t _adf4360_regs;

    void set_lo_freq(double freq);
    void set_rx_ant(const std::string &ant);
    void set_rx_pga0_gain(float gain);
    void reload_adf4360_regs(void);
};

/***********************************************************************
 * Register the RFX dboards
 **********************************************************************/
static dboard_base::sptr make_rfx_flex400(dboard_base::ctor_args_t const& args){
    return dboard_base::sptr(new rfx_xcvr(args, freq_range_t(400e6, 500e6)));
}

static dboard_base::sptr make_rfx_flex900(dboard_base::ctor_args_t const& args){
    return dboard_base::sptr(new rfx_xcvr(args, freq_range_t(750e6, 1050e6)));
}

static dboard_base::sptr make_rfx_flex1200(dboard_base::ctor_args_t const& args){
    return dboard_base::sptr(new rfx_xcvr(args, freq_range_t(1150e6, 1450e6)));
}

static dboard_base::sptr make_rfx_flex1800(dboard_base::ctor_args_t const& args){
    return dboard_base::sptr(new rfx_xcvr(args, freq_range_t(1500e6, 2100e6)));
}

static dboard_base::sptr make_rfx_flex2400(dboard_base::ctor_args_t const& args){
    return dboard_base::sptr(new rfx_xcvr(args, freq_range_t(2300e6, 2900e6)));
}

UHD_STATIC_BLOCK(reg_rfx_dboards){
    dboard_manager::register_dboard(0x0024, &make_rfx_flex400, "Flex 400 Rx MIMO B");
    dboard_manager::register_dboard(0x0028, &make_rfx_flex400, "Flex 400 Tx MIMO B");

    dboard_manager::register_dboard(0x0025, &make_rfx_flex900, "Flex 900 Rx MIMO B");
    dboard_manager::register_dboard(0x0029, &make_rfx_flex900, "Flex 900 Tx MIMO B");

    dboard_manager::register_dboard(0x0026, &make_rfx_flex1200, "Flex 1200 Rx MIMO B");
    dboard_manager::register_dboard(0x002a, &make_rfx_flex1200, "Flex 1200 Tx MIMO B");

    dboard_manager::register_dboard(0x0034, &make_rfx_flex1800, "Flex 1800 Rx MIMO B");
    dboard_manager::register_dboard(0x0035, &make_rfx_flex1800, "Flex 1800 Tx MIMO B");

    dboard_manager::register_dboard(0x0027, &make_rfx_flex2400, "Flex 2400 Rx MIMO B");
    dboard_manager::register_dboard(0x002b, &make_rfx_flex2400, "Flex 2400 Tx MIMO B");
}

/***********************************************************************
 * Structors
 **********************************************************************/
rfx_xcvr::rfx_xcvr(
    ctor_args_t const& args,
    const freq_range_t &freq_range
) : xcvr_dboard_base(args){
    _freq_range = freq_range;
    set_lo_freq((_freq_range.min + _freq_range.max)/2.0);
    set_rx_ant("rx2");
    set_rx_pga0_gain(0);
}

rfx_xcvr::~rfx_xcvr(void){
    /* NOP */
}

/***********************************************************************
 * Helper Methods
 **********************************************************************/
void rfx_xcvr::set_lo_freq(double freq){
    /* NOP */
    reload_adf4360_regs();
}

void rfx_xcvr::set_rx_ant(const std::string &ant){
    /* NOP */
}

void rfx_xcvr::set_rx_pga0_gain(float gain){
    /* NOP */
}

void rfx_xcvr::reload_adf4360_regs(void){
    std::vector<adf4360_regs_t::addr_t> addrs = list_of
        (adf4360_regs_t::ADDR_CONTROL)
        (adf4360_regs_t::ADDR_NCOUNTER)
        (adf4360_regs_t::ADDR_RCOUNTER)
    ;
    BOOST_FOREACH(adf4360_regs_t::addr_t addr, addrs){
        this->get_interface()->write_spi(
            dboard_interface::UNIT_TYPE_TX,
            spi_config_t::EDGE_RISE,
            _adf4360_regs.get_reg(addr), 24
        );
    }
}

/***********************************************************************
 * RX Get and Set
 **********************************************************************/
void rfx_xcvr::rx_get(const wax::obj &key_, wax::obj &val){
    wax::obj key; std::string name;
    boost::tie(key, name) = extract_named_prop(key_);

    //handle the get request conditioned on the key
    switch(key.as<subdev_prop_t>()){
    case SUBDEV_PROP_NAME:
        val = dboard_id::to_string(get_rx_id());
        return;

    case SUBDEV_PROP_OTHERS:
        val = prop_names_t(); //empty
        return;

    case SUBDEV_PROP_GAIN:
        val = _rx_pga0_gain;
        return;

    case SUBDEV_PROP_GAIN_RANGE:
        val = gain_range_t(0, 45, float(0.022));
        return;

    case SUBDEV_PROP_GAIN_NAMES:
        val = prop_names_t(1, "pga0");
        return;

    case SUBDEV_PROP_FREQ:
        val = _lo_freq;
        return;

    case SUBDEV_PROP_FREQ_RANGE:
        val = _freq_range;
        return;

    case SUBDEV_PROP_ANTENNA:
        val = _rx_ant;
        return;

    case SUBDEV_PROP_ANTENNA_NAMES:{
            prop_names_t ants = list_of("tx/rx")("rx2");
            val = ants;
        }
        return;

    case SUBDEV_PROP_QUADRATURE:
        val = true;
        return;

    case SUBDEV_PROP_IQ_SWAPPED:
        val = true;
        return;

    case SUBDEV_PROP_SPECTRUM_INVERTED:
        val = false;
        return;

    case SUBDEV_PROP_USE_LO_OFFSET:
        val = false;
        return;
    }
}

void rfx_xcvr::rx_set(const wax::obj &key_, const wax::obj &val){
    wax::obj key; std::string name;
    boost::tie(key, name) = extract_named_prop(key_);

    //handle the get request conditioned on the key
    switch(key.as<subdev_prop_t>()){

    case SUBDEV_PROP_GAIN:
        ASSERT_THROW(name == "pga0");
        set_rx_pga0_gain(val.as<float>());
        return;

    case SUBDEV_PROP_ANTENNA:
        set_rx_ant(val.as<std::string>());
        return;

    default:
        throw std::runtime_error(str(boost::format(
            "Error: trying to set read-only property on %s subdev"
        ) % dboard_id::to_string(get_rx_id())));
    }
}

/***********************************************************************
 * TX Get and Set
 **********************************************************************/
void rfx_xcvr::tx_get(const wax::obj &key_, wax::obj &val){
        wax::obj key; std::string name;
    boost::tie(key, name) = extract_named_prop(key_);

    //handle the get request conditioned on the key
    switch(key.as<subdev_prop_t>()){
    case SUBDEV_PROP_NAME:
        val = dboard_id::to_string(get_tx_id());
        return;

    case SUBDEV_PROP_OTHERS:
        val = prop_names_t(); //empty
        return;

    case SUBDEV_PROP_GAIN:
        val = float(0);
        return;

    case SUBDEV_PROP_GAIN_RANGE:
        val = gain_range_t(0, 0, 0);
        return;

    case SUBDEV_PROP_GAIN_NAMES:
        val = prop_names_t(); //empty
        return;

    case SUBDEV_PROP_FREQ:
        val = _lo_freq;
        return;

    case SUBDEV_PROP_FREQ_RANGE:
        val = _freq_range;
        return;

    case SUBDEV_PROP_ANTENNA:
        val = std::string("tx/rx");
        return;

    case SUBDEV_PROP_ANTENNA_NAMES:
        val = prop_names_t(1, "tx/rx");
        return;

    case SUBDEV_PROP_QUADRATURE:
        val = true;
        return;

    case SUBDEV_PROP_IQ_SWAPPED:
        val = false;
        return;

    case SUBDEV_PROP_SPECTRUM_INVERTED:
        val = false;
        return;

    case SUBDEV_PROP_USE_LO_OFFSET:
        val = true;
        return;
    }
}

void rfx_xcvr::tx_set(const wax::obj &key_, const wax::obj &val){
    wax::obj key; std::string name;
    boost::tie(key, name) = extract_named_prop(key_);

    //handle the get request conditioned on the key
    switch(key.as<subdev_prop_t>()){

    case SUBDEV_PROP_GAIN:
        //no gains to set!
        return;

    case SUBDEV_PROP_ANTENNA:
        //its always set to tx/rx, so we only allow this value
        ASSERT_THROW(val.as<std::string>() == "tx/rx");
        return;

    default:
        throw std::runtime_error(str(boost::format(
            "Error: trying to set read-only property on %s subdev"
        ) % dboard_id::to_string(get_tx_id())));
    }
}