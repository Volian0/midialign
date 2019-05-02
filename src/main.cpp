#include "midi/MidiFile.h"
#include "midi/MidiEvent.h"
#include "div.h"

#include <iostream>
#include <string>

uint32_t bpm_to_micro(uint32_t bpm)
{
	safe_divider div;
	return div.divide(60000000, bpm);
}

std::vector<uint64_t> midi_to_realtime(MidiFile& midi)
{
	std::vector<uint64_t> realtimes;
	midi.deltaTicks();
	midi.joinTracks();
	uint32_t size = midi[0].getEventCount();
	uint64_t current_tempo = 0;
	for (uint32_t i = 0; i < size; ++i)
	{
		auto& event = midi[0][i];
		if (current_tempo == 0 && event.tick != 0)
		{
			throw std::runtime_error("Set Tempo event not found");
		}
		realtimes.push_back(current_tempo * event.tick);
		if (event.isTempo())
		{
			current_tempo = event.getTempoMicro();
		}
	}
	return realtimes;
}

void realtime_to_midi(MidiFile& midi, uint32_t bpm, const std::vector<uint64_t>& realtimes)
{
	uint32_t desired_tempo = bpm_to_micro(bpm);
	uint32_t size = midi[0].getEventCount();
	safe_divider div;
	for (uint32_t i = 0; i < size; ++i)
	{
		uint64_t length = div.divide(realtimes.at(i), desired_tempo);
		if (length > 0xFFFFFFF)
		{
			throw std::runtime_error("Time overflow");
		}
		auto& event = midi[0][i];
		event.tick = length;
		if (event.isTempo())
		{
			event.setTempoMicroseconds(desired_tempo);
		}
	}
	midi.splitTracks();
}

int main(int argc, char* argv[])
try
{
	if (argc != 4)
	{
		throw std::invalid_argument("Expected 3 arguments, got " + std::to_string(argc-1));
	}
	std::string input_filename = argv[1];
	uint32_t bpm = std::stoull(argv[2]);
	std::string output_filename = argv[3];
	MidiFile midi;
	midi.read(input_filename);
	if (!midi.status() || midi.getTrackCount() == 0)
	{
		throw std::runtime_error("MIDI file is invalid");
	}
	auto realtimes = midi_to_realtime(midi);
	realtime_to_midi(midi,bpm,realtimes);
	midi.write(output_filename);
	return 0;
}
catch (const std::exception& e)
{
	std::cout << "Exception:" << std::endl;
	std::cout << e.what() << std::endl;
	return -1;
}
catch (...)
{
	std::cout << "Unknown error" << std::endl;
	return -1;
}
