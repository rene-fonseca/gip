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
    unsigned int rows = 0;
  public:

    inline RowsImpl(Pointer value, const Dimension& dimension) noexcept :
      first(value, dimension.getWidth()), rows(dimension.getHeight()) {
    }
    
    inline RowsImpl(const RowsImpl& copy) noexcept
      : first(copy.first), rows(copy.rows) {
    }
    
    inline RowIterator getFirst() const noexcept {
      return first;
    }
    
    inline RowIterator getEnd() const noexcept {
      RowIterator result(first);
      result += rows;
      return result;
    }

    inline RowIterator operator[](unsigned int index) const noexcept {
      BASSERT(index < rows);
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
    unsigned int columns = 0;
  public:
    
    inline ColumnsImpl(Pointer value, const Dimension& dimension) noexcept :
      first(value, dimension), columns(dimension.getWidth()) {
    }
    
    inline ColumnsImpl(const ColumnsImpl& copy) noexcept
      : first(copy.first), columns(copy.columns) {
    }
    
    inline ColumnIterator getFirst() const noexcept {
      return first;
    }

    inline ColumnIterator getEnd() const noexcept {
      ColumnIterator result(first);
      result += columns;
      return result;
    }

    inline ColumnIterator operator[](unsigned int index) const noexcept {
      BASSERT(index < columns);
      ColumnIterator result(first);
      result += index;
      return result;
    }
  };

  typedef ColumnsImpl<IteratorTraits<Pixel> > Columns;
  typedef ColumnsImpl<ReadIteratorTraits<Pixel> > ReadableColumns;
  
  ArrayImage();
  
  /**
    Initializes the image to the specified dimension. The elements are not initialized.

    @param dimension The desired dimension of the image.
  */
  ArrayImage(const Dimension& dimension);

  /**
    Initializes the image from other image.
  */
  ArrayImage(const ArrayImage& copy) noexcept;

  ArrayImage& operator=(const ArrayImage& eq) noexcept {
    Image<Pixel>::operator=(eq);
    elements = eq.elements;
    return *this;
  }
  
  /**
    Returns the rows of the image for modifying access. This will force the
    image to be copied if shared by multiple image objects.
  */
  Rows getRows()  {
    elements.copyOnWrite();
    return Rows(elements->getElements(), Image<PIXEL>::getDimension());
  }

  /**
    Returns the rows of the image for non-modifying access.
  */
  ReadableRows getRows() const noexcept {
    return ReadableRows(elements->getElements(), Image<PIXEL>::getDimension());
  }

  /**
    Returns the columns of the image for modifying access. This will force the
    image to be copied if shared by multiple image objects.
  */
  Columns getColumns()  {
    elements.copyOnWrite();
    return Columns(elements->getElements(), Image<PIXEL>::getDimension());
  }

  /**
    Returns the rows of the image for non-modifying access.
  */
  ReadableColumns getColumns() const noexcept {
    return ReadableColumns(elements->getElements(), Image<PIXEL>::getDimension());
  }

  /**
    Returns the first element of the image for modifying access. This will
    force the image to be copied if shared by multiple image objects.
  */
  Pixel* getElements();

  /**
    Returns the first element of the image for non-modifying access.
  */
  const Pixel* getElements() const noexcept;
};

template<class PIXEL>
ArrayImage<PIXEL>::ArrayImage()  :
  Image<PIXEL>(Dimension(0, 0)), elements(new ReferenceCountedAllocator<Pixel>(0)) {
}

template<class PIXEL>
ArrayImage<PIXEL>::ArrayImage(const Dimension& dimension)  :
  Image<Pixel>(dimension), elements(new ReferenceCountedAllocator<Pixel>(dimension.getSize())) {
}

template<class PIXEL>
ArrayImage<PIXEL>::ArrayImage(const ArrayImage& copy) noexcept :
  Image<Pixel>(copy), elements(copy.elements) {
}

template<class PIXEL>
typename ArrayImage<PIXEL>::Pixel* ArrayImage<PIXEL>::getElements()  {
  elements.copyOnWrite();
  return elements->getElements();
}

template<class PIXEL>
const typename ArrayImage<PIXEL>::Pixel* ArrayImage<PIXEL>::getElements() const noexcept {
  return elements->getElements();
}

typedef ArrayImage<GrayPixel> GrayImage;
typedef ArrayImage<ColorPixel> ColorImage;
typedef ArrayImage<ColorAlphaPixel> ColorAlphaImage;
typedef ArrayImage<float> FloatImage;
typedef ArrayImage<Complex<float> > ComplexImage;
typedef ArrayImage<Complex<double> > ComplexDImage;
typedef ArrayImage<Complex<long double> > ComplexLDImage;

}; // end of gip namespace
