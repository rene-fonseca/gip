/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    See COPYRIGHT.txt for details.

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#include <gip/video/Camera1394.h>
#include <base/UnexpectedFailure.h>
#include <base/mathematics/Math.h>
#include <base/string/StringOutputStream.h>
#include <base/Cast.h>
#include <base/concurrency/Thread.h>
#include <gip/YCbCrPixel.h>

namespace gip {

  namespace Camera1394Impl {

    /** Specifies the maximum number of modes per format. */
    static const unsigned int MAXIMUM_MODES_PER_FORMAT = 32;
    
    struct ModeInformation {
      unsigned int format; // the specification format
      unsigned int mode; // the mode specification
      unsigned int width; // 0 is unspecified
      unsigned int height; // 0 is unspecified
      int pixelFormat; // -1 is unspecified
    };
    
    const ModeInformation MODE_INFORMATION[] = {
      {Camera1394::UNCOMPRESSED_VGA, 0, 160, 120, Camera1394::YUV_444_8BIT},
      {Camera1394::UNCOMPRESSED_VGA, 1, 320, 240, Camera1394::YUV_422_8BIT},
      {Camera1394::UNCOMPRESSED_VGA, 2, 640, 480, Camera1394::YUV_411_8BIT},
      {Camera1394::UNCOMPRESSED_VGA, 3, 640, 480, Camera1394::YUV_422_8BIT},
      {Camera1394::UNCOMPRESSED_VGA, 4, 640, 480, Camera1394::RGB_8BIT},
      {Camera1394::UNCOMPRESSED_VGA, 5, 640, 480, Camera1394::Y_8BIT},
      {Camera1394::UNCOMPRESSED_VGA, 6, 640, 480, Camera1394::Y_16BIT},
      
      {Camera1394::UNCOMPRESSED_SUPER_VGA_I, 0, 800, 600, Camera1394::YUV_422_8BIT},
      {Camera1394::UNCOMPRESSED_SUPER_VGA_I, 1, 800, 600, Camera1394::RGB_8BIT},
      {Camera1394::UNCOMPRESSED_SUPER_VGA_I, 2, 800, 600, Camera1394::Y_8BIT},
      {Camera1394::UNCOMPRESSED_SUPER_VGA_I, 3, 1024, 768, Camera1394::YUV_422_8BIT},
      {Camera1394::UNCOMPRESSED_SUPER_VGA_I, 4, 1024, 768, Camera1394::RGB_8BIT},
      {Camera1394::UNCOMPRESSED_SUPER_VGA_I, 5, 1024, 768, Camera1394::Y_8BIT},
      {Camera1394::UNCOMPRESSED_SUPER_VGA_I, 6, 800, 600, Camera1394::Y_16BIT},
      {Camera1394::UNCOMPRESSED_SUPER_VGA_I, 7, 1024, 768, Camera1394::Y_16BIT},
      
      {Camera1394::UNCOMPRESSED_SUPER_VGA_II, 0, 1280, 960, Camera1394::YUV_422_8BIT},
      {Camera1394::UNCOMPRESSED_SUPER_VGA_II, 1, 1280, 960, Camera1394::RGB_8BIT},
      {Camera1394::UNCOMPRESSED_SUPER_VGA_II, 2, 1280, 960, Camera1394::Y_8BIT},
      {Camera1394::UNCOMPRESSED_SUPER_VGA_II, 3, 1600, 1200, Camera1394::YUV_422_8BIT},
      {Camera1394::UNCOMPRESSED_SUPER_VGA_II, 4, 1600, 1200, Camera1394::RGB_8BIT},
      {Camera1394::UNCOMPRESSED_SUPER_VGA_II, 5, 1600, 1200, Camera1394::Y_8BIT},
      {Camera1394::UNCOMPRESSED_SUPER_VGA_II, 6, 1280, 960, Camera1394::Y_16BIT},
      {Camera1394::UNCOMPRESSED_SUPER_VGA_II, 7, 1600, 1200, Camera1394::Y_16BIT},
      
      {Camera1394::STILL_IMAGE, 0, 0, 0, -1},
      
      {Camera1394::PARTIAL_IMAGE, 0, 0, 0, -1},
      {Camera1394::PARTIAL_IMAGE, 1, 0, 0, -1},
      {Camera1394::PARTIAL_IMAGE, 2, 0, 0, -1},
      {Camera1394::PARTIAL_IMAGE, 3, 0, 0, -1},
      {Camera1394::PARTIAL_IMAGE, 4, 0, 0, -1},
      {Camera1394::PARTIAL_IMAGE, 5, 0, 0, -1},
      {Camera1394::PARTIAL_IMAGE, 6, 0, 0, -1},
      {Camera1394::PARTIAL_IMAGE, 7, 0, 0, -1}
    };
    
    enum Register {
      // camera initialization register
      INITIALIZE = 0x000,

      // inquiry register for video format
      V_FORMAT_INQ = 0x100,

      // inquiry register for video mode
      V_MODE_INQ_0 = 0x180,
      V_MODE_INQ_1 = 0x184,
      V_MODE_INQ_2 = 0x188,
      V_MODE_INQ_6 = 0x198,
      V_MODE_INQ_7 = 0x19c,

      // inquiry register for frame rate
      V_RATE_INQ_0_0 = 0x200,
      V_RATE_INQ_0_1 = 0x204,
      V_RATE_INQ_0_2 = 0x208,
      V_RATE_INQ_0_3 = 0x20c,
      V_RATE_INQ_0_4 = 0x210,
      V_RATE_INQ_0_5 = 0x214,
      V_RATE_INQ_0_6 = 0x218,
      
      V_RATE_INQ_1_0 = 0x220,
      V_RATE_INQ_1_1 = 0x224,
      V_RATE_INQ_1_2 = 0x228,
      V_RATE_INQ_1_3 = 0x22c,
      V_RATE_INQ_1_4 = 0x230,
      V_RATE_INQ_1_5 = 0x234,
      V_RATE_INQ_1_6 = 0x238,
      V_RATE_INQ_1_7 = 0x23c,
      
      V_RATE_INQ_2_0 = 0x240,
      V_RATE_INQ_2_1 = 0x244,
      V_RATE_INQ_2_2 = 0x248,
      V_RATE_INQ_2_3 = 0x24c,
      V_RATE_INQ_2_4 = 0x250,
      V_RATE_INQ_2_5 = 0x254,
      V_RATE_INQ_2_6 = 0x258,
      V_RATE_INQ_2_7 = 0x25c,
      
      V_REV_INQ_6_0 = 0x2c0,
      
      V_CSR_INQ_7_0 = 0x2e0,
      V_CSR_INQ_7_1 = 0x2e4,
      V_CSR_INQ_7_2 = 0x2e8,
      V_CSR_INQ_7_3 = 0x2ec,
      V_CSR_INQ_7_4 = 0x2f0,
      V_CSR_INQ_7_5 = 0x2f4,
      V_CSR_INQ_7_6 = 0x2f8,
      V_CSR_INQ_7_7 = 0x2fc,

      // inquiry registers for feature presence
      BASIC_FUNC_INQ = 0x400,
      FEATURE_HI_INQ = 0x404,
      FEATURE_LO_INQ = 0x408,
      ADVANCED_FEATURE_INQ = 0x480,

      // inquiry registers for features
      BRIGHTNESS_INQ = 0x500,
      AUTO_EXPOSURE_INQ = 0x504,
      SHARPNESS_INQ = 0x508,
      WHITE_BALANCE_INQ = 0x50c,
      HUE_INQ = 0x510,
      SATURATION_INQ = 0x514,
      GAMMA_INQ = 0x518,
      SHUTTER_INQ = 0x51c,
      GAIN_INQ = 0x520,
      IRIS_INQ = 0x524,
      FOCUS_INQ = 0x528,
      TEMPERATURE_INQ = 0x52c,
      TRIGGER_INQ = 0x530,
      ZOOM_INQ = 0x580,
      PAN_INQ = 0x584,
      TILT_INQ = 0x588,
      OPTICAL_FILTER_INQ = 0x58c,
      CAPTURE_SIZE_INQ = 0x5c0,
      CAPTURE_QUALITY_INQ = 0x5c4,

      // status and control registers for camera
      CURRENT_V_RATE = 0x600,
      CURRENT_V_MODE = 0x604,
      CURRENT_V_FORMAT = 0x608,
      ISO_CHANNEL = 0x60c,
      POWER = 0x610,
      ISO_ENABLE = 0x614,
      MEMORY_SAVE = 0x618,
      FINITE_SHOTS = 0x61c,
      MEMORY_SAVE_CHANNEL = 0x620,
      CURRENT_MEMORY_CHANNEL = 0x624,
      ERROR_STATUS = 0x628,

      // feature error status registers
      FEATURE_ERROR_STATUS_HIGH = 0x640,
      FEATURE_ERROR_STATUS_LOW = 0x644,

      // absolute value CSR offset registers
      ABSOLUTE_VALUE_CSR = 0x700,
      
      // status and control registers for features
      FEATURE_BRIGHTNESS = 0x800,
      FEATURE_AUTO_EXPOSURE = 0x804,
      FEATURE_SHARPNESS = 0x808,
      FEATURE_WHITE_BALANCE = 0x80c,
      FEATURE_HUE = 0x810,
      FEATURE_SATURATION = 0x814,
      FEATURE_GAMMA = 0x818,
      FEATURE_SHUTTER = 0x81c,
      FEATURE_GAIN = 0x820,
      FEATURE_IRIS = 0x824,
      FEATURE_FOCUS = 0x828,
      FEATURE_TEMPERATURE = 0x82c,
      FEATURE_TRIGGER = 0x830,
      FEATURE_ZOOM = 0x880,
      FEATURE_PAN = 0x884,
      FEATURE_TILT = 0x888,
      FEATURE_OPTICAL_FILTER = 0x88c,
      FEATURE_CAPTURE_SIZE = 0x8c0,
      FEATURE_CAPTURE_QUALITY = 0x8c4
    };

    enum Format7CSRRegister {
      MAX_IMAGE_SIZE_INQ = 0x000,
      UNIT_SIZE_INQ = 0x004,
      IMAGE_POSITION = 0x008,
      IMAGE_SIZE = 0x00c,
      COLOR_CODING_ID = 0x010,
      COLOR_CODING_INQ = 0x014,
      PIXEL_NUMBER_INQ = 0x034,
      TOTAL_BYTES_HI_INQ = 0x038,
      TOTAL_BYTES_LO_INQ = 0x03c,
      PACKET_PARA_INQ = 0x040,
      BYTE_PER_PACKET = 0x044,
      PACKET_PER_FRAME_INQ = 0x048,
      UNIT_POSITION_INQ = 0x04c, // 1.30 specification or later
      VALUE_SETTING = 0x07c // 1.30 specification or later
    };
    
_COM_AZURE_DEV__BASE__PACKED__BEGIN
    struct BusInfo {
      BigEndian<uint32> name; // "1394"
      BigEndian<uint32> flags;
      uint8 guid[8];
    } _COM_AZURE_DEV__BASE__PACKED busInfo;
    
    struct RootDirectory {
      BigEndian<uint32> crc;
      BigEndian<uint32> vendorId; // 0x03 in MSB
      BigEndian<uint32> flags; // 0x0c in MSB
      BigEndian<uint32> nodeOffset; // 0x8d in MSB
      BigEndian<uint32> deviceDirectoryOffset; // 0xd1 in MSB
    } _COM_AZURE_DEV__BASE__PACKED rootDirectory;
    
    struct DeviceIndependentDirectory {
      BigEndian<uint32> crc;
      BigEndian<uint32> specification; // 0x12 in MSB
      BigEndian<uint32> version; // 0x13 in MSB
      BigEndian<uint32> dependentOffset; // 0xd4 in MSB
    } _COM_AZURE_DEV__BASE__PACKED;
    
    struct DeviceDependentDirectory {
      BigEndian<uint16> size;
      BigEndian<uint16> crc;
      BigEndian<uint32> commandRegisters; // 0x40 in MSB
      BigEndian<uint32> vendorNameLeaf; // 0x81 in MSB
      BigEndian<uint32> modelNameLeaf; // 0x82 in MSB
    } _COM_AZURE_DEV__BASE__PACKED;
    
    struct ConfigurationIntro {
      BigEndian<uint32> crc;
      BusInfo busInfo;
      RootDirectory rootDirectory;
    } _COM_AZURE_DEV__BASE__PACKED;
    
    struct FeatureInquery {
      unsigned int maximumValue : 12;
      unsigned int minimumValue : 12;
      bool manualMode : 1;
      bool autoMode : 1;
      bool switchable : 1;
      bool readable : 1;
      bool onePush : 1;
      bool reserved : 1;
      bool absoluteControl : 1;
      bool presence : 1;
    } _COM_AZURE_DEV__BASE__PACKED;
    
    struct TriggerFeatureInquery {
      unsigned int reserved2 : 12;
      bool mode3 : 1;
      bool mode2 : 1;
      bool mode1 : 1;
      bool mode0 : 1;
      unsigned int reserved1 : 9;
      bool polarity : 1;
      bool switchable : 1;
      bool readable : 1;
      unsigned int reserved0 : 2;
      bool absoluteControl : 1;
      bool presence : 1;
    } _COM_AZURE_DEV__BASE__PACKED;

    struct CommonFeatureControl {
      unsigned int reserved1 : 24;
      bool automaticMode : 1;
      bool enabled : 1;
      bool autoAdjustmentMode : 1;
      unsigned int reserved0 : 3;
      bool absoluteControl : 1;
      bool presence : 1;
    } _COM_AZURE_DEV__BASE__PACKED;
    
    struct FeatureControl {
      unsigned int value : 12;
      unsigned int reserved1 : 12;
      bool automaticMode : 1;
      bool enabled : 1;
      bool autoAdjustmentMode : 1;
      unsigned int reserved0 : 3;
      bool absoluteControl : 1;
      bool presence : 1;
    } _COM_AZURE_DEV__BASE__PACKED;
    
    struct WhiteBalanceFeatureControl {
      unsigned int redRatio : 12;
      unsigned int blueRatio : 12;
      bool automaticMode : 1;
      bool enabled : 1;
      bool autoAdjustmentMode : 1;
      unsigned int reserved0 : 3;
      bool absoluteControl : 1;
      bool presence : 1;
    } _COM_AZURE_DEV__BASE__PACKED;
    
    struct TemperatureFeatureControl {
      unsigned int currentValue : 12;
      unsigned int targetValue : 12;
      bool automaticMode : 1;
      bool enabled : 1;
      bool autoAdjustmentMode : 1;
      unsigned int reserved0 : 3;
      bool absoluteControl : 1;
      bool presence : 1;
    } _COM_AZURE_DEV__BASE__PACKED;
    
    struct TriggerFeatureControl {
      unsigned int parameter : 12;
      unsigned int reserved2 : 4;
      unsigned int mode : 4;
      unsigned int reserved1 : 4;
      bool polarity : 1;
      bool enabled : 1;
      unsigned int reserved0 : 4;
      bool absoluteControl : 1;
      bool presence : 1;
    } _COM_AZURE_DEV__BASE__PACKED;
_COM_AZURE_DEV__BASE__PACKED__END

    static const uint32 FEATURE_CONTROL_REGISTER[] = {
      FEATURE_BRIGHTNESS,
      FEATURE_AUTO_EXPOSURE,
      FEATURE_SHARPNESS,
      FEATURE_WHITE_BALANCE,
      FEATURE_HUE,
      FEATURE_SATURATION,
      FEATURE_GAMMA,
      FEATURE_SHUTTER,
      FEATURE_GAIN,
      FEATURE_IRIS,
      FEATURE_FOCUS,
      FEATURE_TEMPERATURE,
      FEATURE_TRIGGER,
      FEATURE_ZOOM,
      FEATURE_PAN,
      FEATURE_TILT,
      FEATURE_OPTICAL_FILTER,
      FEATURE_CAPTURE_SIZE,
      FEATURE_CAPTURE_QUALITY
    };

    void importGenericFeature(IEEE1394::Quadlet quadlet, Camera1394::GenericFeatureDescriptor& descriptor) throw() {
      Camera1394Impl::FeatureInquery feature = *reinterpret_cast<const Camera1394Impl::FeatureInquery*>(&quadlet); // Cast::impersonate<Camera1394Impl::FeatureInquery, uint32>(quadlet);
      descriptor.available = feature.presence;
      descriptor.autoAdjustmentMode = feature.onePush;
      descriptor.readable = feature.readable;
      descriptor.switchable = feature.switchable;
      descriptor.automaticMode = feature.autoMode;
      descriptor.manualMode = feature.manualMode;
      descriptor.minimum = feature.minimumValue;
      descriptor.maximum = feature.maximumValue;
    }

  }; // end of Camera1394Impl namespace

  const Camera1394::Mode Camera1394::MODES[] = {
    Camera1394::YUV_444_160X120_24BIT,
    Camera1394::YUV_422_320X240_16BIT,
    Camera1394::YUV_411_640X480_12BIT,
    Camera1394::YUV_422_640X480_16BIT,
    Camera1394::RGB_640X480_24BIT,
    Camera1394::Y_640X480_8BIT,
    Camera1394::Y_640X480_16BIT,
    
    Camera1394::YUV_422_800X600_16BIT,
    Camera1394::RGB_800X600_24BIT,
    Camera1394::Y_800X600_8BIT,
    Camera1394::YUV_422_1024X768_16BIT,
    Camera1394::RGB_1024X768_24BIT,
    Camera1394::Y_1024X768_8BIT,
    Camera1394::Y_800X600_16BIT,
    Camera1394::Y_1024X768_16BIT,
    
    Camera1394::YUV_422_1280X960_16BIT,
    Camera1394::RGB_1280X960_24BIT,
    Camera1394::Y_1280X960_8BIT,
    Camera1394::YUV_422_1600X1200_16BIT,
    Camera1394::RGB_1600X1200_24BIT,
    Camera1394::Y_1600X1200_8BIT,
    Camera1394::Y_1280X960_16BIT,
    Camera1394::Y_1600X1200_16BIT,
    
    Camera1394::EXIF,
    
    Camera1394::PARTIAL_IMAGE_MODE_0,
    Camera1394::PARTIAL_IMAGE_MODE_1,
    Camera1394::PARTIAL_IMAGE_MODE_2,
    Camera1394::PARTIAL_IMAGE_MODE_3,
    Camera1394::PARTIAL_IMAGE_MODE_4,
    Camera1394::PARTIAL_IMAGE_MODE_5,
    Camera1394::PARTIAL_IMAGE_MODE_6,
    Camera1394::PARTIAL_IMAGE_MODE_7
  };

