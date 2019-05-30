#include "../JuceLibraryCode/JuceHeader.h"
#include "catch2/catch.hpp"
#include "../Source/SfzRegion.h"
#include "../Source/SfzSynth.h"
using namespace Catch::literals;

TEST_CASE("Basic regions", "File tests")
{
    SECTION("Single region (regions_one.sfz)")
    {
        SfzSynth synth;
        synth.loadSfzFile(File::getCurrentWorkingDirectory().getChildFile("Tests/TestFiles/Regions/regions_one.sfz"));
        REQUIRE( synth.getNumRegions() == 1 );
        REQUIRE( synth.getRegionView(0)->sample == "dummy.wav" );
    }

    SECTION("Multiple regions (regions_many.sfz)")
    {
        SfzSynth synth;
        synth.loadSfzFile(File::getCurrentWorkingDirectory().getChildFile("Tests/TestFiles/Regions/regions_many.sfz"));
        REQUIRE( synth.getNumRegions() == 3 );
        REQUIRE( synth.getRegionView(0)->sample == "dummy.wav" );
        REQUIRE( synth.getRegionView(1)->sample == "dummy.1.wav" );
        REQUIRE( synth.getRegionView(2)->sample == "dummy.2.wav" );
    }

    SECTION("Basic opcodes (regions_opcodes.sfz)")
    {
        SfzSynth synth;
        synth.loadSfzFile(File::getCurrentWorkingDirectory().getChildFile("Tests/TestFiles/Regions/regions_opcodes.sfz"));
        REQUIRE( synth.getNumRegions() == 1 );
        REQUIRE( synth.getRegionView(0)->channelRange == Range<uint8_t>(2, 14) );  
    }

    SECTION("Underscore opcodes (underscore_opcodes.sfz)")
    {
        SfzSynth synth;
        synth.loadSfzFile(File::getCurrentWorkingDirectory().getChildFile("Tests/TestFiles/Regions/underscore_opcodes.sfz"));
        REQUIRE( synth.getNumRegions() == 1 );
        REQUIRE( synth.getRegionView(0)->loopMode == SfzLoopMode::loop_sustain );  
    }
}

TEST_CASE("Includes", "File tests")
{
    SECTION("Local include")
    {
        SfzSynth synth;
        synth.loadSfzFile(File::getCurrentWorkingDirectory().getChildFile("Tests/TestFiles/Includes/root_local.sfz"));
        REQUIRE( synth.getNumRegions() == 1 );
        REQUIRE( synth.getRegionView(0)->sample == "dummy.wav" );
    }

    SECTION("Subdir include")
    {
        SfzSynth synth;
        synth.loadSfzFile(File::getCurrentWorkingDirectory().getChildFile("Tests/TestFiles/Includes/root_subdir.sfz"));
        REQUIRE( synth.getNumRegions() == 1 );
        REQUIRE( synth.getRegionView(0)->sample == "dummy_subdir.wav" );
    }

    SECTION("Recursive include")
    {
        SfzSynth synth;
        synth.loadSfzFile(File::getCurrentWorkingDirectory().getChildFile("Tests/TestFiles/Includes/root_recursive.sfz"));
        REQUIRE( synth.getNumRegions() == 2 );
        REQUIRE( synth.getRegionView(0)->sample == "dummy_recursive2.wav" );
        REQUIRE( synth.getRegionView(1)->sample == "dummy_recursive1.wav" );
    }

    SECTION("Include loops")
    {
        SfzSynth synth;
        synth.loadSfzFile(File::getCurrentWorkingDirectory().getChildFile("Tests/TestFiles/Includes/root_loop.sfz"));
        REQUIRE( synth.getNumRegions() == 2 );
        REQUIRE( synth.getRegionView(0)->sample == "dummy_loop2.wav" );
        REQUIRE( synth.getRegionView(1)->sample == "dummy_loop1.wav" );
    }
}

TEST_CASE("Defines", "File tests")
{
    SECTION("Define test")
    {
        SfzSynth synth;
        synth.loadSfzFile(File::getCurrentWorkingDirectory().getChildFile("Tests/TestFiles/defines.sfz"));
        REQUIRE( synth.getNumRegions() == 3 );
        REQUIRE( synth.getRegionView(0)->keyRange == Range<uint8_t>(36, 36) );
        REQUIRE( synth.getRegionView(1)->keyRange == Range<uint8_t>(38, 38) );
        REQUIRE( synth.getRegionView(2)->keyRange == Range<uint8_t>(42, 42) );
    }
}

