/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    Copyright (C) 2001 by René Møller Fonseca <fonseca@mip.sdu.dk>

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#include <gip/transformation/LinearScale.h>

namespace gip {

LinearScale::LinearScale(DestinationImage* destination, const SourceImage* source) throw(ImageException) :
  Transformation<DestinationImage, SourceImage>(destination, source) {
  assert(source->getDimension().isProper(), ImageException("Unable to scale image"));
}

void LinearScale::operator()() throw() {
  if (!destination->getDimension().isProper()) {
    return;
  }

  unsigned int rows = destination->getDimension().getHeight();
  unsigned int columns = destination->getDimension().getWidth();
  unsigned int srcRows = source->getDimension().getHeight();
  unsigned int srcColumns = source->getDimension().getWidth();
  double stepPerRow = (double)(srcRows - 1)/(rows - 1);
  double stepPerColumn = (double)(srcColumns - 1)/(columns - 1);

  DestinationImage::Rows rowsLookup = destination->getRows();
  SourceImage::ReadableRows srcRowsLookup = source->getRows();

  --rows; // last row is a special case
  --columns; // last column is a special case

  DestinationImage::Rows::RowIterator row = rowsLookup.getFirst();
  double floatRow = 0;
  for (unsigned int rowCount = rows; rowCount > 0; --rowCount) {
    unsigned int srcRowIndex = static_cast<unsigned int>(floatRow); // round to zero
    double weightRow = floatRow - srcRowIndex;

    SourceImage::ReadableRows::RowIterator srcCurrentRow = srcRowsLookup[srcRowIndex];
    SourceImage::ReadableRows::RowIterator srcNextRow = srcCurrentRow;
    ++srcNextRow;

    DestinationImage::Rows::RowIterator::ElementIterator column = row.getFirst();
    double floatColumn = 0;
    for (unsigned int columnCount = columns; columnCount > 0; --columnCount) {
      unsigned int srcColumnIndex = static_cast<unsigned int>(floatColumn); // round to zero
      double weightColumn = floatColumn - srcColumnIndex;

      SourceImage::ReadableRows::RowIterator::ElementIterator srcColumn = srcCurrentRow[srcColumnIndex];
      ColorPixel temp = *srcColumn;
      double weight = (1 - weightRow) * (1 - weightColumn);
      double blue = temp.blue * weight;
      double green = temp.green * weight;
      double red = temp.red * weight;

      ++srcColumn;
      temp = *srcColumn;
      weight = (1 - weightRow) * weightColumn;
      blue += temp.blue * weight;
      green += temp.green * weight;
      red += temp.red * weight;

      srcColumn = srcNextRow[srcColumnIndex];
      temp = *srcColumn;
      weight = weightRow * (1 - weightColumn);
      blue += temp.blue * weight;
      green += temp.green * weight;
      red += temp.red * weight;

      ++srcColumn;
      temp = *srcColumn;
      weight = weightRow * weightColumn;
      blue += temp.blue * weight;
      green += temp.green * weight;
      red += temp.red * weight;

      ColorPixel result;
      result.blue = static_cast<Intensity>(blue); // overflow not possible
      result.green = static_cast<Intensity>(green); // overflow not possible
      result.red = static_cast<Intensity>(red); // overflow not possible
      *column = result;
      ++column;

      floatColumn += stepPerColumn;
    }

    {
      // initialize last column
      unsigned int srcColumnIndex = srcColumns - 1;
      double weight = (1 - weightRow);

      ColorPixel current = *srcCurrentRow[srcColumnIndex];
      ColorPixel next = *srcNextRow[srcColumnIndex];
      ColorPixel result;
      result.blue = static_cast<Intensity>(current.blue * weight + next.blue * weightRow);
      result.green = static_cast<Intensity>(current.green * weight + next.green * weightRow);
      result.red = static_cast<Intensity>(current.red * weight + next.red * weightRow);
      *column = result; // no need to increment column
    }

    floatRow += stepPerRow;
    ++row;
  }

  // initialize last row
  SourceImage::ReadableRows::RowIterator srcCurrentRow = srcRowsLookup[srcRows - 1];
  DestinationImage::Rows::RowIterator::ElementIterator column = row.getFirst();
  double floatColumn = 0;
  for (unsigned int columnCount = columns + 1; columnCount > 0; --columnCount) { // also the last pixel of the row
    unsigned int srcColumnIndex = static_cast<unsigned int>(floatColumn); // round to zero
    double weightColumn = floatColumn - srcColumnIndex;
    double weight = 1 - weightColumn;

    SourceImage::ReadableRows::RowIterator::ElementIterator srcColumn = srcCurrentRow[srcColumnIndex];
    ColorPixel current = *srcColumn;
    ++srcColumn;
    ColorPixel next = *srcColumn;
    ColorPixel result;
    result.blue = static_cast<Intensity>(current.blue * weight + next.blue * weightColumn);
    result.green = static_cast<Intensity>(current.green * weight + next.green * weightColumn);
    result.red = static_cast<Intensity>(current.red * weight + next.red * weightColumn);
    *column = result;
    ++column;

    floatColumn += stepPerColumn;
  }
}

}; // end of gip namespace