  String Camera1394::getFormatAsString(Mode mode) throw() {
    static const Literal FORMATS[] = {
      MESSAGE("Uncompressed VGA"),
      MESSAGE("Uncompressed Super VGA I"),
      MESSAGE("Uncompressed Super VGA II"),
      MESSAGE("Format 3"),
      MESSAGE("Format 4"),
      MESSAGE("Format 5"),
      MESSAGE("Still image"),
      MESSAGE("Partial image")
    };
    return FORMATS[Camera1394Impl::MODE_INFORMATION[mode].format]; // TAG: need support for UNSPECIFIED
  }
  
  String Camera1394::getModeAsString(Mode mode) throw() {
    static const Literal MODES[] = {
      MESSAGE("YUV (4:4:4) 160x120 24bit/pixel"),
      MESSAGE("YUV (4:2:2) 320x240 16bit/pixel"),
      MESSAGE("YUV (4:1:1) 640x480 12bit/pixel"),
      MESSAGE("YUV (4:2:2) 640x480 16bit/pixel"),
      MESSAGE("RGB 640x480 24bit/pixel"),
      MESSAGE("Y (mono) 640x480 8bit/pixel"),
      MESSAGE("Y (mono) 640x480 16bit/pixel"),

      MESSAGE("YUV (4:2:2) 800x600 16bit/pixel"),
      MESSAGE("RGB 800x600 24bit/pixel"),
      MESSAGE("Y (mono) 800x600 8bit/pixel"),
      MESSAGE("YUV (4:2:2) 1024x768 16bit/pixel"),
      MESSAGE("RGB 1024x768 24bit/pixel"),
      MESSAGE("Y (mono) 1024x768 8bit/pixel"),
      MESSAGE("Y (mono) 800x600 16bit/pixel"),
      MESSAGE("Y (mono) 1024x768 16bit/pixel"),

      MESSAGE("YUV (4:2:2) 1280x960 16bit/pixel"),
      MESSAGE("RGB 1280x960 24bit/pixel"),
      MESSAGE("Y (mono) 1280x960 8bit/pixel"),
      MESSAGE("YUV (4:2:2) 1600x1200 16bit/pixel"),
      MESSAGE("RGB 1600x1200 24bit/pixel"),
      MESSAGE("Y (mono) 1600x1200 8bit/pixel"),
      MESSAGE("Y (mono) 1280x960 16bit/pixel"),
      MESSAGE("Y (mono) 1600x1200 16bit/pixel"),

      MESSAGE("Still image"),
      
      MESSAGE("Partial image format mode 0"),
      MESSAGE("Partial image format mode 1"),
      MESSAGE("Partial image format mode 2"),
      MESSAGE("Partial image format mode 3"),
      MESSAGE("Partial image format mode 4"),
      MESSAGE("Partial image format mode 5"),
      MESSAGE("Partial image format mode 6"),
      MESSAGE("Partial image format mode 7"),
      
      MESSAGE("Unsupported")
    };
    return MODES[mode];
  }

  unsigned int Camera1394::getFrameRateAsValue(FrameRate frameRate) throw() {
    static const unsigned int RATES[] = {
      15 * 65536/8, 15 * 65536/4, 15 * 65536/2, 15 * 65536, 30 * 65536, 60 * 65536
    };
    return RATES[frameRate];
  }

  void Camera1394::reset() throw(IEEE1394Exception) {
    BigEndian<uint32> buffer;
    buffer = 1 << 31;
    adapter.write(
      camera,
      commandRegisters + Camera1394Impl::INITIALIZE,
      Cast::getAddress(buffer),
      sizeof(buffer)
    );
    // TAG: could be set to unsupported mode
    // TAG: could be set to unsupported pixel format
    readModeSpecificState();
  }

  Camera1394::Camera1394() throw()
    : camera(IEEE1394::BROADCAST + 1) {
    // TAG: init attributes FIXME
    adapter.open(); // TAG: temporary fix
    readChannel = adapter.getReadChannel(4096 /*transmission.packetsPerFrame*/, Camera1394::ISOCHRONOUS_SUBCHANNELS);
  }
  
  bool Camera1394::isCamera(unsigned int node) throw(OutOfDomain, IEEE1394Exception) {
    bassert(node < IEEE1394::BROADCAST, OutOfDomain(this));
    try {
      Camera1394Impl::ConfigurationIntro config;
      adapter.read(
        node,
        IEEE1394::CSR_BASE_ADDRESS + IEEE1394::CONFIGURATION_ROM,
        Cast::getAddress(config),
        sizeof(config)
      );
      
      if (((config.crc >> 24) * sizeof(IEEE1394::Quadlet) >= sizeof(Camera1394Impl::BusInfo)) && // check for general ROM format
          (config.busInfo.name == 0x31333934) && // "1394"
          ((config.busInfo.flags & 0xf0ff0000) == 0x20ff0000) &&
          ((config.rootDirectory.crc & 0xffff0000) >= 0x00040000) &&
          ((config.rootDirectory.vendorId & 0xff000000) == 0x03000000) &&
          (((config.rootDirectory.flags & 0xff000000) != 0x0c000000) || ((config.rootDirectory.flags & 0xff00ffff) == 0x0c0083c0)) &&
          ((config.rootDirectory.nodeOffset & 0xff000000) == 0x8d000000) &&
          ((config.rootDirectory.deviceDirectoryOffset & 0xff000000) == 0xd1000000)) {
        
        const unsigned int deviceIndependentDirectoryOffset =
          (config.rootDirectory.deviceDirectoryOffset & 0x00ffffff) * sizeof(IEEE1394::Quadlet) +
          IEEE1394::CONFIGURATION_ROM +
          OFFSETOF(Camera1394Impl::ConfigurationIntro, rootDirectory.deviceDirectoryOffset);
        
        Camera1394Impl::DeviceIndependentDirectory deviceIndependentDirectory;
        adapter.read(
          node,
          IEEE1394::CSR_BASE_ADDRESS + deviceIndependentDirectoryOffset,
          Cast::getAddress(deviceIndependentDirectory),
          sizeof(deviceIndependentDirectory)
        );
        if ((deviceIndependentDirectory.specification == 0x1200a02d) && // (ID for 1394TA)
            ((deviceIndependentDirectory.version & 0xff000000) == 0x13000000) &&
            ((deviceIndependentDirectory.dependentOffset & 0xff000000) == 0xd4000000)) {
          return true;
        }
      }
    } catch (IOException&) {
      // continue with next node
    }
    return false;
  }
  
  bool Camera1394::isCamera(const EUI64& guid) throw(Camera1394Exception, IEEE1394Exception) {
    int node = adapter.getPhysicalId(guid);
    bassert(
      node >= 0,
      bindCause(Camera1394Exception("Device not found", this), IEEE1394::NODE_NOT_PRESENT)
    );
    return isCamera(node);
  }

  Array<EUI64> Camera1394::getCameras() throw(IEEE1394Exception) {
    Array<EUI64> cameras;

    for (unsigned int node = 0; node < adapter.getNumberOfNodes(); ++node) {      
      try {
        Camera1394Impl::ConfigurationIntro config;
        adapter.read(
          node,
          IEEE1394::CSR_BASE_ADDRESS + IEEE1394::CONFIGURATION_ROM,
          Cast::getAddress(config),
          sizeof(config)
        );
        
        if (((config.crc >> 24) * sizeof(IEEE1394::Quadlet) >= sizeof(Camera1394Impl::BusInfo)) && // check for general ROM format
            (config.busInfo.name == 0x31333934) && // "1394"
            ((config.busInfo.flags & 0xf0ff0000) == 0x20ff0000) &&
            ((config.rootDirectory.crc & 0xffff0000) >= 0x00040000) &&
            ((config.rootDirectory.vendorId & 0xff000000) == 0x03000000) &&
            (((config.rootDirectory.flags & 0xff000000) != 0x0c000000) || ((config.rootDirectory.flags & 0xff00ffff) == 0x0c0083c0)) &&
            ((config.rootDirectory.nodeOffset & 0xff000000) == 0x8d000000) &&
            ((config.rootDirectory.deviceDirectoryOffset & 0xff000000) == 0xd1000000)) {
          
          const unsigned int deviceIndependentDirectoryOffset =
            (config.rootDirectory.deviceDirectoryOffset & 0x00ffffff) * sizeof(IEEE1394::Quadlet) +
            IEEE1394::CONFIGURATION_ROM +
            OFFSETOF(Camera1394Impl::ConfigurationIntro, rootDirectory.deviceDirectoryOffset);
          
          Camera1394Impl::DeviceIndependentDirectory deviceIndependentDirectory;
          adapter.read(
            node,
            IEEE1394::CSR_BASE_ADDRESS + deviceIndependentDirectoryOffset,
            Cast::getAddress(deviceIndependentDirectory),
            sizeof(deviceIndependentDirectory)
          );

          if ((deviceIndependentDirectory.specification == 0x1200a02d) && // (ID for 1394TA)
              ((deviceIndependentDirectory.version & 0xff000000) == 0x13000000) &&
              ((deviceIndependentDirectory.dependentOffset & 0xff000000) == 0xd4000000)) {
            cameras.append(EUI64(config.busInfo.guid));
          }
        }
      } catch (IOException&) {
        // continue with next node
      }
    }
    return cameras;
  }

  void Camera1394::open(const EUI64& guid) throw(Camera1394Exception, IEEE1394Exception) {
    int node = adapter.getPhysicalId(guid);
    bassert(
      node >= 0,
      bindCause(Camera1394Exception("Device not found", this), IEEE1394::NODE_NOT_PRESENT)
    );
    bassert(
      isCamera(node),
      bindCause(Camera1394Exception("Not a camera", this), Camera1394::NOT_A_CAMERA)
    );
    
    Camera1394Impl::ConfigurationIntro config;
    adapter.read(
      node,
      IEEE1394::CSR_BASE_ADDRESS + IEEE1394::CONFIGURATION_ROM,
      Cast::getAddress(config),
      sizeof(config)
    );
    
    bassert(
      ((config.crc >> 24) * sizeof(IEEE1394::Quadlet) >= sizeof(Camera1394Impl::BusInfo)) && // check for general ROM format
      (config.busInfo.name == 0x31333934) && // "1394"
      ((config.busInfo.flags & 0xf0ff0000) == 0x20ff0000) &&
      ((config.rootDirectory.crc & 0xffff0000) >= 0x00040000) &&
      ((config.rootDirectory.vendorId & 0xff000000) == 0x03000000) &&
//      (((config.rootDirectory.flags & 0xff000000) != 0x0c000000) || ((config.rootDirectory.flags & 0xff00ffff) == 0x0c0083c0)) &&
      ((config.rootDirectory.nodeOffset & 0xff000000) == 0x8d000000) &&
      ((config.rootDirectory.deviceDirectoryOffset & 0xff000000) == 0xd1000000),
      bindCause(Camera1394Exception(this), Camera1394::NOT_A_CAMERA)
    );

    // TAG: store module vendor id (24 bit)

    const unsigned int deviceIndependentDirectoryOffset =
      (config.rootDirectory.deviceDirectoryOffset & 0x00ffffff) * sizeof(IEEE1394::Quadlet) +
      IEEE1394::CONFIGURATION_ROM +
      OFFSETOF(Camera1394Impl::ConfigurationIntro, rootDirectory.deviceDirectoryOffset);
    
    Camera1394Impl::DeviceIndependentDirectory deviceIndependentDirectory;
    adapter.read(
      node,
      IEEE1394::CSR_BASE_ADDRESS + deviceIndependentDirectoryOffset,
      Cast::getAddress(deviceIndependentDirectory),
      sizeof(deviceIndependentDirectory)
    );
    
    bassert(
      (deviceIndependentDirectory.specification == 0x1200a02d) && // (ID for 1394TA)
      ((deviceIndependentDirectory.version & 0xff000000) == 0x13000000) &&
      ((deviceIndependentDirectory.dependentOffset & 0xff000000) == 0xd4000000),
      bindCause(Camera1394Exception(this), IEEE1394::INVALID_DEVICE_INDEPENDENT_BLOCK)
    );

    switch (deviceIndependentDirectory.version & 0x00ffffff) {
    case 0x000100:
      specification = Camera1394::SPECIFICATION_1_04;
      break;
    case 0x000101:
      specification = Camera1394::SPECIFICATION_1_20;
      break;
    case 0x000102:
      specification = Camera1394::SPECIFICATION_1_30;
      break;
    default:
      specification = Camera1394::SPECIFICATION_LATER_THAN_1_30;
    }
    
    const unsigned int deviceDependentDirectoryOffset =
      (deviceIndependentDirectory.dependentOffset & 0x00ffffff) * sizeof(IEEE1394::Quadlet) +
      getFieldOffset(&Camera1394Impl::DeviceIndependentDirectory::dependentOffset) +
      deviceIndependentDirectoryOffset;
    
    Camera1394Impl::DeviceDependentDirectory deviceDependentDirectory;
    adapter.read(
      node,
      IEEE1394::CSR_BASE_ADDRESS + deviceDependentDirectoryOffset,
      Cast::getAddress(deviceDependentDirectory),
      sizeof(deviceDependentDirectory)
    );
    
    bassert(
      ((deviceDependentDirectory.commandRegisters & 0xff000000) == 0x40000000) &&
      ((deviceDependentDirectory.vendorNameLeaf & 0xff000000) == 0x81000000) &&
      ((deviceDependentDirectory.modelNameLeaf & 0xff000000) == 0x82000000),
      bindCause(Camera1394Exception(this), IEEE1394::INVALID_DEVICE_DEPENDENT_BLOCK)
    );

    commandRegisters = IEEE1394::CSR_BASE_ADDRESS +
      (deviceDependentDirectory.commandRegisters & 0x00ffffff) * sizeof(IEEE1394::Quadlet);
    
    IEEE1394::Quadlet quadlet;
    
    const unsigned int vendorNameOffset =
      (deviceDependentDirectory.vendorNameLeaf & 0x00ffffff) * sizeof(IEEE1394::Quadlet) +
      getFieldOffset(&Camera1394Impl::DeviceDependentDirectory::vendorNameLeaf) +
      deviceDependentDirectoryOffset;
    
    adapter.read(
      node,
      IEEE1394::CSR_BASE_ADDRESS + vendorNameOffset,
      Cast::getAddress(quadlet),
      sizeof(quadlet)
    );
    unsigned int vendorLeafSize = quadlet >> 16;
    ASSERT(vendorLeafSize >= 2);
    if (vendorLeafSize > 2) {
      PrimitiveArray<char> leaf(vendorLeafSize * sizeof(IEEE1394::Quadlet));
      adapter.read(
        node,
        IEEE1394::CSR_BASE_ADDRESS + vendorNameOffset + sizeof(IEEE1394::Quadlet),
        Cast::pointer<uint8*>(leaf),
        vendorLeafSize * sizeof(IEEE1394::Quadlet)
      );
      vendorName = String(leaf + 2 * sizeof(IEEE1394::Quadlet), (vendorLeafSize - 2) * sizeof(IEEE1394::Quadlet));
    }

    const unsigned int modelNameOffset =
      (deviceDependentDirectory.modelNameLeaf & 0x00ffffff) * sizeof(IEEE1394::Quadlet) +
      getFieldOffset(&Camera1394Impl::DeviceDependentDirectory::modelNameLeaf) +
      deviceDependentDirectoryOffset;
    
    adapter.read(
      node,
      IEEE1394::CSR_BASE_ADDRESS + modelNameOffset,
      Cast::getAddress(quadlet),
      sizeof(quadlet)
    );
    unsigned int modelLeafSize = quadlet >> 16;
    ASSERT(modelLeafSize >= 2);
    if (modelLeafSize > 2) {
      PrimitiveArray<char> leaf(modelLeafSize * sizeof(IEEE1394::Quadlet));
      adapter.read(
        node,
        IEEE1394::CSR_BASE_ADDRESS + modelNameOffset + sizeof(IEEE1394::Quadlet),
        Cast::pointer<uint8*>(leaf),
        modelLeafSize * sizeof(IEEE1394::Quadlet)
      );
      modelName = String(
        leaf + 2 * sizeof(IEEE1394::Quadlet),
        (modelLeafSize - 2) * sizeof(IEEE1394::Quadlet)
      );
    }
    
    // get supported formats
    adapter.read(
      node,
      commandRegisters + Camera1394Impl::V_FORMAT_INQ,
      Cast::getAddress(quadlet),
      sizeof(quadlet)
    );
    formats = Math::getBitReversal(quadlet);
    
    // get supported modes
    {
      IEEE1394::Quadlet buffer[8];
      for (unsigned int i = 0; i < 8; ++i) {
        buffer[i] = 0;
        if (isFormatSupported(static_cast<Format>(i))) {
          adapter.read(
            node,
            commandRegisters + Camera1394Impl::V_MODE_INQ_0 + i * sizeof(IEEE1394::Quadlet),
            Cast::getAddress(buffer[i]),
            sizeof(buffer[i])
          );
        }
      }
      for (unsigned int i = 0; i < getArraySize(supportedModes); ++i) {
        supportedModes[i] = buffer[Camera1394Impl::MODE_INFORMATION[i].format] & (1 << (31 - Camera1394Impl::MODE_INFORMATION[i].mode));
      }
    }

    // get supported frame rates
    {
      for (unsigned int i = 0; i < Camera1394::EXIF; ++i) { // only fixed modes
        frameRates[i] = 0;
        if (isModeSupported(static_cast<Mode>(i))) {
          unsigned int offset = Camera1394Impl::MODE_INFORMATION[i].format * 8 + Camera1394Impl::MODE_INFORMATION[i].mode;
          adapter.read(
            node,
            commandRegisters + Camera1394Impl::V_RATE_INQ_0_0 + offset * sizeof(IEEE1394::Quadlet),
            Cast::getAddress(quadlet),
            sizeof(quadlet)
          );
          frameRates[i] = Math::getBitReversal(quadlet);
        }
      }
    }
    
    frameRates[Camera1394::EXIF] = 0;
    
    {
      fill<unsigned int>(partialImageModeOffset, getArraySize(partialImageModeOffset), 0);
      for (unsigned int i = 0; i < getArraySize(partialImageModeOffset); ++i) {
        Mode mode = static_cast<Mode>(Camera1394::PARTIAL_IMAGE_MODE_0 + i);
        if (!isModeSupported(mode)) {
          continue;
        }
        adapter.read(
          node,
          commandRegisters + Camera1394Impl::V_CSR_INQ_7_0 + i * sizeof(IEEE1394::Quadlet),
          Cast::getAddress(quadlet),
          sizeof(quadlet)
        );
        partialImageModeOffset[i] = quadlet * sizeof(IEEE1394::Quadlet); // TAG: check for overflow
        
        if (supportedModes[PARTIAL_IMAGE_MODE_0 + i]) { // partial image modes  are guaranteed to be consecutive
          IEEE1394::Quadlet maximumImageSize;
          adapter.read(
            node,
            IEEE1394::CSR_BASE_ADDRESS + partialImageModeOffset[i] + Camera1394Impl::MAX_IMAGE_SIZE_INQ,
            Cast::getAddress(maximumImageSize),
            sizeof(maximumImageSize)
          );
          partialImageMode[i].maximumDimension = Dimension(maximumImageSize >> 16, maximumImageSize & 0xffff);
          
          IEEE1394::Quadlet unitSize;
          adapter.read(
            node,
            IEEE1394::CSR_BASE_ADDRESS + partialImageModeOffset[i] + Camera1394Impl::UNIT_SIZE_INQ,
            Cast::getAddress(unitSize),
            sizeof(unitSize)
          );
          unsigned int unitWidth = unitSize >> 16;
          unsigned int unitHeight = unitSize & 0xffff;
          
          IEEE1394::Quadlet colorCodingInquery;
          adapter.read(
            node,
            IEEE1394::CSR_BASE_ADDRESS + partialImageModeOffset[i] + Camera1394Impl::COLOR_CODING_INQ,
            Cast::getAddress(colorCodingInquery),
            sizeof(colorCodingInquery)
          );
          
          unsigned int unitHorizontalOffset = 0;
          unsigned int unitVerticalOffset = 0;
          if (specification >= Camera1394::SPECIFICATION_1_30) {
            IEEE1394::Quadlet unitOffsetInquery;
            adapter.read(
              node,
              IEEE1394::CSR_BASE_ADDRESS + partialImageModeOffset[i] + Camera1394Impl::UNIT_POSITION_INQ,
              Cast::getAddress(unitOffsetInquery),
              sizeof(unitOffsetInquery)
            );
            unitHorizontalOffset = unitOffsetInquery >> 16;
            unitVerticalOffset = unitOffsetInquery & 0xffff;
            
            //IEEE1394::Quadlet valueSetting;
            //adapter.read(node, IEEE1394::CSR_BASE_ADDRESS + partialImageModeOffset[i] + Camera1394Impl::VALUE_SETTING, Cast::getAddress(valueSetting), sizeof(valueSetting));
          }
          if (unitHorizontalOffset == 0) {
            unitHorizontalOffset = unitWidth;
          }
          if (unitVerticalOffset == 0) {
            unitVerticalOffset = unitHeight;
          }
          
          
          frameRates[PARTIAL_IMAGE_MODE_0 + i] = 0;
          partialImageMode[i].unitDimension = Dimension(unitWidth, unitHeight);
          partialImageMode[i].unitOffset = Point2D(unitVerticalOffset, unitHorizontalOffset);
          
          uint32 colorCodings = colorCodingInquery;
          unsigned int pixelFormats = 0;
          pixelFormats |= ((colorCodings >> 31) & 1) ? (1 << Y_8BIT) : 0;
          pixelFormats |= ((colorCodings >> 30) & 1) ? (1 << YUV_411_8BIT) : 0;
          pixelFormats |= ((colorCodings >> 29) & 1) ? (1 << YUV_422_8BIT) : 0;
          pixelFormats |= ((colorCodings >> 28) & 1) ? (1 << YUV_444_8BIT) : 0;
          pixelFormats |= ((colorCodings >> 27) & 1) ? (1 << RGB_8BIT) : 0;
          pixelFormats |= ((colorCodings >> 26) & 1) ? (1 << Y_16BIT) : 0;
          pixelFormats |= ((colorCodings >> 25) & 1) ? (1 << RGB_16BIT) : 0;
          partialImageMode[i].pixelFormats = pixelFormats;
        }
      }
    }

    // get capabilities
    adapter.read(
      node,
      commandRegisters + Camera1394Impl::BASIC_FUNC_INQ,
      Cast::getAddress(quadlet),
      sizeof(quadlet)
    );
    capabilities = 0;
    capabilities |= (quadlet & (1 << 15)) ? Camera1394::POWER_CONTROL : 0;
    capabilities |= (quadlet & (1 << 12)) ? Camera1394::SINGLE_ACQUISITION : 0;
    capabilities |= (quadlet & (1 << 11)) ? Camera1394::MULTI_ACQUISITION : 0;
    capabilities |= (quadlet & (1 << 31)) ? Camera1394::ADVANCED_FEATURES : 0;
    capabilities |= (quadlet & (1 << 30)) ? Camera1394::MODE_ERROR_STATUS : 0;
    capabilities |= (quadlet & (1 << 29)) ? Camera1394::FEATURE_ERROR_STATUS : 0;
    // maximumMemoryChannel = quadlet & 0x0f;
    camera = node;

    // reset
    // check if mode/pixel format is supported
    // if not activate code to set mode/pixel format after reset
    readModeSpecificState();
  }

