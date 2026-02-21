#include "PluginProcessor.h"

//==============================================================================
#pragma pack(push, 1)
struct VBANHeader
{
  char vban[4];
  uint8_t format_SR;
  uint8_t format_nbs;
  uint8_t format_nbc;
  uint8_t format_bit;
  char streamname[16];
  uint32_t nuFrame;
};
#pragma pack(pop)

void buildVBANHeader(VBANHeader& header, const char* name, uint16_t numSamples, uint32_t frameCounter)
{
  memset(&header, 0, sizeof(VBANHeader));
  memcpy(header.vban, "VBAN", 4);

  header.format_SR = 0x03;
  header.format_nbs = static_cast<uint8_t>(numSamples - 1);
  header.format_nbc = 1;
  header.format_bit = 0x01;

  auto len = std::min<size_t>(std::strlen(name), 15);
  std::memcpy(header.streamname, name, len);
  header.streamname[len] = '\0';
  header.nuFrame = frameCounter;
}

juce::File AudioPluginAudioProcessor::getConfigFile()
{
  auto dir = juce::File::getSpecialLocation(
    juce::File::userApplicationDataDirectory);

  auto folder = dir.getChildFile("VBANTX");

  if (!folder.exists())
    folder.createDirectory();

  return folder.getChildFile("config.json");
}

void AudioPluginAudioProcessor::loadConfig()
{
    auto file = getConfigFile();

    // If config doesn't exist → create default
    if (!file.existsAsFile())
    {
        juce::DynamicObject::Ptr obj = new juce::DynamicObject();

        obj->setProperty("ip", "127.0.0.1");
        obj->setProperty("port", 6980);
        obj->setProperty("streamName", "AudioStream");

        juce::var json(obj);
        file.create();
        file.replaceWithText(juce::JSON::toString(json, true)); // pretty print

        DBG("Created default config at: " << file.getFullPathName());
        return; // keep using default member values
    }

    // Otherwise load existing file
    auto json = juce::JSON::parse(file);

    if (!json.isObject())
        return;

    auto* obj = json.getDynamicObject();

    if (auto v = obj->getProperty("ip"); !v.isVoid())
        remoteIP = v.toString();

    if (auto v = obj->getProperty("port"); !v.isVoid())
        remotePort = (int)v;

    if (auto v = obj->getProperty("streamName"); !v.isVoid())
        streamName = v.toString();
}
//==============================================================================
AudioPluginAudioProcessor::AudioPluginAudioProcessor()
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
{
  loadConfig();

  audioBuffer.resize(fifo.getTotalSize() * 2); // 2 channels
  networkRunning = true;
  networkThread = std::thread([this]() {
    juce::DatagramSocket udpSocket{ false }; // UDP
    udpSocket.bindToPort(0);
    udpSocket.setEnablePortReuse(true);

    const int samplesPerPacket = 256;
    std::vector<int16_t> tempBuffer(samplesPerPacket * 2);
    std::vector<uint8_t> packetVBAN(sizeof(VBANHeader) + samplesPerPacket * 2 * sizeof(int16_t));
    uint32_t frameCounter = 0;

    while (networkRunning)
    {
      int start1, size1, start2, size2;
      fifo.prepareToRead(samplesPerPacket, start1, size1, start2, size2);

      int samplesRead = size1 + size2;
      if (samplesRead > 0)
      {
        for (int i = 0; i < size1; ++i)
        {
          tempBuffer[i * 2 + 0] = static_cast<int16_t>(audioBuffer[(start1 + i) * 2 + 0] * 32767.0f);
          tempBuffer[i * 2 + 1] = static_cast<int16_t>(audioBuffer[(start1 + i) * 2 + 1] * 32767.0f);
        }

        if (size2 > 0)
        {
          for (int i = 0; i < size2; ++i)
          {
            tempBuffer[(size1 + i) * 2 + 0] = static_cast<int16_t>(audioBuffer[(start2 + i) * 2 + 0] * 32767.0f);
            tempBuffer[(size1 + i) * 2 + 1] = static_cast<int16_t>(audioBuffer[(start2 + i) * 2 + 1] * 32767.0f);
          }
        }

        VBANHeader header;
        auto localStream = streamName;
        buildVBANHeader(header, localStream.toRawUTF8(), static_cast<uint16_t>(samplesRead), frameCounter++);

        int packetSize = sizeof(VBANHeader) + samplesRead * 2 * sizeof(int16_t);
        memcpy(packetVBAN.data(), &header, sizeof(VBANHeader));
        memcpy(packetVBAN.data() + sizeof(VBANHeader), tempBuffer.data(), samplesRead * 2 * sizeof(int16_t));

        int bytesSent = udpSocket.write(remoteIP, remotePort, packetVBAN.data(), packetSize);

        if (!(bytesSent > 0))
        {
          DBG("Failed to send packet");
        }

        fifo.finishedRead(samplesRead);
      }
      else
      {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
      }
    }
    DBG("Network thread stopped");
  });
}

