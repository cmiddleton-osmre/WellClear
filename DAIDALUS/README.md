![](../docs/DAIDALUS.jpeg)

Reference Manual - DAIDALUS-v1.0.1 (Work in Progress)
===

Table of Contents
=================

   * [Introduction](#introduction)
   * [Software Library](#software-library)
   * [Preliminaries](#preliminaries)
      * [Packages and Name Space](#packages-and-name-space)
      * [Units](#units)
      * [Earth Projection and Aircraft States](#earth-projection-and-aircraft-states)
      * [Conventions, Misnomers, and Gotchas](#conventions-misnomers-and-gotchas)
   * [The Class Daidalus](#the-class-daidalus)
      * [Creating and Configuring a Daidalus Object](#creating-and-configuring-a-daidalus-object)
      * [Adding Ownship and Traffic States](#adding-ownship-and-traffic-states)
      * [Providing Wind Information](#providing-wind-information)
      * [Conflict Detection Logic](#conflict-detection-logic)
      * [Alerting Logic](#alerting-logic)
      * [Maneuver Guidance Logic](#maneuver-guidance-logic)
   * [The Class KinematicMultiBands](#the-class-kinematicmultibands)
      * [Track (or Heading) Bands](#track-or-heading-bands)
      * [Ground Speed (or Air Speed) Bands](#ground-speed-or-air-speed-bands)
      * [Vertical Speed Bands](#vertical-speed-bands)
      * [Altitude Bands](#altitude-bands)
   * [Parameters](#parameters)
   * [Pre-Defined Configurations](#pre-defined-configurations)
   * [Advanced Features](#advanced-features)
   * [Contact](#contact)

# Introduction

DAIDALUS (Detect and AvoID Alerting Logic for Unmanned Systems) is a
reference implementation of the detect and avoid (DAA) functional
requirements  described in
Appendix G of the Minimum Operational Performance Standards (MOPS)
Phase I for Unmanned Aircraft Systems (UAS) developed by RTCA
Special Committee 228 (SC-228).
At the core of the  RTCA SC-228 DAA concept,
there is a mathematical definition of the well-clear concept. Two
aircraft are considered to be *well clear* of each other if
appropriate distance and time variables determined by the relative
aircraft states remain outside a set of predefined threshold
values. These distance and time variables are closely related to
variables used in the Resolution Advisory (RA) logic of the Traffic
Alert and Collision Avoidance System Version II (TCAS II).

DAIDALUS includes algorithms for determining the current well-clear
status between two aircraft and for predicting a well-clear violation
within a lookahead time, assuming non-maneuvering trajectories. In the
case of a predicted well-clear violation, DAIDALUS also includes an
algorithm that computes the time interval of well-clear
violation. DAIDALUS implements algorithms for computing maneuver
guidance, assuming a simple kinematic trajectory model for the
ownship. Manuever guidance is computed in the form of range of track,
ground speed, vertical speed, and altitude values called
*bands*. These bands represent areas in the airspace the ownship has
to avoid in order to maintain well-clear with respect to traffic
aircraft. In the case of a loss of well-clear, or when a well-clear
violation is unavoidable, the maneuver guidance algorithms provide
well-clear recovery bands. Recovery bands represents areas in the
airspace that allow the ownship to to regain well-clear status in a
timely manner according to its performance limits. Recovery bands are
designed so that they also protect against a user-specified minimum
horizontal and vertical separation.  DAIDALUS also implements a
pair-wise alerting logic that is based on a set set of increasingly
conservative alert levels called *preventive*, *corrective*, and
*warning*.

DAIDALUS is implemented in C++ and Java and the code is available
under [NASA's Open Source Agreement](../LICENSES/).  The
implementations are modular and highly configurable. The DAIDALUS core
algorithms have been [formally specified and verified](../PVS) in the Prototype
Verification System ([PVS](http://pvs.csl.sri.com)).  The examples
provided in this document are written in Java.  Except for language
idiosyncrasies, both Java and C++ interfaces are identical.

#  Software Library
DAIDALUS is available as a software library. After getting the source
code from [GitHub/WellClear](https://github.com/nasa/WellClear), the
library can be compiled using the Unix utility `make` with the
provided `Makefile` in both the [Java](Java/Makefile) and [C++](C++/Makefile) directories. In Java,
the `make` command produces a jar file:

```
$ make lib
** Building library lib/DAIDALUS.jar
...
** Library lib/DAIDALUS.jar built
```

In C++, the `make` command will generate the static library
`lib/DAIDALUS.a`.

The sample application `DaidalusExample`, which is available in
[Java](Java/src/DaidalusExample.java) and
[C++](C++/src/DaidalusExample.cpp), illustrates the main
functionalities provided by DAIDALUS including reading/writing
configuration files, detection logic, alerting logic, maneuver
guidance logic, and computation of loss of well-clear contours.  This
application can be compiled using the provided `Makefile`. In Java:

```
** Building example applications
/usr/bin/c++ -o DaidalusExample -Iinclude  -Wall -O  src/DaidalusExample.cpp lib/DAIDALUS.a
/usr/bin/c++ -o DaidalusAlerting -Iinclude  -Wall -O
src/DaidalusAlerting.cpp lib/DAIDALUS.a
** To run DaidalusExample type:
./DaidalusExample
** To run DaidalusAlerting type, e.g.,
./DaidalusAlerting --nomb --out H1.csv ../Scenarios/H1.daa
```
To run the example application in a Unix environment, type

```
$ ./DaidalusExample
```

To run the example batch application in a Unix environment, type, for example,

```
./DaidalusAlerting --nomb --out H1.csv ../Scenarios/H1.daa
```

In this case, DAIDAILUS will compute alerting information for [Nominal
B](../Configurations/WC_SC_228_nom_b.txt) configuration with batch scenario [H1.daa](../Scenarios/H1.daa).

Scripts are provided to produce graphs containing guidance and alerting
information. For example, 

```
./DrawMultiBands --conf ../Configurations/WC_SC_228_std.txt ../Scenarios/H1.daa
```

produces a file `H1.draw`, which can be processed with the Python
script `drawmultibands.py` to produce a PDF file, e.g.,

```
../Scripts/drawmultibands.py H1.draw
```

# Preliminaries

## Packages and Name Space
In Java, DAIDALUS consists of three packages in the hierarchy
`gov.nasa.larcfm`: `IO`, `Util`, and `ACCoRD`. In C++, the DAIDALUS
code uses the name space `larcfm`. This document
will refer to classes in these packages and name space through unqualified
names.  The following table lists the Java packages for the main
DAIDALUS classes and interfaces.

| Class/Interface | Package |
| --|--|
| `AlertLevels` | `ACCoRD` |
| `AlertThresholds` | `ACCoRD` |
| `BandsRegion` | `ACCoRD` |
| `CD3DTable` |  `ACCoRD` |
| `CDCylinder` |  `ACCoRD` |
| `ConflictData` |  `ACCoRD` |
| `Daidalus` |  `ACCoRD` |
| `DaidalusFileWalker` |  `ACCoRD` |
| `Detection3D` |  `ACCoRD` |
| `Horizontal` |  `ACCoRD` |
| `KinematicBandsParameters` |  `ACCoRD` |
| `KinematicMultiBands` |  `ACCoRD` |
| `TCASTable` |  `ACCoRD` |
| `TCAS3D` |  `ACCoRD` |
| `TrafficState` | `ACCoRD` |
| `Vertical` |  `ACCoRD` |
| `WCVTable` |  `ACCoRD` |
| `WCV_TAUMOD` |  `ACCoRD` |
| `WCV_TCPA` |  `ACCoRD` |
| `WCV_TEP` |  `ACCoRD` |
| `Interval` | `Util` |
| `Position` |  `Util` |
| `Units` |  `Util` |
| `Vect2` | `Util` |
| `Vect3` | `Util` |
| `Velocity` |  `Util` |

## Units
DAIDALUS core algorithms use as internal units meters, seconds, and
radians. However,  interface methods that set or get a value have a String argument, where the units are
explicitly specified. The following table provides a list of symbols and the corresponding
string representation supported by DAIDALUS.

| Units  | String |
| --|-- |
| milliseconds | `"ms"` |
| seconds | `"s"` |
| minutes | `"min"` |
| hours | `"h"` | `"hr"` |
| meters | `"m"` |
| kilometers | `"km"` |
| nautical miles | `"nmi"`, `"NM"`  |
| feet | `"ft"` |
|  knots | `"knot"`, `"kn"`, `"kts"` |
|  meters per second | `"m/s"` |
|  kilometers per hour | `"kph"`, `"km/h"` |
| feet per minute | `"fpm"`, `"ft/min"` |
|  meters per second<sup>2</sup>|`"m/s^2"` |
| 9.80665 m/s<sup>2</sup> | `"G"` |
|  degrees | `"deg"` |
| radians | `"rad"` |
| degrees per second | `"deg/s"` |
| radians per second | `"rad/s"` |

The class `Units` provides the following static methods for converting
to and from internal units and  from one unit into another one.

 * `static double to(String unit, double value)`: Converts `value` to the units indicated by
  the parameter `unit` from internal units.
 * `static double from(String unit, double value)`: Converts `value` from the units indicated by
  the parameter `unit` to internal units.
* `static double convert(double fromUnit, double toUnit, double
value)`: Converts `value` from the units indicated by the parameter `fromUnit` to the units indicated by the parameter
`toUnit`.
  
## Earth Projection and Aircraft States
DAIDALUS core algorithms use a Euclidean a local East, North, Up (ENU)
Cartesian coordinate system.  However, aircraft sates can be
provided in geodesic coordinates. In this case, DAIDALUS uses an
orthogonal projection of the ownship and traffic geodesic coordinates
onto a plane tangent to the projected ownship position on the surface
of the earth. The vertical component is not transformed.

Aircraft positions are represented by the class
`Position`. Positions can be specified in either geodesic
coordinates or ENU Cartesian coordinates through
the following static methods. 

* `static Position makeLatLonAlt(double lat, String lat_unit, double
  lon, String lon_unit, double alt, String alt_unit) `: Creates  
  geodesic position at latitude `lat`, longitude `lon`, and altitude
  `alt` given in `lat_unit`, `lon_unit`, and `alt_unit` units, respectively.
  Northern latitudes and eastern longitudes are positive. 
* `static Position makeXYZ(double x, double x_unit, double y, double
  y_unit, double z, double z_unit)`: Creates  ENU position with
  Euclidean coordinates (`x`,`y`,`z`) given in 
  `x_unit`, `y_unit`, and `z_unit` units, respectively.

Aircraft velocities are represented by the class
`Velocity`. Velocities are specified relative to the ground in
either polar or ENU Cartesian coordinates using the following static
methods.

* `static Velocity makeTrkGsVs(double trk, String trk_unit, double gs, String gs_unit,
      double vs, String vs_unit)`: Creates velocity with horizontal direction `trk` (true north,
      clockwise convention),  horizontal  magnitude `gs`, and vertical component `vs` given in
	  `trk_unit`, `gs_unit`, and `vs_unit` units, respectively.
* `static Velocity makeVxyz(double vx,
	  double vy, String
      vxy_unit, double vz, String vz_unit)`: Creates ENU velocity
      with Euclidean coordinates (`vx`,`vy`,`vz`) given in `vxy_unit`,
      `vxy_unit`, and `vz_unit` units respectively.

## Conventions, Misnomers, and Gotchas
The following conventions are used through the code.

* Northern latitudes and eastern longitudes are positive.
* Angles representing aircraft direction are specified in true north clockwise convention.
* Wind velocities are specified  using the *TO* direction, i.e., the
direction the wind blows, as opposed to the *FROM* direction, i.e., the direction the wind originates. Furthermore, the
vertical component of a wind velocity is assumed to be `0`. 
* Aircraft positions can be specified in geodesic or ENU
  coordinates. However, one of the two systems has to be used
  consistently for all aircraft.
* DAIDALUS input states are assumed to be ground-based. DAIDALUS
  outputs are ground-based except when a wind vector is provided, in
  which case outputs are air-based.

DAIDALUS uses a simple wind model. When a wind vector is provided,
DAIDALUS uniformly applies the wind vector to all aircraft states.  An
important consequence of setting a wind vector is that all
computations and outputs become relative to the wind. In this case,
methods whose names use the word *Track* and *GroundSpeed* become
misnomers. These methods will indeed provide heading and airspeed
information, respectively.

DAIDALUS provides methods to retrieve aircraft states as they are
passed to its core logics. However, these states are not necessarily
the same states provided as inputs. In particular, before any
computation, aircraft states may be projected in time to synchronize
them in time to the applicability time. Furthermore, if wind vector is
provided, ground-based input velocities are transformed relative to the
wind.

Methods in DAIDALUS fail silently and return invalid values when
called with invalid parameters. The following tables list methods that
check the validity of values in DAIDALUS classes.

| Class/Type | Validity Check (Java) |
| -- | -- |
| `double d;` | `Double.isFinite(d)` |
| `BandsRegion r;` | `r.isValidBand()` |
| `Interval i;` | `i.isEmpty()` |
| `Velocity v;` | `!v.isInvalid()` |
| `Position p;` | `!p.isInvalid()` |
| `TrafficState s;` | `s.isValid()` |

In C++, the methods are the same except in the following cases.

| Class/Type | Validity Check (C++) |
| -- | -- |
| `double d;` | `ISFINITE(d)` |
| `BandsRegion r;` | `BandsRegion::isValidBand(r)` |

Furthermore, negative integer values are returned as invalid values
in methods that under normal conditions return a natural number. 

# The Class `Daidalus`

The DAIDALUS software library is ownship
centric. Its main functional features are provided through the class
`Daidalus`, which maintains and computes information from
the point of view of the ownship. In a multi-threaded application, a
`Daidalus` object should not be shared by different threads.

Except for the information kept in a `Daidalus` object, DAIDALUS
functionalities are memoryless, i.e., they process information at a
given moment in time and do not keep information computed in
previous calls. A typical DAIDALUS application has the
following steps:

1. Create and configure a `Daidalus` object. A `Daidalus` object can
be reconfigured at any time. However, in a typical application, a `Daidalus` object is
configured at the beginning of the application and the configuration
remains invariant through the execution of the program.
1. Get state information for ownship and traffic aircraft from avionics systems.
    DAIDALUS does not provide any functionality to filter or
   pre-process state information. If needed, any pre-processing has to be
   implemented outside DAIDALUS.
1. Set ownship and traffic states into `Daidalus`
   object. 
1. If available, set wind information into `Daidalus` object.
1. Get detection, alerting, and guidance information from `Daidalus`
object.
1. Display information. DAIDALUS does not provide any functionality to
display or post-process its outputs. If needed, any
post-processing has to be implemented outside DAIDALUS.
1. Repeat from 2.

## Creating and Configuring a `Daidalus` Object
In Java, a `Daidalus` object is created through the invokation
```java
Daidalus daa = new Daidalus();
```
The variable `daa` is initialized to default values, which corresponds
to an unbuffered well-clear volume, i.e., DMOD=HMD=0.66 nmi, TAUMOD=35 s,
ZTHR=450 ft, with kinematic bands computations.  The default configuration can
be changed either programmatically or via a configuration file. For
instance,

```java
daa.set_Buffered_WC_SC_228_MOPS(nom);
```
changes the configuration of the `daa` object to a buffered well-clear
volume, i.e.,  DMOD=HMD=1.0 nmi, TAUMOD=35 s,
ZTHR=450 ft, and TCOA=20 s, with kinematic bands computation. The parameter `nom`
represents a boolean value. When this value is `false` the configuration
is called *Nominal A* and sets the maximum turn rate of the ownship to 1.5 deg/s.
The configuration *Nominal B* is obtained by
setting the parameter `nom` to `true`. This configuration is exactly as
Nominal A, but sets the ownship maximum turn rate to 3.0 deg/s.

DAIDALUS supports a large set of configurable parameters that
govern the behavior of the detection, alerting, and
maneuver guidance logics. These parameters are described in the
Section [Parameters](#parameters). The simplest way to configure a
`Daidalus` object is through a configuration file. Examples of
configuration files are provided in the directory
[`Configurations`](Configurations/), which are 
explained in the Section
[Pre-Defined Configurations](#pre-defined-configurations).  A
configuration file only needs to provide values to the
parameters that change. The method call
```java
daa.loadFromFile(filename);
```
loads a configuration file, whose name is indicated by the parameter
`filename`, into the `Daidalus` object `daa`.
The current configurations of a `Daidalus` object `daa` can be written into a file
using the method call
```java
daa.saveToFile(filename);
```
The methods
`loadFromFile` and `saveToFile` return a boolean value. The value
`false` indicates that an input/output error has occurred, e.g.,a file
cannot be read because it does not exist or a file cannot be written because
insufficient permissions.

## Adding Ownship and Traffic States
A `Daidalus` object `daa` maintains a list of aircraft states at a
given time of applicability. The ownship state
can be added into a `Daidalus` object `daa`  through the method invokation
```java
daa.setOwnshipState(ido,so,vo,to);
```
where `ido` is a string that indicates the ownship identifier (string), `so` is a
`Position` object that indicates ownship position, `vo` is a
`Velocity` object that indicates ownship velocity, and `to` is a time
stamp in seconds of the ownship state, i.e., the time of applicability. Setting the ownship state into
a `daa` object, resets the list of aircraft. Thus, for a given time of
applicability, the ownship state has to be added before any other
aircraft state.

Traffic states can be added into `daa` using the method invokation
```java
daa.addTrafficState(idi,si,vi);
```
where `idi`, `si`, and `vi` are the traffic identifier, position, and
velocity, respectively.  Traffic states do not require a time stamp since it is
assumed to be the same the ownship's. If a time stamp is 
provided, e.g., ` daa.addTrafficState(idi,si,vi,ti)`, the
position of the traffic aircraft is linearly projected (forwards or
backwards) from `ti` to `to`, the time stamp of the ownship, so that
all traffic states are synchronized in time with the ownship.

Aircraft identifiers are assumed to be unique within a `daa`
object. Furthermore, the order in which traffic aircraft are added is
relevant. Indeed, several `Daidalus` methods use the index of an
aircraft in the list of aircraft as a reference to the aircraft. The
index `0` is reserved for the ownship.
The method `addTrafficState`
returns the index of an aircraft after it has been added to the list
of aircraft. The following methods are provided by the class
`Daidalus`.

* `int aircraftIndex(String id)`: Returns the index of the aircraft
identified by the string value of `id`.
The returned value is negative if the list of aircraft does not
include an aircraft identified by the string value of `id`.
* `int numberOfAircraft()`: Returns the number of aircraft in the list
of aircraft, including the ownship.
* `int lastTrafficIndex()`: Returns the index of the last
aircraft added to the list of aircraft.
* `double getCurrentTime()`: Returns the time of applicability in seconds.
* `void setCurrentTime(double time)`: Projects aircraft states to time
  `time`, which is specified in seconds, and sets that time as the time
  of applicability.

## Providing Wind Information
If available, a wind vector can be provided to a `Daidalus` object
`daa` using the method call
```java
daa.setWindField(wind);
```
where `wind` is a `Velocity` object, whose vertical component
is assumed to be `0`.  The wind velocity is specified in the direction
the wind blows. The specified wind vector is uniformity applied to all
traffic states before any computation. Therefore, after this method call, all
computations become relative to the wind.

## Conflict Detection Logic
The time to loss of well-clear, in seconds, between the ownship and the traffic aircraft at index `idx` for
the corrective alert level and lookahead time
configured in the `Daidalus` object `daa` can be 
computed as follows.
```java
double t2v = daa.timeToViolation(idx);
```
If `t2v` is zero, the aircraft are in violation at current time. The
method `timeToViolation` returns positive infinity when the
aircraft are not in conflict within the lookahead time. It returns
Not-A-Number when `idx` is not a valid aircraft index.

To compute time to loss of well-clear with respect to
any alert level, see Section [Advanced Features](#advanced-features).

## Alerting Logic
Given a `Daidalus` object `daa` of type `Daidalus`, 
the alert level between the ownship and the traffic aircraft at
index `idx` can be computed as follows
```java
int alert_level = daa.alerting(idx);
```
The value of `alert_level` is negative when `idx` is not a valid
aircraft index in `daa`. If `alert_level` is zero, no alert is issued
for the ownship and the traffic aircraft at index `idx` at
time of applicability. Otherwise, `alert_level` is a positive
numerical value that indicates the alert level, which by default are
configured as follows.

| `alert_level` | RTCA SC-228 Alert Level |
| -- | -- |
| `1` | Preventive |
| `2` | Corrective |
| `3` | Warning |

The `Daidalus` object `daa` can be configured to an arbitrary number
of alert levels and each alert level is highly configurable
(see Section [Advanced Features](#advanced-features)). The only
restriction is that alert levels are ordered by increased level of
severity.

## Maneuver Guidance Logic
In DAIDALUS, maneuver guidance is provided by the class `KinematicMultiBands`.
The following code creates an object `bands` of type `KinematicMultiBands` for the
aircraft in the `Daidalus` object `daa`.
```java  
KinematicMultiBands bands = daa.getMultiKinematicBands();  
``` 
The previous code is written in Java. The corresponding code in C++ is
as follows.
```c++
KinematicMultiBands bands;
daa.multiKinematicBands(bands); 
```
For efficiency reasons, bands are computed in a lazy way, i.e., the
methods `getKinematicMutliBands` (in Java) and `kinematicMultiBands` (in
C++) do not actually compute the bands. Section [The Class `KinematicMultiBands`](#the-class-kinematicmultibands),
bands are computed when the object `bands` is used. 

# The Class `KinematicMultiBands`
DAIDALUS provides maneuver guidance in the form of ranges of ownship
maneuvers called *bands*.
Four dimensions of bands are computed by DAIDALUS:

1. Horizontal maneuver ranges. These maneuvers indicate true track ranges,
except when wind information is provided. If that is the case, these
maneuvers indicate true heading ranges.
1. Horizontal speed ranges. These maneuvers indicate ground speed ranges,
except when wind information is provided. If that is the case, these
maneuvers indicate  air speed ranges.
1. Vertical speed ranges.
1. Altitude ranges.

In each dimension, bands are represented by an ordered list of
intervals that are disjoint except at the boundaries. These intervals
completely partition a range of values from a configurable minimum
value to a configurable maximum value. Each interval is associated
with a `BandsRegion`, which is defined as an enumerated type
consisting of the following values.

* `NONE`: Maneuvers indicated by intervals of this type are conflict free.
* `FAR`: This type of intervals is not configured by default in the
  maneuver guidance logic. However, it could be configured to indicate
  maneuvers that lead to a preventive alert.
* `MID`: The maneuvers indicated by intervals of this type lead to a corrective
alert.
* ` NEAR`: The maneuvers indicated by intervals of this type lead to a warning
alert.
* ` RECOVERY`: The maneuvers indicated by intervals of this type regain
well-clear status in a timely manner and without violating
configurable separation minima. By definition of the maneuver guidance logic, intervals of type
`RECOVERY` are computed only when there are no intervals of type
`NONE`.
*  `UNKNOWN`: This type of intervals is returned for maneuvers that
are  unassessed, i.e., values that are outside the configured minimum/maximum values.

 Ownship performance limits and other parameters that govern the
 maneuver guidance logic can be configured  in either the
 `Daidalus` object from which the `bands` object
is constructed or in the `bands` object directly. The methods to set
and get these configuration parameters are described in 
 Section [Parameters](#parameters).  

## Track (or Heading) Bands
The computation of track bands (heading bands, when wind information is
provided) requires either a turn rate or a bank angle. If both the turn rate
and the bank angle are zero, track/heading bands are computed assuming
instantaneous track/heading manevuers.  The following loop iterates the list of
intervals in the track/heading  bands and for each interval gets its upper and
lower bounds and its type.
```java
  for (int i = 0; i < bands.trackLength(); i++ ) {  
    Interval iv = bands.track(i,"deg"); //i-th band region
    double lower_trk = iv.low; //[deg]
    double upper_trk = iv.up;  //[deg]
    BandsRegion regionType = bands.trackRegion(i);
	...
  } 
```

## Ground Speed (or Air Speed) Bands
The computation of ground speed bands (air speed bands, when wind
information is provided) requires a horizontal acceleration. If this
acceleration is zero, ground speed/ air speed bands are computed
assuming instantaneous ground speed/air speed manevuers.  The following loop iterates the list of
intervals in the ground speed/air speed  bands and for each interval gets its upper and
lower bounds and its type.
```java
  for (int i = 0; i < bands.groundSpeedLength(); i++ ) {  
    Interval iv = bands.groundSpeed(i,"knot"); //i-th band region
    double lower_gs = iv.low; //[knot]
    double upper_gs = iv.up;  //[knot]
    BandsRegion regionType = bands.groundSpeedRegion(i);
    ... 
  } 
```

## Vertical Speed Bands
The computation of vertical speed bands requires a vertical acceleration. If this
acceleration is zero, vertical speed bands are computed
assuming instantaneous vertical speed manevuers.  The following loop iterates the list of
intervals in the vertical speed  bands and for each interval gets its upper and
lower bounds and its type.
```java
  for (int i = 0; i < bands.verticalSpeedLength(); i++ ) {  
    Interval iv = bands.verticalSpeed(i,"fpm"); //i-th band region
    double lower_vs = iv.low; //[fpm]
    double upper_vs = iv.up;  //[fpm]
    BandsRegion regionType = bands.verticalSpeedRegion(i);
    ... 
  } 
```

## Altitude Bands
The computation of altitude bands requires a vertical acceleration and
a vertical rate. If both the vertical acceleration and the vertical
rate are zero, altitude bands are computed
assuming instantaneous altitude manevuers.  The following loop iterates the list of
intervals in the altitude bands and for each interval gets its upper and
lower bounds and its type.

```java
  for (int i = 0; i < bands.altitudeLength(); i++ ) {  
    Interval iv = bands.altitude(i,"ft"); //i-th band region
    double lower_alt = iv.low; //[ft]
    double upper_alt = iv.up;  //[ft]
    BandsRegion regionType = bands.altitudeRegion(i);
    ... 
  } 
```

# Parameters
(Work in Progress)

# Pre-Defined Configurations
The directory [`Configurations`](Configurations/) includes the following configurations files
that are related to RTCA SC-228 MOPS Phase I.

* [`WC_SC_228_std.txt`](Configurations/WC_SC_228_std.txt):
  This configuration implements the alerting and maneuvering guidance
  logics for a the standard definiton of DAA Well-Clear provided in 
  MOPS  Section 2.2.4.3.1 (also see Appendix C). The configuration uses
  minimum average alerting time and hazard thresholds for computing
  preventive, corrective, and warning alerts and guidance. The
  maneuver guidance logic assumes instantaneous
  maneuvers. Furthermore, recovery bands saturate at violation of the
  cylinder defined by `DMOD` and `ZTHR`. This configuration is used by
  default when a `Daidalus` object is created. However, this
  configuration should only be used as reference to an ideal algorithm
  with perfect information.
  This configuration can be obtained as follows.
  ```java
  Daidalus daa  = new Daidalus();
  ```
  
* [`WC_SC_228_nom_a.txt`](Configurations/WC_SC_228_nom_a.txt): This
  configuration corresponds to a nominal instantiation of DAIDALUS for
  the class of aircraft that are able to perform a turn rate of 1.5
  deg/s and meet the performance maneuverability listed in
  MOPS Section 1.2.3 System Limitations.
  In this configuration, the alerting and maneuvering guidance logics
  use buffered definitions of preventive, corrective, and warning
  volumes to accommodate for certain types of sensor uncertainty.
  The maneuver guidance logic assumes kinematic maneuvers
  maneuvers. Furthermore, recovery bands are computed until NMAC.
  The only difference between configurations `WC_SC_228_nom_a.txt` and
  `WC_SC_228_nom_b.txt` is the turn rate.
  This configuration can be obtained as follows. 
  ```java
  Daidalus daa  = new Daidalus();
  daa.set_Buffered_WC_SC_228_MOPS(false);
  ```

* [`WC_SC_228_nom_b.txt`](Configurations/WC_SC_228_nom_b.txt): This
  configuration corresponds to a nominal instantiation of DAIDALUS for
  the class of aircraft that are able to perform a turn rate of 3.0
  deg/s and meet the performance maneuverability listed in
  MOPS Section 1.2.3 System Limitations.
  In this configuration, the alerting and maneuvering guidance logics
  use buffered definitions of preventive, corrective, and warning
  volumes to accommodate for certain types of sensor uncertainty.
  The maneuver guidance logic assumes kinematic maneuvers
  maneuvers. Furthermore, recovery bands are computed until NMAC.
  The only difference between configurations `WC_SC_228_nom_a.txt` and
  `WC_SC_228_nom_b.txt` is the turn rate.
  This configuration can be obtained as follows. 
  ```java
  Daidalus daa  = new Daidalus();
  daa.set_Buffered_WC_SC_228_MOPS(true);
  ```

* [`WC_SC_228_min.txt`](Configurations/WC_SC_228_min.txt): This
  configuration corresponds to the minimum detect and avoid
  threshold values used for the generation of the encounter
  characterization files in Appendix P.
  In this configuration, the alerting and maneuvering guidance logics use late alerting
  time and hazard volumes for computing preventive, corrective, and warning alerts and
  guidance. The maneuver guidance logic assumes instantaneous
  maneuvers. Furthermore, recovery bands are computed until NMAC.
  This configuration should only be used to check the performance of an actual
  implementation against the minimum values in the
  encounter characterization files in Appendix P.
  
* [`WC_SC_228_max.txt`](Configurations/WC_SC_228_max.txt): This
  configuration corresponds to the maximum detect and avoid
  threshold values used for the generation of the encounter
  characterization files in Appendix P.
  In this configuration, the alerting and maneuvering guidance logics use early alerting
  time and the non-hazard volumes for computing preventive, corrective, and warning alerts and
  guidance. The maneuver guidance logic assumes instantaneous
  maneuvers. Furthermore, recovery bands are computed until NMAC.
  This configuration should only be used to check the performance of an actual
  implementation against the maximum values in the
  encounter characterization files in Appendix P.

# Advanced Features
(Work in progress)

# Contact

[Cesar A. Munoz](http://shemesh.larc.nasa.gov/people/cam) (cesar.a.munoz@nasa.gov)