TEST_CASE("Header hierarchy", "File tests")
{
    
    SECTION("Group from AVL")
    {
        SfzSynth synth;
        synth.loadSfzFile(File::getCurrentWorkingDirectory().getChildFile("Tests/TestFiles/groups_avl.sfz"));
        REQUIRE( synth.getNumRegions() == 5 );
        for (int i = 0; i < synth.getNumRegions(); ++i)
        {
            REQUIRE( synth.getRegionView(i)->volume == 6.0f );
            REQUIRE( synth.getRegionView(i)->keyRange == Range<uint8_t>(36, 36) );
        }
        REQUIRE( synth.getRegionView(0)->velocityRange == Range<uint8_t>(1, 26) );
        REQUIRE( synth.getRegionView(1)->velocityRange == Range<uint8_t>(27, 52) );
        REQUIRE( synth.getRegionView(2)->velocityRange == Range<uint8_t>(53, 77) );
        REQUIRE( synth.getRegionView(3)->velocityRange == Range<uint8_t>(78, 102) );
        REQUIRE( synth.getRegionView(4)->velocityRange == Range<uint8_t>(103, 127) );
    }

    SECTION("Full hierarchy")
    {
        SfzSynth synth;
        synth.loadSfzFile(File::getCurrentWorkingDirectory().getChildFile("Tests/TestFiles/basic_hierarchy.sfz"));
        REQUIRE( synth.getNumRegions() == 8 );
        for (int i = 0; i < synth.getNumRegions(); ++i)
        {
            REQUIRE( synth.getRegionView(i)->width == 40.0f );
        }
        REQUIRE( synth.getRegionView(0)->pan == 30.0f );
        REQUIRE( synth.getRegionView(0)->delay == 67 );
        REQUIRE( synth.getRegionView(0)->keyRange == Range<uint8_t>(60, 60) );

        REQUIRE( synth.getRegionView(1)->pan == 30.0f );
        REQUIRE( synth.getRegionView(1)->delay == 67 );
        REQUIRE( synth.getRegionView(1)->keyRange == Range<uint8_t>(61, 61) );

        REQUIRE( synth.getRegionView(2)->pan == 30.0f );
        REQUIRE( synth.getRegionView(2)->delay == 56 );
        REQUIRE( synth.getRegionView(2)->keyRange == Range<uint8_t>(50, 50) );

        REQUIRE( synth.getRegionView(3)->pan == 30.0f );
        REQUIRE( synth.getRegionView(3)->delay == 56 );
        REQUIRE( synth.getRegionView(3)->keyRange == Range<uint8_t>(51, 51) );

        REQUIRE( synth.getRegionView(4)->pan == -10.0f );
        REQUIRE( synth.getRegionView(4)->delay == 47 );
        REQUIRE( synth.getRegionView(4)->keyRange == Range<uint8_t>(40, 40) );

        REQUIRE( synth.getRegionView(5)->pan == -10.0f );
        REQUIRE( synth.getRegionView(5)->delay == 47 );
        REQUIRE( synth.getRegionView(5)->keyRange == Range<uint8_t>(41, 41) );

        REQUIRE( synth.getRegionView(6)->pan == -10.0f );
        REQUIRE( synth.getRegionView(6)->delay == 36 );
        REQUIRE( synth.getRegionView(6)->keyRange == Range<uint8_t>(30, 30) );

        REQUIRE( synth.getRegionView(7)->pan == -10.0f );
        REQUIRE( synth.getRegionView(7)->delay == 36 );
        REQUIRE( synth.getRegionView(7)->keyRange == Range<uint8_t>(31, 31) );
    }
}

TEST_CASE("MeatBass", "File tests")
{
    SECTION("Pizz basic")
    {
        SfzSynth synth;
        synth.loadSfzFile(File::getCurrentWorkingDirectory().getChildFile("Tests/TestFiles/SpecificBugs/MeatBassPizz/Programs/pizz.sfz"));
        REQUIRE( synth.getNumRegions() == 4 );
        for (int i = 0; i < synth.getNumRegions(); ++i)
        {
            REQUIRE( synth.getRegionView(i)->keyRange == Range<uint8_t>(12, 22) );
            REQUIRE( synth.getRegionView(i)->velocityRange == Range<uint8_t>(97, 127) );
            REQUIRE( synth.getRegionView(i)->pitchKeycenter == 21 );
            REQUIRE( synth.getRegionView(i)->ccConditions.getWithDefault(107) == Range<uint8_t>(0, 13) );
        }
        REQUIRE( synth.getRegionView(0)->randRange == Range<float>(0, 0.25) );
        REQUIRE( synth.getRegionView(1)->randRange == Range<float>(0.25, 0.5) );
        REQUIRE( synth.getRegionView(2)->randRange == Range<float>(0.5, 0.75) );
        REQUIRE( synth.getRegionView(3)->randRange == Range<float>(0.75, 1.0) );
        REQUIRE( synth.getRegionView(0)->sample == R"(..\Samples\pizz\a0_vl4_rr1.wav)" );
        REQUIRE( synth.getRegionView(1)->sample == R"(..\Samples\pizz\a0_vl4_rr2.wav)" );
        REQUIRE( synth.getRegionView(2)->sample == R"(..\Samples\pizz\a0_vl4_rr3.wav)" );
        REQUIRE( synth.getRegionView(3)->sample == R"(..\Samples\pizz\a0_vl4_rr4.wav)" );
    }
}