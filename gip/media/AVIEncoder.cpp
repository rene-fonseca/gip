/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    Copyright (C) 2001-2003 by Rene Moeller Fonseca <fonseca@mip.sdu.dk>

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#include <gip/media/AVIEncoder.h>
#include <base/mem/Allocator.h>
#include <base/ByteOrder.h>
#include <base/string/InvalidFormat.h>

namespace gip {

/**
  Character code identifying a chunk of data with an Audio/Video Interleaved (AVI) file.
*/
struct ChunkId { // TAG: fixme
  LittleEndian<uint32> id;
} _DK_SDU_MIP__BASE__PACKED;
  
// union ChunkId {
//   LittleEndian<uint32> id;
//   char chars[4];
// } _DK_SDU_MIP__BASE__PACKED;

/**
  Chunk of data (ie. ChunkId and size).
*/
struct Chunk {
  ChunkId id;
  LittleEndian<uint32> size;
} _DK_SDU_MIP__BASE__PACKED;

inline ChunkId makeChunkId(char a, char b, char c, char d) throw() {
  ChunkId result;
  result.id = d << 24 | c << 16 | b << 8 | a; // TAG: fixme
//   result.chars[0] = a;
//   result.chars[1] = b;
//   result.chars[2] = c;
//   result.chars[3] = d;
  return result;
}

FormatOutputStream& operator<<(
  FormatOutputStream& stream, const ChunkId& value) throw(IOException) {
  unsigned int temp = value.id;
  stream << static_cast<char>(temp)
         << static_cast<char>(temp >> 8)
         << static_cast<char>(temp >> 16)
         << static_cast<char>(temp >> 24);
//  stream << value.chars[0] << value.chars[1] << value.chars[2] << value.chars[3];
  return stream;
}

inline bool operator==(const ChunkId& left, const ChunkId& right) throw() {
  return left.id == right.id;
}

inline bool operator!=(const ChunkId& left, const ChunkId& right) throw() {
  return left.id != right.id;
}

int getStreamId(const ChunkId& value) throw() {
  int high = static_cast<int>(value.id & 0xff - '0');
  int low = static_cast<int>((value.id >> 8) & 0xff - '0');
//   int high = static_cast<int>(value.chars[0] - '0');
//   int low = static_cast<int>(value.chars[1] - '0');
  if ((high < 0) || (low < 0)) {
    return -1; // invalid
  }
  return high * 256 + low;
}

unsigned int getStreamType(const ChunkId& value) throw() {
  return static_cast<unsigned int>((value.id >> 16) & 0xff) << 8 + static_cast<unsigned int>((value.id >> 24) & 0xff);
//  return static_cast<unsigned int>(value.chars[2]) << 8 + static_cast<unsigned int>(value.chars[3]);
}

struct AVIHeader {
  LittleEndian<uint32> microSecPerFrame; // period between frames
  LittleEndian<uint32> maxBytesPerSec; // approx. maximum data rate
  LittleEndian<uint32> paddingGranularity;

  unsigned int reserved1 : 4;
  bool hasIndex : 1; // AVI file has index at end
  bool mustUseIndex : 1;
  unsigned int reserved2 : 2;
  bool isInterleaved : 1;
  unsigned int reserved3 : 2;
  bool trustCKType : 1; // Use CKType to find key frames
  unsigned int reserved4 : 4;
  bool wasCaptureFile : 1;
  bool copyrighted : 1;
  unsigned int reserved5 : 14;

