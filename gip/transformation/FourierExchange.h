/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    Copyright (C) 2002 by Rene Moeller Fonseca <fonseca@mip.sdu.dk>

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#ifndef _DK_SDU_MIP__GIP_TRANSFORMATION__FOURIER_EXCHANGE_H
#define _DK_SDU_MIP__GIP_TRANSFORMATION__FOURIER_EXCHANGE_H

#include <gip/transformation/UnaryTransformation.h>

namespace gip {

  /**
    This transformation reorganizes the quadrants of an images such that
    region I and III and region II and IV change places respectively. This is
    normally used to move the 0 frequency pixel to the center of the image for
    Fourier (and other transformations) space images.

    Where the regions are defined as follows:
    @li <tt>c Region I:</tt> is the upper right quadrant.
    @li <tt>Region II:</tt> is the upper left quadrant.
    @li <tt>Region III:</tt> is the lower left quadrant.
    @li <tt>Region IV:</tt> is the lower right quadrant.

    @short Fourier region exchange.
    @ingroup transformations
    @author Rene Moeller Fonseca <fonseca@mip.sdu.dk>
    @version 1.0
  */

  template<class DEST>
  class FourierExchange : public UnaryTransformation<DEST> {
  public:

    /**
      Initializes transformation object.
    */
    FourierExchange(DestinationImage* destination) throw();

    /**
      Exchange the regions of the image.
    */
    void operator()() throw();
  };

  template<class DEST>
  FourierExchange<DEST>::FourierExchange(DestinationImage* destination) throw()
    : UnaryTransformation<DestinationImage>(destination) {
  }

  template<class DEST>
  void FourierExchange<DEST>::operator()() throw() {

    unsigned int halfWidth = destination->getDimension().getWidth()/2;
    unsigned int halfHeight = destination->getDimension().getHeight()/2;
    if ((halfWidth == 0) || (halfHeight == 0)) {
      return; // nothing to do
    }

    unsigned int rightOffset = (destination->getDimension().getWidth()+1)/2; // round up
    unsigned int bottomOffset = (destination->getDimension().getHeight()+1)/2; // round up

    typename DestinationImage::Rows rows = destination->getRows();
    typename DestinationImage::Rows::RowIterator end = rows.getEnd();

    // exchange region I and III
    {
      typename DestinationImage::Rows::RowIterator regionI = rows.getFirst();
      typename DestinationImage::Rows::RowIterator regionIII = regionI + bottomOffset;

      while (regionIII < end) {
        typename DestinationImage::Rows::RowIterator::ElementIterator columnI = regionI.getFirst() + rightOffset;
        typename DestinationImage::Rows::RowIterator::ElementIterator endColumnI = regionI.getEnd();
        typename DestinationImage::Rows::RowIterator::ElementIterator columnIII = regionIII.getFirst();
        while (columnI != endColumnI) {
          swapper(*columnI++, *columnIII++);
        }
        ++regionI;
        ++regionIII;
      }
    }

    // exchange region II and IV
    {
      typename DestinationImage::Rows::RowIterator regionII = rows.getFirst();
      typename DestinationImage::Rows::RowIterator regionIV = regionII + bottomOffset;

      while (regionIV < end) {
        typename DestinationImage::Rows::RowIterator::ElementIterator columnII = regionII.getFirst();
        typename DestinationImage::Rows::RowIterator::ElementIterator columnIV = regionIV.getFirst() + rightOffset;
        typename DestinationImage::Rows::RowIterator::ElementIterator endColumnIV = regionIV.getEnd();
        while (columnIV != endColumnIV) {
          swapper(*columnII++, *columnIV++);
        }
        ++regionII;
        ++regionIV;
      }
    }
  }

}; // end of gip namespace

#endif
