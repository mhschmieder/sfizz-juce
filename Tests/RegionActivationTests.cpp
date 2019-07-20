#include "../JuceLibraryCode/JuceHeader.h"
#include "catch2/catch.hpp"
#include "../Source/SfzRegion.h"
using namespace Catch::literals;

TEST_CASE("Region activation", "Region tests")
{
    SfzFilePool openFiles { File::getCurrentWorkingDirectory() };
    SfzRegion region { File::getCurrentWorkingDirectory(), openFiles };
    region.parseOpcode({ "sample", "*sine" });
    SECTION("Basic state")
    {
        REQUIRE( region.prepare() );
        REQUIRE( region.isSwitchedOn() );
    }

    SECTION("Single CC range")
    {
        region.parseOpcode({ "locc4", "56" });
        region.parseOpcode({ "hicc4", "59" });
        REQUIRE( region.prepare() );
        REQUIRE( !region.isSwitchedOn() );
        region.registerCC(1, 4, 57);
        REQUIRE( region.isSwitchedOn() );
        region.registerCC(1, 4, 56);
        REQUIRE( region.isSwitchedOn() );
        region.registerCC(1, 4, 59);
        REQUIRE( region.isSwitchedOn() );
        region.registerCC(1, 4, 43);
        REQUIRE( !region.isSwitchedOn() );
        region.registerCC(1, 4, 65);
        REQUIRE( !region.isSwitchedOn() );
        region.registerCC(1, 6, 57);
        REQUIRE( !region.isSwitchedOn() );
    }

    SECTION("Multiple CC ranges")
    {
        region.parseOpcode({ "locc4", "56" });
        region.parseOpcode({ "hicc4", "59" });
        region.parseOpcode({ "locc54", "18" });
        region.parseOpcode({ "hicc54", "27" });
        REQUIRE( region.prepare() );
        REQUIRE( !region.isSwitchedOn() );
        region.registerCC(1, 4, 57);
        REQUIRE( !region.isSwitchedOn() );
        region.registerCC(1, 54, 19);
        REQUIRE( region.isSwitchedOn() );
        region.registerCC(1, 54, 18);
        REQUIRE( region.isSwitchedOn() );
        region.registerCC(1, 54, 27);
        REQUIRE( region.isSwitchedOn() );
        region.registerCC(1, 4, 56);
        REQUIRE( region.isSwitchedOn() );
        region.registerCC(1, 4, 59);
        REQUIRE( region.isSwitchedOn() );
        region.registerCC(1, 54, 2);
        REQUIRE( !region.isSwitchedOn() );
        region.registerCC(1, 54, 26);
        REQUIRE( region.isSwitchedOn() );
        region.registerCC(1, 4, 65);
        REQUIRE( !region.isSwitchedOn() );
    }

    SECTION("Bend ranges")
    {
        region.parseOpcode({ "lobend", "56" });
        region.parseOpcode({ "hibend", "243" });
        REQUIRE( region.prepare() );
        REQUIRE( !region.isSwitchedOn() );
        region.registerPitchWheel(1, 56);
        REQUIRE( region.isSwitchedOn() );
        region.registerPitchWheel(1, 243);
        REQUIRE( region.isSwitchedOn() );
        region.registerPitchWheel(1, 245);
        REQUIRE( !region.isSwitchedOn() );
    }

    SECTION("Aftertouch ranges")
    {
        region.parseOpcode({ "lochanaft", "56" });
        region.parseOpcode({ "hichanaft", "68" });
        REQUIRE( region.prepare() );
        REQUIRE( !region.isSwitchedOn() );
        region.registerAftertouch(1, 56);
        REQUIRE( region.isSwitchedOn() );
        region.registerAftertouch(1, 68);
        REQUIRE( region.isSwitchedOn() );
        region.registerAftertouch(1, 98);
        REQUIRE( !region.isSwitchedOn() );
    }

    SECTION("BPM ranges")
    {
        region.parseOpcode({ "lobpm", "56" });
        region.parseOpcode({ "hibpm", "68" });
        REQUIRE( region.prepare() );
        REQUIRE( !region.isSwitchedOn() );
        region.registerTempo(1070000);
        REQUIRE( region.isSwitchedOn() );
        region.registerTempo(882354);
        REQUIRE( region.isSwitchedOn() );
        region.registerTempo(132314);
        REQUIRE( !region.isSwitchedOn() );
    }

    // TODO: add keyswitches
    SECTION("Keyswitches: sw_last")
    {
        region.parseOpcode({ "sw_last", "40" });
        REQUIRE( region.prepare() );
        REQUIRE( !region.isSwitchedOn() );
        region.registerNoteOn(1, 40, 64, 0.5f);
        REQUIRE( region.isSwitchedOn() );
        region.registerNoteOff(1, 40, 64, 0.5f);
        REQUIRE( region.isSwitchedOn() );
        region.registerNoteOn(1, 41, 64, 0.5f);
        REQUIRE( !region.isSwitchedOn() );
        region.registerNoteOff(1, 41, 0, 0.5f);
    }

    SECTION("Keyswitches: sw_last with non-default keyswitch range")
    {
        region.parseOpcode({ "sw_lokey", "30" });
        region.parseOpcode({ "sw_hikey", "50" });
        region.parseOpcode({ "sw_last", "40" });
        REQUIRE( region.prepare() );
        REQUIRE( !region.isSwitchedOn() );
        region.registerNoteOn(1, 60, 64, 0.5f);
        REQUIRE( !region.isSwitchedOn() );
        region.registerNoteOff(1, 60, 0, 0.5f);
        REQUIRE( !region.isSwitchedOn() );
        region.registerNoteOn(1, 40, 64, 0.5f);
        REQUIRE( region.isSwitchedOn() );
        region.registerNoteOff(1, 40, 0, 0.5f);
        REQUIRE( region.isSwitchedOn() );
        region.registerNoteOn(1, 60, 64, 0.5f);
        REQUIRE( region.isSwitchedOn() );
        region.registerNoteOff(1, 60, 0, 0.5f);
        region.registerNoteOn(1, 41, 64, 0.5f);
        REQUIRE( !region.isSwitchedOn() );
        region.registerNoteOff(1, 41, 0, 0.5f);
    }

    SECTION("Keyswitches: sw_down with non-default keyswitch range")
    {
        region.parseOpcode({ "sw_lokey", "30" });
        region.parseOpcode({ "sw_hikey", "50" });
        region.parseOpcode({ "sw_down", "40" });
        REQUIRE( region.prepare() );
        REQUIRE( !region.isSwitchedOn() );
        region.registerNoteOn(1, 60, 64, 0.5f);
        REQUIRE( !region.isSwitchedOn() );
        region.registerNoteOff(1, 60, 0, 0.5f);
        REQUIRE( !region.isSwitchedOn() );
        region.registerNoteOn(1, 40, 64, 0.5f);
        REQUIRE( region.isSwitchedOn() );
        region.registerNoteOff(1, 40, 0, 0.5f);
        REQUIRE( !region.isSwitchedOn() );
        region.registerNoteOn(1, 60, 64, 0.5f);
        REQUIRE( !region.isSwitchedOn() );
        region.registerNoteOff(1, 60, 0, 0.5f);
        region.registerNoteOn(1, 41, 64, 0.5f);
        REQUIRE( !region.isSwitchedOn() );
        region.registerNoteOff(1, 41, 0, 0.5f);
    }

    SECTION("Keyswitches: sw_down with non-default keyswitch range")
    {
        region.parseOpcode({ "sw_lokey", "30" });
        region.parseOpcode({ "sw_hikey", "50" });
        region.parseOpcode({ "sw_up", "40" });
        REQUIRE( region.prepare() );
        REQUIRE( region.isSwitchedOn() );
        region.registerNoteOn(1, 40, 64, 0.5f);
        REQUIRE( !region.isSwitchedOn() );
        region.registerNoteOff(1, 40, 0, 0.5f);
        REQUIRE( region.isSwitchedOn() );
        region.registerNoteOn(1, 41, 64, 0.5f);
        REQUIRE( region.isSwitchedOn() );
        region.registerNoteOn(1, 40, 64, 0.5f);
        REQUIRE( !region.isSwitchedOn() );
        region.registerNoteOff(1, 40, 0, 0.5f);
        region.registerNoteOff(1, 41, 0, 0.5f);
        REQUIRE( region.isSwitchedOn() );
    }

    SECTION("Keyswitches: sw_previous")
    {
        region.parseOpcode({ "sw_previous", "40" });
        REQUIRE( region.prepare() );
        REQUIRE( !region.isSwitchedOn() );
        region.registerNoteOn(1, 40, 64, 0.5f);
        REQUIRE( region.isSwitchedOn() );
        region.registerNoteOff(1, 40, 0, 0.5f);
        REQUIRE( region.isSwitchedOn() );
        region.registerNoteOn(1, 41, 64, 0.5f);
        REQUIRE( !region.isSwitchedOn() );
        region.registerNoteOn(1, 40, 64, 0.5f);
        REQUIRE( region.isSwitchedOn() );
        region.registerNoteOff(1, 40, 0, 0.5f);
        region.registerNoteOff(1, 41, 0, 0.5f);
        REQUIRE( region.isSwitchedOn() );
        region.registerNoteOn(1, 41, 64, 0.5f);
        REQUIRE( !region.isSwitchedOn() );
        region.registerNoteOff(1, 41, 0, 0.5f);
        REQUIRE( !region.isSwitchedOn() );
    }
    
    SECTION("Sequences: length 2, default position")
    {
        region.parseOpcode({ "seq_length", "2" });
        region.parseOpcode({ "seq_position", "1" });
        region.parseOpcode({ "key", "40" });
        REQUIRE( region.prepare() );
        REQUIRE( region.isSwitchedOn() );
        region.registerNoteOn(1, 40, 64, 0.5f);
        REQUIRE( !region.isSwitchedOn() );
        region.registerNoteOff(1, 40, 0, 0.5f);
        REQUIRE( !region.isSwitchedOn() );
        region.registerNoteOn(1, 40, 64, 0.5f);
        REQUIRE( region.isSwitchedOn() );
        region.registerNoteOff(1, 40, 0, 0.5f);
        REQUIRE( region.isSwitchedOn() );
        region.registerNoteOn(1, 40, 64, 0.5f);
        REQUIRE( !region.isSwitchedOn() );
        region.registerNoteOff(1, 40, 0, 0.5f);
        REQUIRE( !region.isSwitchedOn() );
 
    }
    SECTION("Sequences: length 2, position 2")
    {
        region.parseOpcode({ "seq_length", "2" });
        region.parseOpcode({ "seq_position", "2" });
        region.parseOpcode({ "key", "40" });
        REQUIRE( region.prepare() );
        REQUIRE( !region.isSwitchedOn() );
        region.registerNoteOn(1, 40, 64, 0.5f);
        REQUIRE( region.isSwitchedOn() );
        region.registerNoteOff(1, 40, 0, 0.5f);
        REQUIRE( region.isSwitchedOn() );
        region.registerNoteOn(1, 40, 64, 0.5f);
        REQUIRE( !region.isSwitchedOn() );
        region.registerNoteOff(1, 40, 0, 0.5f);
        REQUIRE( !region.isSwitchedOn() );
        region.registerNoteOn(1, 40, 64, 0.5f);
        REQUIRE( region.isSwitchedOn() );
        region.registerNoteOff(1, 40, 0, 0.5f);
        REQUIRE( region.isSwitchedOn() );
 
    }
    SECTION("Sequences: length 3, position 2")
    {
        region.parseOpcode({ "seq_length", "3" });
        region.parseOpcode({ "seq_position", "2" });
        region.parseOpcode({ "key", "40" });
        REQUIRE( region.prepare() );
        REQUIRE( !region.isSwitchedOn() );
        region.registerNoteOn(1, 40, 64, 0.5f);
        REQUIRE( region.isSwitchedOn() );
        region.registerNoteOff(1, 40, 0, 0.5f);
        REQUIRE( region.isSwitchedOn() );
        region.registerNoteOn(1, 40, 64, 0.5f);
        REQUIRE( !region.isSwitchedOn() );
        region.registerNoteOff(1, 40, 0, 0.5f);
        REQUIRE( !region.isSwitchedOn() );
        region.registerNoteOn(1, 40, 64, 0.5f);
        REQUIRE( !region.isSwitchedOn() );
        region.registerNoteOff(1, 40, 0, 0.5f);
        REQUIRE( !region.isSwitchedOn() );
        region.registerNoteOn(1, 40, 64, 0.5f);
        REQUIRE( region.isSwitchedOn() );
        region.registerNoteOff(1, 40, 0, 0.5f);
        REQUIRE( region.isSwitchedOn() ); 
    }
}

