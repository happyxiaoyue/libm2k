/*
 * Copyright 2018 Analog Devices, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file LICENSE.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#include <cmath>
#include <libm2k/utils/devicegeneric.hpp>
#include <libm2k/utils/devicein.hpp>
#include <libm2k/utils/deviceout.hpp>
#include <libm2k/analog/m2kpowersupply.hpp>
#include <libm2k/m2kexceptions.hpp>
#include <iio.h>

using namespace libm2k::analog;
using namespace libm2k::utils;

class M2kPowerSupply::M2kPowerSupplyImpl :  public DeviceGeneric  {
public:

	M2kPowerSupplyImpl(iio_context *ctx, std::string write_dev,
				       std::string read_dev, bool sync) :
		DeviceGeneric(ctx, ""),
		m_pos_powerdown_idx(2),
		m_neg_powerdown_idx(3),
		m_individual_powerdown(false)
	{
		if (write_dev != "") {
			m_dev_write = make_shared<DeviceOut>(ctx, write_dev);
			if (!m_dev_write) {
				m_dev_write = nullptr;
				throw_exception(EXC_INVALID_PARAMETER, "M2K Power Supply: No device was found for writing",
						__FILE__, __LINE__);
			}
		}

		if (read_dev != "") {
			m_dev_read = make_shared<DeviceIn>(ctx, read_dev);
			if (!m_dev_read) {
				m_dev_read = nullptr;
				throw_exception(EXC_INVALID_PARAMETER, "M2K Power Supply: No device was found for reading",
						__FILE__, __LINE__);
			}
		}

		if (m_dev_read->isChannel(2, false)) {
			m_read_channel_idx.push_back(2);
		} else {
			throw_exception(EXC_INVALID_PARAMETER, "M2K Power Supply: Unable to find 1st read channel",
					__FILE__, __LINE__);
		}

		if (m_dev_read->isChannel(1, false)) {
			m_read_channel_idx.push_back(1);
		} else {
			throw_exception(EXC_INVALID_PARAMETER, "M2K Power Supply: Unable to find 2nd read channels",
					__FILE__, __LINE__);
		}

		if (m_dev_write->isChannel(0, true)) {
			m_write_channel_idx.push_back(0);
		} else {
			throw_exception(EXC_INVALID_PARAMETER, "M2K Power Supply: Unable to find 1st write channel",
					__FILE__, __LINE__);
		}

		if (m_dev_write->isChannel(1, true)) {
			m_write_channel_idx.push_back(1);
		} else {
			throw_exception(EXC_INVALID_PARAMETER, "M2K Power Supply: Unable to find 2nd write channels",
					__FILE__, __LINE__);
		}

		m_channels_enabled.push_back(false);
		m_channels_enabled.push_back(false);


		m_m2k_fabric = make_shared<DeviceGeneric>(ctx, "m2k-fabric");
		if (!m_m2k_fabric) {
			throw_exception(EXC_INVALID_PARAMETER, "M2K Power supply: Cannot find m2k fabric device",
					__FILE__, __LINE__);
		}

		if (!m_m2k_fabric->isChannel(2, true)) {
			throw_exception(EXC_INVALID_PARAMETER, "M2K Power supply: Cannot find powerdown channels",
					__FILE__, __LINE__);
		}

		/* If neg powerdown channel is available */
		if (m_m2k_fabric->isChannel(3, true)) {
			m_individual_powerdown = true;
		}

		powerDownDacs(true);
		loadCalibrationCoefficients();

		for (unsigned int i : m_write_channel_idx) {
			m_dev_write->setDoubleValue(i, 0.0, "raw", true);
		}

		m_write_coefficients.push_back(4095.0 / (5.02 * 1.2 ));
		m_write_coefficients.push_back(4095.0 / (-5.1 * 1.2 ));
		m_read_coefficients.push_back(6.4 / 4095.0);
		m_read_coefficients.push_back((-6.4)  / 4095.0);

		if (sync) {
			syncDevice();
		}
	}

	~M2kPowerSupplyImpl()
	{

	}

	void syncDevice()
	{
		m_channels_enabled.at(0) = m_dev_write->getBoolValue(m_write_channel_idx.at(0), "powerdown", true);
		m_channels_enabled.at(1) = m_dev_write->getBoolValue(m_write_channel_idx.at(1), "powerdown", true);
	}

	void init()
	{
		for (unsigned int i : m_write_channel_idx) {
			m_dev_write->setDoubleValue(i, 0.0, "raw", true);
		}

		m_write_coefficients.at(0) = 4095.0 / (5.02 * 1.2 );
		m_write_coefficients.at(1) = 4095.0 / (-5.1 * 1.2 );
		m_read_coefficients.at(0) = 6.4 / 4095.0;
		m_read_coefficients.at(1) = (-6.4)  / 4095.0;
	}

	void powerDownDacs(bool powerdown)
	{
		m_m2k_fabric->setBoolValue(m_pos_powerdown_idx, powerdown,
					   "user_supply_powerdown", true);
		if (m_individual_powerdown) {
			m_m2k_fabric->setBoolValue(m_neg_powerdown_idx, powerdown,
						   "user_supply_powerdown", true);
		}

		for (unsigned int i : m_write_channel_idx) {
			m_dev_write->setBoolValue(i, powerdown, "powerdown", true);
		}
	}

	void enableChannel(unsigned int chnIdx, bool en)
	{
		if (chnIdx >= m_write_channel_idx.size()) {
			throw_exception(EXC_OUT_OF_RANGE, "M2k PowerSupply: No such channel", __FILE__, __LINE__);
		}
		m_dev_write->setBoolValue(m_write_channel_idx.at(chnIdx), !en, "powerdown", true);

		m_channels_enabled[chnIdx] = en;

		if (m_individual_powerdown) {
			if (chnIdx == 0) {
				m_m2k_fabric->setBoolValue(m_pos_powerdown_idx, !en,
							   "user_supply_powerdown", true);
			} else {
				m_m2k_fabric->setBoolValue(m_neg_powerdown_idx, !en,
							   "user_supply_powerdown", true);
			}
		} else {
			if (en || (!en && !anyChannelEnabled())) {
				m_m2k_fabric->setBoolValue(m_pos_powerdown_idx, !en,
							   "user_supply_powerdown", true);
			}
		}
	}

	bool anyChannelEnabled()
	{
		for (bool en : m_channels_enabled) {
			if (en) {
				return en;
			}
		}
		return false;
	}

	void loadCalibrationCoefficients()
	{
		/*Load calibration parameters from iio context*/
		m_calib_coefficients.clear();
		for (unsigned int i = 4; i < 12; i++) {
			std::pair<std::string, double> calib_pair;
			__try {
				auto pair = getContextAttr(i);
				calib_pair.first = std::string(pair.first.c_str() + 4);
				calib_pair.second = std::stod(pair.second);
				m_calib_coefficients.push_back(calib_pair);
			} __catch (exception_type &) {
				continue;
			}
		}
	}

	double getCalibrationCoefficient(std::string key)
	{
		for (std::pair<std::string, double> calib_pair : m_calib_coefficients) {
			if (calib_pair.first == key) {
				return calib_pair.second;
			}
		}
		throw_exception(EXC_INVALID_PARAMETER, "M2k Power Supply: No such calibration coefficient",
				__FILE__, __LINE__);
		return 0;
	}

	void enableAll(bool en)
	{

	}

	double readChannel(unsigned int idx)
	{
		double val = 0;
		double offset = 0;
		double gain = 0;
		double value = 0;

		if (idx >= m_read_channel_idx.size()) {
			throw_exception(EXC_OUT_OF_RANGE, "M2k PowerSupply: No such channel",
					__FILE__, __LINE__);
		}

		__try {
			if (idx == 0) {
				offset = getCalibrationCoefficient("offset_pos_dac");
				gain = getCalibrationCoefficient("gain_pos_dac");
			} else {
				offset = getCalibrationCoefficient("offset_neg_dac");
				gain = getCalibrationCoefficient("gain_neg_dac");
			}

			//voltage2 and v1
			val = m_dev_read->getDoubleValue(m_read_channel_idx.at(idx), "raw", false);
			value = ((val * m_read_coefficients.at(idx)) + offset) * gain;
		} __catch (exception_type &e) {
			throw_exception(EXC_INVALID_PARAMETER, e.what(), __FILE__, __LINE__);
		}
		return value;
	}

	void pushChannel(unsigned int chnIdx, double value)
	{
		double offset = 0;
		double gain = 0;
		double val;

		if (chnIdx >= m_write_channel_idx.size()) {
			throw_exception(EXC_OUT_OF_RANGE, "M2k PowerSupply: No such channel", __FILE__, __LINE__);
		}

		if (std::abs(value) > 5) {
			throw_exception(EXC_INVALID_PARAMETER, "M2K power supplies are limited to 5V",
					__FILE__, __LINE__);
		}

		if (chnIdx == 0) {
			offset = getCalibrationCoefficient("offset_pos_dac");
			gain = getCalibrationCoefficient("gain_pos_dac");
		} else {
			offset = getCalibrationCoefficient("offset_neg_dac");
			gain = getCalibrationCoefficient("gain_neg_dac");
		}

		val = (value * gain + offset) * m_write_coefficients.at(chnIdx);
		if (val < 0) {
			val = 0;
		}

		m_dev_write->setDoubleValue(m_write_channel_idx.at(chnIdx), val, "raw", true);
	}


private:
	std::shared_ptr<DeviceOut> m_dev_write;
	std::shared_ptr<DeviceIn> m_dev_read;
	std::shared_ptr<DeviceGeneric> m_m2k_fabric;

	std::vector<std::pair<std::string, double>> m_calib_coefficients;
	std::vector<double> m_write_coefficients;
	std::vector<double> m_read_coefficients;
	unsigned int m_pos_powerdown_idx;
	unsigned int m_neg_powerdown_idx;
	bool m_individual_powerdown;
	std::vector<bool> m_channels_enabled;
	std::vector<unsigned int> m_write_channel_idx;
	std::vector<unsigned int> m_read_channel_idx;
};
