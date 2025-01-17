/*
    ==============================================================================

    Copyright 2019 - Paul Ferrand (paulfd@outlook.fr)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.

    ==============================================================================
*/

#pragma once
#include "../JuceLibraryCode/JuceHeader.h"
#include "SfzGlobals.h"
#include "SfzDefaults.h"
#include "SfzContainer.h"
#include "SfzOpcode.h"
#include "SfzEnvelope.h"
#include "SfzFilePool.h"
#include "JuceHelpers.h"
#include <string>
#include <optional>
#include <array>
#include <map>

struct SfzRegion
{
    SfzRegion() = delete;
    SfzRegion(const File& root, SfzFilePool& filePool);
    void parseOpcode(const SfzOpcode& opcode);
    String stringDescription() const noexcept;
    bool prepare();
    bool isStereo() const noexcept;
    float velocityGain(uint8_t velocity) const noexcept;
    float getBasePitchVariation(int noteNumber, uint8_t velocity) const noexcept
    {
        auto pitchVariationInCents = pitchKeytrack * (noteNumber - (int)pitchKeycenter); // note difference with pitch center
        pitchVariationInCents += tune; // sample tuning
        pitchVariationInCents += config::centPerSemitone * transpose; // sample transpose
        pitchVariationInCents += velocity / 127 * pitchVeltrack; // track velocity
        if (pitchRandom > 0)
            pitchVariationInCents += Random::getSystemRandom().nextInt((int)pitchRandom * 2) - pitchRandom; // random pitch changes
        return centsFactor(pitchVariationInCents);
    }
    float getBaseGain() const noexcept
    {
        float baseGaindB { volume };
        baseGaindB += (2 * Random::getSystemRandom().nextFloat() - 1) * ampRandom;
        return Decibels::decibelsToGain(baseGaindB);
    }

    float getNoteGain(int noteNumber, uint8_t velocity) const noexcept
    {
        float baseGain { 1.0f };
        if (trigger == SfzTrigger::release_key)
            baseGain *= velocityGain(lastNoteVelocities[noteNumber]);
        else
            baseGain *= velocityGain(velocity);

        if (noteNumber < crossfadeKeyInRange.getStart())
            baseGain = 0.0f;
        else if (noteNumber < crossfadeKeyInRange.getEnd())
        {
            const auto crossfadePosition = static_cast<float>(noteNumber - crossfadeKeyInRange.getStart()) / crossfadeKeyInRange.getLength();
            if (crossfadeKeyCurve == SfzCrossfadeCurve::power)
                baseGain *= sqrt(crossfadePosition);
            if (crossfadeKeyCurve == SfzCrossfadeCurve::gain)
                baseGain *= crossfadePosition;
        }

        if (noteNumber > crossfadeKeyOutRange.getEnd())
            baseGain = 0.0f;
        else if (noteNumber > crossfadeKeyOutRange.getStart())
        {
            const auto crossfadePosition = static_cast<float>(noteNumber - crossfadeKeyOutRange.getStart()) / crossfadeKeyOutRange.getLength();
            if (crossfadeKeyCurve == SfzCrossfadeCurve::power)
                baseGain *= sqrt(1 - crossfadePosition);
            if (crossfadeKeyCurve == SfzCrossfadeCurve::gain)
                baseGain *= 1 - crossfadePosition;
        }

        if (velocity < crossfadeVelInRange.getStart())
            baseGain = 0;
        else if (velocity < crossfadeVelInRange.getEnd())
        {
            const auto crossfadePosition = static_cast<float>(noteNumber - crossfadeVelInRange.getStart()) / crossfadeVelInRange.getLength();
            if (crossfadeVelCurve == SfzCrossfadeCurve::power)
                baseGain *= sqrt(crossfadePosition);
            if (crossfadeVelCurve == SfzCrossfadeCurve::gain)
                baseGain *= crossfadePosition;
        }

        if (velocity > crossfadeVelOutRange.getEnd())
            baseGain = 0;
        else if (velocity > crossfadeVelOutRange.getStart())
        {
            const auto crossfadePosition = static_cast<float>(noteNumber - crossfadeVelOutRange.getStart()) / crossfadeVelOutRange.getLength();
            if (crossfadeVelCurve == SfzCrossfadeCurve::power)
                baseGain *= sqrt(1 - crossfadePosition);
            if (crossfadeVelCurve == SfzCrossfadeCurve::gain)
                baseGain *= 1 - crossfadePosition;
        }

        return baseGain;
    }
    bool isRelease() const noexcept { return trigger == SfzTrigger::release || trigger == SfzTrigger::release_key; }
    bool isSwitchedOn() const noexcept;
    bool isGenerator() const noexcept { return sample.startsWithChar('*'); }
    bool shouldLoop() const noexcept { return (loopMode == SfzLoopMode::loop_continuous || loopMode == SfzLoopMode::loop_sustain); }

    bool registerNoteOn(int channel, int noteNumber, uint8_t velocity, float randValue);
    bool registerNoteOff(int channel, int noteNumber, uint8_t velocity, float randValue);
    bool registerCC(int channel, int ccNumber, uint8_t ccValue);
    void registerPitchWheel(int channel, int pitch);
    void registerAftertouch(int channel, uint8_t aftertouch);
    void registerTempo(float secondsPerQuarter);

    // Sound source: sample playback
    String sample {}; // Sample
    float delay { SfzDefault::delay }; // delay
    float delayRandom { SfzDefault::delayRandom }; // delay_random
    uint32_t offset { SfzDefault::offset }; // offset
    uint32_t offsetRandom { SfzDefault::offsetRandom }; // offset_random
    uint32_t sampleEnd { SfzDefault::sampleEndRange.getEnd() }; // end
    std::optional<uint32_t> sampleCount {}; // count
    SfzLoopMode loopMode { SfzDefault::loopMode }; // loopmode
    Range<uint32_t> loopRange { SfzDefault::loopRange }; //loopstart and loopend

