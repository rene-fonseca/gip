/***************************************************************************
    Generic Image Processing (GIP) Framework (Test Suite)
    A framework for developing image processing applications

    Copyright (C) 2002 by Rene Moeller Fonseca <fonseca@mip.sdu.dk>

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#include <base/Application.h>
#include <base/Cast.h>
#include <base/string/FormatOutputStream.h>
#include <base/string/StringOutputStream.h>
#include <base/string/String.h>
#include <base/string/InvalidFormat.h>
#include <base/net/InetAddress.h>
#include <base/net/InetEndPoint.h>
#include <base/concurrency/Thread.h>
#include <base/concurrency/Event.h>
#include <base/concurrency/Semaphore.h>
#include <base/concurrency/MutualExclusion.h>
#include <base/collection/Queue.h>
#include <base/Timer.h>
#include <base/net/ServerSocket.h>
#include <base/UnsignedInteger.h>
#include <base/io/EndOfFile.h>
#include <base/UnsignedInteger.h>
#include <gip/ArrayImage.h>
#include <gip/Pixel.h>
#include <gip/YCbCrPixel.h>
#include <gip/video/Camera1394.h>
#include <gip/transformation/Flip.h>
#include <gip/io/BMPEncoder.h>
#include <gip/transformation/Convert.h>

using namespace base;
using namespace gip;

class RGB24ToRGB : public UnaryOperation<Camera1394::RGB24Pixel, ColorPixel> {
public:
  
  inline Result operator()(const Argument& value) const throw() {
    return makeColorPixel(value.red, value.blue, value.green);
  }
};

class ByteToRGB : public UnaryOperation<uint8, ColorPixel> {
public:
  
  inline Result operator()(const Argument& value) const throw() {
    return makeColorPixel(value, value, value);
  }
};

class VideoPhoneServlet : public virtual Object, public Camera1394::AcquisitionListener {
public:

  class Recorder : public virtual Runnable {
  private:

    VideoPhoneServlet* videoPhoneServlet;
  public:

    Recorder(VideoPhoneServlet* object) throw() : videoPhoneServlet(object) {}

    void run() throw() {
      videoPhoneServlet->record();
    }
  };

  class Player : public virtual Runnable {
  private:

    VideoPhoneServlet* videoPhoneServlet;
  public:

    Player(VideoPhoneServlet* object) throw() : videoPhoneServlet(object) {}
  
    void run() throw() {
      videoPhoneServlet->play();
    }
  };

  class Reader : public virtual Runnable {
  private:

    VideoPhoneServlet* videoPhoneServlet;
  public:

    Reader(VideoPhoneServlet* object) throw() : videoPhoneServlet(object) {}

    void run() throw() {
      videoPhoneServlet->read();
    }
  };

  class Writer : public virtual Runnable {
  private:

    VideoPhoneServlet* videoPhoneServlet;
  public:

    Writer(VideoPhoneServlet* object) throw() : videoPhoneServlet(object) {}

    void run() throw() {
      videoPhoneServlet->write();
    }
  };
private:

  Camera1394& camera;
  Recorder recorder;
  Player player;
  Reader reader;
  Writer writer;
  Queue<ColorImage*> recordingQueue;
  Queue<ColorImage*> playingQueue;
  Queue<ColorImage*> readingQueue;
  Queue<ColorImage*> writingQueue;
  Semaphore recordingSemaphore;
  Semaphore playingSemaphore;
  Semaphore readingSemaphore;
  Semaphore writingSemaphore;
  MutualExclusion guard;
  bool isServer;
  bool loopback;
  InetEndPoint endPoint;
  StreamSocket streamSocket;
public:

  VideoPhoneServlet(Camera1394& _camera, bool loopback, bool isServer, const InetEndPoint& endPoint) throw()
    : camera(_camera),
      recorder(this),
      player(this),
      reader(this),
      writer(this) {
    this->isServer = isServer;
    this->endPoint = endPoint;
    this->loopback = loopback;
  }

  
  bool acquireColorImage(ColorImage& frame) throw() {
    bool success = false;
    try {
      Allocator<uint8> buffer(camera.getRegion().getDimension().getSize() *
                              Camera1394::getBitsPerPixel(camera.getPixelFormat())/8);
      success = camera.acquire(buffer.getElements(), buffer.getSize());
      if (success) {
        Camera1394::convert(frame, camera.getPixelFormat(), buffer.getElements());
      }
    } catch (Camera1394::Camera1394Exception& e) {
    }
    return success;
  }

  bool onAcquisition(unsigned int frame, uint8* buffer) throw() {
    if (Thread::getThread()->isTerminated()) {
      return false;
    }
    recordingSemaphore.wait();
    if (Thread::getThread()->isTerminated()) {
      return false;
    }
    
    if (!recordingQueue.isEmpty()) {
      guard.exclusiveLock();
      ColorImage* frame = recordingQueue.pop();
      guard.releaseLock();
      
      Camera1394::convert(*frame, camera.getPixelFormat(), buffer);
      
      guard.exclusiveLock();
      if (loopback) {
        playingQueue.push(frame);
        playingSemaphore.post();
      } else {
        writingQueue.push(frame);
        writingSemaphore.post();
      }
      guard.releaseLock();
    }
    return true;
  }

  bool onAcquisitionLostSync(unsigned int frame) throw() {
    Trace::message("Lost synchronization with beginning of frame");
    return true;
  }
  
  void recordContinuously() throw() {
    // fill cyclic buffer
    const unsigned int NUMBER_OF_FRAMES = 16;
    Array<Allocator<uint8> > frames(NUMBER_OF_FRAMES, Allocator<uint8>(), NUMBER_OF_FRAMES);
    Array<Camera1394::FrameBuffer> buffers(NUMBER_OF_FRAMES, Camera1394::FrameBuffer(), NUMBER_OF_FRAMES);
    
    Array<Allocator<uint8> >::Iterator frame = frames.getBeginIterator();
    Array<Camera1394::FrameBuffer>::Iterator buffer = buffers.getBeginIterator();
    const unsigned int requiredSize = (camera.getRegion().getDimension().getSize() *
                                       Camera1394::getBitsPerPixel(camera.getPixelFormat()))/8;
    for (unsigned int i = NUMBER_OF_FRAMES; i > 0; --i) {
      frame->setSize(requiredSize);
      *buffer++ = Camera1394::FrameBuffer(frame->getElements(), frame->getSize());
      ++frame;
    }
    bool success = camera.acquireContinuously(buffers, this);
    fout << "record: " << success << ENDL;
  }

  void recordSingle() throw() {
    while (!Thread::getThread()->isTerminated()) {
      recordingSemaphore.wait();
      if (Thread::getThread()->isTerminated()) {
        break;
      }
      
      if (!recordingQueue.isEmpty()) {
        guard.exclusiveLock();
        ColorImage* frame = recordingQueue.pop();
        guard.releaseLock();
        bool success = acquireColorImage(*frame);
        if (!success) {
          // TAG: frame should not be removed in the first place
        }
        guard.exclusiveLock();
        if (loopback) {
          playingQueue.push(frame);
          playingSemaphore.post();
        } else {
          writingQueue.push(frame);
          writingSemaphore.post();
        }
        guard.releaseLock();
      }
    }
  }
  
  void record() throw() {
    recordContinuously();
    fout << MESSAGE("Acquisition thread terminating") << ENDL;
  }
  
  void play() throw() {
    BMPEncoder encoder;
    unsigned int frameNumber = 0;
    
    while (!Thread::getThread()->isTerminated()) {
      playingSemaphore.wait();
      if (Thread::getThread()->isTerminated()) {
        break;
      }
      if (!playingQueue.isEmpty()) {
        guard.exclusiveLock();
        ColorImage* frame = playingQueue.pop();
        guard.releaseLock();

        // TAG: scale if too big
        
        StringOutputStream stream;
        stream << MESSAGE("frame") << setWidth(3) << ZEROPAD << frameNumber << '.' << encoder.getDefaultExtension()
               << FLUSH;
        encoder.write(stream.getString(), frame);
        ++frameNumber %= 128;
        
        guard.exclusiveLock();
        if (loopback) {
          recordingQueue.push(frame);
          recordingSemaphore.post();
        } else {
          readingQueue.push(frame);
          readingSemaphore.post();
        }
        guard.releaseLock();
      }
    }
    fout << MESSAGE("Update thread terminating") << ENDL;
  }

  void write() throw() {
    if (loopback) {
      return;
    }
    while (!Thread::getThread()->isTerminated()) {
      writingSemaphore.wait();
      if (Thread::getThread()->isTerminated()) {
        break;
      }
      if (!writingQueue.isEmpty()) {
        guard.exclusiveLock();
        ColorImage* frame = writingQueue.pop();
        guard.releaseLock();
        
        Allocator<char> buffer(128); // convert frame to stream
        
        try {
          streamSocket.write(pointer_cast<const char*>(buffer.getElements()), buffer.getByteSize());
        } catch (IOException& e) {
          fout << MESSAGE("IOException: ") << e.getMessage() << ENDL;
          Application::getApplication()->terminate();
          break;
        }
        guard.exclusiveLock();
        recordingQueue.push(frame);
        recordingSemaphore.post();
        guard.releaseLock();
      }
    }
    streamSocket.shutdownOutputStream();
    fout << MESSAGE("Writing thread terminating") << ENDL;
  }

  void read() throw() {
    if (loopback) {
      return;
    }
    while (!Thread::getThread()->isTerminated()) {
      readingSemaphore.wait();
      if (Thread::getThread()->isTerminated()) {
        break;
      }
      if (!readingQueue.isEmpty()) {
        guard.exclusiveLock();
        ColorImage* frame = readingQueue.pop();
        guard.releaseLock();

        Allocator<char> buffer(128);
        
        try {
          unsigned int bytesRead = streamSocket.read(pointer_cast<char*>(buffer.getElements()), buffer.getByteSize());
        } catch (EndOfFile& e) {
          fout << MESSAGE("Connection terminated by remote host") << ENDL;
          Application::getApplication()->terminate();
          break;
        } catch (IOException& e) {
          fout << MESSAGE("IO error: ") << e.getMessage() << ENDL;
          Application::getApplication()->terminate();
          break;
        }
        guard.exclusiveLock();
        playingQueue.push(frame);
        playingSemaphore.post();
        guard.releaseLock();
      }
    }
    fout << MESSAGE("Reading thread terminating") << ENDL;
  }

  bool hostAllowed(const InetAddress& host) throw() {
    // check address 172.30.* (mask then check if equal)
    return true; // allow every client
  }

  void server() throw() {
    fout << MESSAGE("Initializing server socket: ") << endPoint << ENDL;
    ServerSocket serverSocket(endPoint.getAddress(), endPoint.getPort(), 1);

    while (true) {
      fout << MESSAGE("Waiting for client...") << ENDL;
      streamSocket = serverSocket.accept();
      fout << MESSAGE("Connection from: ") << InetEndPoint(streamSocket.getAddress(), streamSocket.getPort()) << ENDL;

      if (hostAllowed(streamSocket.getAddress())) {
        break;
      }
      
      fout << MESSAGE("Host denied access") << ENDL;
    }
  }

  void client() throw() {
    fout << MESSAGE("Connecting to server: ") << endPoint << ENDL;
    streamSocket.connect(endPoint.getAddress(), endPoint.getPort());
    fout << MESSAGE("Connected to: ") << InetEndPoint(streamSocket.getAddress(), streamSocket.getPort())<< ENDL;
  }
  
  void run() throw() {
    Dimension dimension(camera.getRegion().getDimension());
    const unsigned int NUMBER_OF_BUFFERS = 16;
    
    fout << MESSAGE("Allocating buffers...") << ENDL;
    for (unsigned int i = 0; i < NUMBER_OF_BUFFERS; ++i) {
      recordingQueue.push(new ColorImage(dimension));
      recordingSemaphore.post();
    }
    for (unsigned int i = 0; i < NUMBER_OF_BUFFERS; ++i) {
      readingQueue.push(new ColorImage(dimension));
      readingSemaphore.post();
    }
    
    fout << MESSAGE("Creating threads...") << ENDL;
    Thread recorderThread(&recorder);
    Thread playerThread(&player);
    Thread readerThread(&reader);
    Thread writerThread(&writer);

    if (!loopback) {
      if (isServer) {
        server();
      } else {
        client();
      }
    }

    fout << MESSAGE("Starting threads...") << ENDL;
    recorderThread.start();
    playerThread.start();
    readerThread.start();
    writerThread.start();

    fout << MESSAGE("Waiting...") << ENDL;
    Timer timer;

    for (unsigned int i = 0; i < 30*(1000/500); ++i) {
      if (Application::getApplication()->isTerminated()) {
        break;
      }
      Thread::millisleep(maximum<int>((i+1)*500 - timer.getLiveMicroseconds()/1000, 0));
      if (!loopback) {
        fout << MESSAGE("Time: ") << setPrecision(5) << timer.getLiveMicroseconds()/1000000. << MESSAGE(" - ")
             << MESSAGE("Recording queue: ") << setWidth(2) << recordingQueue.getSize() << MESSAGE(" - ")
             << MESSAGE("Playing queue: ") << setWidth(2) << playingQueue.getSize() << MESSAGE(" - ")
             << MESSAGE("Reading queue: ") << setWidth(2) << readingQueue.getSize() << MESSAGE(" - ")
             << MESSAGE("Writing queue: ") << setWidth(2) << writingQueue.getSize() << CR
             << FLUSH;
      } else {
        fout << MESSAGE("Time: ") << setPrecision(5) << timer.getLiveMicroseconds()/1000000. << MESSAGE(" - ")
             << MESSAGE("Recording queue: ") << setWidth(2) << recordingQueue.getSize() << MESSAGE(" - ")
             << MESSAGE("Playing queue: ") << setWidth(2) << playingQueue.getSize() << CR
             << FLUSH;
      }
    }
    fout << ENDL;

    if (!Application::getApplication()->isTerminated()) {
      fout << MESSAGE("Voluntary termination") << ENDL;
      Application::getApplication()->terminate();
    }

    fout << MESSAGE("Waiting for threads to terminate...") << ENDL;
    recorderThread.terminate();
    recordingSemaphore.post();
    recorderThread.join();
    playerThread.terminate();
    playingSemaphore.post();
    playerThread.join();

    readerThread.terminate();
    readingSemaphore.post();
    readerThread.join();
    writerThread.terminate();
    writingSemaphore.post();
    writerThread.join();

    fout << MESSAGE("Releasing buffers...") << ENDL;
    while (!recordingQueue.isEmpty()) {
      delete recordingQueue.pop();
    }
    while (!playingQueue.isEmpty()) {
      delete playingQueue.pop();
    }
    while (!readingQueue.isEmpty()) {
      delete readingQueue.pop();
    }
    while (!writingQueue.isEmpty()) {
      delete writingQueue.pop();
    }

    fout << MESSAGE("Completed") << ENDL;
  }
  
  ~VideoPhoneServlet() throw() {}
};

class VideoPhoneApplication : public Application, public Camera1394::AcquisitionListener {
private:

  /** The major version number of the application. */
  static const unsigned int MAJOR_VERSION = 1;
  /** The minor version number of the application. */
  static const unsigned int MINOR_VERSION = 0;
  
  static const StringLiteral PIXEL_FORMAT_DESCRIPTION[7];
  
  bool loopback;
  bool isServer;
  String host;
  unsigned short port;
  VideoPhoneServlet* videoPhoneServlet;
  IEEE1394 ieee1394;
  Camera1394 camera;

  Timer timer;
  unsigned int desiredNumberOfFrames;
  unsigned int numberOfFrames;
