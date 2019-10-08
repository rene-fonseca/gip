/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    Copyright (C) 2002 by Rene Moeller Fonseca <fonseca@mip.sdu.dk>

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#pragma once

#include <gip/video/Camera.h>
#include <gip/ArrayImage.h>
#include <gip/Region.h>
#include <gip/ImageException.h>
#include <base/communication/IEEE1394.h>
#include <base/string/String.h>
#include <base/communication/CommunicationsException.h>
#include <base/NotSupported.h>
#include <base/collection/Array.h>
#include <base/Dimension.h>
#include <base/ByteOrder.h>

namespace gip {

  /**
    The IIDC 1394-based Digital Camera Specification (version 1.30).
    See http://www.1394ta.org.
    
    @short IEEE 1394 Digital Camera support.
    @ingroup video
    @author Rene Moeller Fonseca <fonseca@mip.sdu.dk>
    @version 1.0
  */

  class Camera1394 : public Camera {
  public:

    /**
      Exception raised by the Camera1394 class.
      
      @short Camera exception.
      @ingroup exceptions video
      @author Rene Moeller Fonseca <fonseca@mip.sdu.dk>
      @version 1.0
    */
    
    class Camera1394Exception : public Exception {
    public:
      
      /**
        Initializes the exception object with no message.
      */
      inline Camera1394Exception() throw() {
      }
      
      /**
        Initializes the exception object.
        
        @param message The message.
      */
      inline Camera1394Exception(const char* message) throw()
        : Exception(message) {
      }
      
      /**
        Initializes the exception object without an associated message.
        
        @param type The identity of the type.
      */
      inline Camera1394Exception(Type type) throw() : Exception(type) {
      }
      
      /**
        Initializes the exception object.
        
        @param message An NULL-terminated string (ASCII).
        @param type The identity of the type.
      */
      inline Camera1394Exception(const char* message, Type type) throw()
        : Exception(message, type) {
      }
    };


    
    /** Exception causes. */
    enum ExceptionCause {
      NOT_A_CAMERA = IEEE1394::LAST_CAUSE,
      FORMAT_NOT_SUPPORTED,
      MODE_NOT_SUPPORTED,
      FRAME_RATE_NOT_SUPPORTED,
      PIXEL_FORMAT_NOT_SUPPORTED,
      FEATURE_NOT_SUPPORTED,
      FEATURE_OPERATING_MODE_NOT_SUPPORTED,
      INVALID_FEATURE_MODE,
      FEATURE_NOT_READABLE,
      REGION_NOT_SUPPORTED,
      FRAME_DIMENSION_MISMATCH,
      INVALID_FRAME_BUFFER,
      LAST_CAUSE
    };

    enum Specification {
      SPECIFICATION_1_04 = 0x010400, /**< 1394 based Digital Camera specification version 1.04. */
      SPECIFICATION_1_20 = 0x011400, /**< 1394 based Digital Camera specification version 1.20. */
      SPECIFICATION_1_30 = 0x011e00, /**< 1394 based Digital Camera specification version 1.30. */
      SPECIFICATION_LATER_THAN_1_30 = 0xffffffff /**< Specification is later than 1.30. */
    };
    
    /** Format. */
    enum Format {
      UNCOMPRESSED_VGA = 0, /**< VGA non-compressed formation. */
      UNCOMPRESSED_SUPER_VGA_I = 1, /**< Super VGA non-compressed format (low). */
      UNCOMPRESSED_SUPER_VGA_II = 2, /**< Super VGA non-compressed format (high). */
      STILL_IMAGE = 6, /**< Still image format. */
      PARTIAL_IMAGE = 7 /**< Partial image format. */
    };

    /**
      The video modes.
    */
    enum Mode {
      YUV_444_160X120_24BIT, /**< Only with UNCOMPRESSED_VGA format. */
      YUV_422_320X240_16BIT, /**< Only with UNCOMPRESSED_VGA format. */
      YUV_411_640X480_12BIT, /**< Only with UNCOMPRESSED_VGA format. */
      YUV_422_640X480_16BIT, /**< Only with UNCOMPRESSED_VGA format. */
      RGB_640X480_24BIT, /**< Only with UNCOMPRESSED_VGA format. */
      Y_640X480_8BIT, /**< Only with UNCOMPRESSED_VGA format. */
      Y_640X480_16BIT, /**< Only with UNCOMPRESSED_VGA format. */

