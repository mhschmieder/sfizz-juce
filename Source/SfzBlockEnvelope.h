#pragma once
#include "SfzGlobals.h"
#include <vector>
#include <algorithm>
#include <functional>
#include <cstdint>

template<class OutputType = float, class InputType = uint8_t>
class SfzBlockEnvelope
{
public:
    SfzBlockEnvelope(int maximum = config::defaultSamplesPerBlock, OutputType initialValue = static_cast<OutputType>(0.0))
    : currentValue(initialValue)
    {
        reserve(maximum);
    }

    void reserve(int maximum)
    {
        events.reserve(maximumEvents);
        maximumEvents = maximum;
    }

    void addEvent(int timestamp, InputType value)
    {
        if (events.size() < maximumEvents)
        {
            auto existingEvent = std::find_if(events.begin(), events.end(), [&](const auto& event) {
                return timestamp == event.timestamp;
            });
            if (existingEvent == events.end())
                events.push_back({ timestamp, value });
            else
                existingEvent->value = value;
        }
    }

    void getEnvelope(dsp::AudioBlock<float> output)
    {
        if (events.empty())
        {
            output.fill(currentValue);
            return;
        }

        std::sort(events.begin(), events.end(), EventComparator());
        const auto numSamples = output.getNumSamples();
        int eventIndex { 0 };
        int sampleIndex { 0 };
        int numSteps { 0 };
        OutputType step { 0.0f };

        while (sampleIndex < numSamples)
        {
            if (numSteps == 0)
            {
                if (events[eventIndex].timestamp == sampleIndex)
                {
                    currentValue = transform(events[eventIndex].value);
                    eventIndex++;
                }
                
                if (eventIndex == events.size())
                {
                    output.getSubBlock(sampleIndex, numSamples - sampleIndex).fill(currentValue);
                    clearEvents();
                    return;
                }
                else
                {
                    numSteps = events[eventIndex].timestamp - sampleIndex;
                    step = (transform(events[eventIndex].value) - currentValue) / numSteps;
                }
            }

            for (auto channelIndex = 0; channelIndex < config::numChannels; ++channelIndex)
                output.setSample(channelIndex, sampleIndex, currentValue);
            numSteps--;
            sampleIndex++;
            currentValue += step;
        }

        // Backtrack if we get here because it means we went 1 step too far out
        currentValue -= step;
        clearEvents();
    }
    void clearEvents()
    {
        events.clear();
    }

    void setFunction(std::function<OutputType(InputType)> function)
    {
        transform = function;
    }

    void setDefaultValue(InputType inputValue)
    {
        currentValue = transform(inputValue);
    }
    
private:
    struct Event
    {
        int timestamp;
        InputType value;
    };
    struct EventComparator
    {
        bool operator()(const Event& lhs, const Event& rhs) const { return lhs.timestamp < rhs.timestamp; }
    };
    std::vector<Event> events;
    int maximumEvents { 0 };
    OutputType currentValue { static_cast<OutputType>(0.0) };
    std::function<OutputType(InputType)> transform { [](InputType in){ return static_cast<OutputType>(in); } };
};