public:
  
  /** The default port. */
  static const unsigned short DEFAULT_PORT = 1234;
  static const unsigned int DEFAULT_CHANNELS = 1;
  static const unsigned int DEFAULT_SAMPLE_RATE = 44100;

  enum Verbosity {
    VERBOSITY_SILENT,
    VERBOSITY_NORMAL,
    VERBOSITY_VERBOSE,
    VERBOSITY_DEBUG,
    VERBOSITY_EVERYTHING
  };
  
  Verbosity verbosity;
  
  /** Command. */
  enum Command {
    COMMAND_ERROR,
    COMMAND_NO_ARGUMENTS,
    COMMAND_HELP,
    COMMAND_DUMP_IDENTIFIER,
    COMMAND_DUMP_VERSION,
    COMMAND_LIST_ADAPTERS,
    COMMAND_LIST_NODES,
    COMMAND_LIST_CAMERAS,
    COMMAND_DUMP_CAPABILITIES,
    COMMAND_DUMP_MODE_CAPABILITIES,
    COMMAND_ACQUIRE,
    COMMAND_LOOPBACK,
    COMMAND_CONNECT
  };

  Command command;
  
  EUI64 adapterGuid;
  EUI64 cameraGuid;

  // Camera mode settings
  bool setMode;
  Camera1394::Mode mode;
  bool setFrameRate;
  Camera1394::FrameRate rate;
  bool setPixelFormat;
  Camera1394::PixelFormat pixelFormat;
  Dimension frameDimension;
  Point2D frameOffset;
  bool setBrightness;
  unsigned int brightnessValue;
  bool setAutoExposure;
  unsigned int autoExposureValue;
  bool setWhiteBalance;
  unsigned int blueRatioValue;
  unsigned int redRatioValue;
  bool setHue;
  unsigned int hueValue;
  bool setSaturation;
  unsigned int saturationValue;
  bool setGamma;
  unsigned int gammaValue;
  bool setShutter;
  unsigned int shutterValue;
  bool setGain;
  unsigned int gainValue;
  
  String filename;
  
  VideoPhoneApplication(int numberOfArguments, const char* arguments[], const char* environment[]) throw()
    : Application(MESSAGE("videophone"), numberOfArguments, arguments, environment) {
    
    verbosity = VERBOSITY_NORMAL;
    
    command = COMMAND_ERROR;
    loopback = false;
    isServer = true;
    port = DEFAULT_PORT;
    
    // camera mode settings
    setMode = false;
    setFrameRate = false;
    setPixelFormat = false;
    setBrightness = false;
    setAutoExposure = false;
    setWhiteBalance = false;
    setHue = false;
    setSaturation = false;
    setGamma = false;
    setShutter = false;
    setGain = false;
  }

  class InvalidArgument : public Exception {
  public:

    inline InvalidArgument(const char* message) throw() : Exception(message) {
    }
  };
  
  void processArguments() throw(InvalidArgument, InvalidFormat) {
    Array<String> arguments = getArguments();
    
    if (arguments.getSize() == 0) {
      command = COMMAND_NO_ARGUMENTS;
      return;
    }
    
    if (arguments.getSize() == 1) {
      String argument = arguments[0];
      if (argument == "--dumpidentifier") {
        command = COMMAND_DUMP_IDENTIFIER;
      } else if (argument == "--dumpversion") {
        command = COMMAND_DUMP_VERSION;
      } else if (argument == "--listadapters") {
        command = COMMAND_LIST_ADAPTERS;
      }
    }

    if (command == COMMAND_ERROR) {
      Array<String>::ReadEnumerator enu = arguments.getReadEnumerator();
      while (enu.hasNext()) {
        const String* argument = enu.next();
        if (*argument == "--help") {
          command = COMMAND_HELP; // no need to continue argument processing
          return;
        } else if (*argument == "--adapter") {
          assert(adapterGuid.isInvalid(), InvalidArgument("IEEE 1394 adapter has already been specified"));
          assert(enu.hasNext(), InvalidArgument("EUI-64 of IEEE 1394 adapter is missing"));
          adapterGuid = EUI64(*enu.next());
        } else if (*argument == "--camera") {
          assert(cameraGuid.isInvalid(), InvalidArgument("IEEE 1394 camera has already been specified"));
          assert(enu.hasNext(), InvalidArgument("EUI-64 of IEEE 1394 camera is missing"));
          cameraGuid = EUI64(*enu.next());
        } else if (*argument == "--listnodes") {
          assert(command == COMMAND_ERROR, InvalidArgument("Command has already been specified"));
          command = COMMAND_LIST_NODES;
        } else if (*argument == "--listcameras") {
          assert(command == COMMAND_ERROR, InvalidArgument("Command has already been specified"));
          command = COMMAND_LIST_CAMERAS;
        } else if (*argument == "--capabilities") {
          assert(command == COMMAND_ERROR, InvalidArgument("Command has already been specified"));
          command = COMMAND_DUMP_CAPABILITIES;
        } else if (*argument == "--modecaps") {
          assert(command == COMMAND_ERROR, InvalidArgument("Command has already been specified"));
          command = COMMAND_DUMP_MODE_CAPABILITIES;
        } else if (*argument == "--acquire") {
          assert(command == COMMAND_ERROR, InvalidArgument("Command has already been specified"));
          assert(enu.hasNext(), InvalidArgument("File name is missing"));
          filename = *enu.next();
          command = COMMAND_ACQUIRE;
        } else if (*argument == "--loopback") {
          assert(command == COMMAND_ERROR, InvalidArgument("Command has already been specified"));
          command = COMMAND_LOOPBACK;
        } else if (*argument == "--host") {
          assert(command == COMMAND_ERROR, InvalidArgument("Command has already been specified"));
          assert(enu.hasNext(), InvalidArgument("Host value missing"));
          host = *enu.next();
          command = COMMAND_CONNECT;
        } else if (*argument == "--port") {
          assert(enu.hasNext(), InvalidArgument("Port value missing"));
          const String* rateString = enu.next();
          unsigned int temp = UnsignedInteger(*rateString).getValue();
          assert((temp > 0) && (temp <= 0xffff), InvalidArgument("Port is invalid"));
          port = temp;
        } else if (*argument == "--mode") {
          assert(enu.hasNext(), InvalidArgument("Camera mode missing"));
          unsigned int temp = UnsignedInteger::parse(*enu.next(), UnsignedInteger::DEC|UnsignedInteger::HEX);
          assert(temp <= Camera1394::PARTIAL_IMAGE_MODE_7, InvalidArgument("Invalid camera mode"));
          mode = static_cast<Camera1394::Mode>(temp);
          setMode = true;
        } else if (*argument == "--rate") {
          assert(enu.hasNext(), InvalidArgument("Frame rate missing"));
          const String temp = *enu.next();
          if (temp == "60") {
            rate = Camera1394::RATE_60;
          } else if (temp == "30") {
            rate = Camera1394::RATE_30;
          } else if (temp == "15") {
            rate = Camera1394::RATE_15;
          } else if (temp == "7.5") {
            rate = Camera1394::RATE_7_5;
          } else if (temp == "3.75") {
            rate = Camera1394::RATE_3_75;
          } else if (temp == "1.875") { 
            rate = Camera1394::RATE_1_875;
          } else {
            throw InvalidArgument("Invalid frame rate");
          }
          setFrameRate = true;
        } else if (*argument == "--format") {
          assert(enu.hasNext(), InvalidArgument("Pixel format missing"));
          unsigned int temp = UnsignedInteger::parse(*enu.next(), UnsignedInteger::DEC|UnsignedInteger::HEX);
          assert(temp <= Camera1394::RGB_16BIT, InvalidArgument("Invalid pixel format"));
          pixelFormat = static_cast<Camera1394::PixelFormat>(temp);
          setPixelFormat = true;
        } else if (*argument == "--dimension") {
          assert(enu.hasNext(), InvalidArgument("Frame dimension"));
          // FIXME
        } else if (*argument == "--offset") {
          assert(enu.hasNext(), InvalidArgument("Frame offset missing"));
          // FIXME
        } else if (*argument == "--brightness") {
          assert(enu.hasNext(), InvalidArgument("Brightness value missing"));
          brightnessValue = UnsignedInteger::parse(*enu.next(), UnsignedInteger::DEC|UnsignedInteger::HEX);
          setBrightness = true;
        } else if (*argument == "--exposure") {
          assert(enu.hasNext(), InvalidArgument("Auto exposure value missing"));
          autoExposureValue = UnsignedInteger::parse(*enu.next(), UnsignedInteger::DEC|UnsignedInteger::HEX);
          setAutoExposure = true;
        } else if (*argument == "--balance") {
          assert(enu.hasNext(), InvalidArgument("Blue ratio missing for white balance"));
          blueRatioValue = UnsignedInteger::parse(*enu.next(), UnsignedInteger::DEC|UnsignedInteger::HEX);
          assert(enu.hasNext(), InvalidArgument("Red ratio missing for white balance"));
          redRatioValue = UnsignedInteger::parse(*enu.next(), UnsignedInteger::DEC|UnsignedInteger::HEX);
          setWhiteBalance = true;
        } else if (*argument == "--hue") {
          assert(enu.hasNext(), InvalidArgument("Hue value missing"));
          hueValue = UnsignedInteger::parse(*enu.next(), UnsignedInteger::DEC|UnsignedInteger::HEX);
          setHue = true;
        } else if (*argument == "--saturation") {
          assert(enu.hasNext(), InvalidArgument("Saturation value missing"));
          saturationValue = UnsignedInteger::parse(*enu.next(), UnsignedInteger::DEC|UnsignedInteger::HEX);
          setSaturation = true;
        } else if (*argument == "--gamma") {
          assert(enu.hasNext(), InvalidArgument("Gamma value missing"));
          gammaValue = UnsignedInteger::parse(*enu.next(), UnsignedInteger::DEC|UnsignedInteger::HEX);
          setGamma = true;
        } else if (*argument == "--shutter") {
          assert(enu.hasNext(), InvalidArgument("Shutter value missing"));
          shutterValue = UnsignedInteger::parse(*enu.next(), UnsignedInteger::DEC|UnsignedInteger::HEX);
          setShutter = true;
        } else if (*argument == "--gain") {
          assert(enu.hasNext(), InvalidArgument("Gain value missing"));
          gainValue = UnsignedInteger::parse(*enu.next(), UnsignedInteger::DEC|UnsignedInteger::HEX);
          setGain = true;
        } else {
          command = COMMAND_ERROR; // invalid argument
        }
      }
    }

    // add constraints here
  }
  
  void dumpHeader() throw() {
    fout << getFormalName() << MESSAGE(" version ") << MAJOR_VERSION << '.' << MINOR_VERSION << EOL
         << MESSAGE("Generic Image Processing (GIP) Framework (Test Suite)") << EOL
         << MESSAGE("http://www.mip.sdu.dk/~fonseca/gip") << EOL
         << MESSAGE("Copyright (C) 2002 by Rene Moeller Fonseca <fonseca@mip.sdu.dk>") << EOL
         << ENDL;
  }

  void dumpHelp() throw() {
    fout << MESSAGE("Usage: ") << getFormalName() << MESSAGE(" [--adapter EUI-64] [--camera EUI-64] [--host host]") << EOL
         << EOL
         << MESSAGE("Options:") << EOL
         << MESSAGE(" --help           Dumps this message.") << EOL
         << MESSAGE(" --dumpidentifier Dumps the unique identifier of the application.") << EOL
         << MESSAGE(" --dumpversion    Dumps the version.") << EOL
         << EOL
         << MESSAGE(" --loopback       Selects loop-back mode. This option is mutual exclusive with") << EOL
         << MESSAGE("                  --host and --port.") << EOL
         << MESSAGE(" --host           Selects the host to connect to (name or IP address)") << EOL
         << MESSAGE(" --port           Selects the post to connect to (name or value). The default") << EOL
         << MESSAGE("                  port (") << DEFAULT_PORT <<  MESSAGE(") is used if this option is omitted.") << EOL
         << EOL
         << MESSAGE(" --adapter EUI-64 The EUI-64 of the IEEE 1394 adapter. The default adapter is") << EOL
         << MESSAGE("                  used if this option is omitted.") << EOL
         << MESSAGE(" --camera EUI-64  Specifies the EUI-64 of the camera to be used. If the camera") << EOL
         << MESSAGE("                  is not specified the camera selected automatically.") << EOL
         << EOL
         << MESSAGE(" --listadapters   Lists the available IEEE 1394 adapters.") << EOL
         << MESSAGE(" --listnodes      Lists all nodes on the IEEE 1394 bus.") << EOL
         << MESSAGE(" --listcameras    Lists the available IEEE 1394 cameras.") << EOL
         << MESSAGE(" --capabilities   Lists the capabilities of the specified IEEE 1394 camera.") << EOL
         << MESSAGE(" --modecaps       Lists the mode specific capabilities for the specified") << EOL
         << MESSAGE("                  camera and mode. You can use this to list the valid range of") << EOL
         << MESSAGE("                  the supported features.") << EOL
         << EOL
         << MESSAGE(" --mode #         Specifies the camera mode.") << EOL
         << MESSAGE(" --format #       Specifies the pixel format.") << EOL
         << MESSAGE(" --rate #         Specifies the frame rate.") << EOL
         << MESSAGE(" --dimension w h  Specifies the dimension of the frame (width and height).") << EOL
         << MESSAGE(" --offset h v     Specifies the offset of the frame (horizontal and vertical).") << EOL
         << MESSAGE(" --brightness #   Specifies the brightness value.") << EOL
         << MESSAGE(" --exposure #     Specifies the auto exposure value.") << EOL
         << MESSAGE(" --balance # #    Specifies the white balance (Cb/blue and Cr/red ratios).") << EOL
         << MESSAGE(" --hue #          Specifies the hue value.") << EOL
         << MESSAGE(" --saturation #   Specifies the saturation value.") << EOL
         << MESSAGE(" --gamma #        Specifies the gamma value.") << EOL
         << MESSAGE(" --shutter #      Specifies the shutter value.") << EOL
         << MESSAGE(" --gain #         Specifies the gain value.") << EOL
         << EOL
         << MESSAGE(" --acquire file   Specifies that one frame should be acquired and exported to") << EOL
         << MESSAGE("                  the file.") << EOL
         << MESSAGE(" --export file    Specifies that the first frame should be saved to the file.") << EOL
         << ENDL;
  }

  void dumpIdentifier() throw() {
    fout << MESSAGE("mip.sdu.dk/~fonseca/gip/testsuite/videophone") << ENDL;
  }
  
  void dumpVersion() throw() {
    fout << MAJOR_VERSION << '.' << MINOR_VERSION << ENDL;
  }

  void listAdapters() throw(IEEE1394Exception) {
    Array<EUI64> adapters = ieee1394.getAdapters();
    if (adapters.getSize() == 0) {
      fout << MESSAGE("No IEEE 1394 adapters available") << ENDL;
    } else {
      Array<EUI64>::ReadEnumerator enu = adapters.getReadEnumerator();
      fout << MESSAGE("IEEE 1394 adapters:") << EOL;
      while (enu.hasNext()) {
        fout << MESSAGE("  ") << *enu.next() << EOL;
      }
      fout << FLUSH;
    }
  }
  
  void listNodes() throw(IEEE1394Exception) {
    if (adapterGuid.isInvalid()) {
      ieee1394.open();
    } else {
      ieee1394.open(adapterGuid);
    }
    
    fout << MESSAGE("Available IEEE 1394 nodes:") << EOL;
    
    for (unsigned int node = 0; node < ieee1394.getNumberOfNodes(); ++node) {
      fout << MESSAGE("  ") << MESSAGE("Node: ") << ieee1394.getIdentifier(node) << EOL
           << MESSAGE("    ") << MESSAGE("Physical id: ") << node << ENDL;
      
      unsigned int vendor = ieee1394.getVendorId(node);
      fout << MESSAGE("    ") << MESSAGE("Vendor id: ")
           << HEX << setWidth(2) << ZEROPAD << NOPREFIX << ((vendor >> 16) & 0xff) << ':'
           << HEX << setWidth(2) << ZEROPAD << NOPREFIX << ((vendor >> 8) & 0xff) << ':'
           << HEX << setWidth(2) << ZEROPAD << NOPREFIX << (vendor & 0xff) << ENDL;
      
      unsigned int capabilities = ieee1394.getCapabilities(node);
      if (capabilities) {
        fout << MESSAGE("      ") << MESSAGE("Capabilities:") << EOL;
        if (capabilities & IEEE1394::ISOCHRONOUS_RESOURCE_MANAGER_CAPABLE) {
          fout << MESSAGE("        ") << MESSAGE("Isochronous resource manager") << EOL;
        }
        if (capabilities & IEEE1394::CYCLE_MASTER_CAPABLE) {
          fout << MESSAGE("        ") << MESSAGE("Cycle master") << EOL;
        }
        if (capabilities & IEEE1394::ISOCHRONOUS_TRANSACTION_CAPABLE) {
          fout << MESSAGE("        ") << MESSAGE("Isochronous transmission") << EOL;
        }
        if (capabilities & IEEE1394::BUS_MASTER_CAPABLE) {
          fout << MESSAGE("        ") << MESSAGE("Bus master") << EOL;
        }
        if (capabilities & IEEE1394::POWER_MANAGER_CAPABLE) {
          fout << MESSAGE("        ") << MESSAGE("Power manager") << EOL;
        }
      }
        fout << MESSAGE("    ") << MESSAGE("Maximum asynchronous payload: ") << ieee1394.getMaximumPayload(node) << EOL
             << ENDL;
    }
  }
  
  void listCameras() throw(Camera1394::Camera1394Exception, IEEE1394Exception) {
    Array<EUI64> cameras = camera.getCameras();
    
    if (cameras.getSize() == 0) {
      fout << MESSAGE("No IEEE 1394 cameras available") << ENDL;
    } else {
      unsigned int i = 0;
      Array<EUI64>::ReadEnumerator enu = cameras.getReadEnumerator();
      fout << MESSAGE("IEEE 1394 cameras:") << EOL;
      while (enu.hasNext()) {
        fout << MESSAGE("  ") << *enu.next() << EOL;
      }
      fout << FLUSH;
    }
  }
  
  void dumpFeatureInquery(const String& feature, const Camera1394::GenericFeatureDescriptor& descriptor) const throw() {
    fout << MESSAGE("Feature: ") << feature << EOL
         << MESSAGE("  Available: ") << descriptor.available << EOL
         << MESSAGE("  Automatic adjustment mode: ") << descriptor.autoAdjustmentMode << EOL
         << MESSAGE("  Readable: ") << descriptor.readable << EOL
         << MESSAGE("  Switchable: ") << descriptor.switchable << EOL
         << MESSAGE("  Automatic mode: ") << descriptor.automaticMode << EOL
         << MESSAGE("  Manual mode: ") << descriptor.manualMode << EOL
         << MESSAGE("  Minimume value: ") << descriptor.minimum << EOL
         << MESSAGE("  Maximum value: ") << descriptor.maximum << ENDL;
  }
  
  void dumpTriggerFeatureInquery(const String& feature, const Camera1394::TriggerFeatureDescriptor& descriptor) const throw() {
    fout << MESSAGE("Feature: ") << feature << EOL
         << MESSAGE("  Available: ") << descriptor.available << EOL
         << MESSAGE("  Readable: ") << descriptor.readable << EOL
         << MESSAGE("  Switchable: ") << descriptor.switchable << EOL
         << MESSAGE("  Polarity: ") << descriptor.polarity << EOL
         << MESSAGE("  Signals: ") << HEX << setWidth(10) << ZEROPAD << descriptor.availableSignals << ENDL;
  }

  void dumpCameraMode() throw(Camera1394::Camera1394Exception) {
    dumpCamera();
    
    if (setMode) {
      assert(camera.isModeSupported(mode), Camera1394::Camera1394Exception("Mode is not supported"));
      camera.setMode(mode);
    }
    if (setFrameRate) {
      assert(camera.isFrameRateSupported(rate), Camera1394::Camera1394Exception("Frame rate is not supported"));
      camera.setFrameRate(rate);
    }
    if (setPixelFormat) {
      assert(camera.getPixelFormats() & (1 << pixelFormat), Camera1394::Camera1394Exception("Pixel format is not supported"));
      camera.setPixelFormat(pixelFormat);
    }
    
    const Camera1394::IsochronousTransmission& transmission = camera.getTransmission();
    
    fout << MESSAGE("Current mode: ") << camera.getModeAsString(camera.getMode()) << EOL
         << MESSAGE("  Frame rate: ") << Camera1394::getFrameRateAsValue(camera.getFrameRate())/65536.0 << MESSAGE(" frames/second") << EOL
         << MESSAGE("  Pixel format: ") << PIXEL_FORMAT_DESCRIPTION[camera.getPixelFormat()] << EOL
         << MESSAGE("  Region: ") << EOL
         << MESSAGE("    Dimension: ") << camera.getRegion().getDimension() << EOL
         << MESSAGE("    Offset: ") << camera.getRegion().getOffset() << EOL
         << MESSAGE("  Isochronous transmission: ") << EOL
         << MESSAGE("    pixelsPerFrame: ") << transmission.pixelsPerFrame << EOL
         << MESSAGE("    totalBytesPerFrame: ") << transmission.totalBytesPerFrame << EOL
         << MESSAGE("    unitBytesPerPacket: ") << transmission.unitBytesPerPacket << EOL
         << MESSAGE("    maximumBytesPerPacket: ") << transmission.maximumBytesPerPacket << EOL
         << MESSAGE("    bytesPerPacket: ") << transmission.bytesPerPacket << EOL
         << MESSAGE("    recommendedBytesPerPacket: ") << transmission.recommendedBytesPerPacket << EOL
         << MESSAGE("    packetsPerFrame: ") << transmission.packetsPerFrame << EOL
         << ENDL;
    
    fout << MESSAGE("Isochronous channel: ") << camera.getIsochronousChannel() << EOL
         << MESSAGE("Isochronous speed: ") << camera.getIsochronousSpeed() << EOL
         << ENDL;
    
    if (camera.isFeatureSupported(Camera1394::BRIGHTNESS_CONTROL)) {
      dumpFeatureInquery(MESSAGE("Brightness"), camera.getFeatureDescriptor(Camera1394::BRIGHTNESS_CONTROL));
    }
    if (camera.isFeatureSupported(Camera1394::AUTO_EXPOSURE_CONTROL)) {
      dumpFeatureInquery(MESSAGE("Auto exposure"), camera.getFeatureDescriptor(Camera1394::AUTO_EXPOSURE_CONTROL));
    }
    if (camera.isFeatureSupported(Camera1394::SHARPNESS_CONTROL)) {
      dumpFeatureInquery(MESSAGE("Sharpness"), camera.getFeatureDescriptor(Camera1394::SHARPNESS_CONTROL));
    }
    if (camera.isFeatureSupported(Camera1394::WHITE_BALANCE_CONTROL)) {
      dumpFeatureInquery(MESSAGE("White balance"), camera.getFeatureDescriptor(Camera1394::WHITE_BALANCE_CONTROL));
    }
    if (camera.isFeatureSupported(Camera1394::HUE_CONTROL)) {
      dumpFeatureInquery(MESSAGE("Hue"), camera.getFeatureDescriptor(Camera1394::HUE_CONTROL));
    }
    if (camera.isFeatureSupported(Camera1394::SATURATION_CONTROL)) {
      dumpFeatureInquery(MESSAGE("Saturation"), camera.getFeatureDescriptor(Camera1394::SATURATION_CONTROL));
    }
    if (camera.isFeatureSupported(Camera1394::GAMMA_CONTROL)) {
      dumpFeatureInquery(MESSAGE("Gamma"), camera.getFeatureDescriptor(Camera1394::GAMMA_CONTROL));
    }
    if (camera.isFeatureSupported(Camera1394::SHUTTER_CONTROL)) {
      dumpFeatureInquery(MESSAGE("Shutter"), camera.getFeatureDescriptor(Camera1394::SHUTTER_CONTROL));
    }
    if (camera.isFeatureSupported(Camera1394::GAIN_CONTROL)) {
      dumpFeatureInquery(MESSAGE("Gain"), camera.getFeatureDescriptor(Camera1394::GAIN_CONTROL));
    }
    if (camera.isFeatureSupported(Camera1394::IRIS_CONTROL)) {
      dumpFeatureInquery(MESSAGE("IRIS"), camera.getFeatureDescriptor(Camera1394::IRIS_CONTROL));
    }
    if (camera.isFeatureSupported(Camera1394::FOCUS_CONTROL)) {
      dumpFeatureInquery(MESSAGE("Focus"), camera.getFeatureDescriptor(Camera1394::FOCUS_CONTROL));
    }
    if (camera.isFeatureSupported(Camera1394::TEMPERATURE_CONTROL)) {
      dumpFeatureInquery(MESSAGE("Temperature"), camera.getFeatureDescriptor(Camera1394::TEMPERATURE_CONTROL));
    }
    if (camera.isFeatureSupported(Camera1394::TRIGGER_CONTROL)) {
      dumpTriggerFeatureInquery(MESSAGE("Trigger"), camera.getTriggerFeatureDescriptor());
    }
    if (camera.isFeatureSupported(Camera1394::ZOOM_CONTROL)) {
      dumpFeatureInquery(MESSAGE("Zoom"), camera.getFeatureDescriptor(Camera1394::ZOOM_CONTROL));
    }
    if (camera.isFeatureSupported(Camera1394::PAN_CONTROL)) {
      dumpFeatureInquery(MESSAGE("Pan"), camera.getFeatureDescriptor(Camera1394::PAN_CONTROL));
    }
    if (camera.isFeatureSupported(Camera1394::TILT_CONTROL)) {
      dumpFeatureInquery(MESSAGE("Tilt"), camera.getFeatureDescriptor(Camera1394::TILT_CONTROL));
    }
    if (camera.isFeatureSupported(Camera1394::OPTICAL_FILTER_CONTROL)) {
      dumpFeatureInquery(MESSAGE("Optical filter"), camera.getFeatureDescriptor(Camera1394::OPTICAL_FILTER_CONTROL));
    }
    if (camera.isFeatureSupported(Camera1394::CAPTURE_SIZE)) {
      dumpFeatureInquery(MESSAGE("Capture size"), camera.getFeatureDescriptor(Camera1394::CAPTURE_SIZE));
    }
    if (camera.isFeatureSupported(Camera1394::CAPTURE_QUALITY)) {
      dumpFeatureInquery(MESSAGE("Capture quality"), camera.getFeatureDescriptor(Camera1394::CAPTURE_QUALITY));
    }
    fout << EOL << ENDL;
    
    static const StringLiteral OPERATING_MODES[] = {
      MESSAGE("disabled"), MESSAGE("automatic"), MESSAGE("automatic adjustment"), MESSAGE("manual")
    };
    
    if (camera.isFeatureReadable(Camera1394::BRIGHTNESS_CONTROL)) {
      fout << MESSAGE("Feature state - Brightness") << EOL
           << MESSAGE("  Mode: ") << OPERATING_MODES[camera.getFeatureOperatingMode(Camera1394::BRIGHTNESS_CONTROL)] << EOL
           << MESSAGE("  Value: ") << camera.getBrightness() << ENDL;
    }
    
    if (camera.isFeatureReadable(Camera1394::AUTO_EXPOSURE_CONTROL)) {
      fout << MESSAGE("Feature state - Auto exposure") << EOL
           << MESSAGE("  Mode: ") << OPERATING_MODES[camera.getFeatureOperatingMode(Camera1394::AUTO_EXPOSURE_CONTROL)] << EOL
           << MESSAGE("  Value: ") << camera.getAutoExposure() << ENDL;
    }
    
    if (camera.isFeatureReadable(Camera1394::SHARPNESS_CONTROL)) {
      fout << MESSAGE("Feature state - Sharpness") << EOL
           << MESSAGE("  Mode: ") << OPERATING_MODES[camera.getFeatureOperatingMode(Camera1394::SHARPNESS_CONTROL)] << EOL
           << MESSAGE("  Value: ") << camera.getSharpness() << ENDL;
    }
    
    if (camera.isFeatureReadable(Camera1394::WHITE_BALANCE_CONTROL)) {
      fout << MESSAGE("Feature state - White balance") << EOL
           << MESSAGE("  Mode: ") << OPERATING_MODES[camera.getFeatureOperatingMode(Camera1394::WHITE_BALANCE_CONTROL)] << EOL
           << MESSAGE("  Cb/blue ratio: ") << camera.getWhiteBalanceBlueRatio() << EOL
           << MESSAGE("  Cr/red ratio: ") << camera.getWhiteBalanceRedRatio() << ENDL;
    }
    
    if (camera.isFeatureReadable(Camera1394::HUE_CONTROL)) {
      fout << MESSAGE("Feature state - Hue") << EOL
           << MESSAGE("  Mode: ") << OPERATING_MODES[camera.getFeatureOperatingMode(Camera1394::HUE_CONTROL)] << EOL
           << MESSAGE("  Value: ") << camera.getHue() << ENDL;
    }
    
    if (camera.isFeatureReadable(Camera1394::SATURATION_CONTROL)) {
      fout << MESSAGE("Feature state - Saturation") << EOL
           << MESSAGE("  Mode: ") << OPERATING_MODES[camera.getFeatureOperatingMode(Camera1394::SATURATION_CONTROL)] << EOL
           << MESSAGE("  Value: ") << camera.getSaturation() << ENDL;
    }
    
    if (camera.isFeatureReadable(Camera1394::GAMMA_CONTROL)) {
      fout << MESSAGE("Feature state - Gamma") << EOL
           << MESSAGE("  Mode: ") << OPERATING_MODES[camera.getFeatureOperatingMode(Camera1394::GAMMA_CONTROL)] << EOL
           << MESSAGE("  Value: ") << camera.getGamma() << ENDL;
    }

    if (camera.isFeatureReadable(Camera1394::SHUTTER_CONTROL)) {
      fout << MESSAGE("Feature state - Shutter") << EOL
           << MESSAGE("  Mode: ") << OPERATING_MODES[camera.getFeatureOperatingMode(Camera1394::SHUTTER_CONTROL)] << EOL
           << MESSAGE("  Value: ") << camera.getShutter() << ENDL;
    }
    
    if (camera.isFeatureReadable(Camera1394::GAIN_CONTROL)) {
      fout << MESSAGE("Feature state - Gain") << EOL
           << MESSAGE("  Mode: ") << OPERATING_MODES[camera.getFeatureOperatingMode(Camera1394::GAIN_CONTROL)] << EOL
           << MESSAGE("  Value: ") << camera.getGain() << ENDL;
    }
    
    if (camera.isFeatureReadable(Camera1394::IRIS_CONTROL)) {
      fout << MESSAGE("Feature state - IRIS") << EOL
           << MESSAGE("  Mode: ") << OPERATING_MODES[camera.getFeatureOperatingMode(Camera1394::IRIS_CONTROL)] << EOL
           << MESSAGE("  Value: ") << camera.getIRIS() << ENDL;
    }
    
    if (camera.isFeatureReadable(Camera1394::FOCUS_CONTROL)) {
      fout << MESSAGE("Feature state - Focus") << EOL
           << MESSAGE("  Mode: ") << OPERATING_MODES[camera.getFeatureOperatingMode(Camera1394::FOCUS_CONTROL)] << EOL
           << MESSAGE("  Value: ") << camera.getFocus() << ENDL;
    }
    
    if (camera.isFeatureReadable(Camera1394::TEMPERATURE_CONTROL)) {
      fout << MESSAGE("Feature state - Temperature") << EOL
           << MESSAGE("  Mode: ") << OPERATING_MODES[camera.getFeatureOperatingMode(Camera1394::TEMPERATURE_CONTROL)] << EOL
           << MESSAGE("  Target value: ") << camera.getTargetTemperature() << EOL
           << MESSAGE("  Current value: ") << camera.getTemperature() << ENDL;
    }
    
    if (camera.isFeatureReadable(Camera1394::ZOOM_CONTROL)) {
      fout << MESSAGE("Feature state - Zoom") << EOL
           << MESSAGE("  Mode: ") << OPERATING_MODES[camera.getFeatureOperatingMode(Camera1394::ZOOM_CONTROL)] << EOL
           << MESSAGE("  Value: ") << camera.getZoom() << ENDL;
    }
    
    if (camera.isFeatureReadable(Camera1394::PAN_CONTROL)) {
      fout << MESSAGE("Feature state - Pan") << EOL
           << MESSAGE("  Mode: ") << OPERATING_MODES[camera.getFeatureOperatingMode(Camera1394::PAN_CONTROL)] << EOL
           << MESSAGE("  Value: ") << camera.getPan() << ENDL;
    }
    
    if (camera.isFeatureReadable(Camera1394::TILT_CONTROL)) {
      fout << MESSAGE("Feature state - Tilt") << EOL
           << MESSAGE("  Mode: ") << OPERATING_MODES[camera.getFeatureOperatingMode(Camera1394::TILT_CONTROL)] << EOL
           << MESSAGE("  Value: ") << camera.getTilt() << ENDL;
    }
    
    if (camera.isFeatureReadable(Camera1394::OPTICAL_FILTER_CONTROL)) {
      fout << MESSAGE("Feature state - Optical filter") << EOL
           << MESSAGE("  Mode: ") << OPERATING_MODES[camera.getFeatureOperatingMode(Camera1394::OPTICAL_FILTER_CONTROL)] << EOL
           << MESSAGE("  Value: ") << camera.getOpticalFilter() << ENDL;
    }
    
    if (camera.isFeatureReadable(Camera1394::CAPTURE_SIZE)) {
      fout << MESSAGE("Feature state - Capture size") << EOL
           << MESSAGE("  Mode: ") << OPERATING_MODES[camera.getFeatureOperatingMode(Camera1394::CAPTURE_SIZE)] << EOL
           << MESSAGE("  Value: ") << camera.getCaptureSize() << ENDL;
    }
    
    if (camera.isFeatureReadable(Camera1394::CAPTURE_QUALITY)) {
      fout << MESSAGE("Feature state - Capture quality") << EOL
           << MESSAGE("  Mode: ") << OPERATING_MODES[camera.getFeatureOperatingMode(Camera1394::CAPTURE_QUALITY)] << EOL
           << MESSAGE("  Value: ") << camera.getCaptureQuality() << ENDL;
    }
    fout << ENDL;
  }

  void setupCamera(bool fast = false) throw(Camera1394::Camera1394Exception) {
    if (cameraGuid.isInvalid()) { // check if EUI-64 has been specified
      Array<EUI64> cameras = camera.getCameras();
      assert(cameras.getSize() > 0, Camera1394::Camera1394Exception("No cameras available", this));
      assert(cameras.getSize() == 1, Camera1394::Camera1394Exception("More than one camera available", this));
      cameraGuid = cameras[0];
      if (verbosity >= VERBOSITY_SILENT) {
        fout << MESSAGE("Opening camera: ") << cameraGuid << EOL << ENDL;
      }
    }
    // TAG: make sure adapter is open
    camera.open(cameraGuid);
    
    if (setMode) {
      assert(camera.isModeSupported(mode), Camera1394::Camera1394Exception("Mode is not supported"));
      if (!fast && (verbosity >= VERBOSITY_NORMAL)) {
        fout << MESSAGE("Selecting mode: ") << camera.getModeAsString(mode) << ENDL;
      }
      camera.setMode(mode);
    }
    if (setFrameRate) {
      assert(camera.isFrameRateSupported(rate), Camera1394::Camera1394Exception("Frame rate is not supported"));
      if (!fast && (verbosity >= VERBOSITY_NORMAL)) {
        fout << MESSAGE("Selecting frame rate: ") << Camera1394::getFrameRateAsValue(rate)/65536.0 << MESSAGE(" frames/second") << ENDL;
      }
      camera.setFrameRate(rate);
    }
    if (setPixelFormat) {
      assert(camera.getPixelFormats() & (1 << pixelFormat), Camera1394::Camera1394Exception("Pixel format is not supported"));
      if (!fast && (verbosity >= VERBOSITY_NORMAL)) {
        fout << MESSAGE("Selecting pixel format: ") << PIXEL_FORMAT_DESCRIPTION[camera.getPixelFormat()] << ENDL;
      }
      camera.setPixelFormat(pixelFormat);
    }

    if (!fast) {
      if (setBrightness) {
        assert(
          camera.isFeatureSupported(Camera1394::BRIGHTNESS_CONTROL),
          Camera1394::Camera1394Exception("Brightness is not supported")
        );
        camera.setBrightness(brightnessValue);
      }
      if (setAutoExposure) {
        assert(
          camera.isFeatureSupported(Camera1394::AUTO_EXPOSURE_CONTROL),
          Camera1394::Camera1394Exception("Auto exposure is not supported")
        );
        camera.setAutoExposure(autoExposureValue);
      }
      if (setWhiteBalance) {
        assert(
          camera.isFeatureSupported(Camera1394::WHITE_BALANCE_CONTROL),
          Camera1394::Camera1394Exception("White balance is not supported")
        );
        camera.setWhiteBalance(blueRatioValue, redRatioValue);
      }
      if (setHue) {
        assert(
          camera.isFeatureSupported(Camera1394::HUE_CONTROL),
          Camera1394::Camera1394Exception("Hue is not supported")
        );
        camera.setHue(hueValue);
      }
      if (setSaturation) {
        assert(
          camera.isFeatureSupported(Camera1394::SATURATION_CONTROL),
          Camera1394::Camera1394Exception("Saturation is not supported")
        );        
        camera.setSaturation(saturationValue);
      }
      if (setGamma) {
        assert(
          camera.isFeatureSupported(Camera1394::GAMMA_CONTROL),
          Camera1394::Camera1394Exception("Gamma is not supported")
        );
        camera.setGamma(gammaValue);
      }
      if (setShutter) {
        assert(
          camera.isFeatureSupported(Camera1394::SHUTTER_CONTROL),
          Camera1394::Camera1394Exception("Shutter is not supported")
        );
        camera.setShutter(shutterValue);
      }
      if (setGain) {
        assert(
          camera.isFeatureSupported(Camera1394::GAIN_CONTROL),
          Camera1394::Camera1394Exception("Gain is not supported")
        );
        camera.setGain(gainValue);
      }
    }
  }
  
  void dumpCamera() throw(Camera1394::Camera1394Exception) {
    setupCamera(true);
    
    unsigned int specification = camera.getSpecification();
    
    fout << MESSAGE("Vendor: ") << camera.getVendorName() << EOL
         << MESSAGE("Model: ") << camera.getModelName() << EOL
         << MESSAGE("Specification: ") << ((specification >> 16) & 0xff) << '.' << ((specification >> 8) & 0xff) << EOL
         << ENDL;
    
    fout << MESSAGE("Register base address: ") << HEX << camera.getCommandRegisters() << ENDL;
      
    unsigned int capabilities = camera.getCapabilities();
    fout << MESSAGE("Capabilities:");
    if (capabilities == 0) {
      fout << ' ' << MESSAGE("NONE");
    } else {
      if (capabilities & Camera1394::POWER_CONTROL) {
        fout << ' ' << MESSAGE("POWER");
      }
      if (capabilities & Camera1394::SINGLE_ACQUISITION) {
        fout << ' ' << MESSAGE("SINGLE-SHOT");
      }
      if (capabilities & Camera1394::MULTI_ACQUISITION) {
        fout << ' ' << MESSAGE("MULTI-SHOT");
      }
      if (capabilities & Camera1394::ADVANCED_FEATURES) {
        fout << ' ' << MESSAGE("ADVANCED");
      }
      if (capabilities & Camera1394::MODE_ERROR_STATUS) {
        fout << ' ' << MESSAGE("MODE-STATUS");
      }
      if (capabilities & Camera1394::FEATURE_ERROR_STATUS) {
        fout << ' ' << MESSAGE("FEATURE-STATUS");
      }
    }
    fout << ENDL;
    
    fout << MESSAGE("Power: ") << camera.isUpAndRunning() << ENDL;
    
    if (capabilities & Camera1394::ADVANCED_FEATURES) {
      fout << MESSAGE("Advanced feature address: ") << HEX << setWidth(18) << ZEROPAD << camera.getAdvancedFeatureAddress() << ENDL;
    }

    fout << ENDL;
    
    camera.reset();
    Thread::millisleep(500);
    
    fout << MESSAGE("Supported formats:") << EOL
         << MESSAGE("  Uncompressed VGA: ") << camera.isFormatSupported(Camera1394::UNCOMPRESSED_VGA) << EOL
         << MESSAGE("  Uncompressed Super VGA I: ") << camera.isFormatSupported(Camera1394::UNCOMPRESSED_SUPER_VGA_I) << EOL
         << MESSAGE("  Uncompressed Super VGA II: ") << camera.isFormatSupported(Camera1394::UNCOMPRESSED_SUPER_VGA_II) << EOL
         << MESSAGE("  Still image: ") << camera.isFormatSupported(Camera1394::STILL_IMAGE) << EOL
         << MESSAGE("  Partial image: ") << camera.isFormatSupported(Camera1394::PARTIAL_IMAGE) << EOL
         << ENDL;
    
    fout << MESSAGE("Supported modes:") << EOL;
    for (unsigned int i = 0; i < getArraySize(Camera1394::MODES); ++i) {
      Camera1394::Mode mode = Camera1394::MODES[i];
      if (camera.isModeSupported(mode)) {
        fout << MESSAGE("Mode ") << i << ' ' << camera.getModeAsString(mode) << EOL
             << MESSAGE("  maximum dimension: ") << camera.getMaximumDimension(mode) << EOL
             << MESSAGE("  unit dimension: ") << camera.getUnitDimension(mode) << EOL
             << MESSAGE("  unit offset: ") << camera.getUnitOffset(mode) << EOL
             << MESSAGE("  frame rate(s): ");
        
        unsigned int frameRates = camera.getFrameRates(mode);
        if (frameRates != 0) {
          fout << EOL;
          for (int rate = Camera1394::RATE_1_875; rate <= Camera1394::RATE_60; ++rate) {
            if ((frameRates >> rate) & 1) {
              fout << MESSAGE("    ")
                   << Camera1394::getFrameRateAsValue(static_cast<Camera1394::FrameRate>(rate))/65536.0
                   << MESSAGE(" frames/second") << EOL;
            }
          }
        } else {
          fout << MESSAGE("unspecified") << EOL;
        }
        
        fout << MESSAGE("  pixel format(s): ");
        
        static const Camera1394::PixelFormat PIXEL_FORMAT[] = {
          Camera1394::Y_8BIT,
          Camera1394::Y_16BIT,
          Camera1394::YUV_411_8BIT,
          Camera1394::YUV_422_8BIT,
          Camera1394::YUV_444_8BIT,
          Camera1394::RGB_8BIT,
          Camera1394::RGB_16BIT
        };
        
        unsigned int pixelFormats = camera.getPixelFormats(mode);
        if (Math::isPowerOf2(pixelFormats)) {
          for (unsigned int i = 0; i < getArraySize(PIXEL_FORMAT); ++i) {
            if (pixelFormats & (1 << PIXEL_FORMAT[i])) {
              fout << PIXEL_FORMAT_DESCRIPTION[i] << EOL;
            }
          }
        } else {
          fout << EOL;
          for (unsigned int i = 0; i < getArraySize(PIXEL_FORMAT); ++i) {
            if (pixelFormats & (1 << PIXEL_FORMAT[i])) {
              fout << MESSAGE("    ") << i << '.' << ' ' << PIXEL_FORMAT_DESCRIPTION[i] << EOL;
            }
          }
        }
        fout << FLUSH;
      } else {
        fout << MESSAGE("Mode ") << i << ' ' << camera.getModeAsString(mode) << MESSAGE(" (NOT SUPPORTED)") << ENDL;
      }
    }
    fout << ENDL;

    fout << MESSAGE("Supported features:") << EOL
         << MESSAGE("  Brightness: ") << camera.isFeatureSupported(Camera1394::BRIGHTNESS_CONTROL) << EOL
         << MESSAGE("  Auto exposure: ") << camera.isFeatureSupported(Camera1394::AUTO_EXPOSURE_CONTROL) << EOL
         << MESSAGE("  White balance: ") << camera.isFeatureSupported(Camera1394::WHITE_BALANCE_CONTROL) << EOL
         << MESSAGE("  Hue: ") << camera.isFeatureSupported(Camera1394::HUE_CONTROL) << EOL
         << MESSAGE("  Saturation: ") << camera.isFeatureSupported(Camera1394::SATURATION_CONTROL) << EOL
         << MESSAGE("  Gamma: ") << camera.isFeatureSupported(Camera1394::GAMMA_CONTROL) << EOL
         << MESSAGE("  Shutter: ") << camera.isFeatureSupported(Camera1394::SHUTTER_CONTROL) << EOL
         << MESSAGE("  Gain: ") << camera.isFeatureSupported(Camera1394::GAIN_CONTROL) << EOL
         << MESSAGE("  IRIS: ") << camera.isFeatureSupported(Camera1394::IRIS_CONTROL) << EOL
         << MESSAGE("  Focus: ") << camera.isFeatureSupported(Camera1394::FOCUS_CONTROL) << EOL
         << MESSAGE("  Temperature: ") << camera.isFeatureSupported(Camera1394::TEMPERATURE_CONTROL) << EOL
         << MESSAGE("  Trigger: ") << camera.isFeatureSupported(Camera1394::TRIGGER_CONTROL) << EOL
         << MESSAGE("  Zoom: ") << camera.isFeatureSupported(Camera1394::ZOOM_CONTROL) << EOL
         << MESSAGE("  Pan: ") << camera.isFeatureSupported(Camera1394::PAN_CONTROL) << EOL
         << MESSAGE("  Tilt: ") << camera.isFeatureSupported(Camera1394::TILT_CONTROL) << EOL
         << MESSAGE("  Optical filter: ") << camera.isFeatureSupported(Camera1394::OPTICAL_FILTER_CONTROL) << EOL
         << MESSAGE("  Capture size: ") << camera.isFeatureSupported(Camera1394::CAPTURE_SIZE) << EOL
         << MESSAGE("  Capture quality: ") << camera.isFeatureSupported(Camera1394::CAPTURE_QUALITY) << EOL
         << ENDL;
  }

  bool onAcquisition(unsigned int frame, uint8* buffer) throw() {
    // mark frame for transmission
    ++numberOfFrames;
    fout << MESSAGE("Frame acquired: index=") << frame
         << MESSAGE(" frame=") << numberOfFrames
         << MESSAGE(" time=") << timer.getLiveMicroseconds() << ENDL;
    return numberOfFrames < desiredNumberOfFrames;
  }

  bool onAcquisitionLostSync(unsigned int frame) throw() {
    Trace::message("Lost synchronization with beginning of frame");
    return true;
  }
  
  void acquire() throw(Camera1394::Camera1394Exception) {
    setupCamera();
    
    bool success = true;
    BMPEncoder encoder;
    desiredNumberOfFrames = 128;
    numberOfFrames = 0;

    switch (camera.getPixelFormat()) {
    case Camera1394::Y_8BIT:
    case Camera1394::Y_16BIT:
      {
        Allocator<uint8> buffer(camera.getRegion().getDimension().getSize() *
                                Camera1394::getBitsPerPixel(camera.getPixelFormat())/8);
        GrayImage frame(camera.getRegion().getDimension());
        timer.start();
        while ((success) && (numberOfFrames < desiredNumberOfFrames)) {
          success = camera.acquire(buffer.getElements(), buffer.getSize());
          ++numberOfFrames;
        }
        timer.stop();
        if (success) {
          Camera1394::convert(frame, camera.getPixelFormat(), buffer.getElements());
          encoder.writeGray(filename, &frame);
        }
      }
      break;
    case Camera1394::RGB_8BIT:
    case Camera1394::RGB_16BIT:
    case Camera1394::YUV_411_8BIT:
    case Camera1394::YUV_444_8BIT:
      {
        Allocator<uint8> buffer(camera.getRegion().getDimension().getSize() *
                                Camera1394::getBitsPerPixel(camera.getPixelFormat())/8);
        ColorImage frame(camera.getRegion().getDimension());
        timer.start();
        while ((success) && (numberOfFrames < desiredNumberOfFrames)) {
          success = camera.acquire(buffer.getElements(), buffer.getSize());
          ++numberOfFrames;
        }
        timer.stop();
        if (success) {
          Camera1394::convert(frame, camera.getPixelFormat(), buffer.getElements());
          encoder.write(filename, &frame);
        }
      }
      break;
    case Camera1394::YUV_422_8BIT:
      {
        // fill cyclic buffer
        const unsigned int NUMBER_OF_FRAMES = 16;
        Array<Allocator<uint8> > frames(NUMBER_OF_FRAMES, Allocator<uint8>(), NUMBER_OF_FRAMES);
        Array<Camera1394::FrameBuffer> buffers(NUMBER_OF_FRAMES, Camera1394::FrameBuffer(), NUMBER_OF_FRAMES);
        
        {
          Array<Allocator<uint8> >::Iterator frame = frames.getBeginIterator();
          Array<Camera1394::FrameBuffer>::Iterator buffer = buffers.getBeginIterator();
          for (unsigned int i = NUMBER_OF_FRAMES; i > 0; --i) {
            frame->setSize(camera.getRegion().getDimension().getSize() * 2); // 16 effective bits per pixel
            *buffer++ = Camera1394::FrameBuffer(frame->getElements(), frame->getSize());
            ++frame;
          }
        }
        
        timer.start();
        success = camera.acquireContinuously(buffers, this);
        timer.stop();
        
//         Allocator<uint8> frame(camera.getRegion().getDimension().getSize() * 2); // 16 effective bits per pixel
//         while ((success) && (numberOfFrames < desiredNumberOfFrames)) {
//           success = camera.acquire(Cast::pointer<char*>(frame.getElements()), frame.getSize());
//           ++numberOfFrames;
//         }
        
        if (success) {
          ColorImage image(camera.getRegion().getDimension());
          
          Array<Allocator<uint8> >::Iterator frame = frames.getBeginIterator();
          Array<Allocator<uint8> >::Iterator end = frames.getEndIterator();
          unsigned int number = 0;
          
          while (frame < end) {
            Camera1394::convert(image, Camera1394::YUV_422_8BIT, frame->getElements());
            StringOutputStream stream;
            stream << MESSAGE("frame") << number++ << '.' << encoder.getDefaultExtension() << FLUSH;
            encoder.write(stream.getString(), &image);
            ++frame;
          }
        }
        
      }
      break;
    default:
      fout << ENDL;
      ferr << MESSAGE("Error: Pixel format is not supported") << ENDL;
      setExitCode(EXIT_CODE_ERROR);
      success = false;
    }

    if ((success) && (verbosity >= VERBOSITY_NORMAL)) {
      fout << MESSAGE("Number of acquired frames: ") << numberOfFrames << EOL
           << MESSAGE("Total elapsed acquisition time: ") << timer.getMicroseconds() << MESSAGE(" microsecond(s)") << EOL
           << MESSAGE("Average frame rate: ")
           << FIXED << setPrecision(2) << (numberOfFrames * 1000000.0)/timer.getMicroseconds() << MESSAGE(" frames/second") << ENDL;
    }
    
  }
  
  void connect() throw(Camera1394::Camera1394Exception) {
    setupCamera();
    
    InetEndPoint endPoint;
    if (isServer) {
      endPoint.setAddress(InetAddress());
      endPoint.setPort(port);
    } else {
      List<InetAddress>::ReadEnumerator enu = InetAddress::getAddressesByName(host).getReadEnumerator();
      endPoint.setAddress(*enu.next());
      endPoint.setPort(port);
    }
    VideoPhoneServlet(camera, loopback, isServer, endPoint).run();
  }
  
  void onTermination() throw() {
    // override default application termination
  }

  void main() throw() {
    try {
      processArguments();
      
      switch (command) {
      case COMMAND_ERROR:
        dumpHeader();
        ferr << MESSAGE("Error: Invalid argument(s)") << EOL
             << MESSAGE("For help: ") << getFormalName() << MESSAGE(" --help") << ENDL;
        setExitCode(EXIT_CODE_ERROR);
        break;
      case COMMAND_DUMP_IDENTIFIER:
        dumpIdentifier();
        break;
      case COMMAND_DUMP_VERSION:
        dumpVersion();
        break;
      case COMMAND_HELP:
        dumpHeader();
        dumpHelp();
        break;
      case COMMAND_LIST_ADAPTERS:
        listAdapters();
        break;
      case COMMAND_LIST_NODES:
        listNodes();
        break;
      case COMMAND_LIST_CAMERAS:
        listCameras();
        break;
      case COMMAND_DUMP_CAPABILITIES:
        dumpCamera();
        break;
      case COMMAND_DUMP_MODE_CAPABILITIES:
        dumpCameraMode();
        break;
      case COMMAND_ACQUIRE:
        acquire();
        break;
      case COMMAND_LOOPBACK:
        loopback = true;
        connect();
        break;
      case COMMAND_CONNECT:
        isServer = false;
        connect();
        break;
      case COMMAND_NO_ARGUMENTS:
        isServer = true;
        connect();
        //dumpHeader();
        //ferr << MESSAGE("For help: ") << getFormalName() << MESSAGE(" --help") << ENDL;
        break;
      }
    } catch (Exception& e) {
      fout << ENDL;
      exceptionHandler(e);
    }
  }
  
};

const StringLiteral VideoPhoneApplication::PIXEL_FORMAT_DESCRIPTION[] = {
  MESSAGE("Mono 8bit/pixel"),
  MESSAGE("Mono 16bit/pixel"),
  MESSAGE("YUV 4:1:1 12bit/pixel"),
  MESSAGE("YUV 4:2:2 16bit/pixel"),
  MESSAGE("YUV 4:4:4 24bit/pixel"),
  MESSAGE("RGB 24bit/pixel"),
  MESSAGE("RGB 48bit/pixel")
};

STUB(VideoPhoneApplication);