      YUV_422_800X600_16BIT, /**< Only with UNCOMPRESSED_SUPER_VGA_I format. */
      RGB_800X600_24BIT, /**< Only with UNCOMPRESSED_SUPER_VGA_I format. */
      Y_800X600_8BIT, /**< Only with UNCOMPRESSED_SUPER_VGA_I format. */
      YUV_422_1024X768_16BIT, /**< Only with UNCOMPRESSED_SUPER_VGA_I format. */
      RGB_1024X768_24BIT, /**< Only with UNCOMPRESSED_SUPER_VGA_I format. */
      Y_1024X768_8BIT, /**< Only with UNCOMPRESSED_SUPER_VGA_I format. */
      Y_800X600_16BIT, /**< Only with UNCOMPRESSED_SUPER_VGA_I format. */
      Y_1024X768_16BIT, /**< Only with UNCOMPRESSED_SUPER_VGA_I format. */

      YUV_422_1280X960_16BIT, /**< Only with UNCOMPRESSED_SUPER_VGA_II format. */
      RGB_1280X960_24BIT, /**< Only with UNCOMPRESSED_SUPER_VGA_II format. */
      Y_1280X960_8BIT, /**< Only with UNCOMPRESSED_SUPER_VGA_II format. */
      YUV_422_1600X1200_16BIT, /**< Only with UNCOMPRESSED_SUPER_VGA_II format. */
      RGB_1600X1200_24BIT, /**< Only with UNCOMPRESSED_SUPER_VGA_II format. */
      Y_1600X1200_8BIT, /**< Only with UNCOMPRESSED_SUPER_VGA_II format. */
      Y_1280X960_16BIT, /**< Only with UNCOMPRESSED_SUPER_VGA_II format. */
      Y_1600X1200_16BIT, /**< Only with UNCOMPRESSED_SUPER_VGA_II format. */
      
      EXIF, /**< Only with STILL_IMAGE format. */
      
      PARTIAL_IMAGE_MODE_0, /**< Only with PARTIAL_IMAGE format. */
      PARTIAL_IMAGE_MODE_1, /**< Only with PARTIAL_IMAGE format. */
      PARTIAL_IMAGE_MODE_2, /**< Only with PARTIAL_IMAGE format. */
      PARTIAL_IMAGE_MODE_3, /**< Only with PARTIAL_IMAGE format. */
      PARTIAL_IMAGE_MODE_4, /**< Only with PARTIAL_IMAGE format. */
      PARTIAL_IMAGE_MODE_5, /**< Only with PARTIAL_IMAGE format. */
      PARTIAL_IMAGE_MODE_6, /**< Only with PARTIAL_IMAGE format. */
      PARTIAL_IMAGE_MODE_7 /**< Only with PARTIAL_IMAGE format. */
    };

    /** Frame rate. */
    enum FrameRate {
      RATE_1_875, /**< 1.875fps. */
      RATE_3_75, /**< 3.75fps. */
      RATE_7_5, /**< 7.5fps. */
      RATE_15, /**< 15fps. */
      RATE_30, /**< 30fps. */
      RATE_60 /**< 60fps. */
    };

    /** Pixel format. */
    enum PixelFormat {
      Y_8BIT, /**< 8 bit mono format. */
      Y_16BIT, /**< 16 bit mono format. */
      YUV_411_8BIT, /**< YUV 411 with 8 bit per component (effectively 12 bits per pixel). */
      YUV_422_8BIT, /**< YUV 422 with 8 bit per component (effectively 16 bits per pixel). */
      YUV_444_8BIT, /**< YUV 444 with 8 bit per component (effectively 24 bits per pixel). */
      RGB_8BIT, /**< RGB format with 8 bit per component (effectively 24 bits per pixel). */
      RGB_16BIT /**< RGB format with 16 bit per component (effectively 48 bits per pixel). */
    };

