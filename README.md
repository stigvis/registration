# Co-registration

Library dependencies:
Insight toolkit; For image operations.
Boost regex; Reading .hdr and .conf files.
Libmatio; I/O for Matlab image files.

Usage:

params.conf should be situated in the running folder, and contains all run parameters. See sample params.conf for
configuration.

To read in a hyperspectral image stored in a .img container, append the .img container as an additional input. Make
sure that the header file .hdr is in the same folder as the .img container. The middle band is chosen as the fixed
image for image registration.

./registration ~/sample.img

To read in multispectral images, append all the images as additional inputs. The first input is chosen as the fixed
image for image registration. Currently supports raw file format of size 1024x768. Change variables in
src/multispec.cpp as necessary.

./registration ~/sample1.raw ~/sample2.raw .. ~/sample99.raw

Note: The registration process does not support uint16_t, which is the stored format of the raw images. Thus, these
are cast to float for registration, and cast back before storing.