  void Camera1394::readModeSpecificState() throw(IEEE1394Exception) {
    // TAG: remember to check for errors
    // TAG: if error then reset?
    
    IEEE1394::Quadlet quadlet;

    // get the current mode
    {
      adapter.read(
        camera,
        commandRegisters + Camera1394Impl::CURRENT_V_MODE,
        Cast::getAddress(quadlet),
        sizeof(quadlet)
      );
      unsigned int mode = quadlet >> 29;
      adapter.read(
        camera,
        commandRegisters + Camera1394Impl::CURRENT_V_FORMAT,
        Cast::getAddress(quadlet),
        sizeof(quadlet)
      );
      unsigned int format = quadlet >> 29;
      
      static const int MODE_BASE[] = {
        Camera1394::YUV_444_160X120_24BIT,
        Camera1394::YUV_422_800X600_16BIT,
        Camera1394::YUV_422_1280X960_16BIT,
        -1, // not supported
        -1, // not supported
        -1, // not supported
        Camera1394::EXIF,
        Camera1394::PARTIAL_IMAGE_MODE_0
      };

      static const unsigned int NUMBER_OF_MODES[] = {
        Camera1394::Y_640X480_16BIT - Camera1394::YUV_444_160X120_24BIT + 1,
        Camera1394::Y_1024X768_16BIT - Camera1394::YUV_422_800X600_16BIT + 1,
        Camera1394::Y_1600X1200_16BIT - Camera1394::YUV_422_1280X960_16BIT + 1,
        0, // not supported
        0, // not supported
        0, // not supported
        Camera1394::EXIF - Camera1394::EXIF + 1,
        Camera1394::PARTIAL_IMAGE_MODE_7 - Camera1394::PARTIAL_IMAGE_MODE_0 + 1
      };

      if (mode < NUMBER_OF_MODES[format]) {
        currentMode = static_cast<Mode>(MODE_BASE[format] + mode);
      } else {
        // currentMode = Camera1394::UNSPECIFIED; // mode is unspecified/unsupported
      }
    }

    // TAG: not for revision for format 6 and partial image format
    adapter.read(
      camera,
      commandRegisters + Camera1394Impl::CURRENT_V_RATE,
      Cast::getAddress(quadlet),
      sizeof(quadlet)
    );
    frameRate = static_cast<FrameRate>(quadlet >> 29);
    
    const Camera1394Impl::ModeInformation& info = Camera1394Impl::MODE_INFORMATION[currentMode];
    if (info.format == Camera1394::PARTIAL_IMAGE) {
      this->mode = partialImageMode[info.mode];
      
      IEEE1394::Quadlet imageOffset;
      adapter.read(
        camera,
        IEEE1394::CSR_BASE_ADDRESS + partialImageModeOffset[info.mode] + Camera1394Impl::IMAGE_POSITION,
        Cast::getAddress(imageOffset),
        sizeof(imageOffset)
      );
      IEEE1394::Quadlet imageDimension;
      adapter.read(
        camera,
        IEEE1394::CSR_BASE_ADDRESS + partialImageModeOffset[info.mode] + Camera1394Impl::IMAGE_SIZE,
        Cast::getAddress(imageDimension),
        sizeof(imageDimension)
      );
      IEEE1394::Quadlet colorCoding;
      adapter.read(
        camera,
        IEEE1394::CSR_BASE_ADDRESS + partialImageModeOffset[info.mode] + Camera1394Impl::COLOR_CODING_ID,
        Cast::getAddress(colorCoding),
        sizeof(colorCoding)
      );
      IEEE1394::Quadlet pixelsPerFrameInquery;
      adapter.read(
        camera,
        IEEE1394::CSR_BASE_ADDRESS + partialImageModeOffset[info.mode] + Camera1394Impl::PIXEL_NUMBER_INQ,
        Cast::getAddress(pixelsPerFrameInquery),
        sizeof(pixelsPerFrameInquery)
      );
      BigEndian<uint64> totalBytesPerFrameInquery;
      adapter.read(
        camera,
        IEEE1394::CSR_BASE_ADDRESS + partialImageModeOffset[info.mode] + Camera1394Impl::TOTAL_BYTES_HI_INQ,
        Cast::getAddress(totalBytesPerFrameInquery),
        sizeof(totalBytesPerFrameInquery)
      );
      IEEE1394::Quadlet packetParaInquery;
      adapter.read(
        camera,
        IEEE1394::CSR_BASE_ADDRESS + partialImageModeOffset[info.mode] + Camera1394Impl::PACKET_PARA_INQ,
        Cast::getAddress(packetParaInquery),
        sizeof(packetParaInquery)
      );
      IEEE1394::Quadlet bytesPerPacket;
      adapter.read(
        camera,
        IEEE1394::CSR_BASE_ADDRESS + partialImageModeOffset[info.mode] + Camera1394Impl::BYTE_PER_PACKET,
        Cast::getAddress(bytesPerPacket),
        sizeof(bytesPerPacket)
      );
      
      region.setOffset(Point2D(imageOffset & 0xffff, imageOffset >> 16));
      region.setDimension(Dimension(imageDimension >> 16, imageDimension & 0xffff));
      
      transmission.subchannel = 0;
      transmission.speed = 2;
      transmission.pixelsPerFrame = pixelsPerFrameInquery;
      transmission.totalBytesPerFrame = totalBytesPerFrameInquery;
      transmission.unitBytesPerPacket = packetParaInquery >> 16;
      transmission.maximumBytesPerPacket = packetParaInquery & 0xffff;
      transmission.recommendedBytesPerPacket = bytesPerPacket & 0xffff;
      // TAG: need something like: unsigned int IEEE1394::getMaximumPacketSize() const throw();
//       unsigned int size = adapter.getMaximumPacketSize();
//       if (size > transmission.maximumBytesPerPacket) {
//         size = transmission.maximumBytesPerPacket;
//       }
      transmission.bytesPerPacket = bytesPerPacket >> 16;
      if (transmission.bytesPerPacket == 0) { // repair if invalid
        transmission.bytesPerPacket = transmission.maximumBytesPerPacket;
      }
      transmission.bytesPerPacket = (transmission.bytesPerPacket/transmission.unitBytesPerPacket) * transmission.unitBytesPerPacket;

      // TAG: temporary fix
      bytesPerPacket = transmission.bytesPerPacket << 16;
      adapter.write(
        camera,
        IEEE1394::CSR_BASE_ADDRESS + partialImageModeOffset[info.mode] + Camera1394Impl::BYTE_PER_PACKET,
        Cast::getAddress(bytesPerPacket),
        sizeof(bytesPerPacket)
      );

      // packetsPerFrameInquery is updated when bytesPerPacket is written
      IEEE1394::Quadlet packetsPerFrameInquery;
      adapter.read(
        camera,
        IEEE1394::CSR_BASE_ADDRESS + partialImageModeOffset[info.mode] + Camera1394Impl::PACKET_PER_FRAME_INQ,
        Cast::getAddress(packetsPerFrameInquery),
        sizeof(packetsPerFrameInquery)
      );
      transmission.packetsPerFrame = packetsPerFrameInquery;
      
      // TAG: what about value settings
      
      static const PixelFormat PIXEL_FORMATS[] = {
        Camera1394::Y_8BIT,
        Camera1394::YUV_411_8BIT,
        Camera1394::YUV_422_8BIT,
        Camera1394::YUV_444_8BIT,
        Camera1394::RGB_8BIT,
        Camera1394::Y_16BIT,
        Camera1394::RGB_16BIT
      };
      
      if ((colorCoding >> 24) < getArraySize(PIXEL_FORMATS)) {
        pixelFormat = PIXEL_FORMATS[colorCoding >> 24];
      } else {
        // TAG: unsupported color coding => invalidate state
      }
      
    } else {
      ASSERT((info.pixelFormat >= 0) && info.width && info.height);

      this->mode.maximumDimension = Dimension(info.width, info.height);
      this->mode.unitDimension = this->mode.maximumDimension;
      this->mode.unitOffset = Point2D(info.height, info.width);
      this->mode.pixelFormats = 1 << info.pixelFormat;
      
      pixelFormat = static_cast<PixelFormat>(info.pixelFormat);
      region.setOffset(Point2D(0, 0));
      region.setDimension(Dimension(info.width, info.height));

      static const int QUADLETS_PER_PACKET[][6] = {
        {0, 0, 15, 30, 60, 0}, // YUV_444_160X120_24BIT
        {0, 20, 40, 80, 160, 0}, // YUV_422_320X240_16BIT
        {0, 60, 120, 240, 480, 0}, // YUV_411_640X480_12BIT
        {0, 80, 160, 320, 640, 0}, // YUV_422_640X480_16BIT
        {0, 120, 240, 480, 960, 0}, // RGB_640X480_24BIT
        {0, 40, 80, 160, 320, 640}, // Y_640X480_8BIT
        {0, 80, 160, 320, 640, 0}, // Y_640X480_16BIT

        {0, 125, 250, 500, 1000, 0}, // YUV_422_800X600_16BIT
        {0, 0, 375, 750, 0, 0}, // RGB_800X600_24BIT
        {0, 0, 125, 250, 500, 1000}, // Y_800X600_8BIT
        {96, 192, 384, 768, 0, 0}, // YUV_422_1024X768_16BIT
        {144, 288, 576, 0, 0, 0}, // RGB_1024X768_24BIT
        {48, 96, 192, 384, 768, 0}, // Y_1024X768_8BIT
        {0, 125, 250, 500, 1000, 0}, // Y_800X600_16BIT
        {96, 192, 384, 768, 0, 0}, // Y_1024X768_16BIT

        {160, 320, 640, 0, 0, 0}, // YUV_422_1280X960_16BIT
        {240, 480, 960, 0, 0, 0}, // RGB_1280X960_24BIT
        {80, 160, 320, 640, 0, 0}, // Y_1280X960_8BIT
        {250, 500, 1000, 0, 0, 0}, // YUV_422_1600X1200_16BIT
        {375, 750, 0, 0, 0, 0}, // RGB_1600X1200_24BIT
        {125, 250, 500, 1000, 0, 0}, // Y_1600X1200_8BIT
        {160, 320, 640, 0, 0, 0}, // Y_1280X960_16BIT
        {250, 500, 1000, 0, 0, 0} // Y_1600X1200_16BIT
      };

      static const int BITS_PER_PIXEL[] = {
        24, 16, 12, 16, 24, 8, 16, // format 0
        16, 24, 8, 16, 24, 8, 16, 16, // format 1
        16, 24, 8, 16, 24, 8, 16, 16 // format 2
      };
      
      transmission.subchannel = 0;
      transmission.speed = 2;
      transmission.pixelsPerFrame = region.getDimension().getSize();
      transmission.totalBytesPerFrame = BITS_PER_PIXEL[currentMode] * transmission.pixelsPerFrame/8;
      transmission.bytesPerPacket = QUADLETS_PER_PACKET[currentMode][getFrameRate()] * sizeof(IEEE1394::Quadlet);
      transmission.unitBytesPerPacket = transmission.bytesPerPacket;
      transmission.maximumBytesPerPacket = transmission.bytesPerPacket;
      transmission.recommendedBytesPerPacket = transmission.bytesPerPacket;
      transmission.packetsPerFrame = transmission.totalBytesPerFrame/transmission.bytesPerPacket;
    }
    
    // get supported features
    {
      features = 0;
      
      adapter.read(
        camera,
        commandRegisters + Camera1394Impl::FEATURE_HI_INQ,
        Cast::getAddress(quadlet),
        sizeof(quadlet)
      );
      features |= (quadlet & (1 << 31)) ? (1 << Camera1394::BRIGHTNESS_CONTROL) : 0;
      features |= (quadlet & (1 << 30)) ? (1 << Camera1394::AUTO_EXPOSURE_CONTROL) : 0;
      features |= (quadlet & (1 << 29)) ? (1 << Camera1394::SHARPNESS_CONTROL) : 0;
      features |= (quadlet & (1 << 28)) ? (1 << Camera1394::WHITE_BALANCE_CONTROL) : 0;
      features |= (quadlet & (1 << 27)) ? (1 << Camera1394::HUE_CONTROL) : 0;
      features |= (quadlet & (1 << 26)) ? (1 << Camera1394::SATURATION_CONTROL) : 0;
      features |= (quadlet & (1 << 25)) ? (1 << Camera1394::GAMMA_CONTROL) : 0;
      features |= (quadlet & (1 << 24)) ? (1 << Camera1394::SHUTTER_CONTROL) : 0;
      features |= (quadlet & (1 << 23)) ? (1 << Camera1394::GAIN_CONTROL) : 0;
      features |= (quadlet & (1 << 22)) ? (1 << Camera1394::IRIS_CONTROL) : 0;
      features |= (quadlet & (1 << 21)) ? (1 << Camera1394::FOCUS_CONTROL) : 0;
      features |= (quadlet & (1 << 20)) ? (1 << Camera1394::TEMPERATURE_CONTROL) : 0;
      features |= (quadlet & (1 << 19)) ? (1 << Camera1394::TRIGGER_CONTROL) : 0;

      adapter.read(
        camera,
        commandRegisters + Camera1394Impl::FEATURE_LO_INQ,
        Cast::getAddress(quadlet),
        sizeof(quadlet)
      );
      features |= (quadlet & (1 << 31)) ? (1 << Camera1394::ZOOM_CONTROL) : 0;
      features |= (quadlet & (1 << 30)) ? (1 << Camera1394::PAN_CONTROL) : 0;
      features |= (quadlet & (1 << 29)) ? (1 << Camera1394::TILT_CONTROL) : 0;
      features |= (quadlet & (1 << 28)) ? (1 << Camera1394::OPTICAL_FILTER_CONTROL) : 0;
      features |= (quadlet & (1 << 27)) ? (1 << Camera1394::CAPTURE_SIZE) : 0;
      features |= (quadlet & (1 << 26)) ? (1 << Camera1394::CAPTURE_QUALITY) : 0;

      advancedFeatureAddress = 0;
      if (capabilities & Camera1394::ADVANCED_FEATURES) {
        adapter.read(
          camera,
          commandRegisters + Camera1394Impl::ADVANCED_FEATURE_INQ,
          Cast::getAddress(quadlet),
          sizeof(quadlet)
        );
        advancedFeatureAddress = commandRegisters + quadlet * sizeof(IEEE1394::Quadlet); // TAG: is this allowed to be changed per mode
      }
    }

    fill<uint8>(Cast::getAddress(featureDescriptors), sizeof(featureDescriptors), 0);
    if (isFeatureSupported(Camera1394::BRIGHTNESS_CONTROL)) {
      adapter.read(
        camera,
        commandRegisters + Camera1394Impl::BRIGHTNESS_INQ,
        Cast::getAddress(quadlet),
        sizeof(quadlet)
      );
      Camera1394Impl::importGenericFeature(quadlet, featureDescriptors.brightness);
    }
    if (isFeatureSupported(Camera1394::AUTO_EXPOSURE_CONTROL)) {
      adapter.read(
        camera,
        commandRegisters + Camera1394Impl::AUTO_EXPOSURE_INQ,
        Cast::getAddress(quadlet),
        sizeof(quadlet)
      );
      Camera1394Impl::importGenericFeature(quadlet, featureDescriptors.autoExposure);
    }
    if (isFeatureSupported(Camera1394::SHARPNESS_CONTROL)) {
      adapter.read(
        camera,
        commandRegisters + Camera1394Impl::SHARPNESS_INQ,
        Cast::getAddress(quadlet),
        sizeof(quadlet)
      );
      Camera1394Impl::importGenericFeature(quadlet, featureDescriptors.sharpness);
    }
    if (isFeatureSupported(Camera1394::WHITE_BALANCE_CONTROL)) {
      adapter.read(
        camera,
        commandRegisters + Camera1394Impl::WHITE_BALANCE_INQ,
        Cast::getAddress(quadlet),
        sizeof(quadlet)
      );
      Camera1394Impl::importGenericFeature(quadlet, featureDescriptors.whiteBalance);
    }
    if (isFeatureSupported(Camera1394::HUE_CONTROL)) {
      adapter.read(
        camera,
        commandRegisters + Camera1394Impl::HUE_INQ,
        Cast::getAddress(quadlet),
        sizeof(quadlet)
      );
      Camera1394Impl::importGenericFeature(quadlet, featureDescriptors.hue);
    }
    if (isFeatureSupported(Camera1394::SATURATION_CONTROL)) {
      adapter.read(
        camera,
        commandRegisters + Camera1394Impl::SATURATION_INQ,
        Cast::getAddress(quadlet),
        sizeof(quadlet)
      );
      Camera1394Impl::importGenericFeature(quadlet, featureDescriptors.saturation);
    }
    if (isFeatureSupported(Camera1394::GAMMA_CONTROL)) {
      adapter.read(
        camera,
        commandRegisters + Camera1394Impl::GAMMA_INQ,
        Cast::getAddress(quadlet),
        sizeof(quadlet)
      );
      Camera1394Impl::importGenericFeature(quadlet, featureDescriptors.gamma);
    }
    if (isFeatureSupported(Camera1394::SHUTTER_CONTROL)) {
      adapter.read(
        camera,
        commandRegisters + Camera1394Impl::SHUTTER_INQ,
        Cast::getAddress(quadlet),
        sizeof(quadlet)
      );
      Camera1394Impl::importGenericFeature(quadlet, featureDescriptors.shutter);
    }
    if (isFeatureSupported(Camera1394::GAIN_CONTROL)) {
      adapter.read(
        camera,
        commandRegisters + Camera1394Impl::GAIN_INQ,
        Cast::getAddress(quadlet),
        sizeof(quadlet)
      );
      Camera1394Impl::importGenericFeature(quadlet, featureDescriptors.gain);
    }
    if (isFeatureSupported(Camera1394::IRIS_CONTROL)) {
      adapter.read(
        camera,
        commandRegisters + Camera1394Impl::IRIS_INQ,
        Cast::getAddress(quadlet),
        sizeof(quadlet)
      );
      Camera1394Impl::importGenericFeature(quadlet, featureDescriptors.iris);
    }
    if (isFeatureSupported(Camera1394::FOCUS_CONTROL)) {
      adapter.read(
        camera,
        commandRegisters + Camera1394Impl::FOCUS_INQ,
        Cast::getAddress(quadlet),
        sizeof(quadlet)
      );
      Camera1394Impl::importGenericFeature(quadlet, featureDescriptors.focus);
    }
    if (isFeatureSupported(Camera1394::TEMPERATURE_CONTROL)) {
      adapter.read(
        camera,
        commandRegisters + Camera1394Impl::TEMPERATURE_INQ,
        Cast::getAddress(quadlet),
        sizeof(quadlet)
      );
      Camera1394Impl::importGenericFeature(quadlet, featureDescriptors.temperature);
    }
    if (isFeatureSupported(Camera1394::TRIGGER_CONTROL)) {
      adapter.read(
        camera,
        commandRegisters + Camera1394Impl::TRIGGER_INQ,
        Cast::getAddress(quadlet),
        sizeof(quadlet)
      );
      union {
        uint32 integral;
        Camera1394Impl::TriggerFeatureInquery inquery;
      };
      integral = quadlet;
      featureDescriptors.trigger.available = inquery.presence;
//      featureDescriptors.trigger.controlable = inquery.absoluteControl;
      featureDescriptors.trigger.readable = inquery.readable;
      featureDescriptors.trigger.switchable = inquery.switchable;
      featureDescriptors.trigger.polarity = inquery.polarity;
      featureDescriptors.trigger.availableSignals =
        (inquery.mode0 << 0) | (inquery.mode1 << 1) | (inquery.mode2 << 2) | (inquery.mode3 << 3);
    }
    if (isFeatureSupported(Camera1394::ZOOM_CONTROL)) {
      adapter.read(
        camera,
        commandRegisters + Camera1394Impl::ZOOM_INQ,
        Cast::getAddress(quadlet),
        sizeof(quadlet)
      );
      Camera1394Impl::importGenericFeature(quadlet, featureDescriptors.zoom);
    }
    if (isFeatureSupported(Camera1394::PAN_CONTROL)) {
      adapter.read(
        camera,
        commandRegisters + Camera1394Impl::PAN_INQ,
        Cast::getAddress(quadlet),
        sizeof(quadlet)
      );
      Camera1394Impl::importGenericFeature(quadlet, featureDescriptors.pan);
    }
    if (isFeatureSupported(Camera1394::TILT_CONTROL)) {
      adapter.read(
        camera,
        commandRegisters + Camera1394Impl::TILT_INQ,
        Cast::getAddress(quadlet),
        sizeof(quadlet)
      );
      Camera1394Impl::importGenericFeature(quadlet, featureDescriptors.tilt);
    }
    if (isFeatureSupported(Camera1394::OPTICAL_FILTER_CONTROL)) {
      adapter.read(
        camera,
        commandRegisters + Camera1394Impl::OPTICAL_FILTER_INQ,
        Cast::getAddress(quadlet),
        sizeof(quadlet)
      );
      Camera1394Impl::importGenericFeature(quadlet, featureDescriptors.opticalFilter);
    }
    if (isFeatureSupported(Camera1394::CAPTURE_SIZE)) {
      adapter.read(
        camera,
        commandRegisters + Camera1394Impl::CAPTURE_SIZE_INQ,
        Cast::getAddress(quadlet),
        sizeof(quadlet)
      );
      Camera1394Impl::importGenericFeature(quadlet, featureDescriptors.captureSize);
    }
    if (isFeatureSupported(Camera1394::CAPTURE_QUALITY)) {
      adapter.read(
        camera,
        commandRegisters + Camera1394Impl::CAPTURE_QUALITY_INQ,
        Cast::getAddress(quadlet),
        sizeof(quadlet)
      );
      Camera1394Impl::importGenericFeature(quadlet, featureDescriptors.captureQuality);
    }
  }
  
