/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    Copyright (C) 2002 by Rene Moeller Fonseca <fonseca@mip.sdu.dk>

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
    
    struct BusInfo {
      BigEndian<uint32> name; // "1394"
      BigEndian<uint32> flags;
      uint8 guid[8];
    } _DK_SDU_MIP__BASE__PACKED busInfo;
    
    struct RootDirectory {
      BigEndian<uint32> crc;
      BigEndian<uint32> vendorId; // 0x03 in MSB
      BigEndian<uint32> flags; // 0x0c in MSB
      BigEndian<uint32> nodeOffset; // 0x8d in MSB
      BigEndian<uint32> deviceDirectoryOffset; // 0xd1 in MSB
    } _DK_SDU_MIP__BASE__PACKED rootDirectory;
    
    struct DeviceIndependentDirectory {
      BigEndian<uint32> crc;
      BigEndian<uint32> specification; // 0x12 in MSB
      BigEndian<uint32> version; // 0x13 in MSB
      BigEndian<uint32> dependentOffset; // 0xd4 in MSB
    } _DK_SDU_MIP__BASE__PACKED;
    
    struct DeviceDependentDirectory {
      BigEndian<uint16> size;
      BigEndian<uint16> crc;
      BigEndian<uint32> commandRegisters; // 0x40 in MSB
      BigEndian<uint32> vendorNameLeaf; // 0x81 in MSB
      BigEndian<uint32> modelNameLeaf; // 0x82 in MSB
    } _DK_SDU_MIP__BASE__PACKED;
    
    struct ConfigurationIntro {
      BigEndian<uint32> crc;
      BusInfo busInfo;
      RootDirectory rootDirectory;
    } _DK_SDU_MIP__BASE__PACKED;
    
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
    } _DK_SDU_MIP__BASE__PACKED;
    
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
    } _DK_SDU_MIP__BASE__PACKED;

    struct CommonFeatureControl {
      unsigned int reserved1 : 24;
      bool automaticMode : 1;
      bool enabled : 1;
      bool autoAdjustmentMode : 1;
      unsigned int reserved0 : 3;
      bool absoluteControl : 1;
      bool presence : 1;
    } _DK_SDU_MIP__BASE__PACKED;
    
    struct FeatureControl {
      unsigned int value : 12;
      unsigned int reserved1 : 12;
      bool automaticMode : 1;
      bool enabled : 1;
      bool autoAdjustmentMode : 1;
      unsigned int reserved0 : 3;
      bool absoluteControl : 1;
      bool presence : 1;
    } _DK_SDU_MIP__BASE__PACKED;
    
    struct WhiteBalanceFeatureControl {
      unsigned int redRatio : 12;
      unsigned int blueRatio : 12;
      bool automaticMode : 1;
      bool enabled : 1;
      bool autoAdjustmentMode : 1;
      unsigned int reserved0 : 3;
      bool absoluteControl : 1;
      bool presence : 1;
    } _DK_SDU_MIP__BASE__PACKED;
    
    struct TemperatureFeatureControl {
      unsigned int currentValue : 12;
      unsigned int targetValue : 12;
      bool automaticMode : 1;
      bool enabled : 1;
      bool autoAdjustmentMode : 1;
      unsigned int reserved0 : 3;
      bool absoluteControl : 1;
      bool presence : 1;
    } _DK_SDU_MIP__BASE__PACKED;
    
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
    } _DK_SDU_MIP__BASE__PACKED;

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
      Camera1394Impl::FeatureInquery feature = Cast::impersonate<Camera1394Impl::FeatureInquery, uint32>(quadlet);
      descriptor.available = feature.presence;
      descriptor.autoAdjustmentMode = feature.onePush;
      descriptor.readable = feature.readable;
      descriptor.switchable = feature.switchable;
      descriptor.automaticMode = feature.autoMode;
      descriptor.manualMode = feature.manualMode;
      descriptor.minimum = feature.minimumValue;
      descriptor.maximum = feature.maximumValue;
    }

  }; // end of namespace Camera1394Impl

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
    static const StringLiteral FORMATS[] = {
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
    static const StringLiteral MODES[] = {
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
    adapter.write(camera, commandRegisters + Camera1394Impl::INITIALIZE, getCharAddress(buffer), sizeof(buffer));
    // TAG: could be set to unsupported mode
    // TAG: could be set to unsupported pixel format
    readModeSpecificState();
  }

  Camera1394::Camera1394() throw() {
    adapter.open(); // TAG: temporary fix
  }
  
  bool Camera1394::isCamera(unsigned int node) throw(OutOfDomain, IEEE1394Exception) {
    assert(node < IEEE1394::BROADCAST, OutOfDomain(this));
    try {
      Camera1394Impl::ConfigurationIntro config;
      adapter.read(node, IEEE1394::CSR_BASE_ADDRESS + IEEE1394::CONFIGURATION_ROM, getCharAddress(config), sizeof(config));
      
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
        adapter.read(node,
                     IEEE1394::CSR_BASE_ADDRESS + deviceIndependentDirectoryOffset,
                     getCharAddress(deviceIndependentDirectory),
                     sizeof(deviceIndependentDirectory));
        if ((deviceIndependentDirectory.specification == 0x1200a02d) && // (ID for 1394TA)
            ((deviceIndependentDirectory.version & 0xff000000) == 0x13000000) &&
            ((deviceIndependentDirectory.dependentOffset & 0xff000000) == 0xd4000000)) {
          return true;
        }
      }
    } catch (IOException& e) {
      // continue with next node
    }
    return false;
  }
  
  bool Camera1394::isCamera(const EUI64& guid) throw(Camera1394Exception, IEEE1394Exception) {
    int node = adapter.getPhysicalId(guid);
    assert(node >= 0, bindCause(Camera1394Exception("Device not found", this), IEEE1394::NODE_NOT_PRESENT));
    return isCamera(node);
  }

  Array<EUI64> Camera1394::getCameras() throw(IEEE1394Exception) {
    Array<EUI64> cameras;
    
    uint64 presentNodes = adapter.getPresentNodes();
    
    for (unsigned int node = 0; presentNodes && (node < IEEE1394::BROADCAST); ++node) {
      if ((presentNodes & (1ULL << node)) == 0) {
        continue;
      }
      presentNodes &= ~(1ULL << node);
      
      try {
        Camera1394Impl::ConfigurationIntro config;
        adapter.read(node, IEEE1394::CSR_BASE_ADDRESS + IEEE1394::CONFIGURATION_ROM, getCharAddress(config), sizeof(config));
        
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
          adapter.read(node,
                       IEEE1394::CSR_BASE_ADDRESS + deviceIndependentDirectoryOffset,
                       getCharAddress(deviceIndependentDirectory),
                       sizeof(deviceIndependentDirectory));

          if ((deviceIndependentDirectory.specification == 0x1200a02d) && // (ID for 1394TA)
              ((deviceIndependentDirectory.version & 0xff000000) == 0x13000000) &&
              ((deviceIndependentDirectory.dependentOffset & 0xff000000) == 0xd4000000)) {
            cameras.append(EUI64(config.busInfo.guid));
          }
        }
      } catch (IOException& e) {
        // continue with next node
      }
    }
    return cameras;
  }

  void Camera1394::open(const EUI64& guid) throw(Camera1394Exception, IEEE1394Exception) {
    int node = adapter.getPhysicalId(guid);
    assert(node >= 0, bindCause(Camera1394Exception("Device not found", this), IEEE1394::NODE_NOT_PRESENT));
    assert(isCamera(node), bindCause(Camera1394Exception("Not a camera", this), Camera1394::NOT_A_CAMERA));
    
    Camera1394Impl::ConfigurationIntro config;
    adapter.read(node, IEEE1394::CSR_BASE_ADDRESS + IEEE1394::CONFIGURATION_ROM, getCharAddress(config), sizeof(config));
    
    assert(
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
    adapter.read(node,
                 IEEE1394::CSR_BASE_ADDRESS + deviceIndependentDirectoryOffset,
                 getCharAddress(deviceIndependentDirectory),
                 sizeof(deviceIndependentDirectory));
    
    assert(
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
    adapter.read(node,
                 IEEE1394::CSR_BASE_ADDRESS + deviceDependentDirectoryOffset,
                 getCharAddress(deviceDependentDirectory),
                 sizeof(deviceDependentDirectory));
    
    assert(
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
    
    adapter.read(node, IEEE1394::CSR_BASE_ADDRESS + vendorNameOffset, getCharAddress(quadlet), sizeof(quadlet));
    unsigned int vendorLeafSize = quadlet >> 16;
    ASSERT(vendorLeafSize >= 2);
    if (vendorLeafSize > 2) {
      char leaf[vendorLeafSize * sizeof(IEEE1394::Quadlet)];
      adapter.read(node,
                   IEEE1394::CSR_BASE_ADDRESS + vendorNameOffset + sizeof(IEEE1394::Quadlet),
                   leaf,
                   vendorLeafSize * sizeof(IEEE1394::Quadlet));
      vendorName = String(leaf + 2 * sizeof(IEEE1394::Quadlet), (vendorLeafSize - 2) * sizeof(IEEE1394::Quadlet));
    }

    const unsigned int modelNameOffset =
      (deviceDependentDirectory.modelNameLeaf & 0x00ffffff) * sizeof(IEEE1394::Quadlet) +
      getFieldOffset(&Camera1394Impl::DeviceDependentDirectory::modelNameLeaf) +
      deviceDependentDirectoryOffset;
    
    adapter.read(node, IEEE1394::CSR_BASE_ADDRESS + modelNameOffset, getCharAddress(quadlet), sizeof(quadlet));
    unsigned int modelLeafSize = quadlet >> 16;
    ASSERT(modelLeafSize >= 2);
    if (modelLeafSize > 2) {
      char leaf[modelLeafSize * sizeof(IEEE1394::Quadlet)];
      adapter.read(node, IEEE1394::CSR_BASE_ADDRESS + modelNameOffset + sizeof(IEEE1394::Quadlet), leaf, modelLeafSize * sizeof(IEEE1394::Quadlet));
      modelName = String(leaf + 2 * sizeof(IEEE1394::Quadlet), (modelLeafSize - 2) * sizeof(IEEE1394::Quadlet));
    }
    
    // get supported formats
    adapter.read(node, commandRegisters + Camera1394Impl::V_FORMAT_INQ, getCharAddress(quadlet), sizeof(quadlet));
    formats = Math::getBitReversal(quadlet);

    // get supported modes
    {
      IEEE1394::Quadlet buffer[8];
      for (unsigned int i = 0; i < 8; ++i) {
        buffer[i] = 0;
        if (isFormatSupported(static_cast<Format>(i))) {
          adapter.read(node, commandRegisters + Camera1394Impl::V_MODE_INQ_0 + i, getCharAddress(buffer[i]), sizeof(buffer[i]));
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
            getCharAddress(quadlet),
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
        if (!supportedModes[mode]) {
          continue;
        }
        adapter.read(node, commandRegisters + Camera1394Impl::V_CSR_INQ_7_0 + i * sizeof(IEEE1394::Quadlet), getCharAddress(quadlet), sizeof(quadlet));
        partialImageModeOffset[i] = quadlet * sizeof(IEEE1394::Quadlet); // check for overflow
        
        if (supportedModes[PARTIAL_IMAGE_MODE_0 + i]) { // partial image modes  are guaranteed to be consecutive
          IEEE1394::Quadlet maximumImageSize;
          adapter.read(node, IEEE1394::CSR_BASE_ADDRESS + partialImageModeOffset[i] + Camera1394Impl::MAX_IMAGE_SIZE_INQ, getCharAddress(maximumImageSize), sizeof(maximumImageSize));
          partialImageMode[i].maximumDimension = Dimension(maximumImageSize >> 16, maximumImageSize & 0xffff);
          
          IEEE1394::Quadlet unitSize;
          adapter.read(node, IEEE1394::CSR_BASE_ADDRESS + partialImageModeOffset[i] + Camera1394Impl::UNIT_SIZE_INQ, getCharAddress(unitSize), sizeof(unitSize));
          unsigned int unitWidth = unitSize >> 16;
          unsigned int unitHeight = unitSize & 0xffff;
          
          IEEE1394::Quadlet colorCodingInquery;
          adapter.read(node, IEEE1394::CSR_BASE_ADDRESS + partialImageModeOffset[i] + Camera1394Impl::COLOR_CODING_INQ, getCharAddress(colorCodingInquery), sizeof(colorCodingInquery));
          
          unsigned int unitHorizontalOffset = 0;
          unsigned int unitVerticalOffset = 0;
          if (specification >= Camera1394::SPECIFICATION_1_30) {
            IEEE1394::Quadlet unitOffsetInquery;
            adapter.read(node, IEEE1394::CSR_BASE_ADDRESS + partialImageModeOffset[i] + Camera1394Impl::UNIT_POSITION_INQ, getCharAddress(unitOffsetInquery), sizeof(unitOffsetInquery));
            unitHorizontalOffset = unitOffsetInquery >> 16;
            unitVerticalOffset = unitOffsetInquery & 0xffff;
            
            //IEEE1394::Quadlet valueSetting;
            //adapter.read(node, IEEE1394::CSR_BASE_ADDRESS + partialImageModeOffset[i] + Camera1394Impl::VALUE_SETTING, getCharAddress(valueSetting), sizeof(valueSetting));
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
    adapter.read(node, commandRegisters + Camera1394Impl::BASIC_FUNC_INQ, getCharAddress(quadlet), sizeof(quadlet));
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
      adapter.read(camera, commandRegisters + Camera1394Impl::CURRENT_V_MODE, getCharAddress(quadlet), sizeof(quadlet));
      unsigned int mode = quadlet >> 29;
      adapter.read(camera, commandRegisters + Camera1394Impl::CURRENT_V_FORMAT, getCharAddress(quadlet), sizeof(quadlet));
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

      static const int NUMBER_OF_MODES[] = {
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
fout << "currentMode: " << currentMode << ENDL;
    }

Trace::message("23");
    // TAG: not for revision for format 6 and partial image format
    adapter.read(camera, commandRegisters + Camera1394Impl::CURRENT_V_RATE, getCharAddress(quadlet), sizeof(quadlet));
    frameRate = static_cast<FrameRate>(quadlet >> 29);
Trace::message("24");
    
    const Camera1394Impl::ModeInformation& info = Camera1394Impl::MODE_INFORMATION[currentMode];
    if (info.format == Camera1394::PARTIAL_IMAGE) {
Trace::message("24b");
      IEEE1394::Quadlet imageOffset;
      adapter.read(camera, IEEE1394::CSR_BASE_ADDRESS + partialImageModeOffset[info.mode] + Camera1394Impl::IMAGE_POSITION, getCharAddress(imageOffset), sizeof(imageOffset));
Trace::message("24b1");
      IEEE1394::Quadlet imageDimension;
      adapter.read(camera, IEEE1394::CSR_BASE_ADDRESS + partialImageModeOffset[info.mode] + Camera1394Impl::IMAGE_SIZE, getCharAddress(imageDimension), sizeof(imageDimension));
      IEEE1394::Quadlet colorCoding;
      adapter.read(camera, IEEE1394::CSR_BASE_ADDRESS + partialImageModeOffset[info.mode] + Camera1394Impl::COLOR_CODING_ID, getCharAddress(colorCoding), sizeof(colorCoding));
Trace::message("24b2");
      IEEE1394::Quadlet pixelsPerFrameInquery;
      adapter.read(camera, IEEE1394::CSR_BASE_ADDRESS + partialImageModeOffset[info.mode] + Camera1394Impl::PIXEL_NUMBER_INQ, getCharAddress(pixelsPerFrameInquery), sizeof(pixelsPerFrameInquery));
Trace::message("24b3");
      BigEndian<uint64> totalBytesPerFrameInquery;
      adapter.read(camera, IEEE1394::CSR_BASE_ADDRESS + partialImageModeOffset[info.mode] + Camera1394Impl::TOTAL_BYTES_HI_INQ, getCharAddress(totalBytesPerFrameInquery), sizeof(totalBytesPerFrameInquery));
Trace::message("24b4");
      IEEE1394::Quadlet packetParaInquery;
      adapter.read(camera, IEEE1394::CSR_BASE_ADDRESS + partialImageModeOffset[info.mode] + Camera1394Impl::PACKET_PARA_INQ, getCharAddress(packetParaInquery), sizeof(packetParaInquery));
Trace::message("24b5");
      IEEE1394::Quadlet bytesPerPacket;
      adapter.read(camera, IEEE1394::CSR_BASE_ADDRESS + partialImageModeOffset[info.mode] + Camera1394Impl::BYTE_PER_PACKET, getCharAddress(bytesPerPacket), sizeof(bytesPerPacket));
Trace::message("24b6");
      
      region.setOffset(Point2D(imageOffset & 0xffff, imageOffset >> 16));
      region.setDimension(Dimension(imageDimension >> 16, imageDimension & 0xffff));
Trace::message("25");

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
Trace::message("26");

      // TAG: temporary fix
      bytesPerPacket = transmission.bytesPerPacket << 16;
      adapter.write(camera, IEEE1394::CSR_BASE_ADDRESS + partialImageModeOffset[info.mode] + Camera1394Impl::BYTE_PER_PACKET, getCharAddress(bytesPerPacket), sizeof(bytesPerPacket));

      // packetsPerFrameInquery is updated when bytesPerPacket is written
      IEEE1394::Quadlet packetsPerFrameInquery;
      adapter.read(camera, IEEE1394::CSR_BASE_ADDRESS + partialImageModeOffset[info.mode] + Camera1394Impl::PACKET_PER_FRAME_INQ, getCharAddress(packetsPerFrameInquery), sizeof(packetsPerFrameInquery));
      transmission.packetsPerFrame = packetsPerFrameInquery;
      
      // TAG: what about value settings
      
Trace::message("27");
      
      static const PixelFormat PIXEL_FORMATS[] = {
        Y_8BIT, YUV_411_8BIT, YUV_422_8BIT, YUV_444_8BIT, RGB_8BIT, Y_16BIT, RGB_16BIT
      };
      
      if (colorCoding < getArraySize(PIXEL_FORMATS)) {
        pixelFormat = PIXEL_FORMATS[colorCoding];
      } else {
        // TAG: now what
      }
      
    } else {
      Trace::message("24c");
      
      ASSERT((info.pixelFormat >= 0) && info.width && info.height);
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
Trace::message("28");
    
    // get supported features
    {
      features = 0;
      
      adapter.read(camera, commandRegisters + Camera1394Impl::FEATURE_HI_INQ, getCharAddress(quadlet), sizeof(quadlet));
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

      adapter.read(camera, commandRegisters + Camera1394Impl::FEATURE_LO_INQ, getCharAddress(quadlet), sizeof(quadlet));
      features |= (quadlet & (1 << 31)) ? (1 << Camera1394::ZOOM_CONTROL) : 0;
      features |= (quadlet & (1 << 30)) ? (1 << Camera1394::PAN_CONTROL) : 0;
      features |= (quadlet & (1 << 29)) ? (1 << Camera1394::TILT_CONTROL) : 0;
      features |= (quadlet & (1 << 28)) ? (1 << Camera1394::OPTICAL_FILTER_CONTROL) : 0;
      features |= (quadlet & (1 << 27)) ? (1 << Camera1394::CAPTURE_SIZE) : 0;
      features |= (quadlet & (1 << 26)) ? (1 << Camera1394::CAPTURE_QUALITY) : 0;

      advancedFeatureAddress = 0;
      if (capabilities & Camera1394::ADVANCED_FEATURES) {
        adapter.read(camera, commandRegisters + Camera1394Impl::ADVANCED_FEATURE_INQ, getCharAddress(quadlet), sizeof(quadlet));
        advancedFeatureAddress = commandRegisters + quadlet * sizeof(IEEE1394::Quadlet); // TAG: is this allowed to be changed per mode
      }
    }

    fill<char>(getCharAddress(featureDescriptors), sizeof(featureDescriptors), 0);
    if (isFeatureSupported(Camera1394::BRIGHTNESS_CONTROL)) {
      adapter.read(camera, commandRegisters + Camera1394Impl::BRIGHTNESS_INQ, getCharAddress(quadlet), sizeof(quadlet));
      Camera1394Impl::importGenericFeature(quadlet, featureDescriptors.brightness);
    }
    if (isFeatureSupported(Camera1394::AUTO_EXPOSURE_CONTROL)) {
      adapter.read(camera, commandRegisters + Camera1394Impl::AUTO_EXPOSURE_INQ, getCharAddress(quadlet), sizeof(quadlet));
      Camera1394Impl::importGenericFeature(quadlet, featureDescriptors.autoExposure);
    }
    if (isFeatureSupported(Camera1394::SHARPNESS_CONTROL)) {
      adapter.read(camera, commandRegisters + Camera1394Impl::SHARPNESS_INQ, getCharAddress(quadlet), sizeof(quadlet));
      Camera1394Impl::importGenericFeature(quadlet, featureDescriptors.sharpness);
    }
    if (isFeatureSupported(Camera1394::WHITE_BALANCE_CONTROL)) {
      adapter.read(camera, commandRegisters + Camera1394Impl::WHITE_BALANCE_INQ, getCharAddress(quadlet), sizeof(quadlet));
      Camera1394Impl::importGenericFeature(quadlet, featureDescriptors.whiteBalance);
    }
    if (isFeatureSupported(Camera1394::HUE_CONTROL)) {
      adapter.read(camera, commandRegisters + Camera1394Impl::HUE_INQ, getCharAddress(quadlet), sizeof(quadlet));
      Camera1394Impl::importGenericFeature(quadlet, featureDescriptors.hue);
    }
    if (isFeatureSupported(Camera1394::SATURATION_CONTROL)) {
      adapter.read(camera, commandRegisters + Camera1394Impl::SATURATION_INQ, getCharAddress(quadlet), sizeof(quadlet));
      Camera1394Impl::importGenericFeature(quadlet, featureDescriptors.saturation);
    }
    if (isFeatureSupported(Camera1394::GAMMA_CONTROL)) {
      adapter.read(camera, commandRegisters + Camera1394Impl::GAMMA_INQ, getCharAddress(quadlet), sizeof(quadlet));
      Camera1394Impl::importGenericFeature(quadlet, featureDescriptors.gamma);
    }
   if (isFeatureSupported(Camera1394::SHUTTER_CONTROL)) {
      adapter.read(camera, commandRegisters + Camera1394Impl::SHUTTER_INQ, getCharAddress(quadlet), sizeof(quadlet));
      Camera1394Impl::importGenericFeature(quadlet, featureDescriptors.shutter);
    }
    if (isFeatureSupported(Camera1394::GAIN_CONTROL)) {
      adapter.read(camera, commandRegisters + Camera1394Impl::GAIN_INQ, getCharAddress(quadlet), sizeof(quadlet));
      Camera1394Impl::importGenericFeature(quadlet, featureDescriptors.gain);
    }
    if (isFeatureSupported(Camera1394::IRIS_CONTROL)) {
      adapter.read(camera, commandRegisters + Camera1394Impl::IRIS_INQ, getCharAddress(quadlet), sizeof(quadlet));
      Camera1394Impl::importGenericFeature(quadlet, featureDescriptors.iris);
    }
    if (isFeatureSupported(Camera1394::FOCUS_CONTROL)) {
      adapter.read(camera, commandRegisters + Camera1394Impl::FOCUS_INQ, getCharAddress(quadlet), sizeof(quadlet));
      Camera1394Impl::importGenericFeature(quadlet, featureDescriptors.focus);
    }
    if (isFeatureSupported(Camera1394::TEMPERATURE_CONTROL)) {
      adapter.read(camera, commandRegisters + Camera1394Impl::TEMPERATURE_INQ, getCharAddress(quadlet), sizeof(quadlet));
      Camera1394Impl::importGenericFeature(quadlet, featureDescriptors.temperature);
    }
    if (isFeatureSupported(Camera1394::TRIGGER_CONTROL)) {
      adapter.read(camera, commandRegisters + Camera1394Impl::TRIGGER_INQ, getCharAddress(quadlet), sizeof(quadlet));
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
      adapter.read(camera, commandRegisters + Camera1394Impl::ZOOM_INQ, getCharAddress(quadlet), sizeof(quadlet));
      Camera1394Impl::importGenericFeature(quadlet, featureDescriptors.zoom);
    }
    if (isFeatureSupported(Camera1394::PAN_CONTROL)) {
      adapter.read(camera, commandRegisters + Camera1394Impl::PAN_INQ, getCharAddress(quadlet), sizeof(quadlet));
      Camera1394Impl::importGenericFeature(quadlet, featureDescriptors.pan);
    }
    if (isFeatureSupported(Camera1394::TILT_CONTROL)) {
      adapter.read(camera, commandRegisters + Camera1394Impl::TILT_INQ, getCharAddress(quadlet), sizeof(quadlet));
      Camera1394Impl::importGenericFeature(quadlet, featureDescriptors.tilt);
    }
    if (isFeatureSupported(Camera1394::OPTICAL_FILTER_CONTROL)) {
      adapter.read(camera, commandRegisters + Camera1394Impl::OPTICAL_FILTER_INQ, getCharAddress(quadlet), sizeof(quadlet));
      Camera1394Impl::importGenericFeature(quadlet, featureDescriptors.opticalFilter);
    }
    if (isFeatureSupported(Camera1394::CAPTURE_SIZE)) {
      adapter.read(camera, commandRegisters + Camera1394Impl::CAPTURE_SIZE_INQ, getCharAddress(quadlet), sizeof(quadlet));
      Camera1394Impl::importGenericFeature(quadlet, featureDescriptors.captureSize);
    }
    if (isFeatureSupported(Camera1394::CAPTURE_QUALITY)) {
      adapter.read(camera, commandRegisters + Camera1394Impl::CAPTURE_QUALITY_INQ, getCharAddress(quadlet), sizeof(quadlet));
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
    assert(isModeSupported(mode), bindCause(NotSupported(this), Camera1394::MODE_NOT_SUPPORTED));

    const Camera1394Impl::ModeInformation& info = Camera1394Impl::MODE_INFORMATION[mode];
    
    region.setOffset(Point2D(0, 0)); // TAG: also set register if partial image mode (or should we get)
    
    if (info.format == Camera1394::PARTIAL_IMAGE) {
      region.setDimension(partialImageMode[info.mode].maximumDimension);
    } else {
      ASSERT(info.width && info.height);
      region.setDimension(Dimension(info.width, info.height));
    }
    // TAG: also set register if partial image mode (or should we get)

    IEEE1394::Quadlet quadlet;
    quadlet = 0 << 31; // disable continuous
    adapter.write(camera, commandRegisters + Camera1394Impl::ISO_ENABLE, getCharAddress(quadlet), sizeof(quadlet));
    quadlet = (0 << 31) | (0 << 30) | 0; // disable finite shots
    adapter.write(camera, commandRegisters + Camera1394Impl::FINITE_SHOTS, getCharAddress(quadlet), sizeof(quadlet));

    // set mode
    quadlet = frameRate << 29;
    adapter.write(camera, commandRegisters + Camera1394Impl::CURRENT_V_RATE, getCharAddress(quadlet), sizeof(quadlet));
    quadlet = Camera1394Impl::MODE_INFORMATION[mode].mode << 29;
    adapter.write(camera, commandRegisters + Camera1394Impl::CURRENT_V_MODE, getCharAddress(quadlet), sizeof(quadlet));
    quadlet = Camera1394Impl::MODE_INFORMATION[mode].format << 29;
    adapter.write(camera, commandRegisters + Camera1394Impl::CURRENT_V_FORMAT, getCharAddress(quadlet), sizeof(quadlet));
    quadlet = (transmission.subchannel << 28) | (transmission.speed << 24);
    adapter.write(camera, commandRegisters + Camera1394Impl::ISO_CHANNEL, getCharAddress(quadlet), sizeof(quadlet));
    
    readModeSpecificState();
    
    readChannel = adapter.getReadChannel(transmission.packetsPerFrame, Camera1394::ISOCHRONOUS_SUBCHANNELS);
    fout << MESSAGE("Isochronous channels: ") << HEX << setWidth(10) << ZEROPAD << readChannel.getSubchannels() << ENDL;
  }
  
  unsigned int Camera1394::getFrameRates(Mode mode) throw(NotSupported) {
    assert(isModeSupported(mode), bindCause(NotSupported(this), Camera1394::MODE_NOT_SUPPORTED));
    return frameRates[mode];
  }

  void Camera1394::setFrameRate(FrameRate frameRate) throw(NotSupported) {
    assert(frameRates[currentMode] & (1 << frameRate), bindCause(NotSupported(this), Camera1394::FRAME_RATE_NOT_SUPPORTED));
    this->frameRate = frameRate;
  }

  void Camera1394::enable() throw(IEEE1394Exception) {
    IEEE1394::Quadlet quadlet;
    quadlet = 1 << 31;
    adapter.write(camera, commandRegisters + Camera1394Impl::POWER, getCharAddress(quadlet), sizeof(quadlet));
  }
  
  void Camera1394::disable() throw(IEEE1394Exception) {
    IEEE1394::Quadlet quadlet;
    quadlet = 0 << 31;
    adapter.write(camera, commandRegisters + Camera1394Impl::POWER, getCharAddress(quadlet), sizeof(quadlet));
  }
  
  bool Camera1394::isUpAndRunning() const throw(IEEE1394Exception) {
    if ((capabilities & POWER_CONTROL) == 0) { // TAG: check if this is ok
      return true;
    }
    IEEE1394::Quadlet quadlet;
    adapter.read(camera, commandRegisters + Camera1394Impl::POWER, getCharAddress(quadlet), sizeof(quadlet));
    return quadlet >> 31;
  }
    
  bool Camera1394::getFeatureStatus(Feature feature) throw(IEEE1394Exception) {
    static const unsigned int BIT[] = {
      63-0, 63-1, 63-2, 63-3, 63-4, 63-5, 63-6, 63-7, 63-8, 63-9, 63-10, 63-11, 63-12, 63-32, 63-33, 63-34, 63-35, 63-48, 63-49
    };
    ASSERT(features < getArraySize(BIT));
    BigEndian<uint64> status;
    adapter.read(camera, commandRegisters + Camera1394Impl::FEATURE_ERROR_STATUS_HIGH, getCharAddress(status), sizeof(status));
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
    
    assert(available, bindCause(NotSupported(this), Camera1394::FEATURE_NOT_SUPPORTED));
    IEEE1394::Quadlet quadlet;
    adapter.read(camera, commandRegisters + Camera1394Impl::FEATURE_CONTROL_REGISTER[feature], getCharAddress(quadlet), sizeof(quadlet));
    
    switch (feature) {
    case TRIGGER_CONTROL:
      {
        Camera1394Impl::TriggerFeatureControl control = Cast::impersonate<Camera1394Impl::TriggerFeatureControl, uint32>(quadlet);
        return control.enabled ? Camera1394::MANUAL : Camera1394::DISABLED;
      }
    default:
      {
        // includes white balance and temperature features
        Camera1394Impl::FeatureControl control = Cast::impersonate<Camera1394Impl::FeatureControl, uint32>(quadlet);
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
    
    assert(
      available &&
      ((operatingMode != Camera1394::DISABLED) || switchable && (operatingMode == Camera1394::DISABLED)) &&
      ((operatingMode != Camera1394::AUTOMATIC) || switchable && (operatingMode == Camera1394::AUTOMATIC)) &&
      ((operatingMode != Camera1394::AUTO_ADJUST) || switchable && (operatingMode == Camera1394::AUTO_ADJUST)) &&
      ((operatingMode != Camera1394::MANUAL) || manualMode && (operatingMode == Camera1394::MANUAL)),
      bindCause(NotSupported(this), Camera1394::FEATURE_OPERATING_MODE_NOT_SUPPORTED)
    );

    IEEE1394::Quadlet original;
    adapter.read(camera, commandRegisters + Camera1394Impl::FEATURE_CONTROL_REGISTER[feature], getCharAddress(original), sizeof(original));
    
    IEEE1394::Quadlet quadlet;
    switch (feature) {
    case TRIGGER_CONTROL:
      {
        Camera1394Impl::TriggerFeatureControl control = Cast::impersonate<Camera1394Impl::TriggerFeatureControl, uint32>(original);
        switch (operatingMode) {
        case Camera1394::DISABLED:
          control.enabled = false;
          break;
        case Camera1394::MANUAL:
          control.enabled = true;
          break;
        }
        control.absoluteControl = false;
        quadlet = Cast::impersonate<uint32>(control);
      }
      break;
    default:
      {
        // includes white balance and temperature features
        Camera1394Impl::CommonFeatureControl control = Cast::impersonate<Camera1394Impl::CommonFeatureControl, uint32>(original);
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
        quadlet = Cast::impersonate<uint32>(control);
      }
    }
    
    adapter.write(camera, commandRegisters + Camera1394Impl::FEATURE_CONTROL_REGISTER[feature], getCharAddress(quadlet), sizeof(quadlet));
    
    if (!getFeatureStatus(feature)) { // check if error or warning
      adapter.write(camera, commandRegisters + Camera1394Impl::FEATURE_CONTROL_REGISTER[feature], getCharAddress(original), sizeof(original)); // try to restore original value
    }
  }
  
  void Camera1394::setGenericFeature(Feature feature, const GenericFeatureDescriptor& descriptor, int value) throw(NotSupported, OutOfRange, IEEE1394Exception) {
    assert(descriptor.available, bindCause(NotSupported(this), Camera1394::FEATURE_NOT_SUPPORTED));
    assert((value >= descriptor.minimum) && (value <= descriptor.maximum), OutOfDomain(this));
    // TAG: mode must be manual
    ASSERT(feature < getArraySize(feature));
    uint64 featureRegister = commandRegisters + Camera1394Impl::FEATURE_CONTROL_REGISTER[feature];
    IEEE1394::Quadlet original;
    adapter.read(camera, featureRegister, getCharAddress(original), sizeof(original));
    Camera1394Impl::FeatureControl control = Cast::impersonate<Camera1394Impl::FeatureControl, uint32>(original);
    assert(
      control.enabled && !control.automaticMode && !control.autoAdjustmentMode,
      bindCause(Camera1394Exception(this), Camera1394::INVALID_FEATURE_MODE)
    ); // must be in manual mode
    control.absoluteControl = false; // enable value field
    control.value = value;
    IEEE1394::Quadlet quadlet;
    quadlet = Cast::impersonate<uint32>(control);
    adapter.write(camera, featureRegister, getCharAddress(quadlet), sizeof(quadlet));
    if (!getFeatureStatus(feature)) { // check if error or warning
      adapter.write(camera, featureRegister, getCharAddress(original), sizeof(original)); // try to restore original value
    }
  }
  
  int Camera1394::getBrightness() const throw(NotSupported, IEEE1394Exception) {
    assert(
      featureDescriptors.brightness.available && featureDescriptors.brightness.readable,
      bindCause(NotSupported(this), Camera1394::FEATURE_NOT_READABLE)
    );
    IEEE1394::Quadlet quadlet;
    adapter.read(camera, commandRegisters + Camera1394Impl::FEATURE_BRIGHTNESS, getCharAddress(quadlet), sizeof(quadlet));
    return Cast::impersonate<Camera1394Impl::FeatureControl, uint32>(quadlet).value;
  }

  void Camera1394::setBrightness(int value) throw(NotSupported, OutOfRange, IEEE1394Exception) {
    setGenericFeature(BRIGHTNESS_CONTROL, featureDescriptors.brightness, value);
  }

  int Camera1394::getAutoExposure() const throw(NotSupported, IEEE1394Exception) {
    assert(
      featureDescriptors.autoExposure.available && featureDescriptors.autoExposure.readable,
      bindCause(NotSupported(this), Camera1394::FEATURE_NOT_READABLE)
    );
    IEEE1394::Quadlet quadlet;
    adapter.read(camera, commandRegisters + Camera1394Impl::FEATURE_AUTO_EXPOSURE, getCharAddress(quadlet), sizeof(quadlet));
    return Cast::impersonate<Camera1394Impl::FeatureControl, uint32>(quadlet).value;
  }

  void Camera1394::setAutoExposure(int value) throw(NotSupported, OutOfRange, IEEE1394Exception) {
    setGenericFeature(AUTO_EXPOSURE_CONTROL, featureDescriptors.autoExposure, value);
  }

  int Camera1394::getSharpness() const throw(NotSupported, IEEE1394Exception) {
    assert(
      featureDescriptors.sharpness.available && featureDescriptors.sharpness.readable,
      bindCause(NotSupported(this), Camera1394::FEATURE_NOT_READABLE)
    );
    IEEE1394::Quadlet quadlet;
    adapter.read(camera, commandRegisters + Camera1394Impl::FEATURE_SHARPNESS, getCharAddress(quadlet), sizeof(quadlet));
    return Cast::impersonate<Camera1394Impl::FeatureControl, uint32>(quadlet).value;
  }

  void Camera1394::setSharpness(int value) throw(NotSupported, OutOfRange, IEEE1394Exception) {
    setGenericFeature(SHARPNESS_CONTROL, featureDescriptors.sharpness, value);
  }

  int Camera1394::getWhiteBalanceBlueRatio() const throw(NotSupported, IEEE1394Exception) {
    assert(
      featureDescriptors.whiteBalance.available && featureDescriptors.whiteBalance.readable,
      bindCause(NotSupported(this), Camera1394::FEATURE_NOT_READABLE)
    );
    IEEE1394::Quadlet quadlet;
    adapter.read(camera, commandRegisters + Camera1394Impl::FEATURE_WHITE_BALANCE, getCharAddress(quadlet), sizeof(quadlet));
    return Cast::impersonate<Camera1394Impl::WhiteBalanceFeatureControl, uint32>(quadlet).blueRatio;
  }

  int Camera1394::getWhiteBalanceRedRatio() const throw(NotSupported, IEEE1394Exception) {
    assert(
      featureDescriptors.whiteBalance.available && featureDescriptors.whiteBalance.readable,
      bindCause(NotSupported(this), Camera1394::FEATURE_NOT_READABLE)
    );
    IEEE1394::Quadlet quadlet;
    adapter.read(camera, commandRegisters + Camera1394Impl::FEATURE_WHITE_BALANCE, getCharAddress(quadlet), sizeof(quadlet));
    return Cast::impersonate<Camera1394Impl::WhiteBalanceFeatureControl, uint32>(quadlet).redRatio;
  }
  
  void Camera1394::setWhiteBalance(int blueRatio, int redRatio) throw(NotSupported, OutOfRange, IEEE1394Exception) {
    assert(
      featureDescriptors.whiteBalance.available,
      bindCause(NotSupported(this), Camera1394::FEATURE_NOT_SUPPORTED)
    );
    assert(
      (blueRatio >= featureDescriptors.whiteBalance.minimum) && (blueRatio <= featureDescriptors.whiteBalance.maximum) &&
      (redRatio >= featureDescriptors.whiteBalance.minimum) && (redRatio <= featureDescriptors.whiteBalance.maximum),
      OutOfDomain(this)
    );
    uint64 featureRegister = commandRegisters + Camera1394Impl::FEATURE_WHITE_BALANCE;
    IEEE1394::Quadlet original;
    adapter.read(camera, featureRegister, getCharAddress(original), sizeof(original));
    Camera1394Impl::WhiteBalanceFeatureControl control = Cast::impersonate<Camera1394Impl::WhiteBalanceFeatureControl, uint32>(original);
    assert(
      control.enabled && !control.automaticMode && !control.autoAdjustmentMode,
      bindCause(Camera1394Exception(this), Camera1394::INVALID_FEATURE_MODE)
    ); // must be in manual mode
    control.absoluteControl = false; // enable value field
    control.blueRatio = blueRatio;
    control.redRatio = redRatio;
    IEEE1394::Quadlet quadlet;
    quadlet = Cast::impersonate<uint32>(control);
    adapter.write(camera, featureRegister, getCharAddress(quadlet), sizeof(quadlet));
    if (!getFeatureStatus(WHITE_BALANCE_CONTROL)) { // check if error or warning
      adapter.write(camera, featureRegister, getCharAddress(original), sizeof(original)); // try to restore original value
    }
  }

  int Camera1394::getHue() const throw(NotSupported, IEEE1394Exception) {
    assert(
      featureDescriptors.hue.available && featureDescriptors.hue.readable,
      bindCause(NotSupported(this), Camera1394::FEATURE_NOT_READABLE)
    );
    IEEE1394::Quadlet quadlet;
    adapter.read(camera, commandRegisters + Camera1394Impl::FEATURE_HUE, getCharAddress(quadlet), sizeof(quadlet));
    return Cast::impersonate<Camera1394Impl::FeatureControl, uint32>(quadlet).value;
  }

  void Camera1394::setHue(int value) throw(NotSupported, OutOfRange, IEEE1394Exception) {
    setGenericFeature(HUE_CONTROL, featureDescriptors.hue, value);
  }

  int Camera1394::getSaturation() const throw(NotSupported, IEEE1394Exception) {
    assert(
      featureDescriptors.saturation.available && featureDescriptors.saturation.readable,
      bindCause(NotSupported(this), Camera1394::FEATURE_NOT_READABLE)
    );
    IEEE1394::Quadlet quadlet;
    adapter.read(camera, commandRegisters + Camera1394Impl::FEATURE_SATURATION, getCharAddress(quadlet), sizeof(quadlet));
    return Cast::impersonate<Camera1394Impl::FeatureControl, uint32>(quadlet).value;
  }

  void Camera1394::setSaturation(int value) throw(NotSupported, OutOfRange, IEEE1394Exception) {
    setGenericFeature(SATURATION_CONTROL, featureDescriptors.saturation, value);
  }

  int Camera1394::getGamma() const throw(NotSupported, IEEE1394Exception) {
    assert(
      featureDescriptors.gamma.available && featureDescriptors.gamma.readable,
      bindCause(NotSupported(this), Camera1394::FEATURE_NOT_READABLE)
    );
    IEEE1394::Quadlet quadlet;
    adapter.read(camera, commandRegisters + Camera1394Impl::FEATURE_GAMMA, getCharAddress(quadlet), sizeof(quadlet));
    return Cast::impersonate<Camera1394Impl::FeatureControl, uint32>(quadlet).value;
  }

  void Camera1394::setGamma(int value) throw(NotSupported, OutOfRange, IEEE1394Exception) {
    setGenericFeature(GAMMA_CONTROL, featureDescriptors.gamma, value);
  }

  int Camera1394::getShutter() const throw(NotSupported, IEEE1394Exception) {
    assert(
      featureDescriptors.shutter.available && featureDescriptors.shutter.readable,
      bindCause(NotSupported(this), Camera1394::FEATURE_NOT_READABLE)
    );
    IEEE1394::Quadlet quadlet;
    adapter.read(camera, commandRegisters + Camera1394Impl::FEATURE_SHUTTER, getCharAddress(quadlet), sizeof(quadlet));
    return Cast::impersonate<Camera1394Impl::FeatureControl, uint32>(quadlet).value;
  }

  void Camera1394::setShutter(int value) throw(NotSupported, OutOfRange, IEEE1394Exception) {
    setGenericFeature(SHUTTER_CONTROL, featureDescriptors.shutter, value);
  }

  int Camera1394::getGain() const throw(NotSupported, IEEE1394Exception) {
    assert(
      featureDescriptors.gain.available && featureDescriptors.gain.readable,
      bindCause(NotSupported(this), Camera1394::FEATURE_NOT_READABLE)
    );
    IEEE1394::Quadlet quadlet;
    adapter.read(camera, commandRegisters + Camera1394Impl::FEATURE_GAIN, getCharAddress(quadlet), sizeof(quadlet));
    return Cast::impersonate<Camera1394Impl::FeatureControl, uint32>(quadlet).value;
  }

  void Camera1394::setGain(int value) throw(NotSupported, OutOfRange, IEEE1394Exception) {
    setGenericFeature(GAIN_CONTROL, featureDescriptors.gain, value);
  }

  int Camera1394::getIRIS() const throw(NotSupported, IEEE1394Exception) {
    assert(
      featureDescriptors.iris.available && featureDescriptors.iris.readable,
      bindCause(NotSupported(this), Camera1394::FEATURE_NOT_READABLE)
    );
    IEEE1394::Quadlet quadlet;
    adapter.read(camera, commandRegisters + Camera1394Impl::FEATURE_IRIS, getCharAddress(quadlet), sizeof(quadlet));
    return Cast::impersonate<Camera1394Impl::FeatureControl, uint32>(quadlet).value;
  }

  void Camera1394::setIRIS(int value) throw(NotSupported, OutOfRange, IEEE1394Exception) {
    setGenericFeature(IRIS_CONTROL, featureDescriptors.iris, value);
  }

  int Camera1394::getFocus() const throw(NotSupported, IEEE1394Exception) {
    assert(
      featureDescriptors.focus.available && featureDescriptors.focus.readable,
      bindCause(NotSupported(this), Camera1394::FEATURE_NOT_READABLE)
    );
    IEEE1394::Quadlet quadlet;
    adapter.read(camera, commandRegisters + Camera1394Impl::FEATURE_FOCUS, getCharAddress(quadlet), sizeof(quadlet));
    return Cast::impersonate<Camera1394Impl::FeatureControl, uint32>(quadlet).value;
  }

  void Camera1394::setFocus(int value) throw(NotSupported, OutOfRange, IEEE1394Exception) {
    setGenericFeature(FOCUS_CONTROL, featureDescriptors.focus, value);
  }

  int Camera1394::getTemperature() const throw(NotSupported, IEEE1394Exception) {
    assert(
      featureDescriptors.temperature.available && featureDescriptors.temperature.readable,
      bindCause(NotSupported(this), Camera1394::FEATURE_NOT_READABLE)
    );
    IEEE1394::Quadlet quadlet;
    adapter.read(camera, commandRegisters + Camera1394Impl::FEATURE_TEMPERATURE, getCharAddress(quadlet), sizeof(quadlet));
    return Cast::impersonate<Camera1394Impl::TemperatureFeatureControl, uint32>(quadlet).currentValue;
  }

  int Camera1394::getTargetTemperature() const throw(NotSupported, IEEE1394Exception) {
    assert(
      featureDescriptors.temperature.available && featureDescriptors.temperature.readable,
      bindCause(NotSupported(this), Camera1394::FEATURE_NOT_READABLE)
    );
    IEEE1394::Quadlet quadlet;
    adapter.read(camera, commandRegisters + Camera1394Impl::FEATURE_TEMPERATURE, getCharAddress(quadlet), sizeof(quadlet));
    return Cast::impersonate<Camera1394Impl::TemperatureFeatureControl, uint32>(quadlet).targetValue;
  }

  void Camera1394::setTemperature(int value) throw(NotSupported, OutOfRange, IEEE1394Exception) {
    assert(featureDescriptors.temperature.available, bindCause(NotSupported(this), Camera1394::FEATURE_NOT_SUPPORTED));
    assert((value >= featureDescriptors.temperature.minimum) && (value <= featureDescriptors.temperature.maximum), OutOfDomain(this));
    uint64 featureRegister = commandRegisters + Camera1394Impl::FEATURE_TEMPERATURE;
    IEEE1394::Quadlet original;
    adapter.read(camera, featureRegister, getCharAddress(original), sizeof(original));
    Camera1394Impl::TemperatureFeatureControl control = Cast::impersonate<Camera1394Impl::TemperatureFeatureControl, uint32>(original);
    assert(
      control.enabled && !control.automaticMode && !control.autoAdjustmentMode,
      bindCause(Camera1394Exception(this), Camera1394::INVALID_FEATURE_MODE)
    ); // must be in manual mode
    control.absoluteControl = false; // enable value field
    control.targetValue = value;
    IEEE1394::Quadlet quadlet;
    quadlet = Cast::impersonate<uint32>(control);
    adapter.write(camera, featureRegister, getCharAddress(quadlet), sizeof(quadlet));
    if (!getFeatureStatus(TEMPERATURE_CONTROL)) { // check if error or warning
      adapter.write(camera, featureRegister, getCharAddress(original), sizeof(original)); // try to restore original value
    }
  }

  // TAG: getTrigger..???
  // TAG: setTriggerParam...??
  // TAG: setTriggerPolatity(bool polatiry) throw(NotSupported, IEEE1394Exception) {} assert(POLARITY_INQ...
  
  int Camera1394::getZoom() const throw(NotSupported, IEEE1394Exception) {
    assert(
      featureDescriptors.zoom.available && featureDescriptors.zoom.readable,
      bindCause(NotSupported(this), Camera1394::FEATURE_NOT_READABLE)
    );
    IEEE1394::Quadlet quadlet;
    adapter.read(camera, commandRegisters + Camera1394Impl::FEATURE_ZOOM, getCharAddress(quadlet), sizeof(quadlet));
    return Cast::impersonate<Camera1394Impl::FeatureControl, uint32>(quadlet).value;
  }

  void Camera1394::setZoom(int value) throw(NotSupported, OutOfRange, IEEE1394Exception) {
    setGenericFeature(ZOOM_CONTROL, featureDescriptors.zoom, value);
  }

  int Camera1394::getPan() const throw(NotSupported, IEEE1394Exception) {
    assert(
      featureDescriptors.pan.available && featureDescriptors.pan.readable,
      bindCause(NotSupported(this), Camera1394::FEATURE_NOT_READABLE)
    );
    IEEE1394::Quadlet quadlet;
    adapter.read(camera, commandRegisters + Camera1394Impl::FEATURE_PAN, getCharAddress(quadlet), sizeof(quadlet));
    return Cast::impersonate<Camera1394Impl::FeatureControl, uint32>(quadlet).value;
  }

  void Camera1394::setPan(int value) throw(NotSupported, OutOfRange, IEEE1394Exception) {
    setGenericFeature(PAN_CONTROL, featureDescriptors.pan, value);
  }

  int Camera1394::getTilt() const throw(NotSupported, IEEE1394Exception) {
    assert(
      featureDescriptors.tilt.available && featureDescriptors.tilt.readable,
      bindCause(NotSupported(this), Camera1394::FEATURE_NOT_READABLE)
    );
    IEEE1394::Quadlet quadlet;
    adapter.read(camera, commandRegisters + Camera1394Impl::FEATURE_TILT, getCharAddress(quadlet), sizeof(quadlet));
    return Cast::impersonate<Camera1394Impl::FeatureControl, uint32>(quadlet).value;
  }

  void Camera1394::setTilt(int value) throw(NotSupported, OutOfRange, IEEE1394Exception) { // TAG: Camera1394Exception
    setGenericFeature(TILT_CONTROL, featureDescriptors.tilt, value);
  }

  int Camera1394::getOpticalFilter() const throw(NotSupported, IEEE1394Exception) {
    assert(
      featureDescriptors.opticalFilter.available && featureDescriptors.opticalFilter.readable,
      bindCause(NotSupported(this), Camera1394::FEATURE_NOT_READABLE)
    );
    IEEE1394::Quadlet quadlet;
    adapter.read(camera, commandRegisters + Camera1394Impl::FEATURE_OPTICAL_FILTER, getCharAddress(quadlet), sizeof(quadlet));
    return Cast::impersonate<Camera1394Impl::FeatureControl, uint32>(quadlet).value;
  }

  void Camera1394::setOpticalFilter(int value) throw(NotSupported, OutOfRange, IEEE1394Exception) {
    setGenericFeature(OPTICAL_FILTER_CONTROL, featureDescriptors.opticalFilter, value);
  }

  int Camera1394::getCaptureSize() const throw(NotSupported, IEEE1394Exception) {
    assert(
      featureDescriptors.captureSize.available && featureDescriptors.captureSize.readable,
      bindCause(NotSupported(this), Camera1394::FEATURE_NOT_READABLE)
    );
    IEEE1394::Quadlet quadlet;
    adapter.read(camera, commandRegisters + Camera1394Impl::FEATURE_CAPTURE_SIZE, getCharAddress(quadlet), sizeof(quadlet));
    return Cast::impersonate<Camera1394Impl::FeatureControl, uint32>(quadlet).value;
  }

  void Camera1394::setCaptureSize(int value) throw(NotSupported, OutOfRange, IEEE1394Exception) {
    setGenericFeature(CAPTURE_SIZE, featureDescriptors.captureSize, value);
  }

  int Camera1394::getCaptureQuality() const throw(NotSupported, IEEE1394Exception) {
    assert(
      featureDescriptors.captureQuality.available && featureDescriptors.captureQuality.readable,
      bindCause(NotSupported(this), Camera1394::FEATURE_NOT_READABLE)
    );
    IEEE1394::Quadlet quadlet;
    adapter.read(camera, commandRegisters + Camera1394Impl::FEATURE_CAPTURE_QUALITY, getCharAddress(quadlet), sizeof(quadlet));
    return Cast::impersonate<Camera1394Impl::FeatureControl, uint32>(quadlet).value;
  }

  void Camera1394::setCaptureQuality(int value) throw(NotSupported, OutOfRange, IEEE1394Exception) {
    setGenericFeature(CAPTURE_QUALITY, featureDescriptors.captureQuality, value);
  }

  Dimension Camera1394::getMaximumDimension(Mode mode) const throw(NotSupported) {
    assert(isModeSupported(mode), bindCause(NotSupported(this), Camera1394::MODE_NOT_SUPPORTED));
    if (Camera1394Impl::MODE_INFORMATION[mode].format == Camera1394::PARTIAL_IMAGE) {
      return partialImageMode[Camera1394Impl::MODE_INFORMATION[mode].mode].maximumDimension;
    } else {
      const Camera1394Impl::ModeInformation& info = Camera1394Impl::MODE_INFORMATION[mode];
      assert(info.width && info.height, bindCause(NotSupported(this), Camera1394::MODE_NOT_SUPPORTED));
      return Dimension(info.width, info.height);
    }
  }

  Dimension Camera1394::getUnitDimension(Mode mode) const throw(NotSupported) {
    assert(isModeSupported(mode), bindCause(NotSupported(this), Camera1394::MODE_NOT_SUPPORTED));
    if (Camera1394Impl::MODE_INFORMATION[mode].format == Camera1394::PARTIAL_IMAGE) {
      return partialImageMode[Camera1394Impl::MODE_INFORMATION[mode].mode].unitDimension;
    } else {
      const Camera1394Impl::ModeInformation& info = Camera1394Impl::MODE_INFORMATION[mode];
      assert(info.width && info.height, bindCause(NotSupported(this), Camera1394::MODE_NOT_SUPPORTED));
      return Dimension(info.width, info.height);
    }
  }

  Point2D Camera1394::getUnitOffset(Mode mode) const throw(NotSupported) {
    assert(isModeSupported(mode), bindCause(NotSupported(this), Camera1394::MODE_NOT_SUPPORTED));
    if (Camera1394Impl::MODE_INFORMATION[mode].format == Camera1394::PARTIAL_IMAGE) {
      return partialImageMode[Camera1394Impl::MODE_INFORMATION[mode].mode].unitOffset;
    } else {
      const Camera1394Impl::ModeInformation& info = Camera1394Impl::MODE_INFORMATION[mode];
      assert(info.height && info.width, bindCause(NotSupported(this), Camera1394::MODE_NOT_SUPPORTED));
      return Point2D(info.height, info.width);
    }
  }

  unsigned int Camera1394::getPixelFormats(Mode mode) const throw(NotSupported) {
    assert(isModeSupported(mode), bindCause(NotSupported(this), Camera1394::MODE_NOT_SUPPORTED));
    if (Camera1394Impl::MODE_INFORMATION[mode].format == Camera1394::PARTIAL_IMAGE) {
      return partialImageMode[Camera1394Impl::MODE_INFORMATION[mode].mode].pixelFormats;
    } else {
      ASSERT(Camera1394Impl::MODE_INFORMATION[mode].pixelFormat >= 0);
      return 1 << Camera1394Impl::MODE_INFORMATION[mode].pixelFormat;
    }
  }

  void Camera1394::setRegion(const Region& region) throw(OutOfDomain, IEEE1394Exception) {
    assert(
      (region.getOffset().getColumn() % mode.unitOffset.getColumn() == 0) &&
      (region.getOffset().getRow() % mode.unitOffset.getRow() == 0) &&
      (region.getDimension().getWidth() % mode.unitDimension.getWidth() == 0) &&
      (region.getDimension().getHeight() % mode.unitDimension.getHeight() == 0) &&
      (region.getDimension().getWidth() <= mode.maximumDimension.getWidth()) &&
      (region.getDimension().getHeight() <= mode.maximumDimension.getHeight()),
      bindCause(OutOfDomain(this), Camera1394::REGION_NOT_SUPPORTED)
    );
    // write new region to camera
    this->region = region;
  }

  void Camera1394::setPixelFormat(PixelFormat pixelFormat) throw(NotSupported, IEEE1394Exception) {
    if (pixelFormat != this->pixelFormat) {
      assert(mode.pixelFormats & (1 << pixelFormat), bindCause(NotSupported(this), Camera1394::PIXEL_FORMAT_NOT_SUPPORTED));
      ASSERT(Camera1394Impl::MODE_INFORMATION[currentMode].format == Camera1394::PARTIAL_IMAGE); // TAG: what about EXIF
      // TAG: write to register
      this->pixelFormat = pixelFormat;
    }
  }

  bool Camera1394::acquire(char* buffer, unsigned int size) throw(ImageException, IEEE1394Exception) {
    assert(size == transmission.totalBytesPerFrame, bindCause(ImageException(this), Camera1394::FRAME_DIMENSION_MISMATCH));
    Allocator<char> headerBuffer(transmission.packetsPerFrame * sizeof(IEEE1394::Quadlet)); // packets * headerSize

    IEEE1394::IsochronousReadFixedDataRequest request = readChannel.getReadFixedDataRequest();
    request.setSubchannel(transmission.subchannel);
    request.setNumberOfPackets(transmission.packetsPerFrame);
    request.setHeaderSize(sizeof(IEEE1394::Quadlet));
    request.setPayload(transmission.bytesPerPacket);
    request.setBuffer(buffer, size, headerBuffer.getElements());
    
    readChannel.queue(request);
    
    // enable transmission
    IEEE1394::Quadlet quadlet;
    if (capabilities & SINGLE_ACQUISITION) {
      quadlet = 1 << 31; // single shot
      adapter.write(camera, commandRegisters + Camera1394Impl::FINITE_SHOTS, getCharAddress(quadlet), sizeof(quadlet));
    } else if (capabilities & MULTI_ACQUISITION) {
      quadlet = (1 << 30) + 1; // multi shot (ask for one frame)
      adapter.write(camera, commandRegisters + Camera1394Impl::FINITE_SHOTS, getCharAddress(quadlet), sizeof(quadlet));
    } else {
      quadlet = 1 << 31; // use continuous
      adapter.write(camera, commandRegisters + Camera1394Impl::ISO_ENABLE, getCharAddress(quadlet), sizeof(quadlet));
    }
    
    unsigned int period = getFramePeriod(getFrameRate());
    readChannel.wait(minimum<unsigned int>(2 * period/15, 999999999));

    IEEE1394::IsochronousReadRequest completedGenericRequest = readChannel.dequeue();
    if (!completedGenericRequest.isValid()) {
      readChannel.cancel();
    }
    
    IEEE1394::IsochronousReadFixedDataRequest completedRequest = completedGenericRequest.getIsochronousReadFixedDataRequest();

    // disable transmission
    if (capabilities & SINGLE_ACQUISITION) {
      quadlet = 0 << 31; // single shot
      adapter.write(camera, commandRegisters + Camera1394Impl::FINITE_SHOTS, getCharAddress(quadlet), sizeof(quadlet));
    } else if (capabilities & MULTI_ACQUISITION) {
      quadlet = 0 << 30 + 0; // multi shot
      adapter.write(camera, commandRegisters + Camera1394Impl::FINITE_SHOTS, getCharAddress(quadlet), sizeof(quadlet));
    } else {
      // use continuous
      quadlet = 0;
      adapter.write(camera, commandRegisters + Camera1394Impl::ISO_ENABLE, getCharAddress(quadlet), sizeof(quadlet));
    }
    
    return completedRequest.getStatus() == IEEE1394::COMPLETED;
  }

  bool Camera1394::acquire(ArrayImage<uint8>& frame) throw(NotSupported, ImageException, IEEE1394Exception) {
    if (pixelFormat != Camera1394::Y_8BIT) {
      setPixelFormat(Camera1394::Y_8BIT);
    }

    assert(frame.getDimension() == region.getDimension(), bindCause(ImageException(this), Camera1394::FRAME_DIMENSION_MISMATCH));
    Allocator<char> headerBuffer(transmission.packetsPerFrame * sizeof(IEEE1394::Quadlet)); // packets * headerSize

    IEEE1394::IsochronousReadFixedDataRequest request = readChannel.getReadFixedDataRequest();
    request.setSubchannel(transmission.subchannel);
    request.setNumberOfPackets(transmission.packetsPerFrame);
    request.setHeaderSize(sizeof(IEEE1394::Quadlet));
    request.setPayload(transmission.bytesPerPacket);
    request.setBuffer(pointer_cast<char*>(frame.getElements()), frame.getDimension().getSize(), headerBuffer.getElements());
    
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
    
    readChannel.queue(request);
    
    // enable transmission
    IEEE1394::Quadlet quadlet;
    if (capabilities & SINGLE_ACQUISITION) {
      quadlet = 1 << 31; // single shot
      adapter.write(camera, commandRegisters + Camera1394Impl::FINITE_SHOTS, getCharAddress(quadlet), sizeof(quadlet));
    } else if (capabilities & MULTI_ACQUISITION) {
      quadlet = (1 << 30) + 1; // multi shot (ask for one frame)
      adapter.write(camera, commandRegisters + Camera1394Impl::FINITE_SHOTS, getCharAddress(quadlet), sizeof(quadlet));
    } else {
      quadlet = 1 << 31; // use continuous
      adapter.write(camera, commandRegisters + Camera1394Impl::ISO_ENABLE, getCharAddress(quadlet), sizeof(quadlet));
    }

    unsigned int period = getFramePeriod(getFrameRate());
    readChannel.wait(minimum<unsigned int>(2 * period/15, 999999999));

    IEEE1394::IsochronousReadRequest completedGenericRequest = readChannel.dequeue();
    if (!completedGenericRequest.isValid()) {
      readChannel.cancel();
    }

    IEEE1394::IsochronousReadFixedDataRequest completedRequest = completedGenericRequest.getIsochronousReadFixedDataRequest();
    
    // disable transmission
    if (capabilities & SINGLE_ACQUISITION) {
      quadlet = 0 << 31; // single shot
      adapter.write(camera, commandRegisters + Camera1394Impl::FINITE_SHOTS, getCharAddress(quadlet), sizeof(quadlet));
    } else if (capabilities & MULTI_ACQUISITION) {
      quadlet = 0 << 30 + 0; // multi shot
      adapter.write(camera, commandRegisters + Camera1394Impl::FINITE_SHOTS, getCharAddress(quadlet), sizeof(quadlet));
    } else {
      // use continuous
      quadlet = 0;
      adapter.write(camera, commandRegisters + Camera1394Impl::ISO_ENABLE, getCharAddress(quadlet), sizeof(quadlet));
    }
    
    return completedRequest.getStatus() == IEEE1394::COMPLETED;
  }

  bool Camera1394::acquire(ArrayImage<uint16>& frame) throw(NotSupported, ImageException, IEEE1394Exception) {
    if (pixelFormat != Camera1394::Y_16BIT) {
      setPixelFormat(Camera1394::Y_16BIT);
    }
    
    assert(frame.getDimension() == region.getDimension(), bindCause(ImageException(this), Camera1394::FRAME_DIMENSION_MISMATCH));
    Allocator<char> headerBuffer(transmission.packetsPerFrame * sizeof(IEEE1394::Quadlet)); // packets * headerSize

    IEEE1394::IsochronousReadFixedDataRequest request = readChannel.getReadFixedDataRequest();
    request.setSubchannel(transmission.subchannel);
    request.setNumberOfPackets(transmission.packetsPerFrame);
    request.setHeaderSize(sizeof(IEEE1394::Quadlet));
    request.setPayload(transmission.bytesPerPacket);
    request.setBuffer(pointer_cast<char*>(frame.getElements()), frame.getDimension().getSize(), headerBuffer.getElements());
    
    readChannel.queue(request);
    
    // enable transmission
    IEEE1394::Quadlet quadlet;
    if (capabilities & SINGLE_ACQUISITION) {
      quadlet = 1 << 31; // single shot
      adapter.write(camera, commandRegisters + Camera1394Impl::FINITE_SHOTS, getCharAddress(quadlet), sizeof(quadlet));
    } else if (capabilities & MULTI_ACQUISITION) {
      quadlet = (1 << 30) + 1; // multi shot (ask for one frame)
      adapter.write(camera, commandRegisters + Camera1394Impl::FINITE_SHOTS, getCharAddress(quadlet), sizeof(quadlet));
    } else {
      quadlet = 1 << 31; // use continuous
      adapter.write(camera, commandRegisters + Camera1394Impl::ISO_ENABLE, getCharAddress(quadlet), sizeof(quadlet));
    }
    
    unsigned int period = getFramePeriod(getFrameRate());
    readChannel.wait(minimum<unsigned int>(2 * period/15, 999999999));

    IEEE1394::IsochronousReadRequest completedGenericRequest = readChannel.dequeue();
    if (!completedGenericRequest.isValid()) {
      readChannel.cancel();
    }

    IEEE1394::IsochronousReadFixedDataRequest completedRequest = completedGenericRequest.getIsochronousReadFixedDataRequest();

    // disable transmission
    if (capabilities & SINGLE_ACQUISITION) {
      quadlet = 0 << 31; // single shot
      adapter.write(camera, commandRegisters + Camera1394Impl::FINITE_SHOTS, getCharAddress(quadlet), sizeof(quadlet));
    } else if (capabilities & MULTI_ACQUISITION) {
      quadlet = 0 << 30 + 0; // multi shot
      adapter.write(camera, commandRegisters + Camera1394Impl::FINITE_SHOTS, getCharAddress(quadlet), sizeof(quadlet));
    } else {
      // use continuous
      quadlet = 0;
      adapter.write(camera, commandRegisters + Camera1394Impl::ISO_ENABLE, getCharAddress(quadlet), sizeof(quadlet));
    }
    
    return completedRequest.getStatus() == IEEE1394::COMPLETED;
  }

  bool Camera1394::acquire(ArrayImage<RGB24Pixel>& frame) throw(NotSupported, ImageException, IEEE1394Exception) {
    if (pixelFormat != Camera1394::RGB_8BIT) {
      setPixelFormat(Camera1394::RGB_8BIT);
    }

    Trace::message("1");
    assert(frame.getDimension() == region.getDimension(), bindCause(ImageException(this), Camera1394::FRAME_DIMENSION_MISMATCH));
    Allocator<char> headerBuffer(transmission.packetsPerFrame * sizeof(IEEE1394::Quadlet)); // packets * headerSize
    
    Trace::message("2");
    IEEE1394::IsochronousReadFixedDataRequest request = readChannel.getReadFixedDataRequest();
    request.setSubchannel(transmission.subchannel);
    request.setNumberOfPackets(transmission.packetsPerFrame);
    request.setHeaderSize(sizeof(IEEE1394::Quadlet));
    request.setPayload(transmission.bytesPerPacket);
    request.setBuffer(pointer_cast<char*>(frame.getElements()), frame.getDimension().getSize(), headerBuffer.getElements());
    
    Trace::message("3");
    readChannel.queue(request);
    
    Trace::message("4");
    // enable transmission
    IEEE1394::Quadlet quadlet;
    if (capabilities & SINGLE_ACQUISITION) {
    Trace::message("4a");
      quadlet = 1 << 31; // single shot
      adapter.write(camera, commandRegisters + Camera1394Impl::FINITE_SHOTS, getCharAddress(quadlet), sizeof(quadlet));
    } else if (capabilities & MULTI_ACQUISITION) {
    Trace::message("4b");
      quadlet = (1 << 30) + 1; // multi shot (ask for one frame)
      adapter.write(camera, commandRegisters + Camera1394Impl::FINITE_SHOTS, getCharAddress(quadlet), sizeof(quadlet));
    } else {
    Trace::message("4c");
      quadlet = 1 << 31; // use continuous
      adapter.write(camera, commandRegisters + Camera1394Impl::ISO_ENABLE, getCharAddress(quadlet), sizeof(quadlet));
    }
    
    Trace::message("5");
    unsigned int period = getFramePeriod(getFrameRate());
    fout << period << ENDL;
    Trace::message("6");
    readChannel.wait(minimum<unsigned int>(2 * period/15, 999999999));
    Trace::message("7");
    
    IEEE1394::IsochronousReadRequest completedGenericRequest = readChannel.dequeue();
    if (!completedGenericRequest.isValid()) {
      readChannel.cancel();
    }
    Trace::message("8");
    
    IEEE1394::IsochronousReadFixedDataRequest completedRequest = completedGenericRequest.getIsochronousReadFixedDataRequest();
    Trace::message("9");
    
    // disable transmission
    if (capabilities & SINGLE_ACQUISITION) {
      quadlet = 0 << 31; // single shot
      adapter.write(camera, commandRegisters + Camera1394Impl::FINITE_SHOTS, getCharAddress(quadlet), sizeof(quadlet));
    } else if (capabilities & MULTI_ACQUISITION) {
      quadlet = 0 << 30 + 0; // multi shot
      adapter.write(camera, commandRegisters + Camera1394Impl::FINITE_SHOTS, getCharAddress(quadlet), sizeof(quadlet));
    } else {
      // use continuous
      quadlet = 0;
      //adapter.write(camera, commandRegisters + Camera1394Impl::ISO_ENABLE, getCharAddress(quadlet), sizeof(quadlet));
    }
    Trace::message("10");
    
    return completedRequest.getStatus() == IEEE1394::COMPLETED;
  }
  
}; // end of gip namespace