    /** Camera capability. */
    enum Capability {
      POWER_CONTROL = 1 << 0, /**< Support for power control. */
      SINGLE_ACQUISITION = 1 << 1, /**< Support for single shot transmission. */
      MULTI_ACQUISITION = 1 << 2, /**< Support for multishot transmission. */
      ADVANCED_FEATURES = 1 << 3, /**< Advanced features capability. */
      MODE_ERROR_STATUS = 1 << 4, /**< Mode error status capability. */
      FEATURE_ERROR_STATUS = 1 << 5 /**< Feature error status capability. */
    };
    
    /** Camera feature. */
    enum Feature {
      BRIGHTNESS_CONTROL, /**< Brightness control. */
      AUTO_EXPOSURE_CONTROL, /**< Auto exposure control. */
      SHARPNESS_CONTROL, /**< Sharpness control. */
      WHITE_BALANCE_CONTROL, /**< White balance control. */
      HUE_CONTROL, /**< Hue control. */
      SATURATION_CONTROL, /**< Saturation control. */
      GAMMA_CONTROL, /**< Gamma control. */
      SHUTTER_CONTROL, /**< Shutter integration time control. */
      GAIN_CONTROL, /**< Gain control. */
      IRIS_CONTROL, /**< IRIS control. */
      FOCUS_CONTROL, /**< Focus control. */
      TEMPERATURE_CONTROL, /**< Temperature control. */
      TRIGGER_CONTROL, /**< Trigger control. */
      ZOOM_CONTROL, /**< Zoom control. */
      PAN_CONTROL, /**< PAN control. */
      TILT_CONTROL, /**< TILT control. */
      OPTICAL_FILTER_CONTROL, /**< Optical filter control. */
      CAPTURE_SIZE, /**< Capture size for EXIF format. */
      CAPTURE_QUALITY /**< Capture size for EXIF format. */
    };

    /** The operating mode of a camera feature. */
    enum FeatureOperatingMode {
      DISABLED, /**< The feature is disabled. */
      AUTOMATIC, /**< The feature is handled automatically by the camera. */
      AUTO_ADJUST, /**< The feature is adjusted once by camera. */
      MANUAL /**< The feature is set manually. */
    };

    /** Specified the number of modes supported by this class. */
    static const unsigned int NUMBER_OF_MODES = PARTIAL_IMAGE_MODE_7 + 1;
    /** Specifies the supported isochronous subchannels. */
    static const unsigned int ISOCHRONOUS_SUBCHANNELS = (1 << 16) - 1;
    /** The modes supported by this class. */
    static const Mode MODES[NUMBER_OF_MODES];
    
    /* Describes details about a feature. */
    struct GenericFeatureDescriptor {
      /** Specifies whether the feature is available. */
      bool available : 1;
      /** Automatic adjustment mode. */
      bool autoAdjustmentMode : 1;
      /** Specifies whether or not the value may be read. */
      bool readable : 1;
      /** Specifies whether or not the feature may be turned ON and OFF. */
      bool switchable : 1;
      /** Automatic mode. */
      bool automaticMode : 1;
      /** Manual mode. */
      bool manualMode : 1;
      /** The minimum value. */
      unsigned int minimum;
      /** The maximum value. */
      unsigned int maximum;
    };

    /* Describes the trigger feature. */
    struct TriggerFeatureDescriptor {
      /** Specifies whether the feature is available. */
      bool available : 1;
      /** Specifies whether or not the value may be read. */
      bool readable : 1;
      /** Specifies whether or not the feature may be turned ON and OFF. */
      bool switchable : 1;
      /** Specifies whether or not the polarity of the trigger input may be selected. */
      bool polarity : 1;
      /** Specifies the available trigger signals (bit 0 is set if signal 0 is available or so on). */
      unsigned int availableSignals;
    };

    /* Describes the partial image mode. */
    struct ModeDescriptor {
      Dimension maximumDimension;
      Dimension unitDimension;
      Point2D unitOffset;
      unsigned int pixelFormats;
    };

    /* IEEE 1394 isochronous transmission descriptor. */
    struct IsochronousTransmission {
      /** Isochronous channel. */
      unsigned int subchannel;
      /** Isochronous speed. */
      unsigned int speed;
      /** The number of pixels per frame. */
      unsigned int pixelsPerFrame;
      /** The total number of bytes per frame. */
      unsigned int totalBytesPerFrame;
      /** The unit of bytes per packet. */
      unsigned int unitBytesPerPacket;
      /** The maximum number of bytes per packet. */
      unsigned int maximumBytesPerPacket;
      /** The recommended number of bytes per packet. */
      unsigned int recommendedBytesPerPacket;
      /** The number of bytes per isochronous packet. */
      unsigned int bytesPerPacket;
      /** The number of packets per frame. */
      unsigned int packetsPerFrame;
    };
  private:
    
