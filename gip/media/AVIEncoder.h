/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    See COPYRIGHT.txt for details.

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#pragma once

#include <gip/gip.h>
#include <base/Object.h>
#include <base/string/String.h>
#include <base/io/File.h>
#include <base/Primitives.h>
#include <base/mem/Allocator.h>
#include <base/mem/ReferenceCountedAllocator.h>

namespace gip {

  typedef unsigned char byte;

  class _COM_AZURE_DEV__BASE__API MovieDescription : public Object {
  public:

    /**
      Returns the dimension of the frames within the movie.
    */
    virtual Dimension getDimension() const throw() = 0;

    /**
      Returns the number of frames within the movie.
    */
    virtual unsigned int getNumberOfFrames() const throw() = 0;
  };



  /**
    @short AVI encoder.
    @version 1.0
  */

  class _COM_AZURE_DEV__BASE__API AVIEncoder : public Object {
  private:

    String filename;
  public:

    /* Compression formats. */
    class Compression {
    public:

      /** Uncompressed image. */
      static const unsigned int RGB = 0;
      /** Run-length encoded (RLE) 8-bit format. */
      static const unsigned int RLE8 = 1;
      /** Run-length encoded (RLE) 4-bit format. */
      static const unsigned int RLE4 = 2;
    };

    /**
      Initializes AVI encoder.

      @param filename The filename.
    */
    AVIEncoder(const String& filename) throw(IOException);

    /**
      Returns a description of the encoder.
    */
    String getDescription() const throw();

    /**
      Returns the default extension.
    */
    String getDefaultExtension() const throw();

    /**
      Returns true if the format is valid.
    */
    bool isValid() throw(IOException);

    /**
      Reads a color image from the stream.

      @param stream The input stream.
    */
    ArrayImage<ColorPixel>* read() throw(IOException);

    /**
      Writes the specified image to the stream.

      @param stream The output stream.
      @param image The image to be written.
    */
    void write(const ArrayImage<ColorPixel>* image) throw(IOException);

    /**
      Returns a description of the object.
    */
    FormatOutputStream& getInfo(FormatOutputStream& stream) throw(IOException);

    /**
      Destroys the encoder.
    */
    ~AVIEncoder();
  };



  class _COM_AZURE_DEV__BASE__API AVIReader : public Object {
  public:

    typedef enum {RGB, RLE8, RLE4} FrameCompression;

    /*
      This class describes the whole AVI file.
      
      @short Description of the AVI file.
      @version 1.0
    */
    class _COM_AZURE_DEV__BASE__API GlobalDescriptor : public Object {
      friend class AVIReader;
    private:

      /** The period between the frames. */
      unsigned int microSecPerFrame = 0;
      /** Approx. maximum data rate. */
      unsigned int maxBytesPerSec = 0;
      /** NOT USED. */
      unsigned int paddingGranularity = 0;
      /** Specifies that the AVI file has an index at the end. */
      bool hasIndex = false;
      /** Must use index. */
      bool mustUseIndex = false;
      /** Specifies htat the streams are interleaved. */
      bool isInterleaved = false;
      /** Use CKType to find key frames. */
      bool trustCKType = false;
      /** Capture file. */
      bool wasCaptureFile = false;
      /** Specifies that the AVI file contains copyrighted data. */
      bool copyrighted = false;
      /** The total number of frames. */
      unsigned int totalFrames = 0;
      /** Number of frames prior to the initial frame. */
      unsigned int initialFrames = 0;
      /** The number of streams within the object. */
      unsigned int streams = 0;
      /** Suggested buffer size. */
      unsigned int suggestedBufferSize = 0;
      /** Specifies the dimension of the AVI file. */
      Dimension dimension;
      /** Scale. */
      unsigned int scale = 0;
      /** Rate. */
      unsigned int rate = 0;
      /** The starting time of the AVI file. */
      unsigned int start = 0;
      /** The length of the AVI file. */
      unsigned int length = 0;
    public:

      inline unsigned int getTotalNumberOfFrames() const throw() {
        return totalFrames;
      }
    };

