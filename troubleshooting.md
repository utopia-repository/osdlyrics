Here is a place to register some troubleshooting with the development or with the compilation.

>The precisions bellow were got when make a compilation on [UMI](http://umiproject.sf.net) codename *peace&love*;
an Ubuntu 14.04 LTS base system. However it may work for those who have the same issues.

When run the `autogen.sh` script or run command like `autoconf` or `autoreconf -i`:

- if have error with output containing this line : `but option 'subdir-objects' is disabled`, 
  then try to replace the line `AM_INIT_AUTOMAKE([-Wall -Werror])` by `AM_INIT_AUTOMAKE([subdir-objects -Wall -Werror])` in the *configure.ac* files.
  The good practice is to comment the first one and add the other.

- if have error with output containing these lines: `syntax error near unexpected token `0.35.0'` and `IT_PROG_INTLTOOL(0.35.0)`,
  try to install *intltool* package.
  On debian like system: `sudo apt-get install intltool`

- if have a problem with a package dependencies, just install the needed package (particulary the development version).
