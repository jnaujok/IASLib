# IASLib
The Irene Adler Software Library (C++)

This library started as a project, way back in 1994, to write a cross-platform library in C++ that could run on Windows, Mac, or Linux systems equally well in order to write a platform-independent Ray Tracing program that ran on a non-homogenous Parallel Virtual Machine (PVM) network as part of a college Independent study.

This was so successful, that the library kept on growing. Over the years it's been used for lots of different projects, and has kept growing to provide more and more functionality. 

It became the basis of a system I developed for MCI (before the WorldCom days) that provided a WebServer for hundreds of concurrent users back in the late 1990's -- even outperforming the Netscape Enterprise Server that ran alongside it.

It was used to develop a caching solution for a major shipping carrier that performed over 2 billion operations per day. And parts of it were used to replace an Oracle layer that was so effective that it took one operation from 45 minutes to under 10 seconds to run. 

The library includes things like memory debugging and leak detection, including the detection of buffer overwrites or underwrites, including the file and line where the memory was allocated, and where it was freed. It detects memory leaks at program shutdown. It even provides Run-Time Type Information that can allow you to determine the type of any class derived from a CObject base type.

Overall, this library has been majorly helpful, right up to the present day where I was able to implement a working SIP server (SBC) in just a few weeks using the code in the library (and extending some of it, of course) to produce what I think is a very helpful framework for delivering cross-platform solutions. 

All without caring what platform you're running on.
