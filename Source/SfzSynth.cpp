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

#include "SfzSynth.h"
#include <string>
#include <fstream>
#include <regex>
#include <algorithm>
#include <string_view>

using svregex_iterator = std::regex_iterator<std::string_view::const_iterator>;
using svmatch_results = std::match_results<std::string_view::const_iterator>;

SfzSynth::SfzSynth()
{
	resetMidiState();
	initalizeVoices();
}

SfzSynth::~SfzSynth()
{

}

void SfzSynth::initalizeVoices(int numVoices)
{
    voices.clear();
	for (int i = 0; i < numVoices; ++i)
	{
		auto & voice = voices.emplace_back(fileLoadingPool, filePool, ccState);
		voice.prepareToPlay(sampleRate, samplesPerBlock);
	}
}

void removeCommentOnLine(std::string_view& line)
{
	if (auto position = line.find("//"); position != line.npos)
		line.remove_suffix(line.size() - position);
}

std::string joinIntoString(const std::vector<std::string>& lines)
{
	std::size_t fullLength = 0;
	for (const auto &line : lines) 
		fullLength += line.length() + 1;

	std::string fullString;
	fullString.reserve(fullLength);
	for (const auto &line : lines)
	{
		fullString += line;
		fullString += ' ';
	}
	return std::move(fullString);
}

void SfzSynth::readSfzFile(const std::filesystem::path& fileName, std::vector<std::string>& lines) noexcept
{
	std::ifstream fileStream(fileName.c_str());
	if (!fileStream)
		return;

	svmatch_results includeMatch;
	svmatch_results defineMatch;

	std::string tmpString;
	while (std::getline(fileStream, tmpString))
	{
		std::string_view tmpView { tmpString };

		removeCommentOnLine(tmpView);
		trimView(tmpView);

		if (tmpView.empty())
			continue;

		// New #include
		if (std::regex_search(tmpView.begin(), tmpView.end(), includeMatch, SfzRegexes::includes))
		{
			auto includePath = includeMatch.str(1);
			std::replace(includePath.begin(), includePath.end(), '\\', '/');
			const auto newFile = rootDirectory / includePath;			
			auto alreadyIncluded = std::find(includedFiles.begin(), includedFiles.end(), newFile);
			if (std::filesystem::exists(newFile) && alreadyIncluded == includedFiles.end())
			{
				includedFiles.push_back(newFile);
				readSfzFile(newFile, lines);
			}
			continue;
		}

		// New #define
		if (std::regex_search(tmpView.begin(), tmpView.end(), defineMatch, SfzRegexes::defines))
		{
			defines[defineMatch.str(1)] = defineMatch.str(2);
			continue;
		}

		// Replace defined variables starting with $
		std::string newString;
		newString.reserve(tmpView.length());
		std::string::size_type lastPos = 0;
    	std::string::size_type findPos = tmpView.find(config::defineCharacter, lastPos);

		while(findPos < tmpView.npos)
		{
			newString.append(tmpView, lastPos, findPos - lastPos);

			for (auto& definePair: defines)
			{
				std::string_view candidate = tmpView.substr(findPos, definePair.first.length());
				if (candidate == definePair.first)
				{
					newString += definePair.second;
					lastPos = findPos + definePair.first.length();
					break;
				}
			}
			
			if (lastPos <= findPos)
			{
				newString += config::defineCharacter;
				lastPos = findPos + 1;
			}

			findPos = tmpView.find(config::defineCharacter, lastPos);
		}

		// Copy the rest of the string
		newString += tmpView.substr(lastPos);
		lines.push_back(std::move(newString));		
	}
}