    // Instrument settings: voice lifecycle
    uint32_t group { SfzDefault::group }; // group
    std::optional<uint32_t> offBy {}; // off_by
    SfzOffMode offMode { SfzDefault::offMode }; // off_mode

    // Region logic: key mapping
    Range<uint8_t> keyRange{ SfzDefault::keyRange }; //lokey, hikey and key
    Range<uint8_t> velocityRange{ SfzDefault::velocityRange }; // hivel and lovel

    // Region logic: MIDI conditions
    Range<uint8_t> channelRange{ SfzDefault::channelRange }; //lochan and hichan
    Range<int> bendRange{ SfzDefault::bendRange }; // hibend and lobend
    SfzContainer<Range<uint8_t>> ccConditions { SfzDefault::ccRange };
    Range<uint8_t> keyswitchRange{ SfzDefault::keyRange }; // sw_hikey and sw_lokey
    std::optional<uint8_t> keyswitch {}; // sw_last
    std::optional<uint8_t> keyswitchUp {}; // sw_up
    std::optional<uint8_t> keyswitchDown {}; // sw_down
    std::optional<uint8_t> previousNote {}; // sw_previous
    SfzVelocityOverride velocityOverride { SfzDefault::velocityOverride }; // sw_vel

    // Region logic: internal conditions
    Range<uint8_t> aftertouchRange{ SfzDefault::aftertouchRange }; // hichanaft and lochanaft
    Range<float> bpmRange{ SfzDefault::bpmRange }; // hibpm and lobpm
    Range<float> randRange{ SfzDefault::randRange }; // hirand and lorand
    uint8_t sequenceLength { SfzDefault::sequenceLength }; // seq_length
    uint8_t sequencePosition { SfzDefault::sequencePosition }; // seq_position

    // Region logic: triggers
    SfzTrigger trigger { SfzDefault::trigger }; // trigger
    std::array<uint8_t, 128> lastNoteVelocities; // Keeps the velocities of the previous note-ons if the region has the trigger release_key
    SfzContainer<Range<uint8_t>> ccTriggers { SfzDefault::ccTriggerValueRange }; // on_loccN on_hiccN

    // Performance parameters: amplifier
    float volume { SfzDefault::volume }; // volume
    float amplitude { SfzDefault::amplitude }; // amplitude
    float pan { SfzDefault::pan }; // pan
    float width { SfzDefault::width }; // width
    float position { SfzDefault::position }; // position
    std::optional<CCValuePair> volumeCC; // volume_oncc
    std::optional<CCValuePair> amplitudeCC; // amplitude_oncc
    std::optional<CCValuePair> panCC; // pan_oncc
    std::optional<CCValuePair> widthCC; // width_oncc
    std::optional<CCValuePair> positionCC; // position_oncc
    uint8_t ampKeycenter { SfzDefault::ampKeycenter }; // amp_keycenter
    float ampKeytrack { SfzDefault::ampKeytrack }; // amp_keytrack
    float ampVeltrack { SfzDefault::ampVeltrack }; // amp_keytrack
    std::vector<std::pair<int, float>> velocityPoints; // amp_velcurve_N
    float ampRandom { SfzDefault::ampRandom }; // amp_random
    Range<uint8_t> crossfadeKeyInRange { SfzDefault::crossfadeKeyInRange };
    Range<uint8_t> crossfadeKeyOutRange { SfzDefault::crossfadeKeyOutRange };
    Range<uint8_t> crossfadeVelInRange { SfzDefault::crossfadeVelInRange };
    Range<uint8_t> crossfadeVelOutRange { SfzDefault::crossfadeVelOutRange };
    SfzCrossfadeCurve crossfadeKeyCurve { SfzDefault::crossfadeKeyCurve };
    SfzCrossfadeCurve crossfadeVelCurve { SfzDefault::crossfadeVelCurve };


    // Performance parameters: pitch
    uint8_t pitchKeycenter{SfzDefault::pitchKeycenter}; // pitch_keycenter
    int pitchKeytrack{ SfzDefault::pitchKeytrack }; // pitch_keytrack
    int pitchRandom{ SfzDefault::pitchRandom }; // pitch_random
    int pitchVeltrack{ SfzDefault::pitchVeltrack }; // pitch_veltrack
    int transpose { SfzDefault::transpose }; // transpose
    int tune { SfzDefault::tune }; // tune

    // Envelopes
    SfzEnvelopeGeneratorDescription amplitudeEG;
    SfzEnvelopeGeneratorDescription pitchEG;
    SfzEnvelopeGeneratorDescription filterEG;

    double sampleRate { config::defaultSampleRate };
    int numChannels { 1 };

    std::vector<std::string> unknownOpcodes;
    std::shared_ptr<AudioBuffer<float>> preloadedData;
private:
    bool prepared { false };
    File rootDirectory { File::getCurrentWorkingDirectory() };
    
    // File information
    SfzFilePool& filePool;

    // Activation logics
    bool keySwitched { true };
    bool previousKeySwitched { true };
    bool sequenceSwitched { true };
    std::array<bool, 128> ccSwitched;
    bool pitchSwitched { true };
    bool bpmSwitched { true };
    bool aftertouchSwitched { true };
    int activeNotesInRange { -1 };

    int sequenceCounter { 0 };
    bool setupSource();
    void addEndpointsToVelocityCurve();
    void checkInitialConditions();
    JUCE_LEAK_DETECTOR (SfzRegion)
    
};