  void Camera1394::close() throw() {
    adapter.close();
  }

  String Camera1394::getVendorName() const throw() {
    return vendorName;
  }
  
  String Camera1394::getModelName() const throw() {
    return modelName;
  }
  
  bool Camera1394::isFrameRateSupported(FrameRate frameRate) const throw() {
    // return false for partial image mode and EXIF mode
    return frameRates[currentMode] & (1 << frameRate);
  }

  bool Camera1394::isFeatureReadable(Feature feature) const throw() {
    if (!isFeatureSupported(feature)) {
      return false;
    }
    switch (feature) {
    case BRIGHTNESS_CONTROL:
      return featureDescriptors.brightness.readable;
    case AUTO_EXPOSURE_CONTROL:
      return featureDescriptors.autoExposure.readable;
    case SHARPNESS_CONTROL:
      return featureDescriptors.sharpness.readable;
    case WHITE_BALANCE_CONTROL:
      return featureDescriptors.whiteBalance.readable;
    case HUE_CONTROL:
      return featureDescriptors.hue.readable;
    case SATURATION_CONTROL:
      return featureDescriptors.saturation.readable;
    case GAMMA_CONTROL:
      return featureDescriptors.gamma.readable;
    case SHUTTER_CONTROL:
      return featureDescriptors.shutter.readable;
    case GAIN_CONTROL:
      return featureDescriptors.gain.readable;
    case IRIS_CONTROL:
      return featureDescriptors.iris.readable;
    case FOCUS_CONTROL:
      return featureDescriptors.focus.readable;
    case TEMPERATURE_CONTROL:
      return featureDescriptors.temperature.readable;
    case TRIGGER_CONTROL:
      return featureDescriptors.trigger.readable;
    case ZOOM_CONTROL:
      return featureDescriptors.zoom.readable;
    case PAN_CONTROL:
      return featureDescriptors.pan.readable;
    case TILT_CONTROL:
      return featureDescriptors.tilt.readable;
    case OPTICAL_FILTER_CONTROL:
      return featureDescriptors.opticalFilter.readable;
    case CAPTURE_SIZE:
      return featureDescriptors.captureSize.readable;
    case CAPTURE_QUALITY:
      return featureDescriptors.captureQuality.readable;
    default:
      throw OutOfDomain(this);
    }
  }
  
  const Camera1394::GenericFeatureDescriptor& Camera1394::getFeatureDescriptor(Feature feature) const throw(OutOfDomain) {
    switch (feature) {
    case BRIGHTNESS_CONTROL:
      return featureDescriptors.brightness;
    case AUTO_EXPOSURE_CONTROL:
      return featureDescriptors.autoExposure;
    case SHARPNESS_CONTROL:
      return featureDescriptors.sharpness;
    case WHITE_BALANCE_CONTROL:
      return featureDescriptors.whiteBalance;
    case HUE_CONTROL:
      return featureDescriptors.hue;
    case SATURATION_CONTROL:
      return featureDescriptors.saturation;
    case GAMMA_CONTROL:
      return featureDescriptors.gamma;
    case SHUTTER_CONTROL:
      return featureDescriptors.shutter;
    case GAIN_CONTROL:
      return featureDescriptors.gain;
    case IRIS_CONTROL:
      return featureDescriptors.iris;
    case FOCUS_CONTROL:
      return featureDescriptors.focus;
    case TEMPERATURE_CONTROL:
      return featureDescriptors.temperature;
    case ZOOM_CONTROL:
      return featureDescriptors.zoom;
    case PAN_CONTROL:
      return featureDescriptors.pan;
    case TILT_CONTROL:
      return featureDescriptors.tilt;
    case OPTICAL_FILTER_CONTROL:
      return featureDescriptors.opticalFilter;
    case CAPTURE_SIZE:
      return featureDescriptors.captureSize;
    case CAPTURE_QUALITY:
      return featureDescriptors.captureQuality;
    default:
      throw OutOfDomain(this);
    }
  }
  
  const Camera1394::TriggerFeatureDescriptor& Camera1394::getTriggerFeatureDescriptor() const throw() {
    return featureDescriptors.trigger;
  }
  
  void Camera1394::setMode(Mode mode) throw(NotSupported, IEEE1394Exception) {
    bassert(
      isModeSupported(mode),
      bindCause(NotSupported(this), Camera1394::MODE_NOT_SUPPORTED)
    );

    const Camera1394Impl::ModeInformation& info = Camera1394Impl::MODE_INFORMATION[mode];
    
    region.setOffset(Point2D(0, 0)); // TAG: also set register if partial image mode (or should we get)
    
    if (info.format == Camera1394::PARTIAL_IMAGE) {
      region.setDimension(partialImageMode[info.mode].maximumDimension);
      // TAG: also set register if partial image mode (or should we get)
    } else {
      ASSERT(info.width && info.height);
      region.setDimension(Dimension(info.width, info.height));
    }

    IEEE1394::Quadlet quadlet;
    quadlet = 0 << 31; // disable continuous
    adapter.write(
      camera,
      commandRegisters + Camera1394Impl::ISO_ENABLE,
      Cast::getAddress(quadlet),
      sizeof(quadlet)
    );
    quadlet = (0 << 31) | (0 << 30) | 0; // disable finite shots
    adapter.write(
      camera,
      commandRegisters + Camera1394Impl::FINITE_SHOTS,
      Cast::getAddress(quadlet),
      sizeof(quadlet)
    );

    if ((frameRates[mode] & (1 << frameRate)) == 0) { // is current frame rate supported for new mode
      for (unsigned int i = 0; i < 32; ++i) { // find new frame rate
        if (frameRates[mode] & (1 << i)) {
          frameRate = static_cast<FrameRate>(i);
          break;
        }
      }
    }

    // set mode
    quadlet = frameRate << 29;
    setCommandRegister(Camera1394Impl::CURRENT_V_RATE, quadlet);
    //adapter.write(camera, commandRegisters + Camera1394Impl::CURRENT_V_RATE, Cast::getAddress(quadlet), sizeof(quadlet));
    quadlet = Camera1394Impl::MODE_INFORMATION[mode].mode << 29;
    adapter.write(
      camera,
      commandRegisters + Camera1394Impl::CURRENT_V_MODE,
      Cast::getAddress(quadlet),
      sizeof(quadlet)
    );
    quadlet = Camera1394Impl::MODE_INFORMATION[mode].format << 29;
    adapter.write(
      camera,
      commandRegisters + Camera1394Impl::CURRENT_V_FORMAT,
      Cast::getAddress(quadlet),
      sizeof(quadlet)
    );
    quadlet = (transmission.subchannel << 28) | (transmission.speed << 24);
    adapter.write(
      camera,
      commandRegisters + Camera1394Impl::ISO_CHANNEL,
      Cast::getAddress(quadlet),
      sizeof(quadlet)
    );
    
    readModeSpecificState();
  }
  
  unsigned int Camera1394::getFrameRates(Mode mode) throw(NotSupported) {
    bassert(
      isModeSupported(mode),
      bindCause(NotSupported(this), Camera1394::MODE_NOT_SUPPORTED)
    );
    return frameRates[mode];
  }

  void Camera1394::setFrameRate(FrameRate frameRate) throw(NotSupported) {
    bassert(
      frameRates[currentMode] & (1 << frameRate),
      bindCause(NotSupported(this), Camera1394::FRAME_RATE_NOT_SUPPORTED)
    );
    this->frameRate = frameRate;
    
    IEEE1394::Quadlet quadlet = frameRate << 29;
    adapter.write(
      camera,
      commandRegisters + Camera1394Impl::CURRENT_V_RATE,
      Cast::getAddress(quadlet),
      sizeof(quadlet)
    );
//     quadlet = Camera1394Impl::MODE_INFORMATION[currentMode].mode << 29;
//     adapter.write(camera, commandRegisters + Camera1394Impl::CURRENT_V_MODE, Cast::getAddress(quadlet), sizeof(quadlet));
//     quadlet = Camera1394Impl::MODE_INFORMATION[currentMode].format << 29;
//     adapter.write(camera, commandRegisters + Camera1394Impl::CURRENT_V_FORMAT, Cast::getAddress(quadlet), sizeof(quadlet));
//     quadlet = (transmission.subchannel << 28) | (transmission.speed << 24);
//     adapter.write(camera, commandRegisters + Camera1394Impl::ISO_CHANNEL, Cast::getAddress(quadlet), sizeof(quadlet));
    
  }

