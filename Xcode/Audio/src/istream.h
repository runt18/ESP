/*
 * IStream encapsulates a number of stream input devices: serial,
 * audio, etc.
 */
#pragma once

#include "GRT/GRT.h"
#include "ofMain.h"

class IStream {
  public:
    IStream();
    virtual ~IStream() = default;

    virtual void start() = 0;
    virtual void stop() = 0;

    // These two functions are no-op by default.
    virtual void useUSBPort(int i) {};
    virtual void useAnalogPin(int i) {};

    typedef std::function<double(double)> normalizeFunc;
    typedef std::function<vector<double>(vector<double>)> vectorNormalizeFunc;
    
    // Supply a normalization function: double -> double.
    // Applied to each dimension of each vector of incoming data.
    void useNormalizer(normalizeFunc f) {
        normalizer_ = f;
        vectorNormalizer_ = nullptr;
    }
    
    // Supply a normalization function: vector<double> -> vector<double>
    // Applied to each vector of incoming data.
    void useNormalizer(vectorNormalizeFunc f) {
        normalizer_ = nullptr;
        vectorNormalizer_ = f;
    }

    bool hasStarted() { return has_started_; }

    typedef std::function<void(GRT::MatrixDouble)> onDataReadyCallback;

    void onDataReadyEvent(onDataReadyCallback callback) {
        data_ready_callback_ = callback;
    }

    template<typename T1, typename arg, class T>
    void onDataReadyEvent(T1* owner, void (T::*listenerMethod)(arg)) {
        using namespace std::placeholders;
        data_ready_callback_ = std::bind(listenerMethod, owner, _1);
    }

  protected:
    bool has_started_;
    onDataReadyCallback data_ready_callback_;
    normalizeFunc normalizer_;
    vectorNormalizeFunc vectorNormalizer_;
    
    vector<double> normalize(vector<double>);
};

class AudioStream : public ofBaseApp, public IStream {
  public:
    AudioStream();
    void audioIn(float *input, int buffer_size, int nChannel);
    virtual void start() final;
    virtual void stop() final;
  private:
    unique_ptr<ofSoundStream> sound_stream_;
};

class SerialStream : public IStream {
  public:
    SerialStream();
    virtual void start() final;
    virtual void stop() final;
    virtual void useUSBPort(int i);
    virtual void useAnalogPin(int i);
  private:
    int kBaud_ = 115200;
    // Serial buffer size
    int kBufferSize_ = 64;

    unique_ptr<ofSerial> serial_;
    int port_ = -1;
    int pin_;

    // A separate reading thread to read data from Serial.
    unique_ptr<std::thread> reading_thread_;
    void readSerial();
};

class ASCIISerialStream : public IStream {
  public:
    ASCIISerialStream(int kBaud);
    virtual void start() final;
    virtual void stop() final;
    virtual void useUSBPort(int i);
  private:
    int kBaud_;
  
    unique_ptr<ofSerial> serial_;
    int port_ = -1;

    // A separate reading thread to read data from Serial.
    unique_ptr<std::thread> reading_thread_;
    void readSerial();
};

class FirmataStream : public IStream {
  public:
    FirmataStream();
    virtual void start() final;
    virtual void stop() final;
    virtual void useUSBPort(int i);
    virtual void useAnalogPin(int i);
  private:
    int port_ = -1;
    int pin_ = -1;
    
    bool configured_arduino_;
    
    ofArduino arduino_;
    unique_ptr<std::thread> update_thread_;
    void update();
};