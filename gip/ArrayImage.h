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

#include <gip/Image.h>
#include <gip/Pixel.h>
#include <base/Iterator.h>
#include <base/iterator/MatrixRowIterator.h>
#include <base/iterator/MatrixColumnIterator.h>
#include <base/mem/ReferenceCountedAllocator.h>
#include <base/mem/Reference.h>

namespace gip {

/**
  An image with the elements stored in an array.
  
  @short Image containing the image elements in an array for random access.
  @ingroup images
  @author Rene Moeller Fonseca <fonseca@mip.sdu.dk>
  @version 1.0
*/

template<class PIXEL>
class ArrayImage : public Image<PIXEL> {
public:

  /** The type of the pixels. */
  typedef typename Image<PIXEL>::Pixel Pixel;
private:
  
  /** The elements of the image. */
  Reference<ReferenceCountedAllocator<Pixel> > elements;
public:

  template<class TRAITS = IteratorTraits<Pixel> >
  class RowsImpl : public Iterator<TRAITS> {
  public:

    typedef MatrixRowIterator<TRAITS> RowIterator;
    typedef typename Iterator<TRAITS>::Pointer Pointer;
  private:

    RowIterator first;
    unsigned int rows;
  public:

    inline RowsImpl(Pointer value, const Dimension& dimension) throw() :
      first(value, dimension.getWidth()), rows(dimension.getHeight()) {
    }
    
    inline RowsImpl(const RowsImpl& copy) throw()
      : first(copy.first), rows(copy.rows) {
    }
    
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
    typedef typename Iterator<TRAITS>::Pointer Pointer;
  private:

    ColumnIterator first;
    unsigned int columns;
  public:
    
    inline ColumnsImpl(Pointer value, const Dimension& dimension) throw() :
      first(value, dimension), columns(dimension.getWidth()) {
    }
    
    inline ColumnsImpl(const ColumnsImpl& copy) throw()
      : first(copy.first), columns(copy.columns) {
    }
    
    inline ColumnIterator getFirst() const throw() {
      return first;
    }

    inline ColumnIterator getEnd() const throw() {
      ColumnIterator result(first);
      result += columns;
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
  
  ArrayImage() throw(MemoryException);
  
  /**
    Initializes the image to the specified dimension. The elements are not initialized.

    @param dimension The desired dimension of the image.
  */
  ArrayImage(const Dimension& dimension) throw(MemoryException);

  /**
    Initializes the image from other image.
  */
  ArrayImage(const ArrayImage& copy) throw();

  ArrayImage& operator=(const ArrayImage& eq) throw() {
    Image<Pixel>::operator=(eq);
    elements = eq.elements;
    return *this;
  }
  
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
ArrayImage<PIXEL>::ArrayImage() throw(MemoryException) :
  Image<PIXEL>(Dimension(0, 0)), elements(new ReferenceCountedAllocator<Pixel>(0)) {
}

template<class PIXEL>
ArrayImage<PIXEL>::ArrayImage(const Dimension& dimension) throw(MemoryException) :
  Image<Pixel>(dimension), elements(new ReferenceCountedAllocator<Pixel>(dimension.getSize())) {
}

template<class PIXEL>
ArrayImage<PIXEL>::ArrayImage(const ArrayImage& copy) throw() :
  Image<Pixel>(copy), elements(copy.elements) {
}

template<class PIXEL>
typename ArrayImage<PIXEL>::Pixel* ArrayImage<PIXEL>::getElements() throw(MemoryException) {
  elements.copyOnWrite();
  return elements->getElements();
}

template<class PIXEL>
const typename ArrayImage<PIXEL>::Pixel* ArrayImage<PIXEL>::getElements() const throw() {
  return elements->getElements();
}

typedef ArrayImage<GrayPixel> GrayImage;
typedef ArrayImage<ColorPixel> ColorImage;
typedef ArrayImage<ColorAlphaPixel> ColorAlphaImage;
typedef ArrayImage<float> FloatImage;
typedef ArrayImage<Complex> ComplexImage;

}; // end of gip namespace