  void Camera1394::enable() throw(IEEE1394Exception) {
    IEEE1394::Quadlet quadlet;
    quadlet = 1 << 31;
    adapter.write(
      camera,
      commandRegisters + Camera1394Impl::POWER,
      Cast::getAddress(quadlet),
      sizeof(quadlet)
    );
  }
  
  void Camera1394::disable() throw(IEEE1394Exception) {
    IEEE1394::Quadlet quadlet;
    quadlet = 0 << 31;
    adapter.write(
      camera,
      commandRegisters + Camera1394Impl::POWER,
      Cast::getAddress(quadlet),
      sizeof(quadlet)
    );
  }
  
  bool Camera1394::isUpAndRunning() const throw(IEEE1394Exception) {
    if ((capabilities & POWER_CONTROL) == 0) { // TAG: check if this is ok
      return true;
    }
    return getCommandRegister(Camera1394Impl::POWER) >> 31;
//     IEEE1394::Quadlet quadlet;
//     adapter.read(camera, commandRegisters + Camera1394Impl::POWER, Cast::getAddress(quadlet), sizeof(quadlet));
//     return quadlet >> 31;
  }
    
  bool Camera1394::getFeatureStatus(Feature feature) throw(IEEE1394Exception) {
    if ((capabilities & Camera1394::FEATURE_ERROR_STATUS) == 0) {
      return true; // assume ok
    }
    
    static const unsigned int BIT[] = {
      63-0, 63-1, 63-2, 63-3, 63-4, 63-5, 63-6, 63-7,
      63-8, 63-9, 63-10, 63-11, 63-12, 63-32, 63-33, 63-34,
      63-35, 63-48, 63-49
    };
    ASSERT(features < getArraySize(BIT));
    BigEndian<uint64> status;
    adapter.read(
      camera,
      commandRegisters + Camera1394Impl::FEATURE_ERROR_STATUS_HIGH,
      Cast::getAddress(status),
      sizeof(status)
    );
    return (status >> BIT[feature]) == 0; // check if error or warning
  }

  Camera1394::FeatureOperatingMode Camera1394::getFeatureOperatingMode(Feature feature) const throw(NotSupported) {
    bool available = false;
    switch (feature) {
    case BRIGHTNESS_CONTROL:
      available = featureDescriptors.brightness.available;
      break;
    case AUTO_EXPOSURE_CONTROL:
      available = featureDescriptors.autoExposure.available;
      break;
    case SHARPNESS_CONTROL:
      available = featureDescriptors.sharpness.available;
      break;
    case WHITE_BALANCE_CONTROL:
      available = featureDescriptors.whiteBalance.available;
      break;
    case HUE_CONTROL:
      available = featureDescriptors.hue.available;
      break;
    case SATURATION_CONTROL:
      available = featureDescriptors.saturation.available;
      break;
    case GAMMA_CONTROL:
      available = featureDescriptors.gamma.available;
      break;
    case SHUTTER_CONTROL:
      available = featureDescriptors.shutter.available;
      break;
    case GAIN_CONTROL:
      available = featureDescriptors.gain.available;
      break;
    case IRIS_CONTROL:
      available = featureDescriptors.iris.available;
      break;
    case FOCUS_CONTROL:
      available = featureDescriptors.focus.available;
      break;
    case TEMPERATURE_CONTROL:
      available = featureDescriptors.temperature.available;
      break;
    case TRIGGER_CONTROL:
      available = featureDescriptors.trigger.available;
      break;
    case ZOOM_CONTROL:
      available = featureDescriptors.zoom.available;
      break;
    case PAN_CONTROL:
      available = featureDescriptors.pan.available;
      break;
    case TILT_CONTROL:
      available = featureDescriptors.tilt.available;
      break;
    case OPTICAL_FILTER_CONTROL:
      available = featureDescriptors.opticalFilter.available;
      break;
    case CAPTURE_SIZE:
      available = featureDescriptors.captureSize.available;
      break;
    case CAPTURE_QUALITY:
      available = featureDescriptors.captureQuality.available;
      break;
    }
    
    bassert(
      available,
      bindCause(NotSupported(this), Camera1394::FEATURE_NOT_SUPPORTED)
    );
    uint32 quadlet = getCommandRegister(Camera1394Impl::FEATURE_CONTROL_REGISTER[feature]);
    
    //IEEE1394::Quadlet quadlet;
    //adapter.read(camera, commandRegisters + Camera1394Impl::FEATURE_CONTROL_REGISTER[feature], Cast::getAddress(quadlet), sizeof(quadlet));
    
    switch (feature) {
    case TRIGGER_CONTROL:
      {
        Camera1394Impl::TriggerFeatureControl control = *reinterpret_cast<const Camera1394Impl::TriggerFeatureControl*>(&quadlet); //  Cast::impersonate<Camera1394Impl::TriggerFeatureControl>(quadlet);
        return control.enabled ? Camera1394::MANUAL : Camera1394::DISABLED;
      }
    default:
      {
        // includes white balance and temperature features
        Camera1394Impl::FeatureControl control = *reinterpret_cast<const Camera1394Impl::FeatureControl*>(&quadlet); // Cast::impersonate<Camera1394Impl::FeatureControl>(quadlet);
        if (!control.enabled) {
          return Camera1394::DISABLED;
        }
        if (control.automaticMode) {
          return Camera1394::AUTOMATIC;
        }
        if (control.autoAdjustmentMode) {
          return Camera1394::AUTO_ADJUST;
        }
        return Camera1394::MANUAL;
      }
    }
  }
  
  void Camera1394::setFeatureOperatingMode(Feature feature, FeatureOperatingMode operatingMode) throw(NotSupported) {
    bool available = false;
    bool autoAdjustmentMode = false;
    bool switchable = false;
    bool automaticMode = false;
    bool manualMode = false;
    
    switch (feature) {
    case TRIGGER_CONTROL:
      available = featureDescriptors.trigger.available;
      switchable = featureDescriptors.trigger.switchable;
      break;
    default:
      {
        const GenericFeatureDescriptor& descriptor = getFeatureDescriptor(feature);
        available = descriptor.available;
        switchable = descriptor.switchable;
        autoAdjustmentMode = descriptor.autoAdjustmentMode;
        automaticMode = descriptor.automaticMode;
        manualMode = descriptor.manualMode;
      }
    }
    
    bassert(
      available &&
      ((operatingMode != Camera1394::DISABLED) || (switchable && (operatingMode == Camera1394::DISABLED))) &&
      ((operatingMode != Camera1394::AUTOMATIC) || (switchable && (operatingMode == Camera1394::AUTOMATIC))) &&
      ((operatingMode != Camera1394::AUTO_ADJUST) || (switchable && (operatingMode == Camera1394::AUTO_ADJUST))) &&
      ((operatingMode != Camera1394::MANUAL) || (manualMode && (operatingMode == Camera1394::MANUAL))),
      bindCause(NotSupported(this), Camera1394::FEATURE_OPERATING_MODE_NOT_SUPPORTED)
    );
    
    uint32 original = getCommandRegister(Camera1394Impl::FEATURE_CONTROL_REGISTER[feature]);
    
//     IEEE1394::Quadlet original;
//     adapter.read(
//       camera,
//       commandRegisters + Camera1394Impl::FEATURE_CONTROL_REGISTER[feature],
//       Cast::getAddress(original),
//       sizeof(original)
//     );
    
    IEEE1394::Quadlet quadlet;
    switch (feature) {
    case TRIGGER_CONTROL:
      {
        Camera1394Impl::TriggerFeatureControl control = *reinterpret_cast<const Camera1394Impl::TriggerFeatureControl*>(&original); // Cast::impersonate<Camera1394Impl::TriggerFeatureControl>(original);
        switch (operatingMode) {
        case Camera1394::DISABLED:
          control.enabled = false;
          break;
        case Camera1394::MANUAL:
          control.enabled = true;
          break;
        case Camera1394::AUTOMATIC:
          break;
        case Camera1394::AUTO_ADJUST:
          break;
        }
        control.absoluteControl = false;
        quadlet = *reinterpret_cast<const uint32*>(&control); // Cast::impersonate<uint32>(control);
      }
      break;
    default:
      {
        // includes white balance and temperature features
        Camera1394Impl::CommonFeatureControl control = *reinterpret_cast<const Camera1394Impl::CommonFeatureControl*>(&original); // Cast::impersonate<Camera1394Impl::CommonFeatureControl>(original);
        switch (operatingMode) {
        case Camera1394::DISABLED:
          control.enabled = false;
          control.automaticMode = false;
          control.autoAdjustmentMode = false;
          break;
        case Camera1394::AUTOMATIC:
          control.enabled = true;
          control.automaticMode = true;
          control.autoAdjustmentMode = false;
          break;
        case Camera1394::AUTO_ADJUST:
          control.enabled = true;
          control.automaticMode = false;
          control.autoAdjustmentMode = true;
          break;
        case Camera1394::MANUAL:
          control.enabled = true;
          control.automaticMode = false;
          control.autoAdjustmentMode = false;
          break;
        }
        control.absoluteControl = false;
        quadlet = *reinterpret_cast<const uint32*>(&control); // Cast::impersonate<uint32>(control);
      }
    }
    
    adapter.write(
      camera,
      commandRegisters + Camera1394Impl::FEATURE_CONTROL_REGISTER[feature],
      Cast::getAddress(quadlet),
      sizeof(quadlet)
    );
    
    if (!getFeatureStatus(feature)) { // check if error or warning
      adapter.write( // try to restore original value
        camera,
        commandRegisters + Camera1394Impl::FEATURE_CONTROL_REGISTER[feature],
        Cast::getAddress(original),
        sizeof(original)
      );
    }
  }
  
  void Camera1394::setGenericFeature(Feature feature, const GenericFeatureDescriptor& descriptor, int value) throw(NotSupported, OutOfRange, IEEE1394Exception) {
    bassert(
      descriptor.available,
      bindCause(NotSupported(this), Camera1394::FEATURE_NOT_SUPPORTED)
    );
    bassert(
      (value >= static_cast<int>(descriptor.minimum)) && (value <= static_cast<int>(descriptor.maximum)),
      OutOfDomain(this)
    );
    // TAG: mode must be manual
    ASSERT(feature < getArraySize(feature));
    uint64 featureRegister = commandRegisters + Camera1394Impl::FEATURE_CONTROL_REGISTER[feature];
    IEEE1394::Quadlet original;
    adapter.read(
      camera,
      featureRegister,
      Cast::getAddress(original),
      sizeof(original)
    );
    Camera1394Impl::FeatureControl control = *reinterpret_cast<const Camera1394Impl::FeatureControl*>(&original); // Cast::impersonate<Camera1394Impl::FeatureControl, uint32>(original);
    bassert(
      control.enabled && !control.automaticMode && !control.autoAdjustmentMode,
      bindCause(Camera1394Exception(this), Camera1394::INVALID_FEATURE_MODE)
    ); // must be in manual mode
    control.absoluteControl = false; // enable value field
    control.value = value;
    IEEE1394::Quadlet quadlet;
    quadlet = *reinterpret_cast<const uint32*>(&control); // Cast::impersonate<uint32>(control);
    adapter.write(camera, featureRegister, Cast::getAddress(quadlet), sizeof(quadlet));
    if (!getFeatureStatus(feature)) { // check if error or warning
      adapter.write(
        camera,
        featureRegister,
        Cast::getAddress(original),
        sizeof(original)
      ); // try to restore original value
    }
  }
  
  int Camera1394::getBrightness() const throw(NotSupported, IEEE1394Exception) {
    bassert(
      featureDescriptors.brightness.available && featureDescriptors.brightness.readable,
      bindCause(NotSupported(this), Camera1394::FEATURE_NOT_READABLE)
    );
    IEEE1394::Quadlet quadlet;
    adapter.read(
      camera,
      commandRegisters + Camera1394Impl::FEATURE_BRIGHTNESS,
      Cast::getAddress(quadlet),
      sizeof(quadlet)
    );
    return reinterpret_cast<const Camera1394Impl::FeatureControl*>(&quadlet)->value; // Cast::impersonate<Camera1394Impl::FeatureControl, uint32>(quadlet).value;
  }

  void Camera1394::setBrightness(int value) throw(NotSupported, OutOfRange, IEEE1394Exception) {
    setGenericFeature(BRIGHTNESS_CONTROL, featureDescriptors.brightness, value);
  }

  int Camera1394::getAutoExposure() const throw(NotSupported, IEEE1394Exception) {
    bassert(
      featureDescriptors.autoExposure.available && featureDescriptors.autoExposure.readable,
      bindCause(NotSupported(this), Camera1394::FEATURE_NOT_READABLE)
    );
    IEEE1394::Quadlet quadlet;
    adapter.read(
      camera,
      commandRegisters + Camera1394Impl::FEATURE_AUTO_EXPOSURE,
      Cast::getAddress(quadlet),
      sizeof(quadlet)
    );
    return reinterpret_cast<const Camera1394Impl::FeatureControl*>(&quadlet)->value; // Cast::impersonate<Camera1394Impl::FeatureControl, uint32>(quadlet).value;
  }

  void Camera1394::setAutoExposure(int value) throw(NotSupported, OutOfRange, IEEE1394Exception) {
    setGenericFeature(AUTO_EXPOSURE_CONTROL, featureDescriptors.autoExposure, value);
  }

  int Camera1394::getSharpness() const throw(NotSupported, IEEE1394Exception) {
    bassert(
      featureDescriptors.sharpness.available && featureDescriptors.sharpness.readable,
      bindCause(NotSupported(this), Camera1394::FEATURE_NOT_READABLE)
    );
    IEEE1394::Quadlet quadlet;
    adapter.read(
      camera,
      commandRegisters + Camera1394Impl::FEATURE_SHARPNESS,
      Cast::getAddress(quadlet),
      sizeof(quadlet)
    );
    return reinterpret_cast<const Camera1394Impl::FeatureControl*>(&quadlet)->value; // Cast::impersonate<Camera1394Impl::FeatureControl, uint32>(quadlet).value;
  }

  void Camera1394::setSharpness(int value) throw(NotSupported, OutOfRange, IEEE1394Exception) {
    setGenericFeature(SHARPNESS_CONTROL, featureDescriptors.sharpness, value);
  }

  int Camera1394::getWhiteBalanceBlueRatio() const throw(NotSupported, IEEE1394Exception) {
    bassert(
      featureDescriptors.whiteBalance.available && featureDescriptors.whiteBalance.readable,
      bindCause(NotSupported(this), Camera1394::FEATURE_NOT_READABLE)
    );
    IEEE1394::Quadlet quadlet;
    adapter.read(
      camera,
      commandRegisters + Camera1394Impl::FEATURE_WHITE_BALANCE,
      Cast::getAddress(quadlet),
      sizeof(quadlet)
    );
    return reinterpret_cast<const Camera1394Impl::WhiteBalanceFeatureControl*>(&quadlet)->blueRatio; // Cast::impersonate<Camera1394Impl::WhiteBalanceFeatureControl, uint32>(quadlet).blueRatio;
  }

  int Camera1394::getWhiteBalanceRedRatio() const throw(NotSupported, IEEE1394Exception) {
    bassert(
      featureDescriptors.whiteBalance.available && featureDescriptors.whiteBalance.readable,
      bindCause(NotSupported(this), Camera1394::FEATURE_NOT_READABLE)
    );
    IEEE1394::Quadlet quadlet;
    adapter.read(
      camera,
      commandRegisters + Camera1394Impl::FEATURE_WHITE_BALANCE,
      Cast::getAddress(quadlet),
      sizeof(quadlet)
    );
    return reinterpret_cast<const Camera1394Impl::WhiteBalanceFeatureControl*>(&quadlet)->redRatio; // Cast::impersonate<Camera1394Impl::WhiteBalanceFeatureControl, uint32>(quadlet).redRatio;
  }
  
  void Camera1394::setWhiteBalance(
    int blueRatio,
    int redRatio) throw(NotSupported, OutOfRange, IEEE1394Exception) {
    bassert(
      featureDescriptors.whiteBalance.available,
      bindCause(NotSupported(this), Camera1394::FEATURE_NOT_SUPPORTED)
    );
    bassert(
      (blueRatio >= static_cast<int>(featureDescriptors.whiteBalance.minimum)) &&
      (blueRatio <= static_cast<int>(featureDescriptors.whiteBalance.maximum)) &&
      (redRatio >= static_cast<int>(featureDescriptors.whiteBalance.minimum)) &&
      (redRatio <= static_cast<int>(featureDescriptors.whiteBalance.maximum)),
      OutOfDomain(this)
    );
    uint64 featureRegister = commandRegisters + Camera1394Impl::FEATURE_WHITE_BALANCE;
    IEEE1394::Quadlet original;
    adapter.read(
      camera,
      featureRegister,
      Cast::getAddress(original),
      sizeof(original)
    );
    Camera1394Impl::WhiteBalanceFeatureControl control =
      *reinterpret_cast<const Camera1394Impl::WhiteBalanceFeatureControl*>(&original); // Cast::impersonate<Camera1394Impl::WhiteBalanceFeatureControl, uint32>(original);
    bassert(
      control.enabled && !control.automaticMode && !control.autoAdjustmentMode,
      bindCause(Camera1394Exception(this), Camera1394::INVALID_FEATURE_MODE)
    ); // must be in manual mode
    control.absoluteControl = false; // enable value field
    control.blueRatio = blueRatio;
    control.redRatio = redRatio;
    IEEE1394::Quadlet quadlet;
    quadlet = *reinterpret_cast<const uint32*>(&control); // Cast::impersonate<uint32>(control);
    adapter.write(camera, featureRegister, Cast::getAddress(quadlet), sizeof(quadlet));
    if (!getFeatureStatus(WHITE_BALANCE_CONTROL)) { // check if error or warning
      adapter.write(
        camera,
        featureRegister,
        Cast::getAddress(original),
        sizeof(original)
      ); // try to restore original value
    }
  }

