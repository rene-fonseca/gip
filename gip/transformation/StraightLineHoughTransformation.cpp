/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    See COPYRIGHT.txt for details.

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#include <gip/transformation/StraightLineHoughTransformation.h>
#include <base/math/Constants.h>
#include <base/math/Math.h>

namespace gip {

  StraightLineHoughTransformation::StraightLineHoughTransformation(DestinationImage* destination, const SourceImage* source) 
    : Transformation<DestinationImage, SourceImage>(destination, source) {
    
    bassert(
      source->getDimension().isProper(),
      ImageException("Source image has inproper dimension", this)
    );
    bassert(
      destination->getDimension().isProper(),
      ImageException("Destination image has inproper dimension", this)
    );
    
    Dimension dimension = destination->getDimension();
    lookup.setSize(dimension.getHeight());
// TAG: relocateable ???
// TAG: width % 2 == 1 ???
    double deltaTheta = constant::PI/dimension.getHeight();
    double inverseOfDeltaRho = dimension.getWidth() *
      1/Math::sqrt(static_cast<double>(
        static_cast<unsigned long long>(dimension.getHeight()) * dimension.getHeight() +
        static_cast<unsigned long long>(dimension.getWidth()) * dimension.getWidth()
      ));
    Entry* dest = lookup.getElements();
    const Entry* end = dest + dimension.getHeight();
    unsigned int i = 0;
    while (dest < end) {
      double inner = i++ * deltaTheta;
      dest->cosine = Math::cos(inner) * inverseOfDeltaRho;
      dest->sine = Math::sin(inner) * inverseOfDeltaRho;
      ++dest;
    }
  }

  void StraightLineHoughTransformation::operator()() noexcept {
    const unsigned int height = destination->getDimension().getHeight();
    const unsigned int width = destination->getDimension().getWidth();
    const double halfWidth = width * 0.5;
    const Entry* endOfTrigo = lookup.getElements() + height;
    const int halfSrcHeight = source->getDimension().getHeight()/2;
    const int halfSrcWidth = source->getDimension().getWidth()/2;

    DestinationImage::Pixel* dest = destination->getElements();
    fill<DestinationImage::Pixel>(dest, width * height, 0); // reset
    
    SourceImage::ReadableRows srcRowLookup = source->getRows();
    SourceImage::ReadableRows::RowIterator srcRow = srcRowLookup.getFirst();
    for (int y = -halfSrcHeight; srcRow != srcRowLookup.getEnd(); ++srcRow, ++y) { // traverse all rows
      SourceImage::ReadableRows::RowIterator::ElementIterator srcColumn = srcRow.getFirst();
      // TAG: we can precalc y * src->sine here for all theta
      for (int x = -halfSrcWidth; srcColumn != srcRow.getEnd(); ++srcColumn, ++x) { // traverse all columns of current row
        // TAG: need predicate support - see functor header
        if (*srcColumn) {
          const Entry* trigo = lookup.getElements();
          DestinationImage::Pixel* destRow = dest;
          for (int theta = 0; trigo < endOfTrigo; ++trigo, ++theta, destRow += width) { // vote for lines
            int rho = static_cast<int>(x * trigo->cosine + y * trigo->sine + halfWidth);
            BASSERT(static_cast<unsigned int>(rho) < width);
            BASSERT(rho >= 0);
            ++destRow[rho]; // TAG: alternatively use gradient as weight
          }
        }
      }
    }
  }
  
}; // end of gip namespace