    /*
      This class specifies the format of the video stream data.
      
      @short Format of the video stream data.
      @version 1.0
    */
    class VideoStreamDescriptor : public Object {
      friend class AVIReader;
    public:

      enum {RLE};
    private:

      /** Specifies the installable compressor or decompressor. */
      unsigned int handler = 0;
      /** Specifies that the stream is disabled by default. */
      bool disabled = false;
      /** Specifies that the palette must be animated due to palette changes. */
      bool animatePalette = false;
      /** Specifies the priority of the stream. */
      unsigned int priority = 0;
      //  unsigned int initialFrames = 0;
      /** Used to calculate the framerate (=rate/scale). */
      unsigned int scale = 0;
      /** Used to calculate the framerate (=rate/scale). */
      unsigned int rate = 0;
      /** Specifies the starting time of the stream in units specified by 'scale' and 'rate'. */
      unsigned int start = 0;
      /** Specifies the length of the stream in units specified by 'scale' and 'rate'. */
      unsigned int length = 0;
      /** Specifies how large a buffer should be used to read this stream. */
      unsigned int suggestedBufferSize = 0;
      /**
        Specifies an indicator of the quality of the data in the stream.
        Quality is represented as a number between 0 and 10,000. For compressed
        data, this typically represents the value of the quality parameter
        passed to the  compression software. If set to 1, drivers use the
        default quality value.
      */
      unsigned int quality = 0;
      /**
        Specifies the size of a single sample of data. This is set to zero if
        the samples can vary in size. If this number is nonzero, then multiple
        samples of data can be grouped into a single chunk within the file. If
        it is zero, each sample of data (such as a video frame) must be in a
        separate chunk. For video streams, this number is typically zero,
        although it can be nonzero if all video frames are the same size.
      */
      unsigned int sampleSize = 0;

      // frame section
      unsigned int width = 0;
      unsigned int height = 0;
      unsigned int planes = 0;
      unsigned int bitsPerPixel = 0;
      FrameCompression compression = RGB;
      unsigned int sizeImage = 0;
      unsigned int xPelsPerMeter = 0;
      unsigned int yPelsPerMeter = 0;
      unsigned int colorUsed = 0;
      unsigned int colorImportant = 0;
    public:

    };

  private:

    /** The AVI file. */
    File file;
    /** Valid. */
    bool valid = false;
    unsigned int frameIndex = 0;
    GlobalDescriptor globalDescriptor;
    VideoStreamDescriptor videoStreamDescriptor;
    /** Contains data used by the decoder. */
    Allocator<byte> streamData;
    /** Identifies the video stream within the AVI file. */
    int videoStreamIndex = 0;
    /** Read buffer. */
    Allocator<byte> buffer;
    /** Palette (not). */
    Allocator<ColorPixel> palette;
    typedef enum {UNCOMPRESSED, COMPRESSED} FrameType;
  protected:

    /**
      Analyses the AVI file.
    */
    void analyse() throw(IOException);
    
    /**
      @param frame The image to receive the elements.
      @param src The encoded frame data.
      @param size The size of the encoded frame data.
      @param type The encoding.
    */
    void decodeFrame(ColorImage& frame, const byte* src, unsigned int size, FrameType type) throw();
  public:

    /**
      Initializes the frame reader object.

      @param filename The name of the AVI file.
    */
    AVIReader(const String& filename) throw(IOException);

    /**
      Fills the specified image with the next frame and advances the position.
    */
    void getFrame(ColorImage& frame) throw(IOException);

    /**
      Returns the dimension of the video stream.
    */
    Dimension getDimension() const throw() {
      return Dimension(
        videoStreamDescriptor.width,
        videoStreamDescriptor.height
      );
    }

    /**
      Returns the total number of frames within the stream.
    */
    inline unsigned int getNumberOfFrames() const throw() {
      return globalDescriptor.totalFrames;
    }

    /**
      Returns the current frame index.
    */
    inline unsigned int getPosition() const throw() {
      return frameIndex;
    }

    /**
      Returns true if the AVI file is valid.
    */
    inline bool isValid() const throw() {
      return valid;
    }
  };

}; // end of gip namespace
