#include "fm2.h"

using namespace daisysp;

void Fm2::Init(float samplerate)
{
    //init oscillators
    car_.Init(samplerate);
    mod_.Init(samplerate);
    sample_rate_ = samplerate;

    //set some reasonable values
    lfreq_  = 440.f;
    lratio_ = 2.f;
    SetFrequency(lfreq_);
    SetRatio(lratio_);

    car_.SetAmp(1.f);
    mod_.SetAmp(1.f);

    car_.SetWaveform(Oscillator::WAVE_SIN);
    mod_.SetWaveform(Oscillator::WAVE_SIN);

    idx_ = 1.f;
    last_modulator_output_ = 0.0f;
}

float Fm2::Process()
{
    if(lratio_ != ratio_ || lfreq_ != freq_)
    {
        lratio_ = ratio_;
        lfreq_  = freq_;
        car_.SetFreq(lfreq_);
        mod_.SetFreq(lfreq_ * lratio_);
    }

    float modval = mod_.Process();
    
    // Store scaled modulator output for GetModulatorOutput()
    // Scale to match the phase modulation applied to the carrier for consistent timbre
    // Phase deviation per sample = idx_ * modval * (modulator_freq / sample_rate_)
    float modulator_freq = lfreq_ * lratio_;
    if(modulator_freq > 0.0f)
    {
        last_modulator_output_ = modval * idx_;
    }
    else
    {
        last_modulator_output_ = modval;
    }
    
    // Scale index by modulator frequency to maintain constant timbre across carrier frequencies
    // Modulation index I = (peak frequency deviation) / (modulator frequency)
    // Phase deviation per sample = I * modval * (modulator_freq / sample_rate)
    float effective_idx = idx_ * (modulator_freq / sample_rate_);
    car_.PhaseAdd(modval * effective_idx);
    return car_.Process();
}

void Fm2::SetFrequency(float freq)
{
    freq_ = fabsf(freq);
}

void Fm2::SetRatio(float ratio)
{
    ratio_ = fabsf(ratio);
}

void Fm2::SetIndex(float index)
{
    idx_ = index * kIdxScalar;
}

float Fm2::GetIndex()
{
    return idx_ * kIdxScalarRecip;
}

float Fm2::GetModulatorOutput()
{
    // Return the modulator output from the last Process() call
    return last_modulator_output_;
}

void Fm2::Reset()
{
    car_.Reset();
    mod_.Reset();
}