AudioPluginAudioProcessor::~AudioPluginAudioProcessor()
{
  networkRunning = false;
  if (networkThread.joinable())
    networkThread.join();
}

//==============================================================================
const juce::String AudioPluginAudioProcessor::getName() const
{
  return JucePlugin_Name;
}

bool AudioPluginAudioProcessor::acceptsMidi() const
{
  #if JucePlugin_WantsMidiInput
  return true;
  #else
  return false;
  #endif
}

bool AudioPluginAudioProcessor::producesMidi() const
{
  #if JucePlugin_ProducesMidiOutput
  return true;
  #else
  return false;
  #endif
}

bool AudioPluginAudioProcessor::isMidiEffect() const
{
  #if JucePlugin_IsMidiEffect
  return true;
  #else
  return false;
  #endif
}

double AudioPluginAudioProcessor::getTailLengthSeconds() const
{
  return 0.0;
}

int AudioPluginAudioProcessor::getNumPrograms()
{
  return 1;
}

int AudioPluginAudioProcessor::getCurrentProgram()
{
  return 0;
}

void AudioPluginAudioProcessor::setCurrentProgram (int index)
{
  juce::ignoreUnused (index);
}

const juce::String AudioPluginAudioProcessor::getProgramName (int index)
{
  juce::ignoreUnused (index);
  return {};
}

void AudioPluginAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
  juce::ignoreUnused (index, newName);
}

//==============================================================================
void AudioPluginAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
  juce::ignoreUnused (sampleRate, samplesPerBlock);
}

void AudioPluginAudioProcessor::releaseResources()
{
}

bool AudioPluginAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}

void AudioPluginAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                              juce::MidiBuffer& midiMessages)
{
  juce::ignoreUnused (midiMessages);

  juce::ScopedNoDenormals noDenormals;
  auto totalNumInputChannels  = getTotalNumInputChannels();
  auto totalNumOutputChannels = getTotalNumOutputChannels();

  for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
      buffer.clear (i, 0, buffer.getNumSamples());

  int numSamples = buffer.getNumSamples();
  int start1, size1, start2, size2;
  fifo.prepareToWrite(numSamples, start1, size1, start2, size2);

  if (size1 + size2 >= numSamples)
  {
    for (int i = 0; i < size1; ++i)
    {
      float left = buffer.getSample(0, i);
      float right = buffer.getNumChannels() > 1 ? buffer.getSample(1, i) : left;
      audioBuffer[(start1 + i) * 2 + 0] = left;
      audioBuffer[(start1 + i) * 2 + 1] = right;
    }

    if (size2 > 0)
    {
      for (int i = 0; i < size2; ++i)
      {
        float left = buffer.getSample(0, size1 + i);
        float right = buffer.getNumChannels() > 1 ? buffer.getSample(1, size1 + i) : left;
        audioBuffer[(start2 + i) * 2 + 0] = left;
        audioBuffer[(start2 + i) * 2 + 1] = right;
      }
    }

    fifo.finishedWrite(size1 + size2);
  }
}

//==============================================================================
void AudioPluginAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
  juce::ignoreUnused (destData);
}

void AudioPluginAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
  juce::ignoreUnused (data, sizeInBytes);
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AudioPluginAudioProcessor();
}