  int Camera1394::getHue() const throw(NotSupported, IEEE1394Exception) {
    bassert(
      featureDescriptors.hue.available && featureDescriptors.hue.readable,
      bindCause(NotSupported(this), Camera1394::FEATURE_NOT_READABLE)
    );
    IEEE1394::Quadlet quadlet;
    adapter.read(
      camera,
      commandRegisters + Camera1394Impl::FEATURE_HUE,
      Cast::getAddress(quadlet),
      sizeof(quadlet)
    );
    return reinterpret_cast<const Camera1394Impl::FeatureControl*>(&quadlet)->value; // Cast::impersonate<Camera1394Impl::FeatureControl, uint32>(quadlet).value;
  }

  void Camera1394::setHue(int value) throw(NotSupported, OutOfRange, IEEE1394Exception) {
    setGenericFeature(HUE_CONTROL, featureDescriptors.hue, value);
  }

  int Camera1394::getSaturation() const throw(NotSupported, IEEE1394Exception) {
    bassert(
      featureDescriptors.saturation.available && featureDescriptors.saturation.readable,
      bindCause(NotSupported(this), Camera1394::FEATURE_NOT_READABLE)
    );
    IEEE1394::Quadlet quadlet;
    adapter.read(
      camera,
      commandRegisters + Camera1394Impl::FEATURE_SATURATION,
      Cast::getAddress(quadlet),
      sizeof(quadlet)
    );
    return reinterpret_cast<const Camera1394Impl::FeatureControl*>(&quadlet)->value; // Cast::impersonate<Camera1394Impl::FeatureControl, uint32>(quadlet).value;
  }

  void Camera1394::setSaturation(int value) throw(NotSupported, OutOfRange, IEEE1394Exception) {
    setGenericFeature(SATURATION_CONTROL, featureDescriptors.saturation, value);
  }

  int Camera1394::getGamma() const throw(NotSupported, IEEE1394Exception) {
    bassert(
      featureDescriptors.gamma.available && featureDescriptors.gamma.readable,
      bindCause(NotSupported(this), Camera1394::FEATURE_NOT_READABLE)
    );
    IEEE1394::Quadlet quadlet;
    adapter.read(
      camera,
      commandRegisters + Camera1394Impl::FEATURE_GAMMA,
      Cast::getAddress(quadlet),
      sizeof(quadlet)
    );
    return reinterpret_cast<const Camera1394Impl::FeatureControl*>(&quadlet)->value; // Cast::impersonate<Camera1394Impl::FeatureControl, uint32>(quadlet).value;
  }

  void Camera1394::setGamma(int value) throw(NotSupported, OutOfRange, IEEE1394Exception) {
    setGenericFeature(GAMMA_CONTROL, featureDescriptors.gamma, value);
  }

  int Camera1394::getShutter() const throw(NotSupported, IEEE1394Exception) {
    bassert(
      featureDescriptors.shutter.available && featureDescriptors.shutter.readable,
      bindCause(NotSupported(this), Camera1394::FEATURE_NOT_READABLE)
    );
    IEEE1394::Quadlet quadlet;
    adapter.read(
      camera,
      commandRegisters + Camera1394Impl::FEATURE_SHUTTER,
      Cast::getAddress(quadlet),
      sizeof(quadlet)
    );
    return reinterpret_cast<const Camera1394Impl::FeatureControl*>(&quadlet)->value; // Cast::impersonate<Camera1394Impl::FeatureControl, uint32>(quadlet).value;
  }

  void Camera1394::setShutter(int value) throw(NotSupported, OutOfRange, IEEE1394Exception) {
    setGenericFeature(SHUTTER_CONTROL, featureDescriptors.shutter, value);
  }

  int Camera1394::getGain() const throw(NotSupported, IEEE1394Exception) {
    bassert(
      featureDescriptors.gain.available && featureDescriptors.gain.readable,
      bindCause(NotSupported(this), Camera1394::FEATURE_NOT_READABLE)
    );
    IEEE1394::Quadlet quadlet;
    adapter.read(
      camera,
      commandRegisters + Camera1394Impl::FEATURE_GAIN,
      Cast::getAddress(quadlet),
      sizeof(quadlet)
    );
    return reinterpret_cast<const Camera1394Impl::FeatureControl*>(&quadlet)->value; // Cast::impersonate<Camera1394Impl::FeatureControl, uint32>(quadlet).value;
  }

  void Camera1394::setGain(int value) throw(NotSupported, OutOfRange, IEEE1394Exception) {
    setGenericFeature(GAIN_CONTROL, featureDescriptors.gain, value);
  }

  int Camera1394::getIRIS() const throw(NotSupported, IEEE1394Exception) {
    bassert(
      featureDescriptors.iris.available && featureDescriptors.iris.readable,
      bindCause(NotSupported(this), Camera1394::FEATURE_NOT_READABLE)
    );
    IEEE1394::Quadlet quadlet;
    adapter.read(
      camera,
      commandRegisters + Camera1394Impl::FEATURE_IRIS,
      Cast::getAddress(quadlet),
      sizeof(quadlet)
    );
    return reinterpret_cast<const Camera1394Impl::FeatureControl*>(&quadlet)->value; // Cast::impersonate<Camera1394Impl::FeatureControl, uint32>(quadlet).value;
  }

  void Camera1394::setIRIS(int value) throw(NotSupported, OutOfRange, IEEE1394Exception) {
    setGenericFeature(IRIS_CONTROL, featureDescriptors.iris, value);
  }

  int Camera1394::getFocus() const throw(NotSupported, IEEE1394Exception) {
    bassert(
      featureDescriptors.focus.available && featureDescriptors.focus.readable,
      bindCause(NotSupported(this), Camera1394::FEATURE_NOT_READABLE)
    );
    IEEE1394::Quadlet quadlet;
    adapter.read(
      camera,
      commandRegisters + Camera1394Impl::FEATURE_FOCUS,
      Cast::getAddress(quadlet),
      sizeof(quadlet)
    );
    return reinterpret_cast<const Camera1394Impl::FeatureControl*>(&quadlet)->value; // Cast::impersonate<Camera1394Impl::FeatureControl, uint32>(quadlet).value;
  }

  void Camera1394::setFocus(int value) throw(NotSupported, OutOfRange, IEEE1394Exception) {
    setGenericFeature(FOCUS_CONTROL, featureDescriptors.focus, value);
  }

  int Camera1394::getTemperature() const throw(NotSupported, IEEE1394Exception) {
    bassert(
      featureDescriptors.temperature.available && featureDescriptors.temperature.readable,
      bindCause(NotSupported(this), Camera1394::FEATURE_NOT_READABLE)
    );
    IEEE1394::Quadlet quadlet;
    adapter.read(
      camera,
      commandRegisters + Camera1394Impl::FEATURE_TEMPERATURE,
      Cast::getAddress(quadlet),
      sizeof(quadlet)
    );
    return reinterpret_cast<const Camera1394Impl::TemperatureFeatureControl*>(&quadlet)->currentValue; // Cast::impersonate<Camera1394Impl::TemperatureFeatureControl, uint32>(quadlet).currentValue;
  }

  int Camera1394::getTargetTemperature() const throw(NotSupported, IEEE1394Exception) {
    bassert(
      featureDescriptors.temperature.available && featureDescriptors.temperature.readable,
      bindCause(NotSupported(this), Camera1394::FEATURE_NOT_READABLE)
    );
    IEEE1394::Quadlet quadlet;
    adapter.read(
      camera,
      commandRegisters + Camera1394Impl::FEATURE_TEMPERATURE,
      Cast::getAddress(quadlet),
      sizeof(quadlet)
    );
    return reinterpret_cast<const Camera1394Impl::TemperatureFeatureControl*>(&quadlet)->targetValue; // Cast::impersonate<Camera1394Impl::TemperatureFeatureControl, uint32>(quadlet).targetValue;
  }

  void Camera1394::setTemperature(int value) throw(NotSupported, OutOfRange, IEEE1394Exception) {
    bassert(
      featureDescriptors.temperature.available,
      bindCause(NotSupported(this), Camera1394::FEATURE_NOT_SUPPORTED)
    );
    bassert(
      (value >= static_cast<int>(featureDescriptors.temperature.minimum)) &&
      (value <= static_cast<int>(featureDescriptors.temperature.maximum)),
      OutOfDomain(this)
    );
    uint64 featureRegister = commandRegisters + Camera1394Impl::FEATURE_TEMPERATURE;
    IEEE1394::Quadlet original;
    adapter.read(
      camera,
      featureRegister,
      Cast::getAddress(original),
      sizeof(original)
    );
    Camera1394Impl::TemperatureFeatureControl control = *reinterpret_cast<const Camera1394Impl::TemperatureFeatureControl*>(&original); // Cast::impersonate<Camera1394Impl::TemperatureFeatureControl, uint32>(original);
    bassert(
      control.enabled && !control.automaticMode && !control.autoAdjustmentMode,
      bindCause(Camera1394Exception(this), Camera1394::INVALID_FEATURE_MODE)
    ); // must be in manual mode
    control.absoluteControl = false; // enable value field
    control.targetValue = value;
    IEEE1394::Quadlet quadlet;
    quadlet = *reinterpret_cast<const uint32*>(&control); // Cast::impersonate<uint32>(control);
    adapter.write(
      camera,
      featureRegister,
      Cast::getAddress(quadlet),
      sizeof(quadlet)
    );
    if (!getFeatureStatus(TEMPERATURE_CONTROL)) { // check if error or warning
      adapter.write(
        camera,
        featureRegister,
        Cast::getAddress(original),
        sizeof(original)
      ); // try to restore original value
    }
  }

  // TAG: getTrigger..???
  // TAG: setTriggerParam...??
  // TAG: setTriggerPolatity(bool polatiry) throw(NotSupported, IEEE1394Exception) {} bassert(POLARITY_INQ...
  
  int Camera1394::getZoom() const throw(NotSupported, IEEE1394Exception) {
    bassert(
      featureDescriptors.zoom.available && featureDescriptors.zoom.readable,
      bindCause(NotSupported(this), Camera1394::FEATURE_NOT_READABLE)
    );
    IEEE1394::Quadlet quadlet;
    adapter.read(
      camera,
      commandRegisters + Camera1394Impl::FEATURE_ZOOM,
      Cast::getAddress(quadlet),
      sizeof(quadlet)
    );
    return reinterpret_cast<const Camera1394Impl::FeatureControl*>(&quadlet)->value; // Cast::impersonate<Camera1394Impl::FeatureControl, uint32>(quadlet).value;
  }

  void Camera1394::setZoom(int value) throw(NotSupported, OutOfRange, IEEE1394Exception) {
    setGenericFeature(ZOOM_CONTROL, featureDescriptors.zoom, value);
  }

  int Camera1394::getPan() const throw(NotSupported, IEEE1394Exception) {
    bassert(
      featureDescriptors.pan.available && featureDescriptors.pan.readable,
      bindCause(NotSupported(this), Camera1394::FEATURE_NOT_READABLE)
    );
    IEEE1394::Quadlet quadlet;
    adapter.read(
      camera,
      commandRegisters + Camera1394Impl::FEATURE_PAN,
      Cast::getAddress(quadlet),
      sizeof(quadlet)
    );
    return reinterpret_cast<const Camera1394Impl::FeatureControl*>(&quadlet)->value; // Cast::impersonate<Camera1394Impl::FeatureControl, uint32>(quadlet).value;
  }

  void Camera1394::setPan(int value) throw(NotSupported, OutOfRange, IEEE1394Exception) {
    setGenericFeature(PAN_CONTROL, featureDescriptors.pan, value);
  }

  int Camera1394::getTilt() const throw(NotSupported, IEEE1394Exception) {
    bassert(
      featureDescriptors.tilt.available && featureDescriptors.tilt.readable,
      bindCause(NotSupported(this), Camera1394::FEATURE_NOT_READABLE)
    );
    IEEE1394::Quadlet quadlet;
    adapter.read(
      camera,
      commandRegisters + Camera1394Impl::FEATURE_TILT,
      Cast::getAddress(quadlet),
      sizeof(quadlet)
    );
    return reinterpret_cast<const Camera1394Impl::FeatureControl*>(&quadlet)->value; // Cast::impersonate<Camera1394Impl::FeatureControl, uint32>(quadlet).value;
  }

  void Camera1394::setTilt(int value) throw(NotSupported, OutOfRange, IEEE1394Exception) { // TAG: Camera1394Exception
    setGenericFeature(TILT_CONTROL, featureDescriptors.tilt, value);
  }

  int Camera1394::getOpticalFilter() const throw(NotSupported, IEEE1394Exception) {
    bassert(
      featureDescriptors.opticalFilter.available && featureDescriptors.opticalFilter.readable,
      bindCause(NotSupported(this), Camera1394::FEATURE_NOT_READABLE)
    );
    IEEE1394::Quadlet quadlet;
    adapter.read(
      camera,
      commandRegisters + Camera1394Impl::FEATURE_OPTICAL_FILTER,
      Cast::getAddress(quadlet),
      sizeof(quadlet)
    );
    return reinterpret_cast<const Camera1394Impl::FeatureControl*>(&quadlet)->value; // Cast::impersonate<Camera1394Impl::FeatureControl, uint32>(quadlet).value;
  }

  void Camera1394::setOpticalFilter(int value) throw(NotSupported, OutOfRange, IEEE1394Exception) {
    setGenericFeature(OPTICAL_FILTER_CONTROL, featureDescriptors.opticalFilter, value);
  }

  int Camera1394::getCaptureSize() const throw(NotSupported, IEEE1394Exception) {
    bassert(
      featureDescriptors.captureSize.available &&
      featureDescriptors.captureSize.readable,
      bindCause(NotSupported(this), Camera1394::FEATURE_NOT_READABLE)
    );
    IEEE1394::Quadlet quadlet;
    adapter.read(
      camera,
      commandRegisters + Camera1394Impl::FEATURE_CAPTURE_SIZE,
      Cast::getAddress(quadlet),
      sizeof(quadlet)
    );
    return reinterpret_cast<const Camera1394Impl::FeatureControl*>(&quadlet)->value; // Cast::impersonate<Camera1394Impl::FeatureControl, uint32>(quadlet).value;
  }

  void Camera1394::setCaptureSize(int value) throw(NotSupported, OutOfRange, IEEE1394Exception) {
    setGenericFeature(CAPTURE_SIZE, featureDescriptors.captureSize, value);
  }

  int Camera1394::getCaptureQuality() const throw(NotSupported, IEEE1394Exception) {
    bassert(
      featureDescriptors.captureQuality.available &&
      featureDescriptors.captureQuality.readable,
      bindCause(NotSupported(this), Camera1394::FEATURE_NOT_READABLE)
    );
    IEEE1394::Quadlet quadlet;
    adapter.read(
      camera,
      commandRegisters + Camera1394Impl::FEATURE_CAPTURE_QUALITY,
      Cast::getAddress(quadlet),
      sizeof(quadlet)
    );
    return reinterpret_cast<const Camera1394Impl::FeatureControl*>(&quadlet)->value; // Cast::impersonate<Camera1394Impl::FeatureControl, uint32>(quadlet).value;
  }

  void Camera1394::setCaptureQuality(int value) throw(NotSupported, OutOfRange, IEEE1394Exception) {
    setGenericFeature(CAPTURE_QUALITY, featureDescriptors.captureQuality, value);
  }

  Dimension Camera1394::getMaximumDimension(Mode mode) const throw(NotSupported) {
    bassert(
      isModeSupported(mode),
      bindCause(NotSupported(this), Camera1394::MODE_NOT_SUPPORTED)
    );
    if (Camera1394Impl::MODE_INFORMATION[mode].format == Camera1394::PARTIAL_IMAGE) {
      return partialImageMode[Camera1394Impl::MODE_INFORMATION[mode].mode].maximumDimension;
    } else {
      const Camera1394Impl::ModeInformation& info =
        Camera1394Impl::MODE_INFORMATION[mode];
      bassert(
        info.width && info.height,
        bindCause(NotSupported(this), Camera1394::MODE_NOT_SUPPORTED)
      );
      return Dimension(info.width, info.height);
    }
  }

  Dimension Camera1394::getUnitDimension(Mode mode) const throw(NotSupported) {
    bassert(
      isModeSupported(mode),
      bindCause(NotSupported(this), Camera1394::MODE_NOT_SUPPORTED)
    );
    if (Camera1394Impl::MODE_INFORMATION[mode].format == Camera1394::PARTIAL_IMAGE) {
      return partialImageMode[Camera1394Impl::MODE_INFORMATION[mode].mode].unitDimension;
    } else {
      const Camera1394Impl::ModeInformation& info = Camera1394Impl::MODE_INFORMATION[mode];
      bassert(
        info.width && info.height,
        bindCause(NotSupported(this), Camera1394::MODE_NOT_SUPPORTED)
      );
      return Dimension(info.width, info.height);
    }
  }

  Point2D Camera1394::getUnitOffset(Mode mode) const throw(NotSupported) {
    bassert(
      isModeSupported(mode),
      bindCause(NotSupported(this), Camera1394::MODE_NOT_SUPPORTED)
    );
    if (Camera1394Impl::MODE_INFORMATION[mode].format == Camera1394::PARTIAL_IMAGE) {
      return partialImageMode[Camera1394Impl::MODE_INFORMATION[mode].mode].unitOffset;
    } else {
      const Camera1394Impl::ModeInformation& info = Camera1394Impl::MODE_INFORMATION[mode];
      bassert(
        info.height && info.width,
        bindCause(NotSupported(this), Camera1394::MODE_NOT_SUPPORTED)
      );
      return Point2D(info.height, info.width);
    }
  }

