module;

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <vector>
#define SOLOUD_NO_ASSERTS
#include <soloud.h>

module ffxi:audio.adpcmstream;

import lotus;

class ADPCMStream;

class ADPCMStreamInstance : public SoLoud::AudioSourceInstance
{
public:
    ADPCMStreamInstance(ADPCMStream*);
    virtual unsigned int getAudio(float* buffer, unsigned int samples, unsigned int buffer_size);
    virtual bool hasEnded();

private:
    std::vector<float> data;
    size_t offset{0};
    ADPCMStream* adpcm;
};

class ADPCMStream : public SoLoud::AudioSource
{
public:
    ADPCMStream(std::ifstream&& file, uint32_t _blocks, uint32_t _block_size, uint32_t _loop_start, uint32_t _channels, float _sample_rate);
    virtual SoLoud::AudioSourceInstance* createInstance();

    uint32_t samples{};
    uint32_t loop_start{};
    std::streampos data_begin{};
    std::ifstream file;
    static constexpr std::array<int32_t, 5> filter0{0x0000, 0x00F0, 0x01CC, 0x0188, 0x01E8};
    static constexpr std::array<int32_t, 5> filter1{0x0000, 0x0000, -0x00D0, -0x00DC, -0x00F0};
    std::vector<int> decoder_state;
    uint32_t current_block{0};
    std::vector<int> decoder_state_loop_start;
    uint32_t block_size{};

    std::vector<float> getNextBlock();
    void resetLoop();
};

ADPCMStreamInstance::ADPCMStreamInstance(ADPCMStream* adpcm) : adpcm(adpcm)
{
    data = adpcm->getNextBlock();
    mFlags |= PROTECTED;
}

unsigned int ADPCMStreamInstance::getAudio(float* buffer, unsigned int samples, unsigned int buffer_size)
{
    if (!adpcm)
        return 0;

    unsigned int written = 0;
    while (written < samples)
    {
        if (offset == adpcm->block_size && adpcm->file.eof())
        {
            adpcm->resetLoop();
        }
        if ((samples - written) + offset > adpcm->block_size)
        {
            for (uint32_t i = 0; i < mChannels; ++i)
            {
                memcpy(buffer + i * buffer_size + written + offset, data.data() + adpcm->block_size * i + offset, sizeof(float) * (adpcm->block_size - offset));
            }
            data = adpcm->getNextBlock();
            written += (adpcm->block_size - offset);
            offset = 0;
        }
        else
        {
            for (uint32_t i = 0; i < mChannels; ++i)
            {
                memcpy(buffer + i * buffer_size + written + offset, data.data() + adpcm->block_size * i + offset, sizeof(float) * (samples - written));
            }
            offset += (samples - written);
            written += (samples - written);
        }
    }

    if (written < samples)
    {
        memset(buffer + written, 0, sizeof(float) * (samples - written));
    }

    return written;
}

bool ADPCMStreamInstance::hasEnded()
{
    // loops forever
    return false;
}

ADPCMStream::ADPCMStream(std::ifstream&& _file, uint32_t _blocks, uint32_t _block_size, uint32_t _loop_start, uint32_t _channels, float _sample_rate)
    : samples(_blocks * _block_size), loop_start(_loop_start), block_size(_block_size)
{
    file = std::move(_file);
    data_begin = file.tellg();

    mChannels = _channels;
    mFlags = SHOULD_LOOP | SINGLE_INSTANCE;
    mBaseSamplerate = _sample_rate;

    decoder_state.resize(mChannels * 2);
}

SoLoud::AudioSourceInstance* ADPCMStream::createInstance() { return new ADPCMStreamInstance(this); }

std::vector<float> ADPCMStream::getNextBlock()
{
    std::vector<std::byte> block;
    block.resize((1 + block_size / 2) * mChannels);
    file.read((char*)block.data(), (1 + block_size / 2) * mChannels);
    std::vector<float> output{};
    output.reserve(mChannels * block_size);
    std::byte* compressed_block_start = block.data();
    if (current_block == loop_start)
    {
        decoder_state_loop_start = decoder_state;
    }
    for (size_t channel = 0; channel < mChannels; channel++)
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
                    output.push_back(int16_t(decoder_state[channel * 2]) / float(0x8000));
                }
            }
        }
    }
    current_block++;
    return output;
}

void ADPCMStream::resetLoop()
{
    file.clear();
    file.seekg((int)data_begin + loop_start * (1 + block_size / 2) * mChannels);
    decoder_state = decoder_state_loop_start;
    current_block = loop_start;
}
