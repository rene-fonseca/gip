/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    Copyright (C) 2001-2002 by Rene Moeller Fonseca <fonseca@mip.sdu.dk>

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#ifndef _DK_SDU_MIP__GIP_MEDIA__AVI_ENCODER_H
#define _DK_SDU_MIP__GIP_MEDIA__AVI_ENCODER_H

#include <gip/gip.h>
#include <base/Object.h>
#include <base/string/String.h>
#include <base/io/File.h>
#include <base/Primitives.h>
#include <base/mem/Allocator.h>
#include <base/mem/ReferenceCountedAllocator.h>

namespace gip {

  typedef unsigned char byte;

  class MovieDescription : public Object {
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
    @short AVI encoder
    @author Rene Moeller Fonseca <fonseca@mip.sdu.dk>
    @version 1.0
  */

  class AVIEncoder : public Object {
  private:

    String filename;
  public:

    /** Compression formats. */
    class Compression {
    public:

      /** Uncompressed image. */
      static const unsigned int RGB = 0;
      /** Run-length encoded (RLE) 8-bit format. */
      static const unsigned int RLE8 = 1;
      /** Run-length encoded (RLE) 4-bit format. */
      static const unsigned int RLE4 = 2;
    };

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



  class AVIReader : public Object {
  public:

    typedef enum {RGB, RLE8, RLE4} FrameCompression;

    /**
      This class describes the whole AVI file.
      
      @short Description of the AVI file.
      @author Rene Moeller Fonseca <fonseca@mip.sdu.dk>
      @version 1.0
    */
    class GlobalDescriptor : public Object {
      friend class AVIReader;
    private:

      /** The period between the frames. */
      unsigned int microSecPerFrame;
      /** Approx. maximum data rate. */
      unsigned int maxBytesPerSec;
      /** NOT USED. */
      unsigned int paddingGranularity;
      /** Specifies that the AVI file has an index at the end. */
      bool hasIndex;
      /** Must use index. */
      bool mustUseIndex;
      /** Specifies htat the streams are interleaved. */
      bool isInterleaved;
      /** Use CKType to find key frames. */
      bool trustCKType;
      /** . */
      bool wasCaptureFile;
      /** Specifies that the AVI file contains copyrighted data. */
      bool copyrighted;
      /** The total number of frames. */
      unsigned int totalFrames;
      /** Number of frames prior to the initial frame. */
      unsigned int initialFrames;
      /** The number of streams within the object. */
      unsigned int streams;
      /** Suggested buffer size. */
      unsigned int suggestedBufferSize;
      /** Specifies the dimension of the AVI file. */
      Dimension dimension;
      /** Scale. */
      unsigned int scale;
      /** Rate. */
      unsigned int rate;
      /** The starting time of the AVI file. */
      unsigned int start;
      /** The length of the AVI file. */
      unsigned int length;
    public:

      inline unsigned int getTotalNumberOfFrames() const throw() {
        return totalFrames;
      }
    };

    /**
      This class specifies the format of the video stream data.
      
      @short Format of the video stream data
      @version 1.0
    */
    class VideoStreamDescriptor : public Object {
      friend class AVIReader;
    public:

      enum {RLE};
    private:

      /** Specifies the installable compressor or decompressor. */
      unsigned int handler;
      /** Specifies that the stream is disabled by default. */
      bool disabled;
      /** Specifies that the palette must be animated due to palette changes. */
      bool animatePalette;
      /** Specifies the priority of the stream. */
      unsigned int priority;
      //  unsigned int initialFrames;
      /** Used to calculate the framerate (=rate/scale). */
      unsigned int scale;
      /** Used to calculate the framerate (=rate/scale). */
      unsigned int rate;
      /** Specifies the starting time of the stream in units specified by 'scale' and 'rate'. */
      unsigned int start;
      /** Specifies the length of the stream in units specified by 'scale' and 'rate'. */
      unsigned int length;
      /** Specifies how large a buffer should be used to read this stream. */
      unsigned int suggestedBufferSize;
      /**
        Specifies an indicator of the quality of the data in the stream.
        Quality is represented as a number between 0 and 10,000. For compressed
        data, this typically represents the value of the quality parameter
        passed to the  compression software. If set to 1, drivers use the
        default quality value.
      */
      unsigned int quality;
      /**
        Specifies the size of a single sample of data. This is set to zero if
        the samples can vary in size. If this number is nonzero, then multiple
        samples of data can be grouped into a single chunk within the file. If
        it is zero, each sample of data (such as a video frame) must be in a
        separate chunk. For video streams, this number is typically zero,
        although it can be nonzero if all video frames are the same size.
      */
      unsigned int sampleSize;

      // frame section
      unsigned int width;
      unsigned int height;
      unsigned int planes;
      unsigned int bitsPerPixel;
      FrameCompression compression;
      unsigned int sizeImage;
      unsigned int xPelsPerMeter;
      unsigned int yPelsPerMeter;
      unsigned int colorUsed;
      unsigned int colorImportant;
    public:

    };

  private:

    /** The AVI file. */
    File file;
    /** Valid. */
    bool valid;
    unsigned int frameIndex;
    GlobalDescriptor globalDescriptor;
    VideoStreamDescriptor videoStreamDescriptor;
    /** Contains data used by the decoder. */
    Allocator<byte> streamData;
    /** Identifies the video stream within the AVI file. */
    int videoStreamIndex;
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

#endif