    /** IEEE-1394 adapter. */
    mutable IEEE1394 adapter;
    /** Isochronous read channel. */
    IEEE1394::IsochronousReadChannel readChannel;
    /** The identifier of the camera. */
    EUI64 guid;
    /** The node id of the camera. */
    unsigned short camera;
    /** The specification implemented by the camera. */
    uint32 specification;

    /** The name of the vendor. */
    String vendorName;
    /** The model of the camera. */
    String modelName;
    /** The base address of the command registers. */
    uint64 commandRegisters;
    /** Specifies the supported formats. */
    unsigned int formats;
    /** Capabilities. */
    unsigned int capabilities;
    /** Offset to advanced feature registers. */
    uint64 advancedFeatureAddress;
    /** Specifies the available modes. */
    bool supportedModes[NUMBER_OF_MODES];
    /** The supported frame rates for each mode. */
    unsigned int frameRates[NUMBER_OF_MODES];
    
    /** Offset to the command and status registers of the partial image modes. */
    unsigned int partialImageModeOffset[8];
    /** Mode descriptions. */
    ModeDescriptor partialImageMode[8];
    /** Specifies the available features for the current mode. */
    unsigned int features;
    
    struct {
      GenericFeatureDescriptor brightness;
      GenericFeatureDescriptor autoExposure;
      GenericFeatureDescriptor sharpness;
      GenericFeatureDescriptor whiteBalance;
      GenericFeatureDescriptor hue;
      GenericFeatureDescriptor saturation;
      GenericFeatureDescriptor gamma;
      GenericFeatureDescriptor shutter;
      GenericFeatureDescriptor gain;
      GenericFeatureDescriptor iris;
      GenericFeatureDescriptor focus;
      GenericFeatureDescriptor temperature;
      TriggerFeatureDescriptor trigger;
      GenericFeatureDescriptor zoom;
      GenericFeatureDescriptor pan;
      GenericFeatureDescriptor tilt;
      GenericFeatureDescriptor opticalFilter;
      GenericFeatureDescriptor captureSize;
      GenericFeatureDescriptor captureQuality;
    } featureDescriptors;

    /** Specifies the current mode. */
    Mode currentMode;
    /** The current frame rate. */
    FrameRate frameRate;
    /** Description of the current mode. */
    ModeDescriptor mode;
    /** The region of the current mode. */
    Region region;
    /** The current pixel format. */
    PixelFormat pixelFormat;
    /** Isochronous transmission paramters. */
    IsochronousTransmission transmission;
    
    /**
      Returns the value of the specified command register in native byte order.

      @param offset The offset of the register.
    */
    inline uint32 getCommandRegister(
      uint32 offset) const throw(IEEE1394Exception) {
      IEEE1394::Quadlet quadlet;
      adapter.read(
        camera,
        commandRegisters + offset,
        Cast::getAddress(quadlet),
        sizeof(quadlet)
      );
      return quadlet;
    }
    
    /**
      Sets the value of the specified command register in native byte order.

      @param offset The offset of the register.
      @param value The desired value of the register.
    */
    inline void setCommandRegister(
      uint32 offset, uint32 value) throw(IEEE1394Exception) {
      IEEE1394::Quadlet quadlet = value;
      adapter.write(
        camera,
        commandRegisters + offset,
        Cast::getAddress(quadlet),
        sizeof(quadlet)
      );
    }

    /**
      Reads the mode specific state of the camera.
    */
    void readModeSpecificState() throw(IEEE1394Exception);

    /**
      Sets the value of the specified feature.
    */
    void setGenericFeature(Feature feature, const GenericFeatureDescriptor& descriptor, int value) throw(NotSupported, OutOfRange, IEEE1394Exception);
  public:

    /**
      Returns a string representation of the format of the specified mode.
    */
    static String getFormatAsString(Mode mode) throw();

    /**
      Returns a string representation of the specified mode.
    */
    static String getModeAsString(Mode mode) throw();

