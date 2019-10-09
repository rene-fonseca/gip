/***************************************************************************
    Generic Image Processing (GIP) Framework (Test Suite)
    A framework for developing image processing applications

    See COPYRIGHT.txt for details.

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#include <gip/Pixel.h>
#include <gip/video/Camera1394.h>
#include <gip/transformation/Flip.h>
#include <gip/YCbCrPixel.h>
#include <base/Application.h>
#include <base/string/FormatOutputStream.h>
#include <base/string/InvalidFormat.h>
#include <base/concurrency/Thread.h>
#include <base/Timer.h>
#include <gip/ArrayImage.h>
#include <gip/io/BMPEncoder.h>
#include <gip/transformation/Convert.h>
#include <base/Cast.h>

using namespace com::azure::dev::gip;

class RGB24ToRGB : public UnaryOperation<Camera1394::RGB24Pixel, ColorPixel> {
public:
  
  inline RGB24ToRGB() throw() {
  }
  
  inline Result operator()(const Argument& value) const throw() {
    return makeColorPixel(value.red, value.blue, value.green);
  }
};

class Camera1394Application : public Application {
private:

  static const unsigned int MAJOR_VERSION = 1;
  static const unsigned int MINOR_VERSION = 0;
public:

  enum Command {
    COMMAND_LIST_ADAPTERS,
    COMMAND_LIST_CAMERAS,
    COMMAND_DUMP,
    COMMAND_RESET,
    COMMAND_REGISTER_SPACE,
    COMMAND_HELP,
    COMMAND_ERROR
  };
  
  Camera1394Application(int numberOfArguments, const char* arguments[], const char* environment[]) throw()
    : Application(MESSAGE("Camera1394"), numberOfArguments, arguments, environment) {
  }

  void listAdapters() throw() {
    try {
      IEEE1394 ieee1394;
      fout << MESSAGE("Adapters: ") << ieee1394.getAdapters() << ENDL;
    } catch (Exception& e) {
      fout << ENDL;
      ferr << MESSAGE("Unable to list adapters") << ENDL;
      Application::getApplication()->setExitCode(EXIT_CODE_ERROR);
      return;
    }
  }

  void openAdapter(IEEE1394& ieee1394, const EUI64& id) throw() {
    fout << MESSAGE("Opening IEEE 1394 adapter (") << id << ')' << ENDL;
    ieee1394.open(id);
  }
  
  void dumpNodes(IEEE1394& ieee1394) throw() {
    fout << MESSAGE("Available nodes: ") << EOL;
    
    for (unsigned int node = 0; node < ieee1394.getNumberOfNodes(); ++node) {
      try {
        fout << MESSAGE("Node: ") << ieee1394.getIdentifier(node) << EOL
             << MESSAGE("  Physical id: ") << node << ENDL;
        
        unsigned int vendor = ieee1394.getVendorId(node);
        fout << MESSAGE("  Vendor id: ")
             << HEX << setWidth(2) << ZEROPAD << NOPREFIX << ((vendor >> 16) & 0xff) << ':'
             << HEX << setWidth(2) << ZEROPAD << NOPREFIX << ((vendor >> 8) & 0xff) << ':'
             << HEX << setWidth(2) << ZEROPAD << NOPREFIX << (vendor & 0xff) << ENDL;
        
        unsigned int capabilities = ieee1394.getCapabilities(node);
        if (capabilities) {
          fout << MESSAGE("  Capabilities:") << EOL;
          if (capabilities & IEEE1394::ISOCHRONOUS_RESOURCE_MANAGER_CAPABLE) {
            fout << ' ' << MESSAGE("    Isochronous resource manager") << EOL;
          }
          if (capabilities & IEEE1394::CYCLE_MASTER_CAPABLE) {
            fout << ' ' << MESSAGE("    Cycle master") << EOL;
          }
          if (capabilities & IEEE1394::ISOCHRONOUS_TRANSACTION_CAPABLE) {
            fout << ' ' << MESSAGE("    Isochronous transmission") << EOL;
          }
          if (capabilities & IEEE1394::BUS_MASTER_CAPABLE) {
            fout << ' ' << MESSAGE("    Bus master") << EOL;
          }
          if (capabilities & IEEE1394::POWER_MANAGER_CAPABLE) {
            fout << ' ' << MESSAGE("    Power manager") << EOL;
          }
        }
        
        fout << MESSAGE("  Maximum asynchronous payload: ") << ieee1394.getMaximumPayload(node) << ENDL;
      } catch (IEEE1394Exception& e) {
        fout << MESSAGE("Exception: ") << e << ENDL;
        // check cause
      }
    }
    fout << EOL << ENDL;
  }
  
  void listCameras() throw() {
    try {
      IEEE1394 ieee1394;

      Array<EUI64> adapters = ieee1394.getAdapters();

      if (adapters.getSize() == 0) {
        ferr << MESSAGE("No adapters available") << ENDL;
      }
      
      openAdapter(ieee1394, adapters[0]);
      dumpNodes(ieee1394);
      
      fout << MESSAGE("Closing IEEE 1394 adapter") << ENDL;
      ieee1394.close();
    } catch (Exception& e) {
      fout << ENDL;
      ferr << MESSAGE("Unable to list adapters: ") << e << ENDL;
      Application::getApplication()->setExitCode(EXIT_CODE_ERROR);
      return;
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
  
  void dump() throw() {
    static const Literal PIXEL_FORMAT_DESCRIPTION[] = {
      MESSAGE("Mono 8bit/pixel"),
      MESSAGE("Mono 16bit/pixel"),
      MESSAGE("YUV 4:1:1 12bit/pixel"),
      MESSAGE("YUV 4:2:2 16bit/pixel"),
      MESSAGE("YUV 4:4:4 24bit/pixel"),
      MESSAGE("RGB 24bit/pixel"),
      MESSAGE("RGB 48bit/pixel")
    };
    
    try {
      IEEE1394 ieee1394;

      Array<EUI64> adapters = ieee1394.getAdapters();

      if (adapters.getSize() == 0) {
        ferr << MESSAGE("No adapters available") << ENDL;
        return;
      }
      
      // TAG: get bus id's, get nodes for specific bus, is bus present methods?
      openAdapter(ieee1394, adapters[0]);
      dumpNodes(ieee1394);
      
      Camera1394 camera;
      EUI64 cameraGuid;
      
      {
        Array<EUI64> cameras = camera.getCameras();
        if (cameras.getSize() == 0) {
          ferr << MESSAGE("No cameras available") << ENDL;
          return;
        }

        unsigned int i = 0;
        Array<EUI64>::ReadEnumerator enu = cameras.getReadEnumerator();
        while (enu.hasNext()) {
          const EUI64* guid = enu.next();
          fout << MESSAGE("Camera ") << i << MESSAGE(": ") << *guid << ENDL;
        }
        
        cameraGuid = cameras[0];
      }

      fout << MESSAGE("Status: ") << ieee1394.getStatus() << ENDL;

      fout << MESSAGE("Opening camera: ") << cameraGuid << ENDL;
      camera.open(cameraGuid);
      unsigned int specification = camera.getSpecification();
      
      fout << MESSAGE("Vendor: ") << camera.getVendorName() << EOL
           << MESSAGE("Model: ") << camera.getModelName() << EOL
           << MESSAGE("Specification: ") << ((specification >> 16) & 0xff) << '.' << ((specification >> 8) & 0xff) << ENDL;
      
      fout << MESSAGE("Register base address: ") << HEX << camera.getCommandRegisters() << ENDL; // TAG: only done
      
      unsigned int capabilities = camera.getCapabilities();
      fout << MESSAGE("Capabilities:");
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
      fout << EOL << ENDL;

      fout << MESSAGE("Power: ") << camera.isUpAndRunning() << ENDL;
      
      if (capabilities & Camera1394::ADVANCED_FEATURES) {
        fout << MESSAGE("Advanced feature address: ") << HEX << setWidth(18) << ZEROPAD << camera.getAdvancedFeatureAddress() << ENDL;
      }

      camera.reset();
      Thread::millisleep(100);
      
      fout << MESSAGE("Supported formats:") << EOL
           << MESSAGE("  Uncompressed VGA: ") << camera.isFormatSupported(Camera1394::UNCOMPRESSED_VGA) << EOL
           << MESSAGE("  Uncompressed Super VGA I: ") << camera.isFormatSupported(Camera1394::UNCOMPRESSED_SUPER_VGA_I) << EOL
           << MESSAGE("  Uncompressed Super VGA II: ") << camera.isFormatSupported(Camera1394::UNCOMPRESSED_SUPER_VGA_II) << EOL
           << MESSAGE("  Still image: ") << camera.isFormatSupported(Camera1394::STILL_IMAGE) << EOL
           << MESSAGE("  Partial image: ") << camera.isFormatSupported(Camera1394::PARTIAL_IMAGE) << EOL
           << EOL << ENDL;

      fout << MESSAGE("Modes: ") << EOL;
      for (unsigned int i = 0; i < getArraySize(Camera1394::MODES); ++i) {
        Camera1394::Mode mode = Camera1394::MODES[i];
        if (camera.isModeSupported(mode)) {
          fout << MESSAGE("Mode: ") << camera.getModeAsString(mode) << EOL
               << MESSAGE("  maximum dimension: ") << camera.getMaximumDimension(mode) << EOL
               << MESSAGE("  unit dimension: ") << camera.getUnitDimension(mode) << EOL
               << MESSAGE("  unit offset: ") << camera.getUnitOffset(mode) << EOL
               << MESSAGE("  frame rate(s): ");

          unsigned int frameRates = camera.getFrameRates(mode);
          if (frameRates != 0) {
            fout << EOL;
            for (int rate = Camera1394::RATE_1_875; rate <= Camera1394::RATE_60; ++rate) {
              if ((frameRates >> rate) & 1) {
                fout << MESSAGE("    ") << Camera1394::getFrameRateAsValue(static_cast<Camera1394::FrameRate>(rate))/65536.0 << MESSAGE(" frames/second") << EOL;
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
          fout << MESSAGE("Mode: ") << camera.getModeAsString(mode) << MESSAGE(" (NOT SUPPORTED)") << ENDL;
        }
      }
      fout << EOL << ENDL;

//      camera.setMode(Camera1394::PARTIAL_IMAGE_MODE_1);
//      camera.setMode(Camera1394::PARTIAL_IMAGE_MODE_0);
//      camera.setMode(Camera1394::Y_1024X768_8BIT);
//      camera.setMode(Camera1394::Y_640X480_8BIT);
//      camera.setMode(Camera1394::RGB_640X480_24BIT);

//       Region region = camera.getRegion();
//       region.setDimension(Dimension(762, 500));
//       camera.setRegion(region);
      
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
           << EOL << ENDL;

      fout << MESSAGE("Isochronous channel: ") << camera.getIsochronousChannel() << EOL
           << MESSAGE("Isochronous speed: ") << camera.getIsochronousSpeed() << EOL
           << ENDL;

      
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
           << EOL << ENDL;

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

      static const Literal OPERATING_MODES[] = {
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

      // TAG: need getTrigger...()
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

      // if Sony 700...
      static const uint8 BASLER_ID[8] = {0x00, 0x30, 0x53, 0x19, 0x6d, 0x6c, 0x98, 0x8e};
      
      if (cameraGuid == EUI64(BASLER_ID)) {
        if (camera.getFeatureDescriptor(Camera1394::SHUTTER_CONTROL).manualMode) {
          camera.setFeatureOperatingMode(Camera1394::SHUTTER_CONTROL, Camera1394::MANUAL);
          camera.setShutter(1000);
          fout << MESSAGE("Feature state - Shutter") << EOL
               << MESSAGE("  Value: ") << camera.getShutter() << ENDL;
        }
        if (camera.getFeatureDescriptor(Camera1394::GAIN_CONTROL).manualMode) {
          camera.setFeatureOperatingMode(Camera1394::GAIN_CONTROL, Camera1394::MANUAL);
          camera.setGain(175);
          fout << MESSAGE("Feature state - Gain") << EOL
               << MESSAGE("  Value: ") << camera.getGain() << ENDL;
        }
        if (camera.getFeatureDescriptor(Camera1394::BRIGHTNESS_CONTROL).manualMode) {
          camera.setFeatureOperatingMode(Camera1394::BRIGHTNESS_CONTROL, Camera1394::MANUAL);
          camera.setBrightness(80);
          fout << MESSAGE("Feature state - Brightness") << EOL
               << MESSAGE("  Value: ") << camera.getBrightness() << ENDL;
        }
        if (camera.getFeatureDescriptor(Camera1394::WHITE_BALANCE_CONTROL).manualMode) {
          camera.setFeatureOperatingMode(Camera1394::WHITE_BALANCE_CONTROL, Camera1394::MANUAL);
          camera.setWhiteBalance(100, 100);
          fout << MESSAGE("Feature state - White balance") << EOL
               << MESSAGE("  Cb/blue: ") << camera.getWhiteBalanceBlueRatio() << EOL
               << MESSAGE("  Cr/red: ") << camera.getWhiteBalanceRedRatio() << ENDL;
        }
      }
      
      String filename(MESSAGE("frame0.bmp"));

      switch (camera.getPixelFormat()) {
      case Camera1394::Y_8BIT:
        {
          ArrayImage<uint8> frame(camera.getRegion().getDimension());
          GrayImage finalImage(frame.getDimension());
          
          fout << MESSAGE("Acquiring frame") << ENDL;
          if (!camera.acquire(frame)) {
            ferr << MESSAGE("Unable to acquire frame") << ENDL;
            setExitCode(EXIT_CODE_ERROR);
            return;
          }

          Flip<ArrayImage<uint8> > flip(&frame);
          flip();
          
          Convert<GrayImage, ArrayImage<uint8>, Same<uint8> > convert(&finalImage, &frame, Same<uint8>());
          convert();
          
          BMPEncoder encoder;
          encoder.writeGray(filename, &finalImage);
        }
        break;
      case Camera1394::RGB_8BIT:
        {
          ArrayImage<Camera1394::RGB24Pixel> frame(camera.getRegion().getDimension());
          ColorImage finalImage(frame.getDimension());
        
          Timer timer;
          for (int i = 0; i < 128; ++i) {
            fout << MESSAGE("Acquiring frame: ") << timer.getLiveMicroseconds() << ENDL;
            camera.acquire(frame);
          }
          
          Flip<ArrayImage<Camera1394::RGB24Pixel> > flip(&frame);
          flip();
          
          Convert<ColorImage, ArrayImage<Camera1394::RGB24Pixel>, RGB24ToRGB> convert(&finalImage, &frame, RGB24ToRGB());
          convert();
          
          BMPEncoder encoder;
          encoder.write(filename, &finalImage);
        }
        break;
      case Camera1394::Y_16BIT:
        {
          ArrayImage<uint16> frame(camera.getRegion().getDimension());
          ColorImage finalImage(frame.getDimension());
          
          fout << MESSAGE("Acquiring frame") << ENDL;
          if (!camera.acquire(frame)) {
            ferr << MESSAGE("Unable to acquire frame") << ENDL;
            setExitCode(EXIT_CODE_ERROR);
            return;
          }

          Flip<ArrayImage<uint16> > flip(&frame);
          flip();

          // TAG: fixme - uint16 to ColorPixel
          //Convert<ColorImage, ArrayImage<uint16>, Same<uint16> > convert(&finalImage, &frame, Same<uint16>());
          //convert();
          
          BMPEncoder encoder;
          encoder.write(filename, &finalImage);
        }
        break;
      case Camera1394::YUV_422_8BIT:
        {
          Allocator<uint8> frame(camera.getRegion().getDimension().getSize() * 2); // 16 effective bits per pixel
          ColorImage finalImage(camera.getRegion().getDimension());
          
          camera.acquire(frame.getElements(), frame.getSize());

          const uint8* src = frame.getElements();
          
          ColorImage::Rows rowLookup = finalImage.getRows();
          ColorImage::Rows::RowIterator row = rowLookup.getFirst();
          const ColorImage::Rows::RowIterator endRow = rowLookup.getEnd();
          while (row < endRow) {
            ColorImage::Rows::RowIterator::ElementIterator column = row.getFirst();
            const ColorImage::Rows::RowIterator::ElementIterator endColumn = row.getEnd();
            while (column < endColumn) {
              uint8 Cb = *src++;
              uint8 Y0 = *src++;
              uint8 Cr = *src++;
              uint8 Y1 = *src++;
              *column++ = YCbCrToRGB(makeYCbCrPixel(Y0, Cb, Cr));
              *column++ = YCbCrToRGB(makeYCbCrPixel(Y1, Cb, Cr));
            }
            ++row;
          }
          
          Flip<ColorImage> flip(&finalImage); // TAG: should be handled by YUV 422 to RGB conversion
          flip();
          
          BMPEncoder encoder;
          encoder.write(filename, &finalImage);
        }
        break;
      case Camera1394::YUV_411_8BIT:
      case Camera1394::YUV_444_8BIT:
      case Camera1394::RGB_16BIT:
      default:
        ferr << MESSAGE("Pixel format is not supported") << ENDL;
      }
      
      fout << MESSAGE("Closing IEEE 1394 adapter") << ENDL;
      ieee1394.close();
    } catch (Exception& e) {
      fout << ENDL;
      ferr << MESSAGE("Unable to get device information: ") << e << ENDL;
      Application::getApplication()->setExitCode(EXIT_CODE_ERROR);
      return;
    }
  }

  void dumpRegisterSpace(uint64 firstAddress, uint64 lastAddress /*, EUI64 guid*/) throw() {
    try {
      IEEE1394 ieee1394;

      Array<EUI64> adapters = ieee1394.getAdapters();

      if (adapters.getSize() == 0) {
        ferr << MESSAGE("No adapters available") << ENDL;
        return;
      }
      
      openAdapter(ieee1394, adapters[0]);
      
//       Camera1394 camera;
//       Array<EUI64> cameras = camera.getCameras();
//       if (cameras.getSize() == 0) {
//         ferr << MESSAGE("No cameras available") << ENDL;
//         return;
//       }
      
//       unsigned int i = 0;
//       Array<EUI64>::ReadEnumerator enu = cameras.getReadEnumerator();
//       while (enu.hasNext()) {
//         const EUI64* guid = enu.next();
//         fout << MESSAGE("Camera ") << i << MESSAGE(": ") << *guid << ENDL;
//       }
      
//       fout << MESSAGE("Opening camera: ") << cameras[0] << ENDL;
//       camera.open(cameras[0]);

//      const unsigned int node = ieee1394.getPhysicalId(cameras[0]);
      const unsigned int node = 0;
      
      const uint32 DEFAULT_VALUE = 0xdccd2332;
      const uint64 endAddress = lastAddress + sizeof(uint32);
      
      uint32 buffer[4096];
      while (firstAddress < endAddress) {
        uint32 bytesToRead = endAddress - firstAddress;
        if (bytesToRead > getArraySize(buffer) * sizeof(uint32)) {
          bytesToRead = getArraySize(buffer) * sizeof(uint32);
        }
        
        ieee1394.read(node, firstAddress, buffer, bytesToRead/sizeof(uint32), DEFAULT_VALUE);
        // TAG: could read twice with difference default values
        
        for (unsigned int i = 0; i < bytesToRead/sizeof(uint32); ++i) {
          if (buffer[i] != DEFAULT_VALUE) {
            fout << HEX << setWidth(16) << ZEROPAD << NOPREFIX << (firstAddress + i * sizeof(uint32)) << ' ' << ' '
                 << HEX << setWidth(8) << ZEROPAD << NOPREFIX << buffer[i] << EOL;
          } else {
            fout << HEX << setWidth(16) << ZEROPAD << NOPREFIX << (firstAddress + i * sizeof(uint32)) << ' ' << ' '
                 << MESSAGE("........") << EOL;
          }
        }
        
        firstAddress += bytesToRead;
      }
    } catch (Exception& e) {
      fout << ENDL;
      ferr << MESSAGE("Exception: ") << e << ENDL;
      Application::getApplication()->setExitCode(EXIT_CODE_ERROR);
    }
  }
  
  void main() throw() {
    fout << getFormalName() << MESSAGE(" version ") << MAJOR_VERSION << '.' << MINOR_VERSION << EOL
         << MESSAGE("Generic Image Processing Framework (Test Suite)") << EOL
         << MESSAGE("https://dev.azure.com/renefonseca/gip") << EOL
         << MESSAGE("Copyright (C) 2002-2019 by Rene Moeller Fonseca") << EOL << ENDL;
    
    Command command = COMMAND_ERROR;
    String guid;
    uint64 firstAddress;
    uint64 lastAddress;
    
    const Array<String> arguments = getArguments();

    if (arguments.getSize() == 1) {
      if (arguments[0] == "help") {
        command = COMMAND_HELP;
      } else if (arguments[0] == "adapters") {
        command = COMMAND_LIST_ADAPTERS;
      } else if (arguments[0] == "cameras") {
        command = COMMAND_LIST_CAMERAS;
      } else if (arguments[0] == "dump") {
        command = COMMAND_DUMP;
      }
    } else if (arguments.getSize() == 2) {
      if (arguments[0] == "dump") {
        command = COMMAND_DUMP;
        guid = arguments[1];
      } else if (arguments[0] == "reset") {
        command = COMMAND_RESET;
        guid = arguments[1];
      }
    } else if (arguments.getSize() == 3) {
      if (arguments[0] == "registers") {
        command = COMMAND_REGISTER_SPACE;
      }
      // firstAddress <= arguments[1];
      // lastAddress <= arguments[2];
      firstAddress = IEEE1394::CSR_BASE_ADDRESS; // 0xfffff0000000ULL
      lastAddress = firstAddress + 0x1000;
      if ((firstAddress % 4 != 0) || (lastAddress % 4 != 0) || (firstAddress > lastAddress)) {
        command = COMMAND_ERROR;
      }
    } else {
      command = COMMAND_HELP;
    }

    EUI64 id;
    if (guid.isProper()) {
      try {
        id = EUI64(guid);
      } catch (...) {
        command = COMMAND_ERROR;
      }
    }
    
    switch (command) {
    case COMMAND_LIST_ADAPTERS:
      listAdapters();
      break;
    case COMMAND_LIST_CAMERAS:
      listCameras();
      break;
    case COMMAND_DUMP:
      dump();
      break;
    case COMMAND_RESET:
      break;
    case COMMAND_REGISTER_SPACE:
      dumpRegisterSpace(firstAddress, lastAddress);
      break;
    case COMMAND_HELP:
      fout << MESSAGE("Usage: ") << getFormalName() << MESSAGE(" [help|adapters|registers|dump|reset] [EUI-64]") << ENDL;
      break;
    case COMMAND_ERROR:
      // fout has been flushed
      ferr << MESSAGE("ERROR: Invalid parameter(s)") << EOL
           << MESSAGE("For help: ") << getFormalName() << MESSAGE(" help") << ENDL;
      setExitCode(Application::EXIT_CODE_ERROR);
      break;
    }
  }
};

STUB(Camera1394Application);