bool SfzSynth::loadSfzFile(const std::filesystem::path &file)
{
	clear();
	const auto sfzFile = file.is_absolute() ? file : rootDirectory / file;
	if (!std::filesystem::exists(sfzFile))
		return false;

	rootDirectory = file.parent_path();
	filePool.setRootDirectory(File(rootDirectory.string()));
	std::vector<std::string> lines;
	readSfzFile(file, lines);

	const auto fullString = joinIntoString(lines);
	const std::string_view fullStringView { fullString };

	svregex_iterator headerIterator(fullStringView.cbegin(), fullStringView.cend(), SfzRegexes::headers);
	const auto regexEnd = svregex_iterator();

	std::optional<uint8_t> defaultSwitch {};
	std::vector<SfzOpcode> globalMembers;
	std::vector<SfzOpcode> masterMembers;
	std::vector<SfzOpcode> groupMembers;
	std::vector<SfzOpcode> regionMembers;
	std::vector<SfzOpcode> curveMembers;
	std::vector<SfzOpcode> effectMembers;
	bool regionStarted = false;
	bool hasGlobal = false;
	bool hasControl = false;
	
	auto buildRegion = [&, this]() {
		regions.emplace_back(File(rootDirectory.string()), filePool);
		auto& region = regions.back(); // For some reason using auto& region up there does not work?!
		// Successively apply the opcodes alread read to the parameter structure
		for (auto& opcode: globalMembers)
			region.parseOpcode(opcode);
		for (auto& opcode: masterMembers)
			region.parseOpcode(opcode);
		for (auto& opcode: groupMembers)
			region.parseOpcode(opcode);
		for (auto& opcode: regionMembers)
			region.parseOpcode(opcode);
		regionMembers.clear();	
	};

	for (; headerIterator != regexEnd; ++headerIterator)
  	{
		svmatch_results headerMatch = *headerIterator;

		// Can't use uniform initialization here because it generates narrowing conversions
		const std::string_view header(&*headerMatch[1].first, headerMatch[1].length());
		const std::string_view members(&*headerMatch[2].first, headerMatch[2].length());
		auto paramIterator = svregex_iterator (members.cbegin(), members.cend(), SfzRegexes::members);

		// If we had a building region and we encounter a new header we have to build it
		if (regionStarted)
		{
			buildRegion();
			regionStarted = false;
		}

		// Header logic
		switch (hash(header))
		{
			case hash("global"):
				if (hasGlobal)
					// We shouldn't have multiple global headers in file
					jassertfalse;
				else
					hasGlobal = true;
				break;
			case hash("control"):
				if (hasControl)
					// We shouldn't have multiple control headers in file
					jassertfalse;
				else
					hasControl = true;
				break;
			case hash("master"):
				numMasters += 1;
				groupMembers.clear();
				masterMembers.clear();
				break;
			case hash("group"):
				numGroups += 1;
				groupMembers.clear();
				break;
			case hash("region"):
				regionStarted = true;
				break;
			// TODO: handle these
			case hash("curve"):
				DBG("Curve header not implemented");
				break;
			case hash("effect"):
				DBG("Effect header not implemented");
				break;
			default:
				DBG("unknown header: " << std::string(header));
		}

		// Store or handle members
		for (; paramIterator != regexEnd; ++paramIterator)
		{
			svmatch_results paramMatch = *paramIterator;
			const std::string_view opcode(&*paramMatch[1].first, paramMatch[1].length());
			const std::string_view value(&*paramMatch[2].first, paramMatch[2].length());

			// Store the members depending on the header
			switch (hash(header))
			{
			case hash("global"):
				if (opcode == "sw_default")
					setValueFromOpcode({opcode, value}, defaultSwitch, SfzDefault::keyRange);
				else
					globalMembers.emplace_back(opcode, value);
				break;
			case hash("master"):
				masterMembers.emplace_back(opcode, value);
				break;
			case hash("group"):
				groupMembers.emplace_back(opcode, value);
				break;
			case hash("region"):
				regionMembers.emplace_back(opcode, value);
				break;
			case hash("control"):
			{
				SfzOpcode lastOpcode{opcode, value};
				switch (hash(lastOpcode.opcode))
				{
				case hash("set_cc"):
					if (lastOpcode.parameter && withinRange(SfzDefault::ccRange, *lastOpcode.parameter))
						setValueFromOpcode(lastOpcode, ccState[*lastOpcode.parameter], SfzDefault::ccRange);
					break;
				case hash("label_cc"):
					if (lastOpcode.parameter && withinRange(SfzDefault::ccRange, *lastOpcode.parameter))
					{
						String ccName{lastOpcode.value.data(), lastOpcode.value.size()};
						ccNames.emplace_back(*lastOpcode.parameter, std::move(ccName));
					}
					break;
				case hash("default_path"):
				{
					String newPath{lastOpcode.value.data(), lastOpcode.value.size()};
					File newRootDirectory{newPath};
					filePool.setRootDirectory(newRootDirectory);
				}
					break;
				default:
					DBG("Unknown/unsupported opcode in <control> header: " << std::string(lastOpcode.opcode));
				}
			}
			break;
			case hash("curve"):
				curveMembers.emplace_back(opcode, value);
				break;
			case hash("effect"):
				effectMembers.emplace_back(opcode, value);          
			}
		}
	}
	
	// Build the last region
	if (regionStarted)
	{
		buildRegion();
		regionStarted = false;
	}

	// Sort the CC labels
	std::sort(begin(ccNames), end(ccNames), [](auto& lhs, auto& rhs) { return lhs.first < rhs.first; });

	for (auto& region: regions)
	{
		region.prepare();
		
		for (int ccIdx = 1; ccIdx < 128; ccIdx++)
		{
			region.registerCC(region.channelRange.getStart(), ccIdx, ccState[ccIdx]);
		}

		if (defaultSwitch)
		{
			region.registerNoteOn(region.channelRange.getStart(), *defaultSwitch, 127, 1.0f);
			region.registerNoteOff(region.channelRange.getStart(), *defaultSwitch, 0, 1.0f);
		}
	}
	return true;
}

