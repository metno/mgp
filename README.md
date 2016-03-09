# MGP

## Overview

MGP (<b>M</b>ET <b>G</b>eographic <b>P</b>olygon Library) is a Qt/C++-based tool for processing polygons in a meteorological context.
</b> Polygons are assumed to be defined directly in terms of <a href="https://en.wikipedia.org/wiki/Spherical_coordinate_system">spherical coordinates</a>
and handling of <a href="https://en.wikipedia.org/wiki/Map_projection">map projections</a> is outside the scope of the tool. For finding intersections etc.,
polygon edges are in most cases assumed to follow the <a href="https://en.wikipedia.org/wiki/Great_circle">great circle</a>.

The following use cases are currently supported:

<b>Use case 1:</b> Apply a sequence of filters to a set of input polygons and generate a set of output polygons.
Examples of filters include
* <i>south of a latitude</i>,
* <i>northeast of a line defined by two geographical locations</i>, and
* <i>inside a polygon defined by a sequence of three or more geographical locations</i>.

<b>Use case 2:</b> Generate a SIGMET/AIRMET expression from a sequence of filters.

<b>Use case 3:</b> Generate a sequence of filters from a SIGMET/AIRMET expression, and indicate matched and incomplete ranges.


## API/library

The MGP C++ library is located under lib/. The public API offers classes, typedefs and global functions for handling the three use cases listed above.


## MGPView tool

The MGPView tool is located under apps/mgpview/. It is a stand-alone GUI application that demonstrates the capabilities of MGP.
