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

#include <gip/ImageException.h>

namespace gip {

  /**
    Invocates the specified unary operation (non-modifying) for each element of the specified image.

    @short Invokes the operation for each element of the image
    @version 1.0
  */
  template<class IMAGE, class UNOPR>
  inline void forEach(const IMAGE& image, UNOPR& function) noexcept {
    typename IMAGE::ReadableRows rows = image.getRows();
    typename IMAGE::ReadableRows::RowIterator row = rows.getFirst();
    for (; row != rows.getEnd(); ++row) {
      typename IMAGE::ReadableRows::RowIterator::ElementIterator column = row.getFirst();
      for (; column != row.getEnd(); ++column) {
        function(*column);
      }
    }
  }

  /**
    Applies the specified operation on every element of the specified image.
    
    @short Modifies each element of the image
    @version 1.0
  */
  template<class IMAGE, class UNOPR>
  inline void transform(IMAGE& image, UNOPR& function) {
    typename IMAGE::Rows rows = image.getRows();
    typename IMAGE::Rows::RowIterator row = rows.getFirst();
    for (; row != rows.getEnd(); ++row) {
      typename IMAGE::Rows::RowIterator::ElementIterator column = row.getFirst();
      for (; column != row.getEnd(); ++column) {
        *column = function(*column);
      }
    }
  }

  template<class LEFT, class RIGHT, class BINOPR>
  inline void transform(LEFT& left, const RIGHT& right, BINOPR& function)  {
    // check if function is binary and write nice error if not???
    bassert(
      left.getDimension() == right.getDimension(),
      ImageException("Images must have identical dimension")
    );
    typename LEFT::Rows rows = left.getRows();
    typename LEFT::Rows::RowIterator row = rows.getFirst();
    typename RIGHT::ReadableRows::RowIterator rightRow = right.getRows().getFirst();
    while (row != rows.getEnd()) {
      typename LEFT::Rows::RowIterator::ElementIterator column = row.getFirst();
      typename RIGHT::ReadableRows::RowIterator::ElementIterator rightColumn = rightRow.getFirst();
      while (column != row.getEnd()) {
        *column = function(*column, *rightColumn);
        ++column;
        ++rightColumn;
      }
      ++row;
      ++rightRow;
    }
  }

  /**
    Applies the specified operation on every element of the specified image.

    @version 1.0
  */
  template<class DEST, class SRC, class UNOPR>
  inline void fillWithUnary(DEST& destination, const SRC& source, UNOPR& function)  {
    bassert(
      destination.getDimension() == source.getDimension(),
      ImageException("Images must have identical dimension")
    );
    typename DEST::Rows rows = destination.getRows();
    typename DEST::Rows::RowIterator row = rows.getFirst();
    typename SRC::ReadableRows::RowIterator srcRow = source.getRows().getFirst();
    while (row != rows.getEnd()) {
      typename DEST::Rows::RowIterator::ElementIterator column = row.getFirst();
      typename SRC::ReadableRows::RowIterator::ElementIterator srcColumn = srcRow.getFirst();
      while (column != row.getEnd()) {
        *column = function(*srcColumn);
        ++column;
        ++srcColumn;
      }
      ++row;
      ++srcRow;
    }
  }

  /**
  */
//template<class DEST, class LEFT, class RIGHT, class BINOPR>
//inline void fillWithBinary(DEST& destination, const LSRC& left, const RSRC& right, BINOPR& function)  {
//  // check if function is binary and write nice error if not???
//  // typedef typename function::Result Result
//  // typedef typename function::LeftArgument LeftArgument
//  // typedef typename function::RightArgument RightArgument
//  bassert(
//    (destination.getDimension() == left.getDimension()) && (left.getDimension() == right.getDimension()),
//    ImageException("Images must have identical dimension")
//  );
//  typename DEST::Rows rows = destination.getRows();
//  typename DEST::Rows::RowIterator row = rows.getFirst();
//  typename LEFT::ReadableRows::RowIterator leftRow = source.getRows().getFirst();
//  typename RIGHT::ReadableRows::RowIterator rightRow = source.getRows().getFirst();
//  while (row != rows.getEnd()) {
//    typename DEST::Rows::RowIterator::ElementIterator column = row.getFirst();
//    typename LEFT::ReadableRows::RowIterator::ElementIterator leftColumn = leftRow.getFirst();
//    typename RIGHT::ReadableRows::RowIterator::ElementIterator rightColumn = rightRow.getFirst();
//    while (column != row.getEnd()) {
//      *column = function(*leftColumn, *rightColumn);
//      ++column;
//      ++leftColumn;
//      ++rightColumn;
//    }
//    ++row;
//    ++leftRow;
//    ++rightRow;
//  }
//}

//template<class PIXEL, class BINOPR>
//class MergedImage : public Image<PIXEL> {
//public:
//
//  template<class TRAITS>
//  class RowIteratorImpl : public Iterator<TRAITS> {
//  public:
//
//    typedef ArrayIteratorImpl<TRAITS> ElementIterator;
//
//    inline RowIteratorImpl(Pointer value, unsigned int columns) noexcept
//      : InterleavedIteratorImpl<TRAITS>(value, columns) {
//    }
//
//    inline RowIteratorImpl(const RowIteratorImpl& copy) noexcept
//      : InterleavedIteratorImpl<TRAITS>(copy) {
//    }
//
//    inline ElementIterator getFirst() const noexcept {
//      return ElementIterator(current);
//    }
//
//    inline ElementIterator getEnd() const noexcept {
//      ElementIterator result(current);
//      result += step;
//      return result;
//    }
//
//    inline ElementIterator operator[](unsigned int index) const noexcept {
//      BASSERT(index < step);
//      ElementIterator result(current);
//      result += index;
//      return result;
//    }
//  };
//
//private:
//
//public:
//
//
//};

}; // end of gip namespace
