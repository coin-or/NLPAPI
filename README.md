# NLPAPI

## What is NLPAPI?

NLPAPI is a subroutine library with routines for building nonlinear programming problems.
The general form is an objective with a set of simple bounds, equality and inequality constraints.
It is built around the "Group Partially Separable" structure that [LANCELOT](http://www.numerical.rl.ac.uk/lancelot/blurb.html) defines, but constraints and objective may also be defined as functions of the problem variables.

## How do I use NLPAPI?

The user creates a "NLProblem", which is a pointer to a data structure.
Then the objective and constraints are added with various calls.
Once the problem is defined our interface to LANCELOT may be used to create a NLLancelot data structure, and pass it and the problem to a routine which either minimizes or maximizes the objective subject to the constraints.

## What do I need to build NLPAPI?

The source code for NLPAPI is stand-alone.
If the user wishes to use our LANCELOT interface he must obtain his own copy of [LANCELOT](http://www.numerical.rl.ac.uk/lancelot/blurb.html).
More information can be found [here](http://www.coin-or.org/NLPAPI/).

## Download and Installation Instructions

The **very short version of the download and installation instructions** for UNIX-like system (including Linux and Cygwin) is this:
1. Get the code via subversion using the following command
  
     svn co https://projects.coin-or.org/svn/NLPAPI/trunk NLPAPI

2. Go into the downloaded directory and edit the file share/config.site to give local lib and include dirs for LANCELOT. (Last two lines in the file).

3. Run the configuration script

     ./configure

   Make sure the last line of output says that the configuration was successful.
   The `configure` script has many customization features.
   To see them, type `./configure --help`.
   In particular, to install a COIN-OR package at a location accessible to all
   users on your machine, you can run `./configure --prefix=/usr/local`.

4. Compile the code:

      make

5. Install the generated libraries, executables, and header files:

      make install

   This will create subdirectories `bin`, `lib`, and `include` in the download directory with the product of the compilation.

## On what operating systems can NLPAPI be used?

NLPAPI uses autoconf.
It has been tested on AIX 4.3, AIX5.1, Cygwin, and !RedHat 8.0, but should install cleanly on any UNIX machine.

## Are there any examples or references for the use of NLPAPI?

Source for several examples is included, as well as a SIF decoder which will translate many SIF files (no guarantees!) into C and Fortran source for defining the NLPAPI problem.

Documentation is included in a tex file, or the user may view a pdf version of the [User's Guide](http://www.coin-or.org/NLPAPI/NLPAPI-UG.pdf),
or the [Programming Reference](http://www.coin-or.org/NLPAPI/NLPAPI-Ref.pdf).
