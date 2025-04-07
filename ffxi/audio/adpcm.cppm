module;

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <memory>
#include <vector>

module ffxi:audio.adpcm;

import lotus;

class ADPCM;

class ADPCMInstance : public lotus::AudioInstance
{
public:
    ADPCMInstance(ADPCM*);
    virtual unsigned int getAudio(float* buffer, unsigned int samples);
    virtual bool hasEnded();

private:
    size_t offset{0};
    ADPCM* adpcm;
};

class ADPCM : public lotus::AudioSource
{
public:
    ADPCM(std::ifstream&& file, uint32_t _blocks, uint32_t _block_size, uint32_t _loop_start, uint32_t _channels, float _sample_rate);
    virtual std::unique_ptr<lotus::AudioInstance> CreateInstance() override;

    uint32_t samples{};
    uint32_t loop_start{};
    static constexpr std::array<int32_t, 5> filter0{0x0000, 0x00F0, 0x01CC, 0x0188, 0x01E8};
    static constexpr std::array<int32_t, 5> filter1{0x0000, 0x0000, -0x00D0, -0x00DC, -0x00F0};
    uint32_t block_size{};
    std::vector<std::vector<float>> data;
};

ADPCMInstance::ADPCMInstance(ADPCM* adpcm) : lotus::AudioInstance(adpcm), adpcm(adpcm) {}

unsigned int ADPCMInstance::getAudio(float* buffer, unsigned int samples)
{
    if (!adpcm)
        return 0;

    size_t write_size = samples > adpcm->samples - offset ? adpcm->samples - offset : samples;

    for (const auto& channel_data : adpcm->data)
    {
        memcpy(buffer, channel_data.data() + offset, sizeof(float) * write_size);
    }

    offset += write_size;

    if (offset >= adpcm->samples && (flags & Flags::Looping))
    {
        offset = adpcm->loop_start * adpcm->block_size;
    }

    return write_size;
}

bool ADPCMInstance::hasEnded()
{
    if (!(flags & Flags::Looping) && offset >= adpcm->samples)
        return true;
    return false;
}

ADPCM::ADPCM(std::ifstream&& file, uint32_t _blocks, uint32_t _block_size, uint32_t _loop_start, uint32_t _channels, float _sample_rate)
    : lotus::AudioSource(_channels, _sample_rate), samples(_blocks * _block_size), loop_start(_loop_start), block_size(_block_size)
{
    std::vector<int> decoder_state;
    decoder_state.resize(channels * 2);

    data.resize(channels);
    for (auto& channel_data : data)
    {
        channel_data.reserve(samples);
    }

    std::vector<std::byte> block;
    block.resize((1 + block_size / 2) * channels);
    while (file.good())
    {
        file.read((char*)block.data(), (1 + block_size / 2) * channels);
        std::byte* compressed_block_start = block.data();
        for (size_t channel = 0; channel < channels; channel++)
        {
            int base_index = channel * (1 + block_size / 2);
            int scale = 0x0C - std::to_integer<int>((compressed_block_start[base_index] & std::byte{0b1111}));
            int index = std::to_integer<size_t>(compressed_block_start[base_index] >> 4);
            if (index < 5)
            {
                for (uint8_t sample = 0; sample < (block_size / 2); ++sample)
                {
                    std::byte sample_byte = compressed_block_start[base_index + sample + 1];
                    for (uint8_t nibble = 0; nibble < 2; ++nibble)
                    {
                        int value = std::to_integer<int>(sample_byte >> (4 * nibble) & std::byte(0b1111));
                        if (value >= 8)
                            value -= 16;
                        value <<= scale;
                        value += (decoder_state[channel * 2] * filter0[index] + decoder_state[channel * 2 + 1] * filter1[index]) / 256;
                        decoder_state[channel * 2 + 1] = decoder_state[channel * 2];
                        decoder_state[channel * 2] = value > 0x7FFF ? 0x7FFF : value < -0x8000 ? -0x8000 : value;
                        data[channel].push_back(int16_t(decoder_state[channel * 2]) / float(0x8000));
                    }
                }
            }
        }
    }
}

std::unique_ptr<lotus::AudioInstance> ADPCM::CreateInstance() { return std::make_unique<ADPCMInstance>(this); }
