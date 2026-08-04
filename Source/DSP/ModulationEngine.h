#pragma once

// One chorus engine's LFO: Rate drives a rounded/asymmetric waveform (never a pure sine, per
// BBD-TECHNICAL-NOTES.md - "rounded triangle / slightly asymmetric sine / analog integrator with
// tiny imperfections"), Depth scales how far the resulting delay-time offset swings. Two instances
// exist (Engine I, Engine II), each fully independent - this class owns only the LFO and the
// offset calculation, not a delay line; PluginProcessor adds the returned offset to Delay Center
// and the shared Drift offset before asking BBDDelayLine to read a tap there.
class ModulationEngine
{
public:
    void prepare(double sampleRate);
    void reset();

    // Advances the LFO by one sample and returns the current delay-time offset in milliseconds,
    // centred on 0 (excursion only - never more than a couple of ms even at depth=100, per the
    // technical notes: "much tighter" than a modern chorus's 5-25ms swing).
    float getNextOffsetMs(float rateHz, float depthPercent);

private:
    float nextLfoValue(float rateHz);

    double sampleRate = 44100.0;
    float phase = 0.0f;
    float smoothedLfo = 0.0f;
    float smoothingCoeff = 0.0f;
};
