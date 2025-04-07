module;

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <memory>
#include <vector>

module ffxi:audio.pcm;

import lotus;

class PCM;

class PCMInstance : public lotus::AudioInstance
{
public:
    PCMInstance(PCM*);
    virtual unsigned int getAudio(float* buffer, unsigned int samples);
    virtual bool hasEnded();

private:
    size_t offset{0};
    PCM* pcm;
};

class PCM : public lotus::AudioSource
{
public:
    PCM(std::ifstream&& file, uint32_t samples, uint32_t block_size, uint32_t loop_start, uint32_t channels, float sample_rate);
    virtual std::unique_ptr<lotus::AudioInstance> CreateInstance() override;

    uint32_t samples{};
    uint32_t loop_start{};
    std::vector<std::vector<float>> data;
};

PCMInstance::PCMInstance(PCM* pcm) : lotus::AudioInstance(pcm), pcm(pcm) {}

unsigned int PCMInstance::getAudio(float* buffer, unsigned int samples)
{
    if (!pcm)
        return 0;

    uint32_t length = samples > pcm->data[0].size() - offset ? pcm->data[0].size() - offset : samples;

    for (const auto& channel_data : pcm->data)
    {
        memcpy(buffer, channel_data.data() + offset, sizeof(float) * length);
    }

    offset += length;

    return length;
}

bool PCMInstance::hasEnded()
{
    if (!(flags & Flags::Looping) && offset >= pcm->data[0].size())
    {
        return true;
    }
    return false;
}

PCM::PCM(std::ifstream&& file, uint32_t _samples, uint32_t block_size, uint32_t _loop_start, uint32_t _channels, float _sample_rate)
    : lotus::AudioSource(_channels, _sample_rate), samples(_samples), loop_start(_loop_start)
{
    data.resize(channels);
    for (auto& channel_data : data)
    {
        channel_data.reserve(samples);
    }

    std::vector<int16_t> block;
    block.resize(block_size);
    while (file.good())
    {
        for (auto& channel_data : data)
        {
            file.read((char*)block.data(), block_size * sizeof(int16_t));
            std::ranges::transform(block, std::back_inserter(channel_data), [](auto& u) { return u / float(0x8000); });
        }
    }
}

std::unique_ptr<lotus::AudioInstance> PCM::CreateInstance() { return std::make_unique<PCMInstance>(this); }
