/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    Copyright (C) 2001-2002 by Rene Moeller Fonseca <fonseca@mip.sdu.dk>

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#include <gip/media/AVIEncoder.h>
#include <base/mem/Allocator.h>
#include <base/ByteOrder.h>
#include <gip/io/InvalidFormat.h>

namespace gip {

/**
  Character code identifying a chunk of data with an Audio/Video Interleaved (AVI) file.
*/
typedef union {
  LittleEndian::UnsignedInt id;
  char chars[4];
} __attribute__ ((packed)) ChunkId;

/**
  Chunk of data (ie. ChunkId and size).
*/
typedef struct {
  ChunkId id;
  LittleEndian::UnsignedInt size;
} __attribute__ ((packed)) Chunk;

inline ChunkId makeChunkId(char a, char b, char c, char d) throw() {
  ChunkId result;
  result.chars[0] = a;
  result.chars[1] = b;
  result.chars[2] = c;
  result.chars[3] = d;
  return result;
}

FormatOutputStream& operator<<(FormatOutputStream& stream, const ChunkId& value) {
  stream << value.chars[0] << value.chars[1] << value.chars[2] << value.chars[3];
  return stream;
}

inline bool operator==(const ChunkId& left, const ChunkId& right) throw() {
  return left.id == right.id;
}

inline bool operator!=(const ChunkId& left, const ChunkId& right) throw() {
  return left.id != right.id;
}

int getStreamId(const ChunkId& value) throw() {
  int high = static_cast<int>(value.chars[0] - '0');
  int low = static_cast<int>(value.chars[1] - '0');
  if ((high < 0) || (low < 0)) {
    return -1; // invalid
  }
  return high * 256 + low;
}

unsigned int getStreamType(const ChunkId& value) throw() {
  return static_cast<unsigned int>(value.chars[2]) << 8 + static_cast<unsigned int>(value.chars[3]);
}

typedef struct {
  LittleEndian::UnsignedInt microSecPerFrame; // period between frames
  LittleEndian::UnsignedInt maxBytesPerSec; // approx. maximum data rate
  LittleEndian::UnsignedInt paddingGranularity;

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

  LittleEndian::UnsignedInt totalFrames; // the total number of frames
  LittleEndian::UnsignedInt initialFrames;  // number of frames prior to the initial frame
  LittleEndian::UnsignedInt streams; // the number of streams within the object
  LittleEndian::UnsignedInt suggestedBufferSize;
  LittleEndian::UnsignedInt width;
  LittleEndian::UnsignedInt height;
  LittleEndian::UnsignedInt scale;
  LittleEndian::UnsignedInt rate;
  LittleEndian::UnsignedInt start; // the starting time of the AVI file
  LittleEndian::UnsignedInt length; // the length of the AVI file
} __attribute__ ((packed)) AVIHeader;

typedef struct {
  LittleEndian::UnsignedInt ckid;
  LittleEndian::UnsignedInt flags;
  LittleEndian::UnsignedInt chunkOffset;
  LittleEndian::UnsignedInt chunkLength;
} __attribute__ ((packed)) AVIIndexEntry;

typedef struct {
  ChunkId type;
  ChunkId handler;
  LittleEndian::UnsignedInt flags;
  LittleEndian::UnsignedInt priority;
  LittleEndian::UnsignedInt initialFrames;
  LittleEndian::UnsignedInt scale;
  LittleEndian::UnsignedInt rate;
  LittleEndian::UnsignedInt start;
  LittleEndian::UnsignedInt length;
  LittleEndian::UnsignedInt suggestedBufferSize;
  LittleEndian::UnsignedInt quality;
  LittleEndian::UnsignedInt sampleSize;
  LittleEndian::UnsignedInt left;
  LittleEndian::UnsignedInt top;
  LittleEndian::UnsignedInt right;
  LittleEndian::UnsignedInt bottom;
} __attribute__ ((packed)) AVIStreamHeader;

typedef struct {
  LittleEndian::UnsignedInt size;
  LittleEndian::SignedInt width;
  LittleEndian::SignedInt height;
  LittleEndian::UnsignedShort planes;
  LittleEndian::UnsignedShort bitsPerPixel;
  LittleEndian::UnsignedInt compression;
  LittleEndian::UnsignedInt sizeImage;
  LittleEndian::SignedInt xPelsPerMeter;
  LittleEndian::SignedInt yPelsPerMeter;
  LittleEndian::UnsignedInt colorUsed;
  LittleEndian::UnsignedInt colorImportant;
} __attribute__ ((packed)) BitmapInfoHeader;

typedef struct {
  LittleEndian::UnsignedShort formatTag;
  LittleEndian::UnsignedShort channels;
  LittleEndian::UnsignedInt samplesPerSec;
  LittleEndian::UnsignedInt averageBytesPerSec;
  LittleEndian::UnsignedShort blockAlign;
  LittleEndian::UnsignedShort bitsPerSample;
  LittleEndian::UnsignedShort size;
} __attribute__ ((packed)) WaveFormatExtended;

typedef struct {
  byte blue;
  byte green;
  byte red;
  byte reserved;
} __attribute__ ((packed)) AVIPaletteEntry;

typedef struct {
  byte firstEntry;
  byte numberOfEntries;
  LittleEndian::UnsignedShort flags;
  AVIPaletteEntry entry[0];
} __attribute__ ((packed)) AVIPaletteChange;

enum {BMP_RGB = 0, BMP_RLE8 = 1, BMP_RLE4 = 2, BMP_BITFIELDS = 3};

AVIEncoder::AVIEncoder(const String& f) throw(IOException) : filename(f) {
}

String AVIEncoder::getDescription() const throw() {
  return MESSAGE("Microsoft Audio/Video Interleaved format");
}

String AVIEncoder::getDefaultExtension() const throw() {
  return MESSAGE("avi");
}

bool AVIEncoder::isValid() throw(IOException) {
  return false;
}

ArrayImage<ColorPixel>* AVIEncoder::read() throw(IOException) {
  return 0;
}

void AVIEncoder::write(const ArrayImage<ColorPixel>* image) throw(IOException) {
}

FormatOutputStream& AVIEncoder::getInfo(FormatOutputStream& stream) throw(IOException) {
  stream << MESSAGE("AVIEncoder (Microsoft Audio/Video Interleaved format):") << EOL;

  {
    File file(filename, File::READ, 0);

    {
      Chunk chunk;
      file.read(getCharAddress(chunk), sizeof(chunk));
      ChunkId name;
      file.read(getCharAddress(name), sizeof(name));
      stream << MESSAGE("  ") << chunk.id << MESSAGE("('") << name << MESSAGE("' chunk of size ") << chunk.size << EOL;
      if ((chunk.id != makeChunkId('R', 'I', 'F', 'F')) || (name != makeChunkId('A', 'V', 'I', ' '))) {
        return stream << MESSAGE("INVALID FORMAT") << EOL;
      }
    }

    unsigned int listTotalSize;
    unsigned int listTotalRead = 0;
    {
      Chunk chunk;
      file.read(getCharAddress(chunk), sizeof(chunk));
      ChunkId name;
      file.read(getCharAddress(name), sizeof(name));
      stream << MESSAGE("  ") << chunk.id << MESSAGE("('") << name << MESSAGE("' chunk of size ") << chunk.size << EOL;
      if ((chunk.id != makeChunkId('L', 'I', 'S', 'T')) || (name != makeChunkId('h', 'd', 'r', 'l'))) {
        return stream << MESSAGE("INVALID FORMAT") << EOL;
      }
      listTotalSize = chunk.size;
    }

    ChunkId avih;
    file.read(getCharAddress(avih), sizeof(avih));
    LittleEndian::UnsignedInt size;
    file.read(getCharAddress(size), sizeof(size));
    Allocator<char> buffer(maximum<unsigned int>(size, sizeof(AVIHeader)));
    fill<char>(buffer.getElements(), buffer.getSize(), 0);
    AVIHeader* header = (AVIHeader*)buffer.getElements();
    file.read(buffer.getElements(), size);
    stream << MESSAGE("    'avih'(<Main AVI header>) of size ") << size << EOL
           << MESSAGE("      microSecPerFrame=") << header->microSecPerFrame << EOL
           << MESSAGE("      maxBytesPerSec=") << header->maxBytesPerSec << EOL
           << MESSAGE("      paddingGranularity=") << header->paddingGranularity << EOL
           << MESSAGE("      hasIndex=") << header->hasIndex << EOL
           << MESSAGE("      mustUseIndex=") << header->mustUseIndex << EOL
           << MESSAGE("      isInterleaved=") << header->isInterleaved << EOL
           << MESSAGE("      trustCKType=") << header->trustCKType << EOL
           << MESSAGE("      wasCaptureFile=") << header->wasCaptureFile << EOL
           << MESSAGE("      copyrighted=") << header->copyrighted << EOL
           << MESSAGE("      totalFrames=") << header->totalFrames << EOL
           << MESSAGE("      initialFrames=") << header->initialFrames << EOL
           << MESSAGE("      streams=") << header->streams << EOL
           << MESSAGE("      suggestedBufferSize=") << header->suggestedBufferSize << EOL
           << MESSAGE("      width=") << header->width << EOL
           << MESSAGE("      height=") << header->height << EOL
           << MESSAGE("      scale=") << header->scale << EOL
           << MESSAGE("      rate=") << header->rate << EOL
           << MESSAGE("      start=") << header->start << EOL
           << MESSAGE("      length=") << header->length << EOL;
    listTotalRead += size;

    for (unsigned int i = 0; i < header->streams; ++i) {
      Chunk list;
      file.read(getCharAddress(list), sizeof(list));
      ChunkId name;
      file.read(getCharAddress(name), sizeof(name));
      stream << MESSAGE("  ") << list.id << MESSAGE("('") << name << MESSAGE("' chunk of size ") << list.size << EOL;

      if ((list.id != makeChunkId('L', 'I', 'S', 'T')) || (name != makeChunkId('s', 't', 'r', 'l'))) {
        return stream << MESSAGE("  INVALID FORMAT") << EOL;
      }

      ChunkId str_;
      LittleEndian::UnsignedInt size;
      unsigned int totalRead = sizeof(name);

      // read strh
      file.read(getCharAddress(str_), sizeof(str_));
      if (str_ != makeChunkId('s', 't', 'r', 'h')) {
        return stream << MESSAGE("Expected chunk id: strh") << EOL;
      }
      file.read(getCharAddress(size), sizeof(size));
      totalRead += sizeof(str_) + sizeof(size) + (size+1)/2*2;
      Allocator<char> headerBuffer(maximum((size+1)/2*2, sizeof(AVIStreamHeader)));
      fill<char>(headerBuffer.getElements(), headerBuffer.getSize(), 0);
      AVIStreamHeader* header = (AVIStreamHeader*)headerBuffer.getElements();

      file.read(headerBuffer.getElements(), (size+1)/2*2);
      stream << MESSAGE("    '") << str_ << MESSAGE("'(<AVI stream header>) of size ") << size << EOL
             << MESSAGE("      type=") << header->type << EOL
             << MESSAGE("      handler=") << header->handler << EOL
             << MESSAGE("      flags=") << HEX << header->flags << EOL
             << MESSAGE("      initialFrames=") << header->initialFrames << EOL
             << MESSAGE("      scale=") << header->scale << EOL
             << MESSAGE("      rate=") << header->rate << EOL
             << MESSAGE("      start=") << header->start << EOL
             << MESSAGE("      length=") << header->length << EOL
             << MESSAGE("      suggestedBufferSize=") << header->suggestedBufferSize << EOL
             << MESSAGE("      quality=") << header->quality << EOL
             << MESSAGE("      sampleSize=") << header->sampleSize << EOL;

      // read strf - BITMAPINFO for vids and WAVEFORMATEX for auds
      file.read(getCharAddress(str_), sizeof(str_));
      if (str_ != makeChunkId('s', 't', 'r', 'f')) {
        return stream << MESSAGE("Expected chunk id: strf") << EOL;
      }
      file.read(getCharAddress(size), sizeof(size));
      totalRead += sizeof(str_) + sizeof(size) + (size+1)/2*2;

      Allocator<char> streamFormatBuffer(maximum((size+1)/2*2, maximum(sizeof(BitmapInfoHeader), sizeof(WaveFormatExtended))));
      fill<char>(streamFormatBuffer.getElements(), streamFormatBuffer.getSize(), 0);
      file.read(streamFormatBuffer.getElements(), (size+1)/2*2);

      if (header->type == makeChunkId('v', 'i', 'd', 's')) {
        stream << MESSAGE("    '") << str_ << MESSAGE("'(<BitmapInfo>) of size ") << size << EOL;
        BitmapInfoHeader* header = (BitmapInfoHeader*)streamFormatBuffer.getElements();

        stream << MESSAGE("      size=") << header->size << EOL
               << MESSAGE("      width=") << header->width << EOL
               << MESSAGE("      height=") << header->height << EOL
               << MESSAGE("      planes=") << header->planes << EOL
               << MESSAGE("      bitsPerPixel=") << header->bitsPerPixel << EOL
               << MESSAGE("      compression=") << header->compression << EOL
               << MESSAGE("      sizeImage=") << header->sizeImage << EOL
               << MESSAGE("      xPelsPerMeter=") << header->xPelsPerMeter << EOL
               << MESSAGE("      yPelsPerMeter=") << header->yPelsPerMeter << EOL
               << MESSAGE("      colorUsed=") << header->colorUsed << EOL
               << MESSAGE("      colorImportant=") << header->colorImportant << EOL;

      } else if (header->type == makeChunkId('a', 'u', 'd', 's')) {
        stream << MESSAGE("    '") << str_ << MESSAGE("'(<WaveFormatExtended>) of size ") << size << EOL;
        WaveFormatExtended* header = (WaveFormatExtended*)streamFormatBuffer.getElements();

        stream << MESSAGE("      formatTag=") << header->formatTag << EOL
               << MESSAGE("      channels=") << header->channels << EOL
               << MESSAGE("      samplesPerSec=") << header->samplesPerSec << EOL
               << MESSAGE("      averageBytesPerSec=") << header->averageBytesPerSec << EOL
               << MESSAGE("      blockAlign=") << header->blockAlign << EOL
               << MESSAGE("      bitsPerSample=") << header->bitsPerSample << EOL
               << MESSAGE("      size=") << header->size << EOL;

      } else {
        stream << MESSAGE("    '") << str_ << MESSAGE("'(<Unknown stream type>) of size ") << size << EOL;
      }

      while (totalRead < list.size) {
        file.read(getCharAddress(str_), sizeof(str_));
        file.read(getCharAddress(size), sizeof(size));
        totalRead += sizeof(str_) + sizeof(size) + (size+1)/2*2;
        Allocator<char> buffer((size+1)/2*2);
        file.read(buffer.getElements(), (size+1)/2*2);

        if (str_ == makeChunkId('s', 't', 'r', 'n')) {
          stream << MESSAGE("    '") << str_ << MESSAGE("'(<chars>) of size ") << size << EOL
                 << MESSAGE("      data=") << buffer.getElements() << EOL;
        } else {
          stream << MESSAGE("    '") << str_ << MESSAGE("'(<Unknown stream data>) of size ") << size << EOL;
        }
      }

       // check if: totalRead == listChunk.size
      // listTotalSize += totalRead;
    }

    while (true) {
      if (file.getPosition() >= file.getSize()) {
        return stream << MESSAGE("INVALID FORMAT") << EOL;
      }
      Chunk chunk;
      file.read(getCharAddress(chunk), sizeof(chunk));
      if (chunk.id == makeChunkId('L', 'I', 'S', 'T')) {
        ChunkId name;
        file.read(getCharAddress(name), sizeof(name));
        unsigned int totalSize = chunk.size;
        unsigned int totalRead = sizeof(name);
        stream << MESSAGE("  LIST('") << name << MESSAGE("' chunk of size ") << totalSize << ENDL;

        while (totalRead < totalSize) {
          Chunk dataChunk;
          file.read(getCharAddress(dataChunk), sizeof(dataChunk));
          totalRead += sizeof(dataChunk) + dataChunk.size;
          stream << MESSAGE("    chunk: {id=") << dataChunk.id << MESSAGE(";size=") << dataChunk.size << '}' << ENDL;
          file.setPosition(dataChunk.size, File::CURRENT); // skip JUNK and unknown chunks
        }

        break;
      } else {
        stream << MESSAGE("  chunk: {id=") << chunk.id << MESSAGE(";size=") << chunk.size << '}' << ENDL;
        file.setPosition(chunk.size, File::CURRENT); // skip JUNK and unknown chunks
      }
    }

//    if (header.hasIndex)
    while (true) {
      if (file.getPosition() >= file.getSize()) {
        return stream << MESSAGE("INVALID FORMAT") << EOL;
      }
      Chunk chunk;
      file.read(getCharAddress(chunk), sizeof(chunk));
      if (chunk.id == makeChunkId('i', 'd', 'x', '1')) {
        stream << MESSAGE("  chunk: {id=") << chunk.id << MESSAGE(";size=") << chunk.size << '}' << ENDL;
        file.setPosition(chunk.size, File::CURRENT); // skip chunk
        break;
      } else {
        stream << MESSAGE("  chunk: {id=") << chunk.id << MESSAGE(";size=") << chunk.size << '}' << ENDL;
        file.setPosition(chunk.size, File::CURRENT); // skip JUNK and unknown chunks
      }
    }

    stream << MESSAGE("  file position=") << file.getPosition() << EOL
           << MESSAGE("  file size=") << file.getSize() << EOL;
  }

  return stream;
}

AVIEncoder::~AVIEncoder() {
}






void AVIReader::analyse() throw(IOException) {
  {
    Chunk riff;
    file.read(getCharAddress(riff), sizeof(riff));
    assert(riff.id == makeChunkId('R', 'I', 'F', 'F'), InvalidFormat(this));
    ChunkId name;
    file.read(getCharAddress(name), sizeof(name));
    assert(name == makeChunkId('A', 'V', 'I', ' '), InvalidFormat(this));
    fout << MESSAGE("riff=") << riff.id << MESSAGE(" size=") << riff.size << MESSAGE(" name=") << name << ENDL;
  }

  unsigned int totalRead = 0;

  Chunk list;
  file.read(getCharAddress(list), sizeof(list));
  assert(list.id == makeChunkId('L', 'I', 'S', 'T'), InvalidFormat(this));

  ChunkId name;
  assert(totalRead + sizeof(name) < list.size, InvalidFormat(this));
  file.read(getCharAddress(name), sizeof(name));
  assert(name == makeChunkId('h', 'd', 'r', 'l'), InvalidFormat(this));
  totalRead += sizeof(name);

  {
    Chunk avih;
    assert(totalRead + sizeof(avih) < list.size, InvalidFormat(this));
    file.read(getCharAddress(avih), sizeof(avih));
    assert(avih.id == makeChunkId('a', 'v', 'i', 'h'), InvalidFormat(this));
    Allocator<char> buffer(maximum<unsigned int>(avih.size, sizeof(AVIHeader)));
    fill<char>(buffer.getElements(), buffer.getSize(), 0);
    file.read(buffer.getElements(), (avih.size + 1)/2*2);
    totalRead += sizeof(avih) + (avih.size + 1)/2*2;
    AVIHeader* header = pointer_cast<AVIHeader*>(buffer.getElements());

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
//    assert(totalRead + sizeof(chunk) < totalSize, InvalidFormat(this));
    file.read(getCharAddress(list), sizeof(list));
//    totalRead += sizeof(list);
    assert(list.id == makeChunkId('L', 'I', 'S', 'T'), InvalidFormat(this));

    unsigned int totalRead = 0;
    unsigned int totalSize = list.size; // todo may need to +1/2*2

    ChunkId name;
    file.read(getCharAddress(name), sizeof(name));
    assert(name == makeChunkId('s', 't', 'r', 'l'), InvalidFormat(this));
    totalRead += sizeof(name);

    Chunk chunk;
    enum {VIDEO, AUDIO, TEXT, UNKNOWN} streamType;

    // read strh
    {
      assert(totalRead + sizeof(chunk) < totalSize, InvalidFormat(this));
      file.read(getCharAddress(chunk), sizeof(chunk));
      assert(chunk.id == makeChunkId('s', 't', 'r', 'h'), InvalidFormat(this));
      unsigned int size = (chunk.size+1)/2*2;
      assert(totalRead + size < totalSize, InvalidFormat(this));
      Allocator<char> buffer(maximum(size, sizeof(AVIStreamHeader)));
      fill<char>(buffer.getElements(), buffer.getSize(), 0);
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
    assert(totalRead + sizeof(chunk) < totalSize, InvalidFormat(this));
    file.read(getCharAddress(chunk), sizeof(chunk));
    assert(chunk.id == makeChunkId('s', 't', 'r', 'f'), InvalidFormat(this));
    unsigned int size = (chunk.size+1)/2*2;
    totalRead += sizeof(chunk) + size;

    Allocator<char> buffer(maximum(size, sizeof(BitmapInfoHeader)));
    fill<char>(buffer.getElements(), buffer.getSize(), 0);
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
        assert(
          (videoStreamDescriptor.planes == 1) && (videoStreamDescriptor.bitsPerPixel == 4) ||
          (videoStreamDescriptor.planes == 1) && (videoStreamDescriptor.bitsPerPixel == 8) ||
          (videoStreamDescriptor.planes == 3) && (videoStreamDescriptor.bitsPerPixel == 24) ||
          (videoStreamDescriptor.planes == 3) && (videoStreamDescriptor.bitsPerPixel == 32),
          Exception("Frame format not supported")
        );
        break;
      case AVIEncoder::Compression::RLE8:
        videoStreamDescriptor.compression = RLE8;
        assert(
          (videoStreamDescriptor.planes == 1) && (videoStreamDescriptor.bitsPerPixel == 8),
          Exception("Frame format not supported")
        );
        break;
      case AVIEncoder::Compression::RLE4:
        videoStreamDescriptor.compression = RLE4;
        assert(
          (videoStreamDescriptor.planes == 1) && (videoStreamDescriptor.bitsPerPixel == 4),
          Exception("Frame format not supported")
        );
        break;
      default:
        throw Exception("Compression not supported");
      }

      if (videoStreamDescriptor.bitsPerPixel <= 8) { // do we need to initialize the palette
        const AVIPaletteEntry* srcPalette = (AVIPaletteEntry*)(buffer.getElements() + header->size);
        unsigned int numberOfEntries = (header->colorUsed == 0) ? (1U << videoStreamDescriptor.bitsPerPixel) : static_cast<unsigned int>(header->colorUsed);
        assert(header->size + numberOfEntries * sizeof(AVIPaletteEntry) <= chunk.size, InvalidFormat(this));
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
      assert(totalRead + sizeof(chunk) < totalSize, InvalidFormat(this));
      file.read(getCharAddress(chunk), sizeof(chunk));

      totalRead += sizeof(chunk);
      unsigned int size = (chunk.size+1)/2*2;
      assert(totalRead + size <= totalSize, InvalidFormat(this));
      if (chunk.id == makeChunkId('s', 't', 'r', 'd')) {
        streamData.setSize(size);
        file.read((char*)streamData.getElements(), size);
      } else {
        file.setPosition(size, File::CURRENT); // skip chunk
      }
      totalRead += size;
    }
  }

  while (true) { // stream data
    // check size
    Chunk chunk;
    file.read(getCharAddress(chunk), sizeof(chunk));
    unsigned int size = (chunk.size+1)/2*2;
    if (chunk.id == makeChunkId('L', 'I', 'S', 'T')) {
      ChunkId name;
      file.read(getCharAddress(name), sizeof(name));
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
          assert(size == dimension.getSize() * 1, Exception("Invalid frame"));
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
          assert(size == dimension.getSize() * 3, Exception("Invalid frame"));
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
          assert(size == dimension.getSize() * 4, InvalidFormat(this));
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
  assert(frame.getDimension() == globalDescriptor.dimension, Exception("Invalid arg"));

  Chunk chunk;
  while (true) { // find frame of video stream
    // check size
    file.read(getCharAddress(chunk), sizeof(chunk));
    fout << MESSAGE("getFrame: chunk=") << chunk.id << MESSAGE(" size=") << chunk.size << ENDL;
    // check size
    unsigned int size = (chunk.size+1)/2*2;
  fout << MESSAGE("getFrame: streamId=") << getStreamId(chunk.id) << MESSAGE(" videoStreamIndex=") << videoStreamIndex << ENDL;

    if (getStreamId(chunk.id) == videoStreamIndex) {

      if (buffer.getSize() < size) { // make room for data
        buffer.setSize(size);
      }
      file.read((char*)buffer.getElements(), size);

      unsigned int streamType = getStreamType(chunk.id);
      if (streamType == getStreamType(makeChunkId('#', '#', 'd', 'b'))) { // uncompressed data
        decodeFrame(frame, buffer.getElements(), chunk.size, UNCOMPRESSED);
        break;
      } else if (streamType == getStreamType(makeChunkId('#', '#', 'd', 'c'))) { // compressed data
        decodeFrame(frame, buffer.getElements(), chunk.size, COMPRESSED);
        break;
      } else if (streamType == getStreamType(makeChunkId('#', '#', 'p', 'c'))) { // palette change
        const AVIPaletteChange* src = (AVIPaletteChange*)buffer.getElements();
        assert(static_cast<unsigned int>(src->firstEntry) + static_cast<unsigned int>(src->numberOfEntries) <= 256, InvalidFormat(this));
        assert(sizeof(AVIPaletteChange) + src->numberOfEntries * sizeof(AVIPaletteEntry) <= chunk.size, InvalidFormat(this));
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

}; // end of namespace
