/***************************************************************************
    begin                : Mon Apr 30 2001
    copyright            : (C) 2001 by René Møller Fonseca
    email                : fonseca@mip.sdu.dk
 ***************************************************************************/

#ifndef _DK_SDU_MIP__GIP__ARRAY_IMAGE_H
#define _DK_SDU_MIP__GIP__ARRAY_IMAGE_H

#include <gip/Image.h>
#include <gip/Pixel.h>
#include <base/Iterator.h>
#include <base/iterator/MatrixRowIterator.h>
#include <base/iterator/MatrixColumnIterator.h>
#include <base/mem/ReferenceCountedAllocator.h>
#include <base/mem/ReferenceCountedObjectPointer.h>

namespace gip {

/**
  @short Image containing the image elements in an array for random access.
  @author René Møller Fonseca
*/

template<class PIXEL>
class ArrayImage : public Image<PIXEL> {
private:

  /** The elements of the image. */
  ReferenceCountedObjectPointer<ReferenceCountedAllocator<Pixel> > elements;
public:

  template<class TRAITS = IteratorTraits<Pixel> >
  class RowsImpl : public Iterator<TRAITS> {
  public:

    typedef MatrixRowIterator<TRAITS> RowIterator;
  private:

    RowIterator first;
    unsigned int rows;
  public:

    inline RowsImpl(Pointer value, const Dimension& dimension) throw() :
      first(value, dimension.getWidth()), rows(dimension.getHeight()) {
    }

    inline RowsImpl(const RowsImpl& copy) throw() : first(copy.first), rows(copy.rows) {}

    inline RowIterator getFirst() const throw() {
      return first;
    }

    inline RowIterator getEnd() const throw() {
      RowIterator result(first);
      result += rows;
      return result;
    }

    inline RowIterator operator[](unsigned int index) const throw() {
      ASSERT(index < rows);
      RowIterator result(first);
      result += index;
      return result;
    }
  };

  typedef RowsImpl<IteratorTraits<Pixel> > Rows;
  typedef RowsImpl<ReadIteratorTraits<Pixel> > ReadableRows;



  template<class TRAITS>
  class ColumnsImpl : public Iterator<TRAITS> {
  public:

    typedef MatrixColumnIterator<TRAITS> ColumnIterator;
  private:

    ColumnIterator first;
    unsigned int columns;
  public:

    inline ColumnsImpl(Pointer value, const Dimension& dimension) throw() :
      first(value, dimension), columns(dimension.getWidth()) {
    }

    inline ColumnsImpl(const ColumnsImpl& copy) throw() : first(copy.first), columns(copy.columns) {}

    inline ColumnIterator getFirst() const throw() {
      return first;
    }

    inline ColumnIterator getEnd() const throw() {
      ColumnIterator result(first);
      result += rows;
      return result;
    }

    inline ColumnIterator operator[](unsigned int index) const throw() {
      ASSERT(index < columns);
      ColumnIterator result(first);
      result += index;
      return result;
    }
  };

  typedef ColumnsImpl<IteratorTraits<Pixel> > Columns;
  typedef ColumnsImpl<ReadIteratorTraits<Pixel> > ReadableColumns;

  /**
    Initializes the image to the specified dimension. The elements are not initialized.

    @param dimension The desired dimension of the image.
  */
  ArrayImage(const Dimension& dimension) throw(MemoryException);

  /**
    Initializes the image from other image.
  */
  ArrayImage(const ArrayImage& copy) throw();

  /**
    Returns the rows of the image for modifying access. This will force the
    image to be copied if shared by multiple image objects.
  */
  Rows getRows() throw(MemoryException) {
    elements.copyOnWrite();
    return Rows(elements->getElements(), getDimension());
  }

  /**
    Returns the rows of the image for non-modifying access.
  */
  ReadableRows getRows() const throw() {
    return ReadableRows(elements->getElements(), getDimension());
  }

  /**
    Returns the columns of the image for modifying access. This will force the
    image to be copied if shared by multiple image objects.
  */
  Columns getColumns() throw(MemoryException) {
    elements.copyOnWrite();
    return Columns(elements->getElements(), getDimension());
  }

  /**
    Returns the rows of the image for non-modifying access.
  */
  ReadableColumns getColumns() const throw() {
    return ReadableColumns(elements->getElements(), getDimension());
  }

  /**
    Returns the first element of the image for modifying access. This will
    force the image to be copied if shared by multiple image objects.
  */
  Pixel* getElements() throw(MemoryException);

  /**
    Returns the first element of the image for non-modifying access.
  */
  const Pixel* getElements() const throw();
};

template<class PIXEL>
ArrayImage<PIXEL>::ArrayImage(const Dimension& dimension) throw(MemoryException) :
  Image<PIXEL>(dimension), elements(new ReferenceCountedAllocator<Pixel>(dimension.getSize())) {
}

template<class PIXEL>
ArrayImage<PIXEL>::ArrayImage(const ArrayImage& copy) throw() :
  Image<PIXEL>(copy), elements(copy.elements) {
}

template<class PIXEL>
ArrayImage<PIXEL>::Pixel* ArrayImage<PIXEL>::getElements() throw(MemoryException) {
  elements.copyOnWrite();
  return elements->getElements();
}

template<class PIXEL>
const ArrayImage<PIXEL>::Pixel* ArrayImage<PIXEL>::getElements() const throw() {
  return elements->getElements();
}

typedef ArrayImage<GrayPixel> GrayImage;
typedef ArrayImage<ColorPixel> ColorImage;
typedef ArrayImage<FloatPixel> FloatImage;
typedef ArrayImage<ComplexPixel> ComplexImage;

}; // end of namespace

#endif