    /**
      Returns the the frame rate as an unsigned integer scaled with 65536.
    */
    static unsigned int getFrameRateAsValue(FrameRate frameRate) throw();

    /**
      Returns the specified frame rate as the frame period in microseconds. The
      result is rounded up to nearest microsecond and scaled with 15.
    */
    static inline unsigned int getFramePeriod(FrameRate rate) throw() {
      unsigned int value = getFrameRateAsValue(rate);
      if (value == 0) {
        return 0;
      } else {
        return (15 * 65536 * 1000000ULL + value - 1)/value;
      }
    }
 
    /**
      Initializes the camera object with the specified IEEE 1394 adapter.
    */
    Camera1394() throw(); // TAG: fixme
    
    /**
      Returns true if the specified node is a camera.
    */
    bool isCamera(unsigned int node) throw(OutOfDomain, IEEE1394Exception);
    
    /**
      Returns true if the specified device is available and is a camera.
    */
    bool isCamera(const EUI64& guid) throw(Camera1394Exception, IEEE1394Exception);
    
    /**
      Returns the available cameras.
    */
    Array<EUI64> getCameras() throw(IEEE1394Exception);
    
    /**
      Opens a connection to the specified camera.

      @param guid The identifier of the camera device.
    */
    void open(const EUI64& guid) throw(Camera1394Exception, IEEE1394Exception);
    
    /**
      Closes the connection to the camera.
    */
    void close() throw();
    
    /**
      Returns the camera specification implemented by the camera (e.g. "1.30").
      The version of the specification is represented as a 32 bit integer with
      the major, minor, and micro version numbers stored as octets in order of
      significance (e.g. 1.30 is represented as 0x00011e00).
      
      @see Specification
    */
    inline uint32 getSpecification() const throw() {
      return specification;
    }
    
    /**
      Returns the vendor of the camera.
    */
    String getVendorName() const throw();

    /**
      Returns the model of the camera.
    */
    String getModelName() const throw();

    /**
      Returns the base address of the command registers.
    */
    inline uint64 getCommandRegisters() const throw() {
      return commandRegisters;
    }
    
    /**
      Returns the capabilities of the camera.
    */
    inline unsigned int getCapabilities() const throw() {
      return capabilities;
    }

    /**
      Returns the address of the advanced feature register.
      
      @return 0 if advanced features are not supported.
    */
    inline uint64 getAdvancedFeatureAddress() const throw() {
      return advancedFeatureAddress;
    }
    
    /**
      Resets the camera to the factory default.
    */
    void reset() throw(IEEE1394Exception);

    /**
      Returns true if the feature is supported.
    */
    inline bool isFormatSupported(Format format) const throw() {
      return formats & (1 << format);
    }
    
    /**
      Returns true if the mode is supported.
    */
    inline bool isModeSupported(Mode mode) const throw() {
      return supportedModes[mode];
    }
    
    /**
      Returns true if the frame rate is supported.
    */
    bool isFrameRateSupported(FrameRate frameRate) const throw();
    
    /**
      Returns true if the specified feature is supported.
    */
    inline bool isFeatureSupported(Feature feature) const throw() {
      return features & (1 << feature);
    }
    
    /**
      Returns true if the specified feature is readable (implies supported if true).
    */
    bool isFeatureReadable(Feature feature) const throw();
    
    /**
      Returns the description of the specified feature. Use
      getTriggerFeatureDescriptor for the trigger feature.
    */
    const GenericFeatureDescriptor& getFeatureDescriptor(Feature feature) const throw(OutOfDomain);

    /**
      Returns a description of the trigger feature.
    */
    const TriggerFeatureDescriptor& getTriggerFeatureDescriptor() const throw();
    
    /**
      Returns the current mode.
    */
    inline Mode getMode() const throw() {
      return currentMode;
    }
    
    /**
      Sets the mode.
    */
    void setMode(Mode mode) throw(NotSupported, IEEE1394Exception);

    /**
      Returns the frame rates supported by the specified mode.

      @return 0 if supported frame rates are unknown.
    */
    unsigned int getFrameRates(Mode mode) throw(NotSupported);
    
    /**
      Returns the current frame rate.
    */
    inline FrameRate getFrameRate() const throw() {
      return frameRate;
    }
    
