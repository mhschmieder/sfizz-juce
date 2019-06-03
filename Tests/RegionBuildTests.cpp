#include "../JuceLibraryCode/JuceHeader.h"
#include "catch2/catch.hpp"
#include "../Source/SfzRegion.h"
using namespace Catch::literals;

TEST_CASE("Parsing opcodes", "Region tests")
{
    SfzFilePool openFiles { File::getCurrentWorkingDirectory() };
    SfzRegion region { File::getCurrentWorkingDirectory(), openFiles };
    SECTION("sample")
    {
        REQUIRE( region.sample == "" );
        region.parseOpcode({ "sample", "dummy.wav" });
        REQUIRE( region.sample == "dummy.wav" );
    }

    SECTION("delay")
    {
        REQUIRE( region.delay == 0.0 );
        region.parseOpcode({ "delay", "1.0" });
        REQUIRE( region.delay == 1.0 );
        region.parseOpcode({ "delay", "-1.0" });
        REQUIRE( region.delay == 0.0 );        
        region.parseOpcode({ "delay", "110.0" });
        REQUIRE( region.delay == 100.0 );
    }

    SECTION("delay_random")
    {
        REQUIRE( region.offset == SfzDefault::delayRandom );
        region.parseOpcode({ "delay_random", "1.0" });
        REQUIRE( region.delayRandom == 1.0);
        region.parseOpcode({ "delay_random", "-1.0" });
        REQUIRE( region.delayRandom == SfzDefault::delayRange.getStart() );        
        region.parseOpcode({ "delay_random", "110.0" });
        REQUIRE( region.delayRandom == SfzDefault::delayRange.getEnd() );
    }

    SECTION("offset")
    {
        REQUIRE( region.offset == 0 );
        region.parseOpcode({ "offset", "1" });
        REQUIRE( region.offset == 1 );
        region.parseOpcode({ "offset", "-1" });
        REQUIRE( region.offset == 0 );
    }

    SECTION("offset_random")
    {
        REQUIRE( region.offsetRandom == 0 );
        region.parseOpcode({ "offset_random", "1" });
        REQUIRE( region.offsetRandom == 1 );
        region.parseOpcode({ "offset_random", "-1" });
        REQUIRE( region.offsetRandom == 0);
    }

    SECTION("end")
    {
        region.parseOpcode({ "end", "184" });
        REQUIRE( region.sampleEnd == 184 );
        region.parseOpcode({ "end", "-1" });
        REQUIRE( region.sampleEnd == 0 );
    }

    SECTION("count")
    {
        REQUIRE( !region.sampleCount );
        region.parseOpcode({ "count", "184" });
        REQUIRE( region.sampleCount );
        REQUIRE( *region.sampleCount == 184 );
        region.parseOpcode({ "count", "-1" });
        REQUIRE( region.sampleCount );
        REQUIRE( *region.sampleCount == 0 );
    }

    SECTION("loop_mode")
    {
        REQUIRE( region.loopMode == SfzLoopMode::no_loop );
        region.parseOpcode({ "loop_mode", "no_loop" });
        REQUIRE( region.loopMode == SfzLoopMode::no_loop );
        region.parseOpcode({ "loop_mode", "one_shot" });
        REQUIRE( region.loopMode == SfzLoopMode::one_shot );
        region.parseOpcode({ "loop_mode", "loop_continuous" });
        REQUIRE( region.loopMode == SfzLoopMode::loop_continuous );
        region.parseOpcode({ "loop_mode", "loop_sustain" });
        REQUIRE( region.loopMode == SfzLoopMode::loop_sustain );
    }

    SECTION("loopmode")
    {
        REQUIRE( region.loopMode == SfzLoopMode::no_loop );
        region.parseOpcode({ "loopmode", "no_loop" });
        REQUIRE( region.loopMode == SfzLoopMode::no_loop );
        region.parseOpcode({ "loopmode", "one_shot" });
        REQUIRE( region.loopMode == SfzLoopMode::one_shot );
        region.parseOpcode({ "loopmode", "loop_continuous" });
        REQUIRE( region.loopMode == SfzLoopMode::loop_continuous );
        region.parseOpcode({ "loopmode", "loop_sustain" });
        REQUIRE( region.loopMode == SfzLoopMode::loop_sustain );
    }

    SECTION("loop_end")
    {
        REQUIRE( region.loopRange == Range<uint32_t>(0, 4294967295) );
        region.parseOpcode({ "loop_end", "184" });
        REQUIRE( region.loopRange == Range<uint32_t>(0, 184) );
        region.parseOpcode({ "loop_end", "-1" });
        REQUIRE( region.loopRange == Range<uint32_t>(0, 0) );
    }

    SECTION("loop_start")
    {
        region.parseOpcode({ "loop_start", "184" });
        REQUIRE( region.loopRange == Range<uint32_t>(184, 4294967295) );
        region.parseOpcode({ "loop_start", "-1" });
        REQUIRE( region.loopRange == Range<uint32_t>(0, 4294967295) );
    }

    SECTION("loopend")
    {
        REQUIRE( region.loopRange == Range<uint32_t>(0, 4294967295) );
        region.parseOpcode({ "loopend", "184" });
        REQUIRE( region.loopRange == Range<uint32_t>(0, 184) );
        region.parseOpcode({ "loopend", "-1" });
        REQUIRE( region.loopRange == Range<uint32_t>(0, 0) );
    }

    SECTION("loopstart")
    {
        region.parseOpcode({ "loopstart", "184" });
        REQUIRE( region.loopRange == Range<uint32_t>(184, 4294967295) );
        region.parseOpcode({ "loopstart", "-1" });
        REQUIRE( region.loopRange == Range<uint32_t>(0, 4294967295) );
    }

    SECTION("group")
    {
        REQUIRE( region.group == 0 );
        region.parseOpcode({ "group", "5" });
        REQUIRE( region.group == 5 );
        region.parseOpcode({ "group", "-1" });
        REQUIRE( region.group == 0 );
    }

    SECTION("off_by")
    {
        REQUIRE( !region.offBy );
        region.parseOpcode({ "off_by", "5" });
        REQUIRE( region.offBy );
        REQUIRE( region.offBy == 5 );
        region.parseOpcode({ "off_by", "-1" });
        REQUIRE( region.offBy );
        REQUIRE( region.offBy == 0 );
    }

    SECTION("off_mode")
    {
        REQUIRE( region.offMode == SfzOffMode::fast );
        region.parseOpcode({ "off_mode", "fast" });
        REQUIRE( region.offMode == SfzOffMode::fast );
        region.parseOpcode({ "off_mode", "normal" });
        REQUIRE( region.offMode == SfzOffMode::normal );
    }

    SECTION("lokey, hikey, and key")
    {
        REQUIRE( region.keyRange == Range<uint8_t>(0, 127) );
        region.parseOpcode({ "lokey", "37" });
        REQUIRE( region.keyRange == Range<uint8_t>(37, 127) );
        region.parseOpcode({ "lokey", "c4" });
        REQUIRE( region.keyRange == Range<uint8_t>(60, 127) );
        region.parseOpcode({ "lokey", "128" });
        REQUIRE( region.keyRange == Range<uint8_t>(127, 127) );
        region.parseOpcode({ "lokey", "-3" });
        REQUIRE( region.keyRange == Range<uint8_t>(0, 127) );
        region.parseOpcode({ "hikey", "65" });
        REQUIRE( region.keyRange == Range<uint8_t>(0, 65) );
        region.parseOpcode({ "hikey", "c4" });
        REQUIRE( region.keyRange == Range<uint8_t>(0, 60) );
        region.parseOpcode({ "hikey", "-1" });
        REQUIRE( region.keyRange == Range<uint8_t>(0, 0) );
        region.parseOpcode({ "hikey", "128" });
        REQUIRE( region.keyRange == Range<uint8_t>(0, 127) );
        region.parseOpcode({ "key", "26" });
        REQUIRE( region.keyRange == Range<uint8_t>(26, 26) );
        REQUIRE( region.pitchKeycenter == 26 );
        region.parseOpcode({ "key", "-26" });
        REQUIRE( region.keyRange == Range<uint8_t>(0, 0) );
        REQUIRE( region.pitchKeycenter == 0 );
        region.parseOpcode({ "key", "234" });
        REQUIRE( region.keyRange == Range<uint8_t>(127, 127) );       
        REQUIRE( region.pitchKeycenter == 127 );
        region.parseOpcode({ "key", "c4" });
        REQUIRE( region.keyRange == Range<uint8_t>(60, 60) );
        REQUIRE( region.pitchKeycenter == 60 );
    }

    SECTION("lovel, hivel")
    {
        REQUIRE( region.velocityRange == Range<uint8_t>(0, 127) );
        region.parseOpcode({ "lovel", "37" });
        REQUIRE( region.velocityRange == Range<uint8_t>(37, 127) );
        region.parseOpcode({ "lovel", "128" });
        REQUIRE( region.velocityRange == Range<uint8_t>(127, 127) );
        region.parseOpcode({ "lovel", "-3" });
        REQUIRE( region.velocityRange == Range<uint8_t>(0, 127) );
        region.parseOpcode({ "hivel", "65" });
        REQUIRE( region.velocityRange == Range<uint8_t>(0, 65) );
        region.parseOpcode({ "hivel", "-1" });
        REQUIRE( region.velocityRange == Range<uint8_t>(0, 0) );
        region.parseOpcode({ "hivel", "128" });
        REQUIRE( region.velocityRange == Range<uint8_t>(0, 127) );
    }

    SECTION("lochan, hichan")
    {
        REQUIRE( region.channelRange == Range<uint8_t>(1, 16) );
        region.parseOpcode({ "lochan", "4" });
        REQUIRE( region.channelRange == Range<uint8_t>(4, 16) );
        region.parseOpcode({ "lochan", "128" });
        REQUIRE( region.channelRange == Range<uint8_t>(16, 16) );
        region.parseOpcode({ "lochan", "-3" });
        REQUIRE( region.channelRange == Range<uint8_t>(1, 16) );
        region.parseOpcode({ "hichan", "13" });
        REQUIRE( region.channelRange == Range<uint8_t>(1, 13) );
        region.parseOpcode({ "hichan", "-1" });
        REQUIRE( region.channelRange == Range<uint8_t>(1, 1) );
        region.parseOpcode({ "hichan", "128" });
        REQUIRE( region.channelRange == Range<uint8_t>(1, 16) );
    }

    SECTION("lobend, hibend")
    {
        REQUIRE( region.bendRange == Range<int>(-8192, 8192) );
        region.parseOpcode({ "lobend", "4" });
        REQUIRE( region.bendRange == Range<int>(4, 8192) );
        region.parseOpcode({ "lobend", "-128" });
        REQUIRE( region.bendRange == Range<int>(-128, 8192) );
        region.parseOpcode({ "lobend", "-10000" });
        REQUIRE( region.bendRange == Range<int>(-8192, 8192) );
        region.parseOpcode({ "hibend", "13" });
        REQUIRE( region.bendRange == Range<int>(-8192, 13) );
        region.parseOpcode({ "hibend", "-1" });
        REQUIRE( region.bendRange == Range<int>(-8192, -1) );
        region.parseOpcode({ "hibend", "10000" });
        REQUIRE( region.bendRange == Range<int>(-8192, 8192) );
    }

    SECTION("locc, hicc")
    {
        REQUIRE( region.ccConditions.getWithDefault(0) == Range<uint8_t>(0, 127) );
        REQUIRE( region.ccConditions[127] == Range<uint8_t>(0, 127) );
        region.parseOpcode({ "locc6", "4" });
        REQUIRE( region.ccConditions[6] == Range<uint8_t>(4, 127) );
        region.parseOpcode({ "locc12", "-128" });
        REQUIRE( region.ccConditions[12] == Range<uint8_t>(0, 127) );
        region.parseOpcode({ "hicc65", "39" });
        REQUIRE( region.ccConditions[65] == Range<uint8_t>(0, 39) );
        region.parseOpcode({ "hicc127", "135" });
        REQUIRE( region.ccConditions[127] == Range<uint8_t>(0, 127) );
    }

    SECTION("sw_lokey, sw_hikey")
    {
        REQUIRE( region.keyswitchRange == Range<uint8_t>(0, 127) );
        region.parseOpcode({ "sw_lokey", "4" });
        REQUIRE( region.keyswitchRange == Range<uint8_t>(4, 127) );
        region.parseOpcode({ "sw_lokey", "128" });
        REQUIRE( region.keyswitchRange == Range<uint8_t>(127, 127) );
        region.parseOpcode({ "sw_lokey", "0" });
        REQUIRE( region.keyswitchRange == Range<uint8_t>(0, 127) );
        region.parseOpcode({ "sw_hikey", "39" });
        REQUIRE( region.keyswitchRange == Range<uint8_t>(0, 39) );
        region.parseOpcode({ "sw_hikey", "135" });
        REQUIRE( region.keyswitchRange == Range<uint8_t>(0, 127) );
        region.parseOpcode({ "sw_hikey", "-1" });
        REQUIRE( region.keyswitchRange == Range<uint8_t>(0, 0) );
    }

    SECTION("sw_last")
    {
        REQUIRE( !region.keyswitch );
        region.parseOpcode({ "sw_last", "4" });
        REQUIRE( region.keyswitch );
        REQUIRE( *region.keyswitch == 4 );
        region.parseOpcode({ "sw_last", "128" });
        REQUIRE( region.keyswitch );
        REQUIRE( *region.keyswitch == 127 );
        region.parseOpcode({ "sw_last", "-1" });
        REQUIRE( region.keyswitch );
        REQUIRE( *region.keyswitch == 0 );
    }

    SECTION("sw_up")
    {
        REQUIRE( !region.keyswitchUp );
        region.parseOpcode({ "sw_up", "4" });
        REQUIRE( region.keyswitchUp );
        REQUIRE( *region.keyswitchUp == 4 );
        region.parseOpcode({ "sw_up", "128" });
        REQUIRE( region.keyswitchUp );
        REQUIRE( *region.keyswitchUp == 127 );
        region.parseOpcode({ "sw_up", "-1" });
        REQUIRE( region.keyswitchUp );
        REQUIRE( *region.keyswitchUp == 0 );
    }

    SECTION("sw_down")
    {
        REQUIRE( !region.keyswitchDown );
        region.parseOpcode({ "sw_down", "4" });
        REQUIRE( region.keyswitchDown );
        REQUIRE( *region.keyswitchDown == 4 );
        region.parseOpcode({ "sw_down", "128" });
        REQUIRE( region.keyswitchDown );
        REQUIRE( *region.keyswitchDown == 127 );
        region.parseOpcode({ "sw_down", "-1" });
        REQUIRE( region.keyswitchDown );
        REQUIRE( *region.keyswitchDown == 0 );
    }

    SECTION("sw_previous")
    {
        REQUIRE( !region.previousNote );
        region.parseOpcode({ "sw_previous", "4" });
        REQUIRE( region.previousNote );
        REQUIRE( *region.previousNote == 4 );
        region.parseOpcode({ "sw_previous", "128" });
        REQUIRE( region.previousNote );
        REQUIRE( *region.previousNote == 127 );
        region.parseOpcode({ "sw_previous", "-1" });
        REQUIRE( region.previousNote );
        REQUIRE( *region.previousNote == 0 );
    }

    SECTION("sw_vel")
    {
        REQUIRE( region.velocityOverride == SfzVelocityOverride::current );
        region.parseOpcode({ "sw_vel", "current" });
        REQUIRE( region.velocityOverride == SfzVelocityOverride::current );
        region.parseOpcode({ "sw_vel", "previous" });
        REQUIRE( region.velocityOverride == SfzVelocityOverride::previous );
    }

    SECTION("lochanaft, hichanaft")
    {
        REQUIRE( region.aftertouchRange == Range<uint8_t>(0, 127) );
        region.parseOpcode({ "lochanaft", "4" });
        REQUIRE( region.aftertouchRange == Range<uint8_t>(4, 127) );
        region.parseOpcode({ "lochanaft", "128" });
        REQUIRE( region.aftertouchRange == Range<uint8_t>(127, 127) );
        region.parseOpcode({ "lochanaft", "0" });
        REQUIRE( region.aftertouchRange == Range<uint8_t>(0, 127) );
        region.parseOpcode({ "hichanaft", "39" });
        REQUIRE( region.aftertouchRange == Range<uint8_t>(0, 39) );
        region.parseOpcode({ "hichanaft", "135" });
        REQUIRE( region.aftertouchRange == Range<uint8_t>(0, 127) );
        region.parseOpcode({ "hichanaft", "-1" });
        REQUIRE( region.aftertouchRange == Range<uint8_t>(0, 0) );
    }

    SECTION("lobpm, hibpm")
    {
        REQUIRE( region.bpmRange == Range<float>(0, 500) );
        region.parseOpcode({ "lobpm", "47.5" });
        REQUIRE( region.bpmRange == Range<float>(47.5, 500) );
        region.parseOpcode({ "lobpm", "594" });
        REQUIRE( region.bpmRange == Range<float>(500, 500) );
        region.parseOpcode({ "lobpm", "0" });
        REQUIRE( region.bpmRange == Range<float>(0, 500) );
        region.parseOpcode({ "hibpm", "78" });
        REQUIRE( region.bpmRange == Range<float>(0, 78) );
        region.parseOpcode({ "hibpm", "895.4" });
        REQUIRE( region.bpmRange == Range<float>(0, 500) );
        region.parseOpcode({ "hibpm", "-1" });
        REQUIRE( region.bpmRange == Range<float>(0, 0) );
    }

    SECTION("lorand, hirand")
    {
        REQUIRE( region.randRange == Range<float>(0, 1) );
        region.parseOpcode({ "lorand", "0.5" });
        REQUIRE( region.randRange == Range<float>(0.5, 1) );
        region.parseOpcode({ "lorand", "4" });
        REQUIRE( region.randRange == Range<float>(1, 1) );
        region.parseOpcode({ "lorand", "0" });
        REQUIRE( region.randRange == Range<float>(0, 1) );
        region.parseOpcode({ "hirand", "39" });
        REQUIRE( region.randRange == Range<float>(0, 1) );
        region.parseOpcode({ "hirand", "0.7" });
        REQUIRE( region.randRange == Range<float>(0, 0.7f) );
        region.parseOpcode({ "hirand", "-1" });
        REQUIRE( region.randRange == Range<float>(0, 0) );
    }

    SECTION("seq_length")
    {
        REQUIRE( region.sequenceLength == 1 );
        region.parseOpcode({ "seq_length", "89" });
        REQUIRE( region.sequenceLength == 89 );
        region.parseOpcode({ "seq_length", "189" });
        REQUIRE( region.sequenceLength == 100 );
        region.parseOpcode({ "seq_length", "-1" });
        REQUIRE( region.sequenceLength == 1 );
    }

    SECTION("seq_position")
    {
        REQUIRE( region.sequencePosition == 1 );
        region.parseOpcode({ "seq_position", "89" });
        REQUIRE( region.sequencePosition == 89 );
        region.parseOpcode({ "seq_position", "189" });
        REQUIRE( region.sequencePosition == 100 );
        region.parseOpcode({ "seq_position", "-1" });
        REQUIRE( region.sequencePosition == 1 );
    }

    SECTION("trigger")
    {
        REQUIRE( region.trigger == SfzTrigger::attack );
        region.parseOpcode({ "trigger", "attack" });
        REQUIRE( region.trigger == SfzTrigger::attack );
        region.parseOpcode({ "trigger", "release" });
        REQUIRE( region.trigger == SfzTrigger::release );
        region.parseOpcode({ "trigger", "first" });
        REQUIRE( region.trigger == SfzTrigger::first );
        region.parseOpcode({ "trigger", "legato" });
        REQUIRE( region.trigger == SfzTrigger::legato );
    }

    SECTION("on_locc, on_hicc")
    {
        for (int ccIdx = 1; ccIdx < 128; ++ccIdx)
        {
            REQUIRE( !region.ccTriggers.contains(ccIdx) );
        }
        region.parseOpcode({ "on_locc45", "15" });
        REQUIRE( region.ccTriggers.contains(45) );
        REQUIRE( region.ccTriggers[45] == Range<uint8_t>(15, 127) );
        region.parseOpcode({ "on_hicc4", "47" });
        REQUIRE( region.ccTriggers.contains(45) );
        REQUIRE( region.ccTriggers[4] == Range<uint8_t>(0, 47) );
    }

    SECTION("volume")
    {
        REQUIRE( region.volume == 0.0f );
        region.parseOpcode({ "volume", "4.2" });
        REQUIRE( region.volume == 4.2f );
        region.parseOpcode({ "volume", "-4.2" });
        REQUIRE( region.volume == -4.2f );
        region.parseOpcode({ "volume", "-123" });
        REQUIRE( region.volume == -123.0f );
        region.parseOpcode({ "volume", "-185" });
        REQUIRE( region.volume == -144.0f );
        region.parseOpcode({ "volume", "19" });
        REQUIRE( region.volume == 6.0f );
    }

    SECTION("pan")
    {
        REQUIRE( region.pan == 0.0f );
        region.parseOpcode({ "pan", "4.2" });
        REQUIRE( region.pan == 4.2f );
        region.parseOpcode({ "pan", "-4.2" });
        REQUIRE( region.pan == -4.2f );
        region.parseOpcode({ "pan", "-123" });
        REQUIRE( region.pan == -100.0f );
        region.parseOpcode({ "pan", "132" });
        REQUIRE( region.pan == 100.0f );
    }

    SECTION("pan_oncc")
    {
        REQUIRE( !region.panCC );
        region.parseOpcode({ "pan_oncc45", "4.2" });
        REQUIRE( region.panCC );
        REQUIRE( region.panCC->first == 45 );
        REQUIRE( region.panCC->second == 4.2f );
    }

    SECTION("width")
    {
        REQUIRE( region.width == 0.0f );
        region.parseOpcode({ "width", "4.2" });
        REQUIRE( region.width == 4.2f );
        region.parseOpcode({ "width", "-4.2" });
        REQUIRE( region.width == -4.2f );
        region.parseOpcode({ "width", "-123" });
        REQUIRE( region.width == -100.0f );
        region.parseOpcode({ "width", "132" });
        REQUIRE( region.width == 100.0f );
    }

    SECTION("width_oncc")
    {
        REQUIRE( !region.widthCC );
        region.parseOpcode({ "width_oncc45", "4.2" });
        REQUIRE( region.widthCC );
        REQUIRE( region.widthCC->first == 45 );
        REQUIRE( region.widthCC->second == 4.2f );
    }

    SECTION("position")
    {
        REQUIRE( region.position == 0.0f );
        region.parseOpcode({ "position", "4.2" });
        REQUIRE( region.position == 4.2f );
        region.parseOpcode({ "position", "-4.2" });
        REQUIRE( region.position == -4.2f );
        region.parseOpcode({ "position", "-123" });
        REQUIRE( region.position == -100.0f );
        region.parseOpcode({ "position", "132" });
        REQUIRE( region.position == 100.0f );
    }

    SECTION("position_oncc")
    {
        REQUIRE( !region.positionCC );
        region.parseOpcode({ "position_oncc45", "4.2" });
        REQUIRE( region.positionCC );
        REQUIRE( region.positionCC->first == 45 );
        REQUIRE( region.positionCC->second == 4.2f );
    }

    SECTION("amp_keycenter")
    {
        REQUIRE( region.ampKeycenter == 60 );
        region.parseOpcode({ "amp_keycenter", "40" });
        REQUIRE( region.ampKeycenter == 40 );
        region.parseOpcode({ "amp_keycenter", "-1" });
        REQUIRE( region.ampKeycenter == 0 );
        region.parseOpcode({ "amp_keycenter", "132" });
        REQUIRE( region.ampKeycenter == 127 );
    }

    SECTION("amp_keytrack")
    {
        REQUIRE( region.ampKeytrack == 0.0f );
        region.parseOpcode({ "amp_keytrack", "4.2" });
        REQUIRE( region.ampKeytrack == 4.2f );
        region.parseOpcode({ "amp_keytrack", "-4.2" });
        REQUIRE( region.ampKeytrack == -4.2f );
        region.parseOpcode({ "amp_keytrack", "-123" });
        REQUIRE( region.ampKeytrack == -96.0f );
        region.parseOpcode({ "amp_keytrack", "132" });
        REQUIRE( region.ampKeytrack == 12.0f );
    }

    SECTION("amp_veltrack")
    {
        REQUIRE( region.ampVeltrack == 100.0f );
        region.parseOpcode({ "amp_veltrack", "4.2" });
        REQUIRE( region.ampVeltrack == 4.2f );
        region.parseOpcode({ "amp_veltrack", "-4.2" });
        REQUIRE( region.ampVeltrack == -4.2f );
        region.parseOpcode({ "amp_veltrack", "-123" });
        REQUIRE( region.ampVeltrack == -100.0f );
        region.parseOpcode({ "amp_veltrack", "132" });
        REQUIRE( region.ampVeltrack == 100.0f );
    }

    SECTION("amp_random")
    {
        REQUIRE( region.ampRandom == 0.0f );
        region.parseOpcode({ "amp_random", "4.2" });
        REQUIRE( region.ampRandom == 4.2f );
        region.parseOpcode({ "amp_random", "-4.2" });
        REQUIRE( region.ampRandom == 0.0f );
        region.parseOpcode({ "amp_random", "132" });
        REQUIRE( region.ampRandom == 24.0f );
    }

    SECTION("amp_velcurve")
    {
        region.parseOpcode({ "amp_velcurve_6", "0.4" });
        REQUIRE( region.velocityPoints.back() == std::make_pair<int, float>(6, 0.4f) );
        region.parseOpcode({ "amp_velcurve_127", "-1.0" });
        REQUIRE( region.velocityPoints.back() == std::make_pair<int, float>(127, 0.0f) );
    }

    SECTION("pitch_keycenter")
    {
        REQUIRE( region.pitchKeycenter == 60 );
        region.parseOpcode({ "pitch_keycenter", "40" });
        REQUIRE( region.pitchKeycenter == 40 );
        region.parseOpcode({ "pitch_keycenter", "-1" });
        REQUIRE( region.pitchKeycenter == 0 );
        region.parseOpcode({ "pitch_keycenter", "132" });
        REQUIRE( region.pitchKeycenter == 127 );
    }

    SECTION("pitch_keytrack")
    {
        REQUIRE( region.pitchKeytrack == 100 );
        region.parseOpcode({ "pitch_keytrack", "40" });
        REQUIRE( region.pitchKeytrack == 40 );
        region.parseOpcode({ "pitch_keytrack", "-1" });
        REQUIRE( region.pitchKeytrack == -1 );
        region.parseOpcode({ "pitch_keytrack", "1320" });
        REQUIRE( region.pitchKeytrack == 1200 );
        region.parseOpcode({ "pitch_keytrack", "-1320" });
        REQUIRE( region.pitchKeytrack == -1200 );
    }

    SECTION("pitch_random")
    {
        REQUIRE( region.pitchRandom == 0 );
        region.parseOpcode({ "pitch_random", "40" });
        REQUIRE( region.pitchRandom == 40 );
        region.parseOpcode({ "pitch_random", "-1" });
        REQUIRE( region.pitchRandom == 0 );
        region.parseOpcode({ "pitch_random", "10320" });
        REQUIRE( region.pitchRandom == 9600 );
    }

    SECTION("pitch_veltrack")
    {
        REQUIRE( region.pitchVeltrack == 0 );
        region.parseOpcode({ "pitch_veltrack", "40" });
        REQUIRE( region.pitchVeltrack == 40 );
        region.parseOpcode({ "pitch_veltrack", "-1" });
        REQUIRE( region.pitchVeltrack == -1 );
        region.parseOpcode({ "pitch_veltrack", "13020" });
        REQUIRE( region.pitchVeltrack == 9600 );
        region.parseOpcode({ "pitch_veltrack", "-13020" });
        REQUIRE( region.pitchVeltrack == -9600 );
    }

    SECTION("transpose")
    {
        REQUIRE( region.transpose == 0 );
        region.parseOpcode({ "transpose", "40" });
        REQUIRE( region.transpose == 40 );
        region.parseOpcode({ "transpose", "-1" });
        REQUIRE( region.transpose == -1 );
        region.parseOpcode({ "transpose", "154" });
        REQUIRE( region.transpose == 127 );
        region.parseOpcode({ "transpose", "-154" });
        REQUIRE( region.transpose == -127 );
    }

    SECTION("tune")
    {
        REQUIRE( region.tune == 0 );
        region.parseOpcode({ "tune", "40" });
        REQUIRE( region.tune == 40 );
        region.parseOpcode({ "tune", "-1" });
        REQUIRE( region.tune == -1 );
        region.parseOpcode({ "tune", "154" });
        REQUIRE( region.tune == 100 );
        region.parseOpcode({ "tune", "-154" });
        REQUIRE( region.tune == -100 );
    }

    SECTION("ampeg")
    {
        // Defaults
        REQUIRE( region.amplitudeEG.attack == 0.0f );
        REQUIRE( region.amplitudeEG.decay == 0.0f );
        REQUIRE( region.amplitudeEG.delay == 0.0f );
        REQUIRE( region.amplitudeEG.hold == 0.0f );
        REQUIRE( region.amplitudeEG.release == 0.0f );
        REQUIRE( region.amplitudeEG.start == 0.0f );
        REQUIRE( region.amplitudeEG.sustain == 100.0f );
        REQUIRE( region.amplitudeEG.depth == 0 );
        REQUIRE( region.amplitudeEG.vel2attack == 0.0f );
        REQUIRE( region.amplitudeEG.vel2decay == 0.0f );
        REQUIRE( region.amplitudeEG.vel2delay == 0.0f );
        REQUIRE( region.amplitudeEG.vel2hold == 0.0f );
        REQUIRE( region.amplitudeEG.vel2release == 0.0f );
        REQUIRE( region.amplitudeEG.vel2sustain == 0.0f );
        REQUIRE( region.amplitudeEG.vel2depth == 0 );
        //
        region.parseOpcode({ "ampeg_attack", "1" });
        region.parseOpcode({ "ampeg_decay", "2" });
        region.parseOpcode({ "ampeg_delay", "3" });
        region.parseOpcode({ "ampeg_hold", "4" });
        region.parseOpcode({ "ampeg_release", "5" });
        region.parseOpcode({ "ampeg_start", "6" });
        region.parseOpcode({ "ampeg_sustain", "7" });
        region.parseOpcode({ "ampeg_depth", "8" });
        region.parseOpcode({ "ampeg_vel2attack", "9" });
        region.parseOpcode({ "ampeg_vel2decay", "10" });
        region.parseOpcode({ "ampeg_vel2delay", "11" });
        region.parseOpcode({ "ampeg_vel2hold", "12" });
        region.parseOpcode({ "ampeg_vel2release", "13" });
        region.parseOpcode({ "ampeg_vel2sustain", "14" });
        region.parseOpcode({ "ampeg_vel2depth", "15" });
        REQUIRE( region.amplitudeEG.attack == 1.0f );
        REQUIRE( region.amplitudeEG.decay == 2.0f );
        REQUIRE( region.amplitudeEG.delay == 3.0f );
        REQUIRE( region.amplitudeEG.hold == 4.0f );
        REQUIRE( region.amplitudeEG.release == 5.0f );
        REQUIRE( region.amplitudeEG.start == 6.0f );
        REQUIRE( region.amplitudeEG.sustain == 7.0f );
        REQUIRE( region.amplitudeEG.depth == 0 ); // ignored for ampeg
        REQUIRE( region.amplitudeEG.vel2attack == 9.0f );
        REQUIRE( region.amplitudeEG.vel2decay == 10.0f );
        REQUIRE( region.amplitudeEG.vel2delay == 11.0f );
        REQUIRE( region.amplitudeEG.vel2hold == 12.0f );
        REQUIRE( region.amplitudeEG.vel2release == 13.0f );
        REQUIRE( region.amplitudeEG.vel2sustain == 14.0f );
        REQUIRE( region.amplitudeEG.vel2depth == 0 ); // ignored for ampeg
        //
        region.parseOpcode({ "ampeg_attack", "1000" });
        region.parseOpcode({ "ampeg_decay", "1000" });
        region.parseOpcode({ "ampeg_delay", "1000" });
        region.parseOpcode({ "ampeg_hold", "1000" });
        region.parseOpcode({ "ampeg_release", "1000" });
        region.parseOpcode({ "ampeg_start", "1000" });
        region.parseOpcode({ "ampeg_sustain", "1000" });
        region.parseOpcode({ "ampeg_depth", "1000" });
        region.parseOpcode({ "ampeg_vel2attack", "1000" });
        region.parseOpcode({ "ampeg_vel2decay", "1000" });
        region.parseOpcode({ "ampeg_vel2delay", "1000" });
        region.parseOpcode({ "ampeg_vel2hold", "1000" });
        region.parseOpcode({ "ampeg_vel2release", "1000" });
        region.parseOpcode({ "ampeg_vel2sustain", "1000" });
        region.parseOpcode({ "ampeg_vel2depth", "1000" });
        REQUIRE( region.amplitudeEG.attack == 100.0f );
        REQUIRE( region.amplitudeEG.decay == 100.0f );
        REQUIRE( region.amplitudeEG.delay == 100.0f );
        REQUIRE( region.amplitudeEG.hold == 100.0f );
        REQUIRE( region.amplitudeEG.release == 100.0f );
        REQUIRE( region.amplitudeEG.start == 100.0f );
        REQUIRE( region.amplitudeEG.sustain == 100.0f );
        REQUIRE( region.amplitudeEG.depth == 0 ); // ignored for ampeg
        REQUIRE( region.amplitudeEG.vel2attack == 100.0f );
        REQUIRE( region.amplitudeEG.vel2decay == 100.0f );
        REQUIRE( region.amplitudeEG.vel2delay == 100.0f );
        REQUIRE( region.amplitudeEG.vel2hold == 100.0f );
        REQUIRE( region.amplitudeEG.vel2release == 100.0f );
        REQUIRE( region.amplitudeEG.vel2sustain == 100.0f );
        REQUIRE( region.amplitudeEG.vel2depth == 0 ); // ignored for ampeg
        //
        region.parseOpcode({ "ampeg_attack", "-101" });
        region.parseOpcode({ "ampeg_decay", "-101" });
        region.parseOpcode({ "ampeg_delay", "-101" });
        region.parseOpcode({ "ampeg_hold", "-101" });
        region.parseOpcode({ "ampeg_release", "-101" });
        region.parseOpcode({ "ampeg_start", "-101" });
        region.parseOpcode({ "ampeg_sustain", "-101" });
        region.parseOpcode({ "ampeg_depth", "-101" });
        region.parseOpcode({ "ampeg_vel2attack", "-101" });
        region.parseOpcode({ "ampeg_vel2decay", "-101" });
        region.parseOpcode({ "ampeg_vel2delay", "-101" });
        region.parseOpcode({ "ampeg_vel2hold", "-101" });
        region.parseOpcode({ "ampeg_vel2release", "-101" });
        region.parseOpcode({ "ampeg_vel2sustain", "-101" });
        region.parseOpcode({ "ampeg_vel2depth", "-101" });
        REQUIRE( region.amplitudeEG.attack == 0.0f );
        REQUIRE( region.amplitudeEG.decay == 0.0f );
        REQUIRE( region.amplitudeEG.delay == 0.0f );
        REQUIRE( region.amplitudeEG.hold == 0.0f );
        REQUIRE( region.amplitudeEG.release == 0.0f );
        REQUIRE( region.amplitudeEG.start == 0.0f );
        REQUIRE( region.amplitudeEG.sustain == 0.0f );
        REQUIRE( region.amplitudeEG.depth == 0 ); // ignored for ampeg
        REQUIRE( region.amplitudeEG.vel2attack == -100.0f );
        REQUIRE( region.amplitudeEG.vel2decay == -100.0f );
        REQUIRE( region.amplitudeEG.vel2delay == -100.0f );
        REQUIRE( region.amplitudeEG.vel2hold == -100.0f );
        REQUIRE( region.amplitudeEG.vel2release == -100.0f );
        REQUIRE( region.amplitudeEG.vel2sustain == -100.0f );
    }

    SECTION("ampeg_XX_onccNN")
    {
        // Defaults
        REQUIRE( !region.amplitudeEG.ccAttack );
        REQUIRE( !region.amplitudeEG.ccDecay );
        REQUIRE( !region.amplitudeEG.ccDelay );
        REQUIRE( !region.amplitudeEG.ccHold );
        REQUIRE( !region.amplitudeEG.ccRelease );
        REQUIRE( !region.amplitudeEG.ccStart );
        REQUIRE( !region.amplitudeEG.ccSustain );
        //
        region.parseOpcode({ "ampeg_attack_oncc1", "1" });
        region.parseOpcode({ "ampeg_decay_oncc2", "2" });
        region.parseOpcode({ "ampeg_delay_oncc3", "3" });
        region.parseOpcode({ "ampeg_hold_oncc4", "4" });
        region.parseOpcode({ "ampeg_release_oncc5", "5" });
        region.parseOpcode({ "ampeg_start_oncc6", "6" });
        region.parseOpcode({ "ampeg_sustain_oncc7", "7" });
        REQUIRE( region.amplitudeEG.ccAttack );
        REQUIRE( region.amplitudeEG.ccDecay );
        REQUIRE( region.amplitudeEG.ccDelay );
        REQUIRE( region.amplitudeEG.ccHold );
        REQUIRE( region.amplitudeEG.ccRelease );
        REQUIRE( region.amplitudeEG.ccStart );
        REQUIRE( region.amplitudeEG.ccSustain );
        REQUIRE( region.amplitudeEG.ccAttack->first == 1 );
        REQUIRE( region.amplitudeEG.ccDecay->first == 2 );
        REQUIRE( region.amplitudeEG.ccDelay->first == 3 );
        REQUIRE( region.amplitudeEG.ccHold->first == 4 );
        REQUIRE( region.amplitudeEG.ccRelease->first == 5 );
        REQUIRE( region.amplitudeEG.ccStart->first == 6 );
        REQUIRE( region.amplitudeEG.ccSustain->first == 7 );
        REQUIRE( region.amplitudeEG.ccAttack->second == 1.0f );
        REQUIRE( region.amplitudeEG.ccDecay->second == 2.0f );
        REQUIRE( region.amplitudeEG.ccDelay->second == 3.0f );
        REQUIRE( region.amplitudeEG.ccHold->second == 4.0f );
        REQUIRE( region.amplitudeEG.ccRelease->second == 5.0f );
        REQUIRE( region.amplitudeEG.ccStart->second == 6.0f );
        REQUIRE( region.amplitudeEG.ccSustain->second == 7.0f );
        //
        region.parseOpcode({ "ampeg_attack_oncc1", "101" });
        region.parseOpcode({ "ampeg_decay_oncc2", "101" });
        region.parseOpcode({ "ampeg_delay_oncc3", "101" });
        region.parseOpcode({ "ampeg_hold_oncc4", "101" });
        region.parseOpcode({ "ampeg_release_oncc5", "101" });
        region.parseOpcode({ "ampeg_start_oncc6", "101" });
        region.parseOpcode({ "ampeg_sustain_oncc7", "101" });
        REQUIRE( region.amplitudeEG.ccAttack->second == 100.0f );
        REQUIRE( region.amplitudeEG.ccDecay->second == 100.0f );
        REQUIRE( region.amplitudeEG.ccDelay->second == 100.0f );
        REQUIRE( region.amplitudeEG.ccHold->second == 100.0f );
        REQUIRE( region.amplitudeEG.ccRelease->second == 100.0f );
        REQUIRE( region.amplitudeEG.ccStart->second == 100.0f );
        REQUIRE( region.amplitudeEG.ccSustain->second == 100.0f ); 
        //
        region.parseOpcode({ "ampeg_attack_oncc1", "-101" });
        region.parseOpcode({ "ampeg_decay_oncc2", "-101" });
        region.parseOpcode({ "ampeg_delay_oncc3", "-101" });
        region.parseOpcode({ "ampeg_hold_oncc4", "-101" });
        region.parseOpcode({ "ampeg_release_oncc5", "-101" });
        region.parseOpcode({ "ampeg_start_oncc6", "-101" });
        region.parseOpcode({ "ampeg_sustain_oncc7", "-101" });
        REQUIRE( region.amplitudeEG.ccAttack->second == -100.0f );
        REQUIRE( region.amplitudeEG.ccDecay->second == -100.0f );
        REQUIRE( region.amplitudeEG.ccDelay->second == -100.0f );
        REQUIRE( region.amplitudeEG.ccHold->second == -100.0f );
        REQUIRE( region.amplitudeEG.ccRelease->second == -100.0f );
        REQUIRE( region.amplitudeEG.ccStart->second == -100.0f );
        REQUIRE( region.amplitudeEG.ccSustain->second == -100.0f ); 
    }
}