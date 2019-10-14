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

#include <base/configuration.h>

#if (_COM_AZURE_DEV__BASE__FLAVOR == _COM_AZURE_DEV__BASE__WIN32)
#if defined(_COM_AZURE_DEV__GIP__SHARED_LIBRARY_BUILD)
#  define _COM_AZURE_DEV__GIP__API __declspec(dllexport)
#elif defined(_COM_AZURE_DEV__GIP__SHARED_LIBRARY)
#  define _COM_AZURE_DEV__GIP__API __declspec(dllimport)
#endif
#endif

#if !defined(_COM_AZURE_DEV__GIP__API)
#  define _COM_AZURE_DEV__GIP__API
#endif