  LittleEndian<uint32> totalFrames; // the total number of frames
  LittleEndian<uint32> initialFrames;  // number of frames prior to the initial frame
  LittleEndian<uint32> streams; // the number of streams within the object
  LittleEndian<uint32> suggestedBufferSize;
  LittleEndian<uint32> width;
  LittleEndian<uint32> height;
  LittleEndian<uint32> scale;
  LittleEndian<uint32> rate;
  LittleEndian<uint32> start; // the starting time of the AVI file
  LittleEndian<uint32> length; // the length of the AVI file
} _DK_SDU_MIP__BASE__PACKED;

struct AVIIndexEntry {
  LittleEndian<uint32> ckid;
  LittleEndian<uint32> flags;
  LittleEndian<uint32> chunkOffset;
  LittleEndian<uint32> chunkLength;
} _DK_SDU_MIP__BASE__PACKED;

struct AVIStreamHeader {
  ChunkId type;
  ChunkId handler;
  LittleEndian<uint32> flags;
  LittleEndian<uint32> priority;
  LittleEndian<uint32> initialFrames;
  LittleEndian<uint32> scale;
  LittleEndian<uint32> rate;
  LittleEndian<uint32> start;
  LittleEndian<uint32> length;
  LittleEndian<uint32> suggestedBufferSize;
  LittleEndian<uint32> quality;
  LittleEndian<uint32> sampleSize;
  LittleEndian<uint32> left;
  LittleEndian<uint32> top;
  LittleEndian<uint32> right;
  LittleEndian<uint32> bottom;
} _DK_SDU_MIP__BASE__PACKED;

struct BitmapInfoHeader {
  LittleEndian<uint32> size;
  LittleEndian<int32> width;
  LittleEndian<int32> height;
  LittleEndian<uint16> planes;
  LittleEndian<uint16> bitsPerPixel;
  LittleEndian<uint32> compression;
  LittleEndian<uint32> sizeImage;
  LittleEndian<int32> xPelsPerMeter;
  LittleEndian<int32> yPelsPerMeter;
  LittleEndian<uint32> colorUsed;
  LittleEndian<uint32> colorImportant;
} _DK_SDU_MIP__BASE__PACKED;

struct WaveFormatExtended {
  LittleEndian<uint16> formatTag;
  LittleEndian<uint16> channels;
  LittleEndian<uint32> samplesPerSec;
  LittleEndian<uint32> averageBytesPerSec;
  LittleEndian<uint16> blockAlign;
  LittleEndian<uint16> bitsPerSample;
  LittleEndian<uint16> size;
} _DK_SDU_MIP__BASE__PACKED;

struct AVIPaletteEntry {
  byte blue;
  byte green;
  byte red;
  byte reserved;
} _DK_SDU_MIP__BASE__PACKED;

struct AVIPaletteChange {
  byte firstEntry;
  byte numberOfEntries;
  LittleEndian<uint16> flags;
  AVIPaletteEntry entry[0];
} _DK_SDU_MIP__BASE__PACKED;

enum {BMP_RGB = 0, BMP_RLE8 = 1, BMP_RLE4 = 2, BMP_BITFIELDS = 3};

AVIEncoder::AVIEncoder(const String& _filename) throw(IOException)
  : filename(_filename) {
}

String AVIEncoder::getDescription() const throw() {
  return Literal("Microsoft Audio/Video Interleaved format");
}

String AVIEncoder::getDefaultExtension() const throw() {
  return Literal("avi");
}

bool AVIEncoder::isValid() throw(IOException) {
  return false;
}

ArrayImage<ColorPixel>* AVIEncoder::read() throw(IOException) {
  return 0;
}

void AVIEncoder::write(const ArrayImage<ColorPixel>* image) throw(IOException) {
}

FormatOutputStream& AVIEncoder::getInfo(
  FormatOutputStream& stream) throw(IOException) {
  stream << "AVIEncoder (Microsoft Audio/Video Interleaved format):" << EOL;

  {
    File file(filename, File::READ, 0);

    {
      Chunk chunk;
      file.read(Cast::getAddress(chunk), sizeof(chunk));
      ChunkId name;
      file.read(Cast::getAddress(name), sizeof(name));
      stream << indent(2) << chunk.id
             << "('" << name
             << "' chunk of size " << chunk.size << EOL;
      if ((chunk.id != makeChunkId('R', 'I', 'F', 'F')) || (name != makeChunkId('A', 'V', 'I', ' '))) {
        return stream << "INVALID FORMAT" << EOL;
      }
    }

    unsigned int listTotalSize;
    unsigned int listTotalRead = 0;
    {
      Chunk chunk;
      file.read(Cast::getAddress(chunk), sizeof(chunk));
      ChunkId name;
      file.read(Cast::getAddress(name), sizeof(name));
      stream << indent(2) << chunk.id << "('"
             << name << "' chunk of size " << chunk.size << EOL;
      if ((chunk.id != makeChunkId('L', 'I', 'S', 'T')) || (name != makeChunkId('h', 'd', 'r', 'l'))) {
        return stream << "INVALID FORMAT" << EOL;
      }
      listTotalSize = chunk.size;
    }

    ChunkId avih;
    file.read(Cast::getAddress(avih), sizeof(avih));
    LittleEndian<uint32> size;
    file.read(Cast::getAddress(size), sizeof(size));
    Allocator<uint8> buffer(maximum<unsigned int>(size, sizeof(AVIHeader)));
    fill<uint8>(buffer.getElements(), buffer.getSize(), 0);
    AVIHeader* header = (AVIHeader*)buffer.getElements();
    file.read(buffer.getElements(), size);
    stream << indent(6) << "'avih'(<Main AVI header>) of size " << size << EOL
           << indent(6) << " microSecPerFrame=" << header->microSecPerFrame << EOL
           << indent(6) << "maxBytesPerSec=" << header->maxBytesPerSec << EOL
           << indent(6) << "paddingGranularity=" << header->paddingGranularity << EOL
           << indent(6) << "hasIndex=" << header->hasIndex << EOL
           << indent(6) << "mustUseIndex=" << header->mustUseIndex << EOL
           << indent(6) << "isInterleaved=" << header->isInterleaved << EOL
           << indent(6) << "trustCKType=" << header->trustCKType << EOL
           << indent(6) << "wasCaptureFile=" << header->wasCaptureFile << EOL
           << indent(6) << "copyrighted=" << header->copyrighted << EOL
           << indent(6) << " totalFrames=" << header->totalFrames << EOL
           << indent(6) << "initialFrames=" << header->initialFrames << EOL
           << indent(6) << "streams=" << header->streams << EOL
           << indent(6) << "suggestedBufferSize=" << header->suggestedBufferSize << EOL
           << indent(6) << "width=" << header->width << EOL
           << indent(6) << "height=" << header->height << EOL
           << indent(6) << "scale=" << header->scale << EOL
           << indent(6) << "rate=" << header->rate << EOL
           << indent(6) << "start=" << header->start << EOL
           << indent(6) << "length=" << header->length << EOL;
    listTotalRead += size;

    for (unsigned int i = 0; i < header->streams; ++i) {
      Chunk list;
      file.read(Cast::getAddress(list), sizeof(list));
      ChunkId name;
      file.read(Cast::getAddress(name), sizeof(name));
      stream << indent(2) << list.id << "('"
             << name << "' chunk of size " << list.size << EOL;

      if ((list.id != makeChunkId('L', 'I', 'S', 'T')) || (name != makeChunkId('s', 't', 'r', 'l'))) {
        return stream << "  INVALID FORMAT" << EOL;
      }

      ChunkId str_;
      LittleEndian<uint32> size;
      unsigned int totalRead = sizeof(name);

      // read strh
      file.read(Cast::getAddress(str_), sizeof(str_));
      if (str_ != makeChunkId('s', 't', 'r', 'h')) {
        return stream << "Expected chunk id: strh" << EOL;
      }
      file.read(Cast::getAddress(size), sizeof(size));
      totalRead += sizeof(str_) + sizeof(size) + (size+1)/2*2;
      Allocator<uint8> headerBuffer(
        maximum<MemorySize>((size+1)/2*2, sizeof(AVIStreamHeader))
      );
      fill<uint8>(headerBuffer.getElements(), headerBuffer.getSize(), 0);
      AVIStreamHeader* header = (AVIStreamHeader*)headerBuffer.getElements();

      file.read(headerBuffer.getElements(), (size+1)/2*2);
      stream << "    '" << str_ << "'(<AVI stream header>) of size " << size << EOL
             << "      type=" << header->type << EOL
             << "      handler=" << header->handler << EOL
             << "      flags=" << HEX << header->flags << EOL
             << "      initialFrames=" << header->initialFrames << EOL
             << "      scale=" << header->scale << EOL
             << "      rate=" << header->rate << EOL
             << "      start=" << header->start << EOL
             << "      length=" << header->length << EOL
             << "      suggestedBufferSize=" << header->suggestedBufferSize << EOL
             << "      quality=" << header->quality << EOL
             << "      sampleSize=" << header->sampleSize << EOL;

      // read strf - BITMAPINFO for vids and WAVEFORMATEX for auds
      file.read(Cast::getAddress(str_), sizeof(str_));
      if (str_ != makeChunkId('s', 't', 'r', 'f')) {
        return stream << "Expected chunk id: strf" << EOL;
      }
      file.read(Cast::getAddress(size), sizeof(size));
      totalRead += sizeof(str_) + sizeof(size) + (size+1)/2*2;

      Allocator<uint8> streamFormatBuffer(
        maximum<MemorySize>(
          (size+1)/2*2,
          maximum<MemorySize>(
            sizeof(BitmapInfoHeader),
            sizeof(WaveFormatExtended)
          )
        )
      );
      fill<uint8>(streamFormatBuffer.getElements(), streamFormatBuffer.getSize(), 0);
      file.read(streamFormatBuffer.getElements(), (size+1)/2*2);

      if (header->type == makeChunkId('v', 'i', 'd', 's')) {
        stream << "    '" << str_ << "'(<BitmapInfo>) of size " << size << EOL;
        BitmapInfoHeader* header = (BitmapInfoHeader*)streamFormatBuffer.getElements();

        stream << "      size=" << header->size << EOL
               << "      width=" << header->width << EOL
               << "      height=" << header->height << EOL
               << "      planes=" << header->planes << EOL
               << "      bitsPerPixel=" << header->bitsPerPixel << EOL
               << "      compression=" << header->compression << EOL
               << "      sizeImage=" << header->sizeImage << EOL
               << "      xPelsPerMeter=" << header->xPelsPerMeter << EOL
               << "      yPelsPerMeter=" << header->yPelsPerMeter << EOL
               << "      colorUsed=" << header->colorUsed << EOL
               << "      colorImportant=" << header->colorImportant << EOL;

      } else if (header->type == makeChunkId('a', 'u', 'd', 's')) {
        stream << "    '" << str_ << "'(<WaveFormatExtended>) of size " << size << EOL;
        WaveFormatExtended* header = (WaveFormatExtended*)streamFormatBuffer.getElements();

        stream << "      formatTag=" << header->formatTag << EOL
               << "      channels=" << header->channels << EOL
               << "      samplesPerSec=" << header->samplesPerSec << EOL
               << "      averageBytesPerSec=" << header->averageBytesPerSec << EOL
               << "      blockAlign=" << header->blockAlign << EOL
               << "      bitsPerSample=" << header->bitsPerSample << EOL
               << "      size=" << header->size << EOL;

      } else {
        stream << "    '" << str_ << "'(<Unknown stream type>) of size " << size << EOL;
      }

      while (totalRead < list.size) {
        file.read(Cast::getAddress(str_), sizeof(str_));
        file.read(Cast::getAddress(size), sizeof(size));
        totalRead += sizeof(str_) + sizeof(size) + (size+1)/2*2;
        Allocator<uint8> buffer((size+1)/2*2);
        file.read(buffer.getElements(), (size+1)/2*2);

        if (str_ == makeChunkId('s', 't', 'r', 'n')) {
          stream << "    '" << str_ << "'(<chars>) of size " << size << EOL
                 << "      data=" << buffer.getElements() << EOL;
        } else {
          stream << "    '" << str_ << "'(<Unknown stream data>) of size " << size << EOL;
        }
      }

       // check if: totalRead == listChunk.size
      // listTotalSize += totalRead;
    }

    while (true) {
      if (file.getPosition() >= file.getSize()) {
        return stream << "INVALID FORMAT" << EOL;
      }
      Chunk chunk;
      file.read(Cast::getAddress(chunk), sizeof(chunk));
      if (chunk.id == makeChunkId('L', 'I', 'S', 'T')) {
        ChunkId name;
        file.read(Cast::getAddress(name), sizeof(name));
        unsigned int totalSize = chunk.size;
        unsigned int totalRead = sizeof(name);
        stream << "  LIST('" << name << "' chunk of size " << totalSize << ENDL;

        while (totalRead < totalSize) {
          Chunk dataChunk;
          file.read(Cast::getAddress(dataChunk), sizeof(dataChunk));
          totalRead += sizeof(dataChunk) + dataChunk.size;
          stream << "    chunk: {id=" << dataChunk.id << ";size=" << dataChunk.size << '}' << ENDL;
          file.setPosition(dataChunk.size, File::CURRENT); // skip JUNK and unknown chunks
        }

        break;
      } else {
        stream << "  chunk: {id=" << chunk.id << ";size=" << chunk.size << '}' << ENDL;
        file.setPosition(chunk.size, File::CURRENT); // skip JUNK and unknown chunks
      }
    }

//    if (header.hasIndex)
    while (true) {
      if (file.getPosition() >= file.getSize()) {
        return stream << "INVALID FORMAT" << EOL;
      }
      Chunk chunk;
      file.read(Cast::getAddress(chunk), sizeof(chunk));
      if (chunk.id == makeChunkId('i', 'd', 'x', '1')) {
        stream << "  chunk: {id=" << chunk.id << ";size=" << chunk.size << '}' << ENDL;
        file.setPosition(chunk.size, File::CURRENT); // skip chunk
        break;
      } else {
        stream << "  chunk: {id=" << chunk.id << ";size=" << chunk.size << '}' << ENDL;
        file.setPosition(chunk.size, File::CURRENT); // skip JUNK and unknown chunks
      }
    }

    stream << "  file position=" << file.getPosition() << EOL
           << "  file size=" << file.getSize() << EOL;
  }

  return stream;
}

AVIEncoder::~AVIEncoder() {
}






void AVIReader::analyse() throw(IOException) {
  {
    Chunk riff;
    file.read(Cast::getAddress(riff), sizeof(riff));
    bassert(riff.id == makeChunkId('R', 'I', 'F', 'F'), InvalidFormat(this));
    ChunkId name;
    file.read(Cast::getAddress(name), sizeof(name));
    bassert(name == makeChunkId('A', 'V', 'I', ' '), InvalidFormat(this));
    fout << "riff=" << riff.id << " size=" << riff.size << " name=" << name << ENDL;
  }

  unsigned int totalRead = 0;

  Chunk list;
  file.read(Cast::getAddress(list), sizeof(list));
  bassert(list.id == makeChunkId('L', 'I', 'S', 'T'), InvalidFormat(this));

  ChunkId name;
  bassert(totalRead + sizeof(name) < list.size, InvalidFormat(this));
  file.read(Cast::getAddress(name), sizeof(name));
  bassert(name == makeChunkId('h', 'd', 'r', 'l'), InvalidFormat(this));
  totalRead += sizeof(name);

  {
    Chunk avih;
    bassert(totalRead + sizeof(avih) < list.size, InvalidFormat(this));
    file.read(Cast::getAddress(avih), sizeof(avih));
    bassert(avih.id == makeChunkId('a', 'v', 'i', 'h'), InvalidFormat(this));
    Allocator<uint8> buffer(maximum<unsigned int>(avih.size, sizeof(AVIHeader)));
    fill<uint8>(buffer.getElements(), buffer.getSize(), 0);
    file.read(buffer.getElements(), (avih.size + 1)/2*2);
    totalRead += sizeof(avih) + (avih.size + 1)/2*2;
    AVIHeader* header = Cast::pointer<AVIHeader*>(buffer.getElements());

    // initialize global descriptor
    globalDescriptor.microSecPerFrame = header->microSecPerFrame;
    globalDescriptor.maxBytesPerSec = header->maxBytesPerSec;
    globalDescriptor.paddingGranularity = header->paddingGranularity;
    globalDescriptor.hasIndex = header->hasIndex;
    globalDescriptor.mustUseIndex = header->mustUseIndex;
    globalDescriptor.isInterleaved = header->isInterleaved;
    globalDescriptor.trustCKType = header->trustCKType;
    globalDescriptor.wasCaptureFile = header->wasCaptureFile;
    globalDescriptor.copyrighted = header->copyrighted;
    globalDescriptor.totalFrames = header->totalFrames;
    globalDescriptor.initialFrames = header->initialFrames;
    globalDescriptor.streams = header->streams;
    globalDescriptor.suggestedBufferSize = header->suggestedBufferSize;
    globalDescriptor.dimension = Dimension(header->width, header->height);
    globalDescriptor.scale = header->scale;
    globalDescriptor.rate = header->rate;
    globalDescriptor.start = header->start;
    globalDescriptor.length = header->length;
  }

  videoStreamIndex = -1; // no video stream has been found
  for (unsigned int streamIndex = 0; streamIndex < globalDescriptor.streams; ++streamIndex) { // read descriptions of all streams
    Chunk list;
//    bassert(totalRead + sizeof(chunk) < totalSize, InvalidFormat(this));
    file.read(Cast::getAddress(list), sizeof(list));
//    totalRead += sizeof(list);
    bassert(list.id == makeChunkId('L', 'I', 'S', 'T'), InvalidFormat(this));

    unsigned int totalRead = 0;
    unsigned int totalSize = list.size; // todo may need to +1/2*2

    ChunkId name;
    file.read(Cast::getAddress(name), sizeof(name));
    bassert(name == makeChunkId('s', 't', 'r', 'l'), InvalidFormat(this));
    totalRead += sizeof(name);

    Chunk chunk;
    enum {VIDEO, AUDIO, TEXT, UNKNOWN} streamType;

    // read strh
    {
      bassert(totalRead + sizeof(chunk) < totalSize, InvalidFormat(this));
      file.read(Cast::getAddress(chunk), sizeof(chunk));
      bassert(chunk.id == makeChunkId('s', 't', 'r', 'h'), InvalidFormat(this));
      unsigned int size = (chunk.size+1)/2*2;
      bassert(totalRead + size < totalSize, InvalidFormat(this));
      Allocator<uint8> buffer(
        maximum<MemorySize>(size, sizeof(AVIStreamHeader))
      );
      fill<uint8>(buffer.getElements(), buffer.getSize(), 0);
      file.read(buffer.getElements(), size);
      totalRead += sizeof(chunk) + size;

      AVIStreamHeader* header = (AVIStreamHeader*)buffer.getElements();
      if (header->type == makeChunkId('v', 'i', 'd', 's')) {
        streamType = VIDEO;
//      } else if (header->type == makeChunkId('a', 'u', 'd', 's')) {
//        streamType = AUDIO;
//      } else if (header->type == makeChunkId('t', 'e', 'x', 't')) {
//        streamType = TEXT;
      } else {
        streamType = UNKNOWN;
      }

      if (streamType == VIDEO) {
        if (videoStreamIndex < 0) { // only use first video stream
          videoStreamIndex = streamIndex;
//          videoStreamDescriptor.handler = header->handler;
//          videoStreamDescriptor.disabled = header->disabled;
//          videoStreamDescriptor.animatePalette = header->animatePalette;
          videoStreamDescriptor.priority = header->priority;
          videoStreamDescriptor.scale = header->scale;
          videoStreamDescriptor.rate = header->rate;
          videoStreamDescriptor.start = header->start;
          videoStreamDescriptor.length = header->length;
          videoStreamDescriptor.suggestedBufferSize = header->suggestedBufferSize;
          videoStreamDescriptor.quality = header->quality;
          videoStreamDescriptor.sampleSize = header->sampleSize;
        }
      }
    }

    // read strf
    bassert(totalRead + sizeof(chunk) < totalSize, InvalidFormat(this));
    file.read(Cast::getAddress(chunk), sizeof(chunk));
    bassert(chunk.id == makeChunkId('s', 't', 'r', 'f'), InvalidFormat(this));
    unsigned int size = (chunk.size+1)/2*2;
    totalRead += sizeof(chunk) + size;
    
    Allocator<uint8> buffer(
      maximum<MemorySize>(size, sizeof(BitmapInfoHeader))
    );
    fill<uint8>(buffer.getElements(), buffer.getSize(), 0);
    file.read(buffer.getElements(), size);

    if (streamType == VIDEO) {
      BitmapInfoHeader* header = (BitmapInfoHeader*)buffer.getElements();
      videoStreamDescriptor.width = header->width;
      videoStreamDescriptor.height = header->height;
      videoStreamDescriptor.planes = header->planes;
      videoStreamDescriptor.bitsPerPixel = header->bitsPerPixel;
      videoStreamDescriptor.sizeImage = header->sizeImage;
      videoStreamDescriptor.xPelsPerMeter = header->xPelsPerMeter;
      videoStreamDescriptor.yPelsPerMeter = header->yPelsPerMeter;
      videoStreamDescriptor.colorUsed = header->colorUsed;
      videoStreamDescriptor.colorImportant = header->colorImportant;

      switch (header->compression) {
      case AVIEncoder::Compression::RGB:
        videoStreamDescriptor.compression = RGB;
        bassert(
          (videoStreamDescriptor.planes == 1) && (videoStreamDescriptor.bitsPerPixel == 4) ||
          (videoStreamDescriptor.planes == 1) && (videoStreamDescriptor.bitsPerPixel == 8) ||
          (videoStreamDescriptor.planes == 3) && (videoStreamDescriptor.bitsPerPixel == 24) ||
          (videoStreamDescriptor.planes == 3) && (videoStreamDescriptor.bitsPerPixel == 32),
          Exception("Frame format not supported")
        );
        break;
      case AVIEncoder::Compression::RLE8:
        videoStreamDescriptor.compression = RLE8;
        bassert(
          (videoStreamDescriptor.planes == 1) &&
          (videoStreamDescriptor.bitsPerPixel == 8),
          Exception("Frame format not supported")
        );
        break;
      case AVIEncoder::Compression::RLE4:
        videoStreamDescriptor.compression = RLE4;
        bassert(
          (videoStreamDescriptor.planes == 1) &&
          (videoStreamDescriptor.bitsPerPixel == 4),
          Exception("Frame format not supported")
        );
        break;
      default:
        throw Exception("Compression not supported");
      }

      if (videoStreamDescriptor.bitsPerPixel <= 8) { // do we need to initialize the palette
        const AVIPaletteEntry* srcPalette = (AVIPaletteEntry*)(buffer.getElements() + header->size);
        unsigned int numberOfEntries = (header->colorUsed == 0) ? (1U << videoStreamDescriptor.bitsPerPixel) : static_cast<unsigned int>(header->colorUsed);
        bassert(
          header->size + numberOfEntries * sizeof(AVIPaletteEntry) <= chunk.size,
          InvalidFormat(this)
        );
        palette.setSize(256);
        ColorPixel* destPalette = palette.getElements();
        for (unsigned int i = 0; i < numberOfEntries; ++i) {
          destPalette[i].blue = srcPalette[i].blue;
          destPalette[i].green = srcPalette[i].green;
          destPalette[i].red = srcPalette[i].red;
        }
      }
    }

    while (totalRead < totalSize) {
      bassert(totalRead + sizeof(chunk) < totalSize, InvalidFormat(this));
      file.read(Cast::getAddress(chunk), sizeof(chunk));

      totalRead += sizeof(chunk);
      unsigned int size = (chunk.size+1)/2*2;
      bassert(totalRead + size <= totalSize, InvalidFormat(this));
      if (chunk.id == makeChunkId('s', 't', 'r', 'd')) {
        streamData.setSize(size);
        file.read(streamData.getElements(), size);
      } else {
        file.setPosition(size, File::CURRENT); // skip chunk
      }
      totalRead += size;
    }
  }

  while (true) { // stream data
    // check size
    Chunk chunk;
    file.read(Cast::getAddress(chunk), sizeof(chunk));
    unsigned int size = (chunk.size+1)/2*2;
    if (chunk.id == makeChunkId('L', 'I', 'S', 'T')) {
      ChunkId name;
      file.read(Cast::getAddress(name), sizeof(name));
      if (name == makeChunkId('m', 'o', 'v', 'i')) {
        break; // we have found what we were looking for
      }
      size -= sizeof(name); // skip entire LIST
    }
    file.setPosition(size, File::CURRENT); // skip JUNK and unknown chunks
  }

  valid = videoStreamIndex > 0;
  if (valid) {
    buffer.setSize(videoStreamDescriptor.suggestedBufferSize); // TAG: max size?
  }
}

AVIReader::AVIReader(const String& filename) throw(IOException) : file(filename, File::READ, 0), valid(false) {
  analyse();
}

void AVIReader::decodeFrame(ColorImage& frame, const byte* src, unsigned int size, FrameType type) throw() {
  ColorPixel* dest = frame.getElements();
  const Dimension dimension = globalDescriptor.dimension;

  switch (videoStreamDescriptor.compression) {
  case RGB: // no compression
    {
      switch (videoStreamDescriptor.bitsPerPixel) {
      case 8:
        {
          // todo: line alignment
          bassert(size == dimension.getSize() * 1, Exception("Invalid frame"));
          const ColorPixel* pal = palette.getElements();
          unsigned int zeroPad = (dimension.getWidth()+3)/4*4 - dimension.getWidth(); // number of zero pads
          --dest;
          --src;
          for (unsigned int rowCount = dimension.getHeight(); rowCount > 0; --rowCount) {
            for (unsigned int columnCount = dimension.getWidth(); columnCount > 0; --columnCount) {
              *++dest = pal[*++src]; // order of args is blue, green, and red
            }
            switch (zeroPad) {
            case 3:
              ++src;
            case 2:
              ++src;
            case 1:
              ++src;
            }
          }
          break;
        }
      case 24:
        {
          // todo: line alignment
          bassert(size == dimension.getSize() * 3, Exception("Invalid frame"));
          const byte* end = src + size;
          --dest;
          --src;
          while (src < end) {
            unsigned char blue = *++src;
            unsigned char green = *++src;
            unsigned char red = *++src;
            *++dest = makeColorPixel(red, green, blue);
          }
        }
        break;
      case 32:
        {
          bassert(size == dimension.getSize() * 4, InvalidFormat(this));
          const byte* end = src + size;
          --dest;
          --src;
          while (src < end) {
            unsigned char blue = *++src;
            unsigned char green = *++src;
            unsigned char red = *++src;
            *++dest = makeColorPixel(red, green, blue);
            ++src; // skip
          }
        }
        break;
      }
    }
    break;
  case RLE8: // 8 bit run-length encoding
    {
      const ColorPixel* pal = palette.getElements();
      const byte* srcEnd = src + size;
      ColorPixel* elements = frame.getElements();
      unsigned int row = 0;
      unsigned int column = 0;

      // todo - check for validity
      while (src < srcEnd) {
        unsigned int first = *src++;
        unsigned int second = *src++;

        if (first == 0) { // escape follows - or absolute mode
          switch (second) {
          case 0: // end of line
            ++row;
            column = 0;
            break;
          case 1: // end of image
            row = dimension.getHeight();
            column = 0;
            break;
          case 2: // delta
            column += *src++;
            row += *src++;
            break;
          default: // absolute mode
            ColorPixel* dest = elements + dimension.getWidth() * row + column;
            for (unsigned int count = second; count > 0; --count) {
              *dest++ = pal[*src++];
            }
            if (second % 2) { // skip to word boundary
              ++src;
            }
            column += second;
          }
        } else {
          fill<ColorPixel>(elements + dimension.getWidth() * row + column, first, pal[second]);
          column += first;
        }
      }
    }
    break;
  case RLE4: // 4 bit run-length encoding
    {
      const ColorPixel* pal = palette.getElements();
      const byte* srcEnd = src + size;
      ColorPixel* elements = frame.getElements();
      unsigned int row = 0;
      unsigned int column = 0;

      // todo - check for validity
      while (src < srcEnd) {
        unsigned int first = *src++;
        unsigned int second = *src++;

        if (first == 0) { // escape follows - or absolute mode
          switch (second) {
          case 0: // end of line
            ++row;
            column = 0;
            break;
          case 1: // end of image
            row = dimension.getHeight();
            column = 0;
            break;
          case 2: // delta
            column += *src++;
            row += *src++;
            break;
          default: // absolute mode
            ColorPixel* dest = elements + dimension.getWidth() * row + column;
            for (unsigned int count = second/2; count > 0; --count) {
              unsigned int data = *src++;
              *dest++ = pal[data >> 4];
              *dest++ = pal[data & 0x0f];
            }
            if (second % 2) { // set last pixel and skip to word boundary
              *dest++ = pal[*src++ >> 4];
              ++src;
            }
            column += second;
          }
        } else {
          ColorPixel* dest = elements + dimension.getWidth() * row + column;
          ColorPixel color = pal[second >> 4];
          ColorPixel previousColor = pal[second & 0x0f];
          const ColorPixel* end = dest + first;
          while (dest < end) {
            *dest++ = color;
            swapper(color, previousColor);
          }
          column += first;
        }
      }
    }
    break;
  }
}

void AVIReader::getFrame(ColorImage& frame) throw(IOException) {
  bassert(
    frame.getDimension() == globalDescriptor.dimension,
    Exception("Invalid arg")
  );

  Chunk chunk;
  while (true) { // find frame of video stream
    // check size
    file.read(Cast::getAddress(chunk), sizeof(chunk));
    fout << "getFrame: chunk=" << chunk.id
         << " size=" << chunk.size << ENDL;
    // check size
    unsigned int size = (chunk.size+1)/2*2;
    fout << "getFrame: streamId=" << getStreamId(chunk.id) << " videoStreamIndex=" << videoStreamIndex << ENDL;

    if (getStreamId(chunk.id) == videoStreamIndex) {

      if (buffer.getSize() < size) { // make room for data
        buffer.setSize(size);
      }
      file.read(buffer.getElements(), size);

      unsigned int streamType = getStreamType(chunk.id);
      if (streamType == getStreamType(makeChunkId('#', '#', 'd', 'b'))) { // uncompressed data
        decodeFrame(frame, buffer.getElements(), chunk.size, UNCOMPRESSED);
        break;
      } else if (streamType == getStreamType(makeChunkId('#', '#', 'd', 'c'))) { // compressed data
        decodeFrame(frame, buffer.getElements(), chunk.size, COMPRESSED);
        break;
      } else if (streamType == getStreamType(makeChunkId('#', '#', 'p', 'c'))) { // palette change
        const AVIPaletteChange* src = (AVIPaletteChange*)buffer.getElements();
        bassert(static_cast<unsigned int>(src->firstEntry) + static_cast<unsigned int>(src->numberOfEntries) <= 256, InvalidFormat(this));
        bassert(sizeof(AVIPaletteChange) + src->numberOfEntries * sizeof(AVIPaletteEntry) <= chunk.size, InvalidFormat(this));
        ColorPixel* destPalette = palette.getElements(); // palette has 256 entries
        for (unsigned int i = src->firstEntry; i < static_cast<unsigned int>(src->firstEntry) + src->numberOfEntries; ++i) {
          destPalette[i].blue = src->entry[i].blue;
          destPalette[i].green = src->entry[i].green;
          destPalette[i].red = src->entry[i].red;
        }
      }
    } else {
      file.setPosition(size, File::CURRENT); // skip JUNK and unknown chunks
    }
  }
  ++frameIndex;
}

}; // end of gip namespace