  unsigned int Camera1394::getPixelFormats(Mode mode) const throw(NotSupported) {
    bassert(
      isModeSupported(mode),
      bindCause(NotSupported(this), Camera1394::MODE_NOT_SUPPORTED)
    );
    if (Camera1394Impl::MODE_INFORMATION[mode].format == Camera1394::PARTIAL_IMAGE) {
      return partialImageMode[Camera1394Impl::MODE_INFORMATION[mode].mode].pixelFormats;
    } else {
      ASSERT(Camera1394Impl::MODE_INFORMATION[mode].pixelFormat >= 0);
      return 1 << Camera1394Impl::MODE_INFORMATION[mode].pixelFormat;
    }
  }

  void Camera1394::setRegion(const Region& region) throw(OutOfDomain, IEEE1394Exception) {
    bassert(
      (region.getOffset().getColumn() % mode.unitOffset.getColumn() == 0) &&
      (region.getOffset().getRow() % mode.unitOffset.getRow() == 0) &&
      (region.getDimension().getWidth() % mode.unitDimension.getWidth() == 0) &&
      (region.getDimension().getHeight() % mode.unitDimension.getHeight() == 0) &&
      (region.getDimension().getWidth() <= mode.maximumDimension.getWidth()) &&
      (region.getDimension().getHeight() <= mode.maximumDimension.getHeight()),
      bindCause(OutOfDomain(this), Camera1394::REGION_NOT_SUPPORTED)
    );
    
    // if we end up here we are in partial mode

    Point2D offset = region.getOffset();
    Dimension dimension = region.getDimension();

    const Camera1394Impl::ModeInformation& info = Camera1394Impl::MODE_INFORMATION[currentMode];

    IEEE1394::Quadlet imageOffset = (offset.getColumn() << 16) | offset.getRow();
    adapter.write(
      camera,
      IEEE1394::CSR_BASE_ADDRESS + partialImageModeOffset[info.mode] + Camera1394Impl::IMAGE_POSITION,
      Cast::getAddress(imageOffset),
      sizeof(imageOffset)
    );
    IEEE1394::Quadlet imageDimension = (dimension.getWidth() << 16) | dimension.getHeight();
    adapter.write(
      camera,
      IEEE1394::CSR_BASE_ADDRESS + partialImageModeOffset[info.mode] + Camera1394Impl::IMAGE_SIZE,
      Cast::getAddress(imageDimension),
      sizeof(imageDimension)
    );
    
    readModeSpecificState(); // reloads new region
  }

  void Camera1394::setPixelFormat(PixelFormat pixelFormat) throw(NotSupported, IEEE1394Exception) {
    if (pixelFormat != this->pixelFormat) {
      bassert(
        mode.pixelFormats & (1 << pixelFormat),
        bindCause(NotSupported(this), Camera1394::PIXEL_FORMAT_NOT_SUPPORTED)
      );
      ASSERT(Camera1394Impl::MODE_INFORMATION[currentMode].format == Camera1394::PARTIAL_IMAGE); // TAG: what about EXIF
      // TAG: write to register
      this->pixelFormat = pixelFormat;
    }
  }

  bool Camera1394::acquire(
    uint8* buffer,
    unsigned int size) throw(ImageException, IEEE1394Exception) {
    bassert(
      size == transmission.totalBytesPerFrame,
      bindCause(ImageException(this), Camera1394::FRAME_DIMENSION_MISMATCH)
    );
    PrimitiveArray<IEEE1394::Quadlet> headers(transmission.packetsPerFrame);

    const unsigned int bytesInLastPacket = transmission.totalBytesPerFrame -
      transmission.bytesPerPacket * (transmission.packetsPerFrame - 1);
    
    IEEE1394::IsochronousReadFixedDataRequest request =
      readChannel.getReadFixedDataRequest();
    request.setSubchannel(transmission.subchannel);
    request.setNumberOfPackets(transmission.packetsPerFrame - 1);
    request.setHeaderSize(sizeof(IEEE1394::Quadlet));
    request.setPayload(transmission.bytesPerPacket);
    request.setBuffer(
      buffer,
      transmission.bytesPerPacket * (transmission.packetsPerFrame - 1),
      Cast::getAddress(headers)
    );

    PrimitiveArray<uint8> lastPacket(transmission.bytesPerPacket);
    IEEE1394::IsochronousReadFixedDataRequest lastRequest =
      readChannel.getReadFixedDataRequest();
    lastRequest.setSubchannel(transmission.subchannel);
    lastRequest.setNumberOfPackets(1);
    lastRequest.setHeaderSize(sizeof(IEEE1394::Quadlet));
    lastRequest.setPayload(transmission.bytesPerPacket);
    lastRequest.setBuffer(
      Cast::getAddress(lastPacket),
      bytesInLastPacket,
      Cast::getAddress(headers[transmission.packetsPerFrame - 1])
    );

    readChannel.queue(request);
    readChannel.queue(lastRequest);
    
    // enable transmission
    IEEE1394::Quadlet quadlet;
    if (capabilities & SINGLE_ACQUISITION) {
      quadlet = 1 << 31; // single shot
      adapter.write(
        camera,
        commandRegisters + Camera1394Impl::FINITE_SHOTS,
        Cast::getAddress(quadlet),
        sizeof(quadlet)
      );
    } else if (capabilities & MULTI_ACQUISITION) {
      quadlet = (1 << 30) + 1; // multi shot (ask for one frame)
      adapter.write(
        camera,
        commandRegisters + Camera1394Impl::FINITE_SHOTS,
        Cast::getAddress(quadlet),
        sizeof(quadlet)
      );
    } else {
      quadlet = 1 << 31; // use continuous
      adapter.write(
        camera,
        commandRegisters + Camera1394Impl::ISO_ENABLE, Cast::getAddress(quadlet),
        sizeof(quadlet)
      );
    }

    unsigned int period = getFramePeriod(getFrameRate());
    unsigned int requests = readChannel.dequeue(2, minimum<unsigned int>(2 * period/15, 999999999));
    if (requests < 2) {
      readChannel.cancel();
      readChannel.dequeue(2 - requests, 999999999); // TAG: must wait forever
    }
    
    bool success = (request.getStatus() == IEEE1394::COMPLETED) && (lastRequest.getStatus() == IEEE1394::COMPLETED);
    
    // disable transmission
    if (capabilities & SINGLE_ACQUISITION) {
      if (!success) { // only required on failure
        quadlet = 0 << 31; // single shot
        adapter.write(
          camera,
          commandRegisters + Camera1394Impl::FINITE_SHOTS,
          Cast::getAddress(quadlet),
          sizeof(quadlet)
        );
      }
    } else if (capabilities & MULTI_ACQUISITION) {
      if (!success) { // only required on failure
        quadlet = (0 << 30) + 0; // multi shot
        adapter.write(
          camera,
          commandRegisters + Camera1394Impl::FINITE_SHOTS,
          Cast::getAddress(quadlet),
          sizeof(quadlet)
        );
      }
    } else {
      // always required to be stopped - continuous
      quadlet = 0;
      adapter.write(
        camera,
        commandRegisters + Camera1394Impl::ISO_ENABLE,
        Cast::getAddress(quadlet),
        sizeof(quadlet)
      );
    }
    
    if (success) {
      copy<uint8>(buffer + transmission.bytesPerPacket * (transmission.packetsPerFrame - 1), lastPacket, bytesInLastPacket);
    }
    
    return success;
  }

  bool Camera1394::acquire(ArrayImage<uint8>& frame) throw(NotSupported, ImageException, IEEE1394Exception) {
    if (pixelFormat != Camera1394::Y_8BIT) {
      setPixelFormat(Camera1394::Y_8BIT);
    }
    bassert(
      frame.getDimension() == region.getDimension(),
      bindCause(ImageException(this), Camera1394::FRAME_DIMENSION_MISMATCH)
    );
    PrimitiveArray<IEEE1394::Quadlet> headers(transmission.packetsPerFrame);
    
    const unsigned int bytesInLastPacket = transmission.totalBytesPerFrame -
      transmission.bytesPerPacket * (transmission.packetsPerFrame - 1);
    
    IEEE1394::IsochronousReadFixedDataRequest request =
      readChannel.getReadFixedDataRequest();
    request.setSubchannel(transmission.subchannel);
    request.setNumberOfPackets(transmission.packetsPerFrame - 1);
    request.setHeaderSize(sizeof(IEEE1394::Quadlet));
    request.setPayload(transmission.bytesPerPacket);
    request.setBuffer(
      Cast::pointer<uint8*>(frame.getElements()),
      transmission.bytesPerPacket * (transmission.packetsPerFrame - 1),
      Cast::getAddress(headers)
    );

    PrimitiveArray<uint8> lastPacket(transmission.bytesPerPacket);
    IEEE1394::IsochronousReadFixedDataRequest lastRequest =
      readChannel.getReadFixedDataRequest();
    lastRequest.setSubchannel(transmission.subchannel);
    lastRequest.setNumberOfPackets(1);
    lastRequest.setHeaderSize(sizeof(IEEE1394::Quadlet));
    lastRequest.setPayload(transmission.bytesPerPacket /*bytesInLastPacket*/);
    lastRequest.setBuffer(
      Cast::getAddress(lastPacket), // Cast::pointer<char*>(frame.getElements() + transmission.bytesPerPacket * (transmission.packetsPerFrame - 1))
      bytesInLastPacket,
      Cast::getAddress(headers[transmission.packetsPerFrame - 1])
    );

    readChannel.queue(request);
    readChannel.queue(lastRequest);
    
    // enable transmission
    IEEE1394::Quadlet quadlet;
    if (capabilities & SINGLE_ACQUISITION) {
      quadlet = 1 << 31; // single shot
      adapter.write(
        camera,
        commandRegisters + Camera1394Impl::FINITE_SHOTS,
        Cast::getAddress(quadlet),
        sizeof(quadlet)
      );
    } else if (capabilities & MULTI_ACQUISITION) {
      quadlet = (1 << 30) + 1; // multi shot (ask for one frame)
      adapter.write(
        camera,
        commandRegisters + Camera1394Impl::FINITE_SHOTS,
        Cast::getAddress(quadlet),
        sizeof(quadlet)
      );
    } else {
      quadlet = 1 << 31; // use continuous
      adapter.write(
        camera,
        commandRegisters + Camera1394Impl::ISO_ENABLE,
        Cast::getAddress(quadlet),
        sizeof(quadlet)
      );
    }

    unsigned int period = getFramePeriod(getFrameRate());
    unsigned int requests = readChannel.dequeue(2, 9999999+0*minimum<unsigned int>(2 * period/15, 999999999));
    if (requests < 2) {
      readChannel.cancel();
      readChannel.dequeue(2 - requests, 999999999); // TAG: must wait forever
    }
    
    bool success = (request.getStatus() == IEEE1394::COMPLETED) && (lastRequest.getStatus() == IEEE1394::COMPLETED);
    
//     fout << MESSAGE("Sub channel: ") << request.getSubchannel() << EOL
//          << MESSAGE("Valid: ") << request.isValid() << EOL
//          << MESSAGE("Status: ") << request.getStatus() << EOL
//          << MESSAGE("Pending: ") << request.isPending() << EOL
//          << MESSAGE("Packets: ") << request.getNumberOfPackets() << EOL
//          << MESSAGE("Header size: ") << request.getHeaderSize() << EOL
//          << MESSAGE("Payload: ") << request.getPayload() << EOL
//          << MESSAGE("Buffer size: ") << request.getBufferSize() << EOL
//          << MESSAGE("Buffer: ") << static_cast<void*>(request.getBuffer()) << EOL
//          << MESSAGE("Secondary buffer: ") << static_cast<void*>(request.getSecondaryBuffer()) << EOL
//          << MESSAGE("Received packets: ") << request.getReceivedPackets() << EOL
//          << ENDL;
    
//     fout << MESSAGE("Sub channel: ") << lastRequest.getSubchannel() << EOL
//          << MESSAGE("Valid: ") << lastRequest.isValid() << EOL
//          << MESSAGE("Status: ") << lastRequest.getStatus() << EOL
//          << MESSAGE("Pending: ") << lastRequest.isPending() << EOL
//          << MESSAGE("Packets: ") << lastRequest.getNumberOfPackets() << EOL
//          << MESSAGE("Header size: ") << lastRequest.getHeaderSize() << EOL
//          << MESSAGE("Payload: ") << lastRequest.getPayload() << EOL
//          << MESSAGE("Buffer size: ") << lastRequest.getBufferSize() << EOL
//          << MESSAGE("Buffer: ") << static_cast<void*>(lastRequest.getBuffer()) << EOL
//          << MESSAGE("Secondary buffer: ") << static_cast<void*>(lastRequest.getSecondaryBuffer()) << EOL
//          << MESSAGE("Received packets: ") << lastRequest.getReceivedPackets() << EOL
//          << ENDL;
    
    // disable transmission
    if (capabilities & SINGLE_ACQUISITION) {
      if (!success) { // only required on failure
        quadlet = 0 << 31; // single shot
        adapter.write(
          camera,
          commandRegisters + Camera1394Impl::FINITE_SHOTS,
          Cast::getAddress(quadlet),
          sizeof(quadlet)
        );
      }
    } else if (capabilities & MULTI_ACQUISITION) {
      if (!success) { // only required on failure
        quadlet = (0 << 30) + 0; // multi shot
        adapter.write(
          camera,
          commandRegisters + Camera1394Impl::FINITE_SHOTS,
          Cast::getAddress(quadlet),
          sizeof(quadlet)
        );
      }
    } else {
      // always required to be stopped - continuous
      quadlet = 0;
      adapter.write(
        camera,
        commandRegisters + Camera1394Impl::ISO_ENABLE,
        Cast::getAddress(quadlet),
        sizeof(quadlet)
      );
    }
    
    if (success) {
      copy<uint8>(frame.getElements() + transmission.bytesPerPacket * (transmission.packetsPerFrame - 1), lastPacket, bytesInLastPacket);
    }
    
    return success;
  }

  bool Camera1394::acquire(ArrayImage<uint16>& frame) throw(NotSupported, ImageException, IEEE1394Exception) {
    if (pixelFormat != Camera1394::Y_16BIT) {
      setPixelFormat(Camera1394::Y_16BIT);
    }
    
    bassert(
      frame.getDimension() == region.getDimension(),
      bindCause(ImageException(this), Camera1394::FRAME_DIMENSION_MISMATCH)
    );
    PrimitiveArray<IEEE1394::Quadlet> headers(transmission.packetsPerFrame);
    
    const unsigned int bytesInLastPacket = transmission.totalBytesPerFrame -
      transmission.bytesPerPacket * (transmission.packetsPerFrame - 1);
    
    IEEE1394::IsochronousReadFixedDataRequest request =
      readChannel.getReadFixedDataRequest();
    request.setSubchannel(transmission.subchannel);
    request.setNumberOfPackets(transmission.packetsPerFrame - 1);
    request.setHeaderSize(sizeof(IEEE1394::Quadlet));
    request.setPayload(transmission.bytesPerPacket);
    request.setBuffer(
      Cast::pointer<uint8*>(frame.getElements()),
      transmission.bytesPerPacket * (transmission.packetsPerFrame - 1),
      Cast::getAddress(headers)
    );
    
    PrimitiveArray<uint8> lastPacket(transmission.bytesPerPacket);
    IEEE1394::IsochronousReadFixedDataRequest lastRequest =
      readChannel.getReadFixedDataRequest();
    lastRequest.setSubchannel(transmission.subchannel);
    lastRequest.setNumberOfPackets(1);
    lastRequest.setHeaderSize(sizeof(IEEE1394::Quadlet));
    lastRequest.setPayload(transmission.bytesPerPacket /*bytesInLastPacket*/);
    lastRequest.setBuffer(
      Cast::getAddress(lastPacket), // Cast::pointer<char*>(frame.getElements() + transmission.bytesPerPacket * (transmission.packetsPerFrame - 1))
      bytesInLastPacket,
      Cast::getAddress(headers[transmission.packetsPerFrame - 1])
    );
    
    readChannel.queue(request);
    readChannel.queue(lastRequest);
    
    // enable transmission
    IEEE1394::Quadlet quadlet;
    if (capabilities & SINGLE_ACQUISITION) {
      quadlet = 1 << 31; // single shot
      adapter.write(
        camera,
        commandRegisters + Camera1394Impl::FINITE_SHOTS,
        Cast::getAddress(quadlet),
        sizeof(quadlet)
      );
    } else if (capabilities & MULTI_ACQUISITION) {
      quadlet = (1 << 30) + 1; // multi shot (ask for one frame)
      adapter.write(
        camera,
        commandRegisters + Camera1394Impl::FINITE_SHOTS,
        Cast::getAddress(quadlet),
        sizeof(quadlet)
      );
    } else {
      quadlet = 1 << 31; // use continuous
      adapter.write(
        camera,
        commandRegisters + Camera1394Impl::ISO_ENABLE,
        Cast::getAddress(quadlet),
        sizeof(quadlet)
      );
    }

    unsigned int period = getFramePeriod(getFrameRate());
    unsigned int requests = readChannel.dequeue(2, minimum<unsigned int>(2 * period/15, 999999999));
    if (requests < 2) {
      readChannel.cancel();
      readChannel.dequeue(2 - requests, 999999999); // TAG: must wait forever
    }
    
    bool success = (request.getStatus() == IEEE1394::COMPLETED) && (lastRequest.getStatus() == IEEE1394::COMPLETED);
    
    // disable transmission
    if (capabilities & SINGLE_ACQUISITION) {
      if (!success) { // only required on failure
        quadlet = 0 << 31; // single shot
        adapter.write(
          camera,
          commandRegisters + Camera1394Impl::FINITE_SHOTS,
          Cast::getAddress(quadlet),
          sizeof(quadlet)
        );
      }
    } else if (capabilities & MULTI_ACQUISITION) {
      if (!success) { // only required on failure
        quadlet = (0 << 30) + 0; // multi shot
        adapter.write(
          camera,
          commandRegisters + Camera1394Impl::FINITE_SHOTS,
          Cast::getAddress(quadlet),
          sizeof(quadlet)
        );
      }
    } else {
      // always required to be stopped - continuous
      quadlet = 0;
      adapter.write(
        camera,
        commandRegisters + Camera1394Impl::ISO_ENABLE,
        Cast::getAddress(quadlet),
        sizeof(quadlet)
      );
    }
    
    if (success) {
      copy<uint8>(
        Cast::pointer<uint8*>(frame.getElements()) + transmission.bytesPerPacket * (transmission.packetsPerFrame - 1),
        lastPacket,
        bytesInLastPacket
      );
    }
    
    return success;
  }