StringArray SfzSynth::getUnknownOpcodes() const
{
	StringArray returnedArray;
	for (auto& region: regions)
	{
		for (auto& opcode: region.unknownOpcodes)
		{
			String s { opcode };
			if (!returnedArray.contains(s))
				returnedArray.add(s);
		}
	}
	return returnedArray;
}
StringArray SfzSynth::getCCLabels() const
{
	StringArray returnedArray;
	for (auto& ccNamePair: ccNames)
	{
		String s;
		s << ccNamePair.first << ": " << ccNamePair.second;
		returnedArray.add(s);
	}
	return returnedArray;
}

const SfzRegion* SfzSynth::getRegionView(int num) const
{
	if (num >= getNumRegions())
		return {};

	auto regionIterator = regions.cbegin();
	for (int i = 0; i < num; i++)
		regionIterator++;
	return &*regionIterator;
}

void SfzSynth::clear()
{
	ccNames.clear();
	regions.clear();
	for (auto& voice: voices)
		voice.reset();
	filePool.clear();
	resetMidiState();
	defines.clear();
}

void SfzSynth::resetMidiState()
{
	for (auto& cc: ccState)
		cc = 0;
}

void SfzSynth::prepareToPlay(double newSampleRate, int newSamplesPerBlock)
{
	this->sampleRate = newSampleRate;
	this->samplesPerBlock = newSamplesPerBlock;
	for (auto& voice: voices)
		voice.prepareToPlay(newSampleRate, newSamplesPerBlock);
	tempBuffer = AudioBuffer<float>(config::numChannels, newSamplesPerBlock);
}

void SfzSynth::registerNoteOn(int channel, int noteNumber, uint8_t velocity, int timestamp)
{
	const auto randValue = Random::getSystemRandom().nextFloat();

	for (auto& region: regions)
	{
		if (region.registerNoteOn(channel, noteNumber, velocity, randValue))
		{
			for (auto& voice: voices)
			{
				const auto triggeringNoteNumber = voice.getTriggeringNoteNumber();
				if (voice.checkOffGroup(region.group, timestamp) && triggeringNoteNumber)
					registerNoteOff(channel, noteNumber, 0, timestamp);
			}

			auto freeVoice = std::find_if(voices.begin(), voices.end(), [](auto& voice) { return voice.isFree(); });
			if (freeVoice != end(voices))
				freeVoice->startVoiceWithNote(region, channel, noteNumber, velocity, timestamp);
		}		
	}
}

void SfzSynth::registerNoteOff(int channel, int noteNumber, uint8_t velocity, int timestamp)
{
	const auto randValue = Random::getSystemRandom().nextFloat();
	
	for (auto& region: regions)
	{
		if (region.registerNoteOff(channel, noteNumber, velocity, randValue))
		{
			auto freeVoice = std::find_if(voices.begin(), voices.end(), [](auto& voice) { return voice.isFree(); });
			if (freeVoice != end(voices))
				freeVoice->startVoiceWithNote(region, channel, noteNumber, velocity, timestamp);
		}
		
	}

	for (auto& voice: voices)
		voice.registerNoteOff(channel, noteNumber, velocity, timestamp);
}

void SfzSynth::registerCC(int channel, int ccNumber, uint8_t ccValue, int timestamp)
{
	ccState[ccNumber] = ccValue;

	for (auto& region: regions)
	{
		if (region.registerCC(channel, ccNumber, ccValue))
		{
			auto freeVoice = std::find_if(voices.begin(), voices.end(), [](auto& voice) { return voice.isFree(); });
			if (freeVoice != end(voices))
				freeVoice->startVoiceWithCC(region, channel, ccNumber, ccValue, timestamp);
		}		
	}

	for (auto& voice: voices)
		voice.registerCC(channel, ccNumber, ccValue, timestamp);
}

void SfzSynth::renderNextBlock(AudioBuffer<float>& outputAudio, int startSample, int numSamples)
{
	// Render all the voices
	for (auto& voice: voices)
	{
		voice.renderNextBlock(tempBuffer, startSample, numSamples);
		for (int channelIdx = 0; channelIdx < config::numChannels; ++channelIdx)
			outputAudio.addFrom(channelIdx, startSample, tempBuffer, channelIdx, startSample, numSamples);
	}
}

void SfzSynth::registerPitchWheel(int channel, int pitch, int timestamp)
{
	for (auto& region: regions)
		region.registerPitchWheel(channel, pitch);
	
	for (auto& voice: voices)
		voice.registerPitchWheel(channel, pitch, timestamp);
}

void SfzSynth::registerAftertouch(int channel, uint8_t aftertouch, int timestamp)
{
	for (auto& region: regions)
		region.registerAftertouch(channel, aftertouch);

	for (auto& voice: voices)
		voice.registerAftertouch(channel, aftertouch, timestamp);
}

void SfzSynth::registerTempo(float secondsPerQuarter, int timestamp [[maybe_unused]])
{
	for (auto& region: regions)
		region.registerTempo(secondsPerQuarter);
}