    /**
      Sets the frame rate.
    */
    void setFrameRate(FrameRate frameRate) throw(NotSupported);

    /**
      Returns the current isochronous channel.
    */
    inline unsigned int getIsochronousChannel() const throw() {
      return transmission.subchannel;
    }

    /**
      Returns the current isochronous transmission speed.
    */
    inline unsigned int getIsochronousSpeed() const throw() {
      return transmission.speed;
    }
    
    /**
      Power up camera.
    */
    void enable() throw(IEEE1394Exception);
    
    /**
      Power down camera.
    */
    void disable() throw(IEEE1394Exception);
    
    /**
      Returns true if the camera is powered up.
    */
    bool isUpAndRunning() const throw(IEEE1394Exception);
    
    /**
      Returns the operating mode of the specified feature.
      
      @param feature The feature to query.
    */
    FeatureOperatingMode getFeatureOperatingMode(Feature feature) const throw(NotSupported);

    /**
      Sets the operating mode of the specified feature.
      
      @param feature The feature to set.
      @param mode The desired mode.
    */
    void setFeatureOperatingMode(Feature feature, FeatureOperatingMode mode) throw(NotSupported);

    /**
      Returns true if the feature is working properly.
    */
    bool getFeatureStatus(Feature feature) throw(IEEE1394Exception);

    /**
      Returns the current brightness level.
    */
    int getBrightness() const throw(NotSupported, IEEE1394Exception);
    
    /**
      Sets the brightness level.
    */
    void setBrightness(int value) throw(NotSupported, OutOfRange, IEEE1394Exception);

    /**
      Returns the current auto exposure level.
    */
    int getAutoExposure() const throw(NotSupported, IEEE1394Exception);

    /**
      Sets the auto exposure level.
    */
    void setAutoExposure(int value) throw(NotSupported, OutOfRange, IEEE1394Exception);

    /**
      Returns the current sharpness level.
    */
    int getSharpness() const throw(NotSupported, IEEE1394Exception);

    /**
      Sets the sharpness level.
    */
    void setSharpness(int value) throw(NotSupported, OutOfRange, IEEE1394Exception);

    /**
      Returns the current white balance level.
    */
    int getWhiteBalanceBlueRatio() const throw(NotSupported, IEEE1394Exception);
    
    /**
      Returns the current white balance level.
    */
    int getWhiteBalanceRedRatio() const throw(NotSupported, IEEE1394Exception);
    
    /**
      Sets the white balance level.
    */
    void setWhiteBalance(int blueRatio, int redRatio) throw(NotSupported, OutOfRange, IEEE1394Exception);

    /**
      Returns the current hue level.
    */
    int getHue() const throw(NotSupported, IEEE1394Exception);

    /**
      Sets the hue level.
    */
    void setHue(int value) throw(NotSupported, OutOfRange, IEEE1394Exception);
    
    /**
      Returns the current saturation level.
    */
    int getSaturation() const throw(NotSupported, IEEE1394Exception);

    /**
      Sets the saturation level.
    */
    void setSaturation(int value) throw(NotSupported, OutOfRange, IEEE1394Exception);

    /**
      Returns the current gamma level.
    */
    int getGamma() const throw(NotSupported, IEEE1394Exception);

    /**
      Sets the gamma level.
    */
    void setGamma(int value) throw(NotSupported, OutOfRange, IEEE1394Exception);

    /**
      Returns the current shutter integration time.
    */
    int getShutter() const throw(NotSupported, IEEE1394Exception);

    /**
      Sets the shutter integration time.
    */
    void setShutter(int value) throw(NotSupported, OutOfRange, IEEE1394Exception);

    /**
      Returns the current gain level.
    */
    int getGain() const throw(NotSupported, IEEE1394Exception);

    /**
      Sets the gain level.
    */
    void setGain(int value) throw(NotSupported, OutOfRange, IEEE1394Exception);

    /**
      Returns the current IRIS level.
    */
    int getIRIS() const throw(NotSupported, IEEE1394Exception);

    /**
      Sets the IRIS level.
    */
    void setIRIS(int value) throw(NotSupported, OutOfRange, IEEE1394Exception);

    /**
      Returns the current focus level.
    */
    int getFocus() const throw(NotSupported, IEEE1394Exception);

