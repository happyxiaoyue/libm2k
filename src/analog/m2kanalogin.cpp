/*
 * Copyright (c) 2019 Analog Devices Inc.
 *
 * This file is part of libm2k
 * (see http://www.github.com/analogdevicesinc/libm2k).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "private/m2kanalogin_impl.cpp"

using namespace libm2k::analog;
using namespace libm2k::utils;
using namespace std;
using namespace std::placeholders;

#define HIGH_MAX 2.5
#define HIGH_MIN -2.5
#define LOW_MAX 25
#define LOW_MIN -25

M2kAnalogIn::M2kAnalogIn(iio_context * ctx,
			 std::string adc_dev, bool sync, libm2k::M2kHardwareTrigger *trigger) :
	m_pimpl(std::unique_ptr<M2kAnalogInImpl>(new M2kAnalogInImpl(ctx, adc_dev, sync, trigger)))
{
}

M2kAnalogIn::~M2kAnalogIn()
{
}

void M2kAnalogIn::init()
{
	m_pimpl->init();
}

void M2kAnalogIn::setAdcCalibGain(ANALOG_IN_CHANNEL channel, double gain)
{
	m_pimpl->setAdcCalibGain(channel, gain);
}

void M2kAnalogIn::setAdcCalibOffset(ANALOG_IN_CHANNEL channel, int calib_offset)
{
	m_pimpl->setAdcCalibOffset(channel, calib_offset);
}

double M2kAnalogIn::convRawToVolts(int sample, double correctionGain,
		double hw_gain, double filterCompensation, double offset) const
{
	return m_pimpl->convRawToVolts(sample, correctionGain,
					  hw_gain, filterCompensation, offset);
}

unsigned int M2kAnalogIn::getNbChannels()
{
	return m_pimpl->getNbChannels();
}

string M2kAnalogIn::getName()
{
	return m_pimpl->getName();
}

void M2kAnalogIn::enableChannel(unsigned int chnIdx, bool enable)
{
	m_pimpl->enableChannel(chnIdx, enable);
}

bool M2kAnalogIn::isChannelEnabled(unsigned int chnIdx)
{
	return m_pimpl->isChannelEnabled(chnIdx);
}

void M2kAnalogIn::convertChannelHostFormat(unsigned int chn_idx, int16_t *avg, int16_t *src)
{
	m_pimpl->convertChannelHostFormat(chn_idx, avg, src, false);
}

void M2kAnalogIn::convertChannelHostFormat(unsigned int chn_idx, double *avg, int16_t *src)
{
	m_pimpl->convertChannelHostFormat(chn_idx, avg, src, false);
}

double M2kAnalogIn::setCalibscale(unsigned int index, double calibscale)
{
	return m_pimpl->setCalibscale(index, calibscale);
}

double M2kAnalogIn::getCalibscale(unsigned int index)
{
	return m_pimpl->getCalibscale(index);
}

libm2k::M2kHardwareTrigger *M2kAnalogIn::getTrigger()
{
	return m_pimpl->getTrigger();
}

libm2k::IIO_OBJECTS M2kAnalogIn::getIioObjects()
{
	return m_pimpl->getIioObjects();
}

void M2kAnalogIn::flushBuffer()
{
	m_pimpl->flushBuffer();
}

std::vector<std::vector<double>> M2kAnalogIn::getSamples(unsigned int nb_samples)
{
	return m_pimpl->getSamples(nb_samples, true);
}

std::vector<std::vector<double> > M2kAnalogIn::getSamplesRaw(unsigned int nb_samples)
{
	return m_pimpl->getSamples(nb_samples, false);
}

const double *M2kAnalogIn::getSamplesInterleaved(unsigned int nb_samples)
{
	return m_pimpl->getSamplesInterleaved(nb_samples, true);
}

const short *M2kAnalogIn::getSamplesRawInterleaved(unsigned int nb_samples)
{
	return m_pimpl->getSamplesRawInterleaved(nb_samples);
}

short M2kAnalogIn::convertVoltsToRaw(unsigned int channel, double voltage)
{
	return m_pimpl->convertVoltsToRaw(channel, voltage);
}

double M2kAnalogIn::convertRawToVolts(unsigned int channel, short raw)
{
	return m_pimpl->convertRawToVolts(channel, raw);
}

short M2kAnalogIn::getVoltageRaw(unsigned int ch)
{
	return m_pimpl->getVoltageRaw(ch);
}

short M2kAnalogIn::getVoltageRaw(ANALOG_IN_CHANNEL ch)
{
	return m_pimpl->getVoltageRaw(ch);
}

std::vector<short> M2kAnalogIn::getVoltageRaw()
{
	return m_pimpl->getVoltageRaw();
}

double M2kAnalogIn::getVoltage(unsigned int ch)
{
	return m_pimpl->getVoltage(ch);
}

double M2kAnalogIn::getVoltage(ANALOG_IN_CHANNEL ch)
{
	return m_pimpl->getVoltage(ch);
}

std::vector<double> M2kAnalogIn::getVoltage()
{
	return m_pimpl->getVoltage();
}

const short *M2kAnalogIn::getVoltageRawP()
{
	return m_pimpl->getVoltageRawP();
}

const double *M2kAnalogIn::getVoltageP()
{
	return m_pimpl->getVoltageP();
}

double M2kAnalogIn::getScalingFactor(ANALOG_IN_CHANNEL ch)
{
	return m_pimpl->getScalingFactor(ch);
}

std::pair<double, double> M2kAnalogIn::getHysteresisRange(ANALOG_IN_CHANNEL chn)
{
	return m_pimpl->getHysteresisRange(chn);
}

void M2kAnalogIn::setRange(ANALOG_IN_CHANNEL channel, M2K_RANGE range)
{
	m_pimpl->setRange(channel, range);
}

void M2kAnalogIn::setRange(ANALOG_IN_CHANNEL channel, double min, double max)
{
	m_pimpl->setRange(channel, min, max);
}

M2K_RANGE M2kAnalogIn::getRange(ANALOG_IN_CHANNEL channel)
{
	return m_pimpl->getRange(channel);
}

std::pair<double, double> M2kAnalogIn::getRangeLimits(M2K_RANGE range)
{
	return m_pimpl->getRangeLimits(range);
}

std::vector<std::pair<std::string, std::pair<double, double>>> M2kAnalogIn::getAvailableRanges()
{
	return m_pimpl->getAvailableRanges();
}

void M2kAnalogIn::setVerticalOffset(ANALOG_IN_CHANNEL channel, double vertOffset)
{
	m_pimpl->setVerticalOffset(channel, vertOffset);
}

double M2kAnalogIn::getVerticalOffset(ANALOG_IN_CHANNEL channel)
{
	return m_pimpl->getVerticalOffset(channel);
}

int M2kAnalogIn::getOversamplingRatio()
{
	return m_pimpl->getOversamplingRatio();
}

int M2kAnalogIn::getOversamplingRatio(unsigned int chn_idx)
{
	return m_pimpl->getOversamplingRatio(chn_idx);
}

int M2kAnalogIn::setOversamplingRatio(int oversampling_ratio)
{
	return m_pimpl->setOversamplingRatio(oversampling_ratio);
}

int M2kAnalogIn::setOversamplingRatio(unsigned int chn_idx, int oversampling_ratio)
{
	return m_pimpl->setOversamplingRatio(chn_idx, oversampling_ratio);
}

double M2kAnalogIn::getSampleRate()
{
	return m_pimpl->getSampleRate();
}

double M2kAnalogIn::getSampleRate(unsigned int chn_idx)
{
	return m_pimpl->getSampleRate(chn_idx);
}

std::vector<double> M2kAnalogIn::getAvailableSampleRates()
{
	return m_pimpl->getAvailableSampleRates();
}

double M2kAnalogIn::setSampleRate(double samplerate)
{
	return m_pimpl->setSampleRate(samplerate);
}

double M2kAnalogIn::setSampleRate(unsigned int chn_idx, double samplerate)
{
	return m_pimpl->setSampleRate(chn_idx, samplerate);
}

double M2kAnalogIn::getFilterCompensation(double samplerate)
{
	return m_pimpl->getFilterCompensation(samplerate);
}

double M2kAnalogIn::getValueForRange(M2K_RANGE range)
{
	return m_pimpl->getValueForRange(range);
}

void M2kAnalogIn::setKernelBuffersCount(unsigned int count)
{
	return m_pimpl->setKernelBuffersCount(count);
}