  bool Camera1394::acquire(ArrayImage<RGB24Pixel>& frame) throw(NotSupported, ImageException, IEEE1394Exception) {
    if (pixelFormat != Camera1394::RGB_8BIT) {
      setPixelFormat(Camera1394::RGB_8BIT);
    }

    bassert(
      frame.getDimension() == region.getDimension(),
      bindCause(ImageException(this), Camera1394::FRAME_DIMENSION_MISMATCH)
    );
    PrimitiveArray<IEEE1394::Quadlet> headers(transmission.packetsPerFrame);
    
    const unsigned int bytesInLastPacket = transmission.totalBytesPerFrame -
      transmission.bytesPerPacket * (transmission.packetsPerFrame - 1);
    
    IEEE1394::IsochronousReadFixedDataRequest request =
      readChannel.getReadFixedDataRequest();
    request.setSubchannel(transmission.subchannel);
    request.setNumberOfPackets(transmission.packetsPerFrame - 1);
    request.setHeaderSize(sizeof(IEEE1394::Quadlet));
    request.setPayload(transmission.bytesPerPacket);
    request.setBuffer(
      Cast::pointer<uint8*>(frame.getElements()),
      transmission.bytesPerPacket * (transmission.packetsPerFrame - 1),
      Cast::getAddress(headers)
    );

    PrimitiveArray<uint8> lastPacket(transmission.bytesPerPacket);
    IEEE1394::IsochronousReadFixedDataRequest lastRequest =
      readChannel.getReadFixedDataRequest();
    lastRequest.setSubchannel(transmission.subchannel);
    lastRequest.setNumberOfPackets(1);
    lastRequest.setHeaderSize(sizeof(IEEE1394::Quadlet));
    lastRequest.setPayload(transmission.bytesPerPacket /*bytesInLastPacket*/);
    lastRequest.setBuffer(
      Cast::getAddress(lastPacket), // Cast::pointer<char*>(frame.getElements() + transmission.bytesPerPacket * (transmission.packetsPerFrame - 1))
      bytesInLastPacket,
      Cast::getAddress(headers[transmission.packetsPerFrame - 1])
    );
    
    readChannel.queue(request);
    readChannel.queue(lastRequest);
    
    // enable transmission
    IEEE1394::Quadlet quadlet;
    if (capabilities & SINGLE_ACQUISITION) {
      quadlet = 1 << 31; // single shot
      adapter.write(
        camera,
        commandRegisters + Camera1394Impl::FINITE_SHOTS,
        Cast::getAddress(quadlet),
        sizeof(quadlet)
      );
    } else if (capabilities & MULTI_ACQUISITION) {
      quadlet = (1 << 30) + 1; // multi shot (ask for one frame)
      adapter.write(
        camera,
        commandRegisters + Camera1394Impl::FINITE_SHOTS,
        Cast::getAddress(quadlet),
        sizeof(quadlet)
      );
    } else {
      quadlet = 1 << 31; // use continuous
      adapter.write(
        camera,
        commandRegisters + Camera1394Impl::ISO_ENABLE,
        Cast::getAddress(quadlet),
        sizeof(quadlet)
      );
    }

    unsigned int period = getFramePeriod(getFrameRate());
    unsigned int requests = readChannel.dequeue(
      2,
      minimum<unsigned int>(2 * period/15, 999999999)
    );
    if (requests < 2) {
      readChannel.cancel();
      readChannel.dequeue(2 - requests, 999999999); // TAG: must wait forever
    }
    
    bool success = (request.getStatus() == IEEE1394::COMPLETED) &&
      (lastRequest.getStatus() == IEEE1394::COMPLETED);
    
    // disable transmission
    if (capabilities & SINGLE_ACQUISITION) {
      if (!success) { // only required on failure
        quadlet = 0 << 31; // single shot
        adapter.write(
          camera,
          commandRegisters + Camera1394Impl::FINITE_SHOTS,
          Cast::getAddress(quadlet),
          sizeof(quadlet)
        );
      }
    } else if (capabilities & MULTI_ACQUISITION) {
      if (!success) { // only required on failure
        quadlet = (0 << 30) + 0; // multi shot
        adapter.write(
          camera,
          commandRegisters + Camera1394Impl::FINITE_SHOTS,
          Cast::getAddress(quadlet),
          sizeof(quadlet)
        );
      }
    } else {
      // always required to be stopped - continuous
      quadlet = 0;
      adapter.write(
        camera,
        commandRegisters + Camera1394Impl::ISO_ENABLE,
        Cast::getAddress(quadlet),
        sizeof(quadlet)
      );
    }
    
    if (success) {
      copy<uint8>(
        Cast::pointer<uint8*>(frame.getElements()) + transmission.bytesPerPacket * (transmission.packetsPerFrame - 1),
        lastPacket,
        bytesInLastPacket
      );
    }
    
    return success;
  }

  bool Camera1394::AcquisitionListener::onAcquisitionLostSync(
    unsigned int frame) throw() {
    return true;
  }
  
  bool Camera1394::AcquisitionListener::onAcquisitionFailure(
    unsigned int frame) throw() {
    return true;
  }
  
  bool Camera1394::acquireContinuously(
    Array<FrameBuffer> frames,
    AcquisitionListener* listener
  ) throw(NotSupported, ImageException, Camera1394Exception, IEEE1394Exception) {
    if ((frames.getSize() == 0) || (listener == 0)) { // empty frame buffer or no listener
      return true; // nothing to do
    }
    
    Array<FrameBuffer>::Iterator first = frames.getBeginIterator();
    Array<FrameBuffer>::Iterator end = frames.getEndIterator();
    
    // check if frames are valid
    for (Array<FrameBuffer>::Iterator i = first; i < end; ++i) {
      bassert(
        i->getSize() == transmission.totalBytesPerFrame,
        bindCause(ImageException(this), Camera1394::FRAME_DIMENSION_MISMATCH)
      );
      bassert(
        i->getBuffer(),
        bindCause(ImageException(this), Camera1394::INVALID_FRAME_BUFFER)
      );
    }
    
    // inititialize requests
    PrimitiveArray<IEEE1394::Quadlet> headers(transmission.packetsPerFrame); // shared by frames
    Allocator<IEEE1394::IsochronousReadFixedDataRequest> requests(frames.getSize() * 2); // 2 requests per frame

    const Allocator<IEEE1394::IsochronousReadFixedDataRequest>::Iterator endRequest = requests.getEndIterator();
    PrimitiveArray<uint8> lastPacket(transmission.bytesPerPacket); // TAG: need one per frame
    
    const unsigned int bytesInLastPacket = transmission.totalBytesPerFrame -
      transmission.bytesPerPacket * (transmission.packetsPerFrame - 1);
    
    Allocator<IEEE1394::IsochronousReadFixedDataRequest>::Iterator request =
      requests.getBeginIterator();
    for (Array<FrameBuffer>::Iterator i = first; i < end; ++i) {
      IEEE1394::IsochronousReadFixedDataRequest firstRequest =
        readChannel.getReadFixedDataRequest();
      firstRequest.setSubchannel(transmission.subchannel);
      firstRequest.setNumberOfPackets(transmission.packetsPerFrame - 1);
      firstRequest.setHeaderSize(sizeof(IEEE1394::Quadlet));
      firstRequest.setPayload(transmission.bytesPerPacket);
      firstRequest.setBuffer(
        Cast::pointer<uint8*>(i->getBuffer()),
        transmission.bytesPerPacket * (transmission.packetsPerFrame - 1),
        Cast::getAddress(headers)
      );
      *request++ = firstRequest;
      
      IEEE1394::IsochronousReadFixedDataRequest lastRequest =
        readChannel.getReadFixedDataRequest();
      lastRequest.setSubchannel(transmission.subchannel);
      lastRequest.setNumberOfPackets(1);
      lastRequest.setHeaderSize(sizeof(IEEE1394::Quadlet));
      lastRequest.setPayload(transmission.bytesPerPacket /*bytesInLastPacket*/);
      lastRequest.setBuffer(
        Cast::getAddress(lastPacket), // Cast::pointer<char*>(frame.getElements() + transmission.bytesPerPacket * (transmission.packetsPerFrame - 1))
        bytesInLastPacket,
        Cast::getAddress(headers[transmission.packetsPerFrame - 1])
      );
      *request++ = lastRequest;
    }
    
    // queue all frame requests
    unsigned int pendingRequests = 0;
    request = requests.getBeginIterator();
    while (request != endRequest) { // TAG: need to cancel all if exception is raised
      readChannel.queue(*request++);
      ++pendingRequests;
      readChannel.queue(*request++);
      ++pendingRequests;
    }
    
    // enable continuous transmission
    IEEE1394::Quadlet quadlet = 0; // first disable continuous transmission as a precaution
    adapter.write(
      camera,
      commandRegisters + Camera1394Impl::ISO_ENABLE,
      Cast::getAddress(quadlet),
      sizeof(quadlet)
    );
    Thread::nanosleep(5000000); // wait a few milliseconds
    quadlet = 1 << 31; // enable continuous transmission
    adapter.write(
      camera,
      commandRegisters + Camera1394Impl::ISO_ENABLE,
      Cast::getAddress(quadlet),
      sizeof(quadlet)
    );
    
    bool success = true;
    bool stopped = false;
    const unsigned int numberOfFrames = frames.getSize();
    unsigned int frame = 0;
    
    request = requests.getBeginIterator();
    while (pendingRequests && !stopped) { // we need to dequeue until queue is empty
      // wait for completion of next frame (2 requests)
      unsigned int dequeuedRequests = readChannel.dequeue(2, 9999999); // TAG: choose nice timeout
      pendingRequests -= dequeuedRequests;
      if (dequeuedRequests < 2) {
        readChannel.cancel(); // cancel all pending requests
        readChannel.dequeue(pendingRequests, 999999999); // TAG: must wait forever
        success = false;
        break;
      }
      // TAG: if stopped then cancel - it is too slow to dequeue
      
      if (!stopped) {
        if (request == endRequest) { // queue 2 requests
          request = requests.getBeginIterator();
        }
        
        success = true;
        success &= (request[0].getStatus() == IEEE1394::COMPLETED); // first request
        success &= (request[1].getStatus() == IEEE1394::COMPLETED); // last request
        
        if (success && ((headers[0] & 1) == 0)) { // if sync is lost
          IEEE1394::Quadlet quadlet = 0; // disable continuous transmission
          adapter.write(
            camera,
            commandRegisters + Camera1394Impl::ISO_ENABLE,
            Cast::getAddress(quadlet),
            sizeof(quadlet)
          );
          
          if (!listener->onAcquisitionLostSync(frame)) {
            success = false;
            stopped = true;
          } else {
            // TAG: do we really need to dequeue every request
            // TAG: queue requests until beginning of frame is found?
            Thread::nanosleep(5000000); // wait a few milliseconds
            quadlet = 1 << 31; // enable continuous transmission
            adapter.write(
              camera,
              commandRegisters + Camera1394Impl::ISO_ENABLE,
              Cast::getAddress(quadlet),
              sizeof(quadlet)
            );
          }
        }
        
        if (success) {
          uint8* buffer = Cast::pointer<uint8*>(request->getBuffer());
          
          // copy last packet content
          copy<uint8>( // TAG: need better solution
            buffer + transmission.bytesPerPacket * (transmission.packetsPerFrame - 1),
            lastPacket,
            bytesInLastPacket
          );
          
          if (!listener->onAcquisition(frame, buffer)) {
            stopped = true;
          }
        } else {
          if (!listener->onAcquisitionFailure(frame)) {
            stopped = true;
          } else {
            success = true; // allowed to continue
          }
        }
      
        // put frame requests back into queue
        request->reset(); // prepare request
        readChannel.queue(*request++);
        ++pendingRequests;
        request->reset(); // prepare request
        readChannel.queue(*request++);
        ++pendingRequests;
      }

      ++frame %= numberOfFrames;      
    }
    
    quadlet = 0; // disable continuous transmission
    adapter.write(
      camera,
      commandRegisters + Camera1394Impl::ISO_ENABLE,
      Cast::getAddress(quadlet),
      sizeof(quadlet)
    );
    
    return success;
  }

  void Camera1394::convert(
    GrayImage& image,
    PixelFormat pixelFormat,
    const uint8* buffer) throw(ImageException) {
    GrayImage::Rows rowLookup = image.getRows();
    GrayImage::Rows::RowIterator row = rowLookup.getEnd();
    const GrayImage::Rows::RowIterator endRow = rowLookup.getFirst();
    
    switch (pixelFormat) {
    case Camera1394::Y_8BIT:
      while (row != endRow) {
        --row;
        GrayImage::Rows::RowIterator::ElementIterator column = row.getFirst();
        const GrayImage::Rows::RowIterator::ElementIterator endColumn =
          row.getEnd();
        while (column < endColumn) {
          *column++ = *buffer++;
        }
      }
      break;
    case Camera1394::RGB_8BIT:
      while (row != endRow) {
        --row;
        GrayImage::Rows::RowIterator::ElementIterator column = row.getFirst();
        const GrayImage::Rows::RowIterator::ElementIterator endColumn =
          row.getEnd();
        while (column < endColumn) {
          unsigned int red = *buffer++;
          unsigned int green = *buffer++;
          unsigned int blue = *buffer++;
          *column++ = (red + green + blue + 1)/3;
        }
      }
      break;
    case Camera1394::YUV_422_8BIT:
      {
        bassert(
          image.getWidth() % 2 == 0,
          ImageException(Type::getType<Camera1394>())
        );
        while (row != endRow) {
          --row;
          GrayImage::Rows::RowIterator::ElementIterator column = row.getFirst();
          const GrayImage::Rows::RowIterator::ElementIterator endColumn = row.getEnd();
          while (column < endColumn) {
            uint8 Cb = *buffer++;
            uint8 Y0 = *buffer++;
            uint8 Cr = *buffer++;
            uint8 Y1 = *buffer++;
            *column++ = Y0;
            *column++ = Y1;
          }
        }
      }
      break;
    case Camera1394::YUV_444_8BIT:
      while (row != endRow) {
        --row;
        GrayImage::Rows::RowIterator::ElementIterator column = row.getFirst();
        const GrayImage::Rows::RowIterator::ElementIterator endColumn =
          row.getEnd();
        while (column < endColumn) {
          uint8 Cb = *buffer++;
          uint8 Y = *buffer++;
          uint8 Cr = *buffer++;
          *column++ = Y;
        }
      }
      break;
    default:
      throw ImageException(Type::getType<Camera1394>());
    }
  }

  void Camera1394::convert(
    ColorImage& image,
    PixelFormat pixelFormat,
    const uint8* buffer) throw(ImageException) {
    ColorImage::Rows rowLookup = image.getRows();
    ColorImage::Rows::RowIterator row = rowLookup.getEnd();
    const ColorImage::Rows::RowIterator endRow = rowLookup.getFirst();

    switch (pixelFormat) {
    case Camera1394::Y_8BIT:
      while (row != endRow) {
        --row;
        ColorImage::Rows::RowIterator::ElementIterator column = row.getFirst();
        const ColorImage::Rows::RowIterator::ElementIterator endColumn = row.getEnd();
        while (column < endColumn) {
          uint8 Y = *buffer++;
          *column++ = makeColorPixel(Y, Y, Y);
        }
      }
      break;
    case Camera1394::RGB_8BIT:
      while (row != endRow) {
        --row;
        ColorImage::Rows::RowIterator::ElementIterator column = row.getFirst();
        const ColorImage::Rows::RowIterator::ElementIterator endColumn = row.getEnd();
        while (column < endColumn) {
          uint8 red = *buffer++;
          uint8 green = *buffer++;
          uint8 blue = *buffer++;
          *column++ = makeColorPixel(red, green, blue);
        }
      }
      break;
    case Camera1394::RGB_16BIT:
      while (row != endRow) {
        --row;
        ColorImage::Rows::RowIterator::ElementIterator column = row.getFirst();
        const ColorImage::Rows::RowIterator::ElementIterator endColumn = row.getEnd();
        while (column < endColumn) {
          ++buffer; // skip MSB
          uint8 red = *buffer++;
          ++buffer; // skip MSB
          uint8 green = *buffer++;
          ++buffer; // skip MSB
          uint8 blue = *buffer++;
          *column++ = makeColorPixel(red, green, blue);
        }
      }
      break;
    case Camera1394::YUV_422_8BIT:
      {
        // TAG: use FIR filter
        bassert(
          image.getWidth() % 2 == 0,
          ImageException(Type::getType<Camera1394>())
        );
        while (row != endRow) {
          --row;
          ColorImage::Rows::RowIterator::ElementIterator column = row.getFirst();
          const ColorImage::Rows::RowIterator::ElementIterator endColumn = row.getEnd();
          while (column < endColumn) {
            uint8 Cb = *buffer++;
            uint8 Y0 = *buffer++;
            uint8 Cr = *buffer++;
            uint8 Y1 = *buffer++;
            *column++ = YCbCrToRGB(makeYCbCrPixel(Y0, Cb, Cr));
            *column++ = YCbCrToRGB(makeYCbCrPixel(Y1, Cb, Cr));
          }
        }
      }
      break;
    case Camera1394::YUV_444_8BIT:
      while (row != endRow) {
        --row;
        ColorImage::Rows::RowIterator::ElementIterator column = row.getFirst();
        const ColorImage::Rows::RowIterator::ElementIterator endColumn = row.getEnd();
        while (column < endColumn) {
          uint8 Cb = *buffer++;
          uint8 Y = *buffer++;
          uint8 Cr = *buffer++;
          *column++ = YCbCrToRGB(makeYCbCrPixel(Y, Cb, Cr));
        }
      }
      break;
    default:
      throw ImageException(Type::getType<Camera1394>()); 
    }
  }
  
}; // end of gip namespace
