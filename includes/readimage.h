//=======================================================================================================
// Copyright 2015 Asgeir Bjorgan, Lise Lyngsnes Randeberg, Norwegian University of Science and Technology
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
// http://opensource.org/licenses/MIT)
//=======================================================================================================

#ifndef READIMAGE_H_DEFINED
#define READIMAGE_H_DEFINED
#include <vector>
#include <boost/regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <string>
#include <iostream>

/**
 * Interleave, BIL or BIP. BSQ not supported.
 **/
enum interleave_t{BIL_INTERLEAVE, BIP_INTERLEAVE};

/**
 * Container for hyperspectral header file.
 **/
struct hyspex_header {
	///Interleave of image
	interleave_t interleave;
	///Number of pixels in the across-track axis
	int samples;
	///Number of wavelength bands
	int bands;
	///Number of lines in the image (along-track)
	int lines;
	///Offset of the image data from start of the hyperspectral file
	int offset;
	///Wavelengths
	std::vector<float> wlens;
	///Datatype of values in hyperspectral file
	int datatype;
};

/**
 * Image subset convenience struct for specifying a subset of the hyperspectral image file for reading.
 **/
struct image_subset {
	///Start pixel in the sample direction
	int start_sample;
	///End pixel in the sample direction
	int end_sample;
	///Start pixel in the line direction
	int start_line;
	///End pixel in the line direction
	int end_line;
	///Start wavelength band
	int start_band;
	///End wavelength band
	int end_band;
};

/**
 * Errors.
 **/
enum hyperspectral_err_t {
	///Successful
	HYPERSPECTRAL_NO_ERR,
	///File not found
	HYPERSPECTRAL_FILE_NOT_FOUND,
	///Could not find the requested property in the header file
	HYPERSPECTRAL_HDR_PROPERTY_NOT_FOUND,
	///Interleave in header file not supported by this software
	HYPERSPECTRAL_INTERLEAVE_UNSUPPORTED,
	///Datatype in hyperspectral image not supported
	HYPERSPECTRAL_DATATYPE_UNSUPPORTED,
	///???
	HYPERSPECTRAL_FILE_READING_ERROR
};

/**
 * Read header information from file.
 *
 * \param filename Filename
 * \param header Output header container
 * \return HYPERSPECTRAL_NO_ERR on success
 **/
hyperspectral_err_t hyperspectral_read_header(const char *filename, struct hyspex_header *header);

/**
 * Read hyperspectral image from file.
 *
 * \param filename Filename
 * \param header Header, already read from file using hyperspectral_read_header
 * \param image_subset Specified image subset
 * \param data Output data, preallocated to neccessary size (bands*samples*lines, get it from header (or in this case, calculate it from the specified subset)). Pixels are accessed by data[SAMPLES*BANDS*line_number + SAMPLES*band_number + sample_number].
 * \return HYPERSPECTRAL_NO_ERR on success
 **/
hyperspectral_err_t hyperspectral_read_image(const char *filename, struct hyspex_header *header, struct image_subset subset, float *data);

/**
 * Overloaded version of hyperspectral_read_image where it isn't neccessary to supply subset information, the full image will be read.
 *
 * \param filename Filename
 * \param header Header information
 * \param data Output image data
 * \return HYPERSPECTRAL_NO_ERR on success
 **/
hyperspectral_err_t hyperspectral_read_image(const char *filename, struct hyspex_header *header, float *data);

/**
 * Write header information to file.
 *
 * \param filename Filename
 * \param bands Number of bands
 * \param samples Number of samples (across-track)
 * \param lines Number of lines (along-track)
 * \param wlens Wavelength array
 **/
void hyperspectral_write_header(const char *filename, int bands, int samples, int lines, std::vector<float> wlens);

/**
 * Write hyperspectral image to file.
 *
 * \param filename Filename
 * \param bands Bands
 * \param samples Samples
 * \param lines Lines
 * \param data Image data
 **/
void hyperspectral_write_image(const char *filename, int bands, int samples, int lines, float *data);

std::string getValue(std::string hdrText, std::string property);

#endif