    /**
      Sets the focus level.
    */
    void setFocus(int value) throw(NotSupported, OutOfRange, IEEE1394Exception);

    /**
      Returns the current temperature level.
    */
    int getTemperature() const throw(NotSupported, IEEE1394Exception);
    
    /**
      Returns the target temperature.
    */
    int getTargetTemperature() const throw(NotSupported, IEEE1394Exception);
    
    /**
      Sets the temperature level.
    */
    void setTemperature(int value) throw(NotSupported, OutOfRange, IEEE1394Exception);

    /**
      Returns the current zoom level.
    */
    int getZoom() const throw(NotSupported, IEEE1394Exception);
    
    /**
      Sets the zoom level.
    */
    void setZoom(int value) throw(NotSupported, OutOfRange, IEEE1394Exception);

    /**
      Returns the current pan level.
    */
    int getPan() const throw(NotSupported, IEEE1394Exception);
    
    /**
      Sets the pan level.
    */
    void setPan(int value) throw(NotSupported, OutOfRange, IEEE1394Exception);

    /**
      Returns the current tilt level.
    */
    int getTilt() const throw(NotSupported, IEEE1394Exception);
    
    /**
      Sets the tilt level.
    */
    void setTilt(int value) throw(NotSupported, OutOfRange, IEEE1394Exception);
    
    /**
      Returns the current optical filter level.
    */
    int getOpticalFilter() const throw(NotSupported, IEEE1394Exception);

    /**
      Sets the optical filter level.
    */
    void setOpticalFilter(int value) throw(NotSupported, OutOfRange, IEEE1394Exception);
    
    /**
      Returns the current capture size.
    */
    int getCaptureSize() const throw(NotSupported, IEEE1394Exception);
    
    /**
      Sets the capture size.
    */
    void setCaptureSize(int value) throw(NotSupported, OutOfRange, IEEE1394Exception);
    
    /**
      Returns the current capture quality.
    */
    int getCaptureQuality() const throw(NotSupported, IEEE1394Exception);

    /**
      Sets the capture quality.
    */
    void setCaptureQuality(int value) throw(NotSupported, OutOfRange, IEEE1394Exception);
    
    
    
    /**
      Returns the maximum dimension for the specified mode.
    */
    Dimension getMaximumDimension(Mode mode) const throw(NotSupported);
    
    /**
      Returns the unit dimension for the specified mode.
    */
    Dimension getUnitDimension(Mode mode) const throw(NotSupported);
    
    /**
      Returns the unit offset for the specified mode.
    */
    Point2D getUnitOffset(Mode mode) const throw(NotSupported);
    
    /**
      Returns the pixel formats supported by the specified mode.
    */
    unsigned int getPixelFormats(Mode mode) const throw(NotSupported);
      
    /**
      Returns the pixel formats supported by the current mode.
    */
    inline unsigned int getPixelFormats() const throw() {
      return mode.pixelFormats;
    }

    /**
      Returns the selected dimension.
    */
    inline const Dimension& getDimension() const throw() {
      return region.getDimension();
    }

    /**
      Returns the selected offset.
    */
    inline const Point2D& getOffset() const throw() {
      return region.getOffset();
    }

    /**
      Returns the selected region.
    */
    inline const Region& getRegion() const throw() {
      return region;
    }

    /**
      Sets the region.
    */
    void setRegion(const Region& region) throw(OutOfDomain, IEEE1394Exception);
    
    /**
      Returns the current pixel format.
    */
    inline PixelFormat getPixelFormat() const throw() {
      return pixelFormat;
    }

    /**
      Returns the effective number of bits per pixel.
    */
    static inline unsigned int getBitsPerPixel(PixelFormat pixelFormat) throw() {
      static unsigned int BITS_PER_PIXEL[] = {
        8, // Y_8BIT
        16, // Y_16BIT
        12, // YUV_411_8BIT
        16, // YUV_422_8BIT
        16, // YUV_444_8BIT
        24, // RGB_8BIT
        48 // RGB_16BIT
      };
      return BITS_PER_PIXEL[pixelFormat];
    }

    /**
      Sets the pixel format. The is only required if the mode supports multiple
      pixel formats.
    */
    void setPixelFormat(PixelFormat pixelFormat) throw(NotSupported, IEEE1394Exception);

    /**
      Returns the isochronous transmission parameters.
    */
    const IsochronousTransmission& getTransmission() const throw() {
      return transmission;
    }

    /**
      Acquires one frame.
      
      @return True if acquisition succeeded.
    */
    bool acquire(
      uint8* buffer,
      unsigned int size) throw(ImageException, IEEE1394Exception);
    
    /**
      Acquires a single frame. The current mode must support the 8 bit mono
      pixel format. Implicit switch to Y_8BIT pixel format if supported.
      ImageException is raised if the dimension of the frame is not equal to
      the current region dimension.

      @return True if acquisition succeeded.
    */
    bool acquire(
      ArrayImage<uint8>& frame)
      throw(NotSupported, ImageException, IEEE1394Exception);
    
    /**
      Acquires a single frame. The current mode must support the 16 bit mono
      pixel format. Implicit switch to Y_16BIT pixel format if supported.
    */
    bool acquire(
      ArrayImage<uint16>& frame)
      throw(NotSupported, ImageException, IEEE1394Exception);
    
    struct RGB24Pixel {
      uint8 red;
      uint8 green;
      uint8 blue;
    } _DK_SDU_MIP__BASE__PACKED;
    
    /**
      Acquires a single frame in RGB 24 bit format. The current mode must
      support the 24 bit RGB format. Implicit switch to RGB_8BIT pixel format
      if supported.
      
      @return True if acquisition succeeded.
    */
    bool acquire(
      ArrayImage<RGB24Pixel>& frame)
      throw(NotSupported, ImageException, IEEE1394Exception);

    /**
      Interface implemented by camera acquisition listeners.
      
      @short Acquisition listener.
    */
    class AcquisitionListener {
    public:

      /**
        Invoked on successful acquisition.

        @param frame The index of the frame within the cyclic buffer.
        @param buffer The image buffer.
        
        @return True if acquisition should continue.
      */
      virtual bool onAcquisition(unsigned int frame, uint8* buffer) throw() = 0;
      
      /**
        Invoked if synchronization with frame beginning is lost. This method
        returns true by default.
        
        @param frame The index of the frame within the cyclic buffer.

        @return True if acquisition should continue.
      */
      virtual bool onAcquisitionLostSync(unsigned int frame) throw();
      
      /**
        Invoked on acquisition failure. This method returns true by default.
        
        @param frame The index of the frame within the cyclic buffer.
        
        @return True if acquisition should continue.
      */
      virtual bool onAcquisitionFailure(unsigned int frame) throw();
    };

    /**
      Frame buffer descriptor for continuous acquisition.

      @short Frame buffer.
    */
    class FrameBuffer {
    private:
      
      /** The frame buffer. */
      uint8* buffer;
      /** The size of the frame buffer. */
      unsigned int size;
    public:

      /**
        Initializes frame buffer as invalid.
      */
      inline FrameBuffer() throw() : buffer(0), size(0) {
      }
      
      /**
        Initializes the buffer descriptor.
        
        @param buffer The buffer.
        @param size The number of bytes in the buffer.
      */
      inline FrameBuffer(uint8* _buffer, unsigned int _size) throw()
        : buffer(_buffer), size(_size) {
      }
      
      /**
        Returns the buffer.
      */
      inline uint8* getBuffer() const throw() {
        return buffer;
      }

      /**
        Returns the buffer size.
      */
      inline unsigned int getSize() const throw() {
        return size;
      }
    };
    
    /**
      Acquire frame continuously into the specified frames in a round-robin
      loop.

      @param frames The frame buffer.
      @param listener The acquisition listener to be notifier on completion of
      each frame.
    */
    bool acquireContinuously(
      Array<FrameBuffer> frames,
      AcquisitionListener* listener)
      throw(NotSupported, ImageException, Camera1394Exception, IEEE1394Exception);

    /**
      Convert the frame of the specified pixel format to a GrayImage.
    */
    static void convert(
      GrayImage& image,
      PixelFormat pixelFormat,
      const uint8* buffer) throw(ImageException);

    /**
      Convert the frame of the specified pixel format to a ColorImage.
    */
    static void convert(
      ColorImage& image,
      PixelFormat pixelFormat,
      const uint8* buffer) throw(ImageException);
  };

}; // end of gip namespace
