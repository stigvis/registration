//=======================================================================================================
// Copyright 2015 Asgeir Bjorgan, Lise Lyngsnes Randeberg, Norwegian University of Science and Technology
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
// http://opensource.org/licenses/MIT)
//=======================================================================================================

#include "readimage.h"
using namespace std;

const int MAX_CHAR = 512;
const int MAX_FILE_SIZE = 4000;

int getMatch(string input_string, regmatch_t *matchArray, int matchNum, string *match);

//extract specified property value from header text as a char array
//string getValue(string hdrText, string property);

//return list of wavelengths. Input: char array containing characters {wlen1, wlen2, wlen3, ...}
vector<float> getWavelengths(int bands, string wavelengthStr);

string getBasename(string filename);

hyperspectral_err_t hyperspectral_read_header(const char* filename, struct hyspex_header *header){
	//find base filename
	string baseName = getBasename(string(filename));

	//open and read header file
	//append .hdr to filename
	string hdrName = baseName + ".hdr";

	FILE *fp = fopen(hdrName.c_str(), "rt");
	if (fp == NULL){
		return HYPERSPECTRAL_FILE_NOT_FOUND;
	}
	char hdrText[MAX_FILE_SIZE] = "";
	int sizeRead = 1;
	int offset = 0;
	while (sizeRead){
		sizeRead = fread(hdrText + offset, sizeof(char), MAX_CHAR, fp);
		offset += sizeRead/sizeof(char);
	}
	fclose(fp);

	//extract properties from header file text
	string samples = getValue(hdrText, "samples");
	string bands = getValue(hdrText, "bands");
	string lines = getValue(hdrText, "lines");
	string wavelengths = getValue(hdrText, "wavelength");
	string hdrOffset = getValue(hdrText, "header offset");
	string interleave = getValue(hdrText, "interleave");
	string datatype = getValue(hdrText, "data type");
  //
  //cerr << samples.empty() << endl;
  //cerr << bands.empty() << endl;
	//cerr << lines.empty() << endl;
	//cerr << hdrOffset.empty() << endl;
	//cerr << interleave.empty() << endl;
	//cerr << datatype.empty() << endl;
  //
	if ((samples.empty()) || (bands.empty()) || (lines.empty()) || (hdrOffset.empty()) || (interleave.empty()) || (datatype.empty())){
		return HYPERSPECTRAL_HDR_PROPERTY_NOT_FOUND;
	}

	//convert strings to values
	header->bands = strtod(bands.c_str(), NULL);
	header->lines = strtod(lines.c_str(), NULL);
	header->samples = strtod(samples.c_str(), NULL);
	header->offset = strtod(hdrOffset.c_str(), NULL);
	header->wlens = getWavelengths(header->bands, wavelengths);
	header->datatype = strtod(datatype.c_str(), NULL);

	if (interleave == "bil"){
		header->interleave = BIL_INTERLEAVE;
	} else if (interleave == "bip"){
		header->interleave = BIP_INTERLEAVE;
	} else {
		return HYPERSPECTRAL_INTERLEAVE_UNSUPPORTED;
	}

	//recap
	fprintf(stderr, "Extracted: lines=%d, samples=%d, bands=%d, offset=%d, data type=%d\n", header->lines, header->samples, header->bands, header->offset, header->datatype);
	fprintf(stderr, "Wavelengths: ");
	for (int i=0; i < header->wlens.size(); i++){
		fprintf(stderr, "%f ", header->wlens[i]);
	}
	fprintf(stderr, "\n");
	return HYPERSPECTRAL_NO_ERR;
}

hyperspectral_err_t hyperspectral_read_image(const char *filename, struct hyspex_header *header, float *data){
	struct image_subset subset;
	subset.start_sample = 0;
	subset.end_sample = header->samples;
	subset.start_line = 0;
	subset.end_line = header->lines;
	subset.start_band = 0;
	subset.end_band = header->bands;

	return hyperspectral_read_image(filename, header, subset, data);
}


hyperspectral_err_t hyperspectral_read_image(const char *filename, struct hyspex_header *header, struct image_subset subset, float *data){
	//find number of bytes for contained element
  size_t elementBytes = 0;
	if (header->datatype == 4){
		elementBytes = sizeof(float);
	} else if (header->datatype == 12){
		elementBytes = sizeof(uint16_t);
	} else if (header->datatype == 2){
		elementBytes = sizeof(int16_t);
	} else {
		return HYPERSPECTRAL_DATATYPE_UNSUPPORTED;
	}

  FILE *fp = fopen(filename, "rb");
  if (fp == NULL){
    return HYPERSPECTRAL_FILE_NOT_FOUND;
  }

  //skip header and lines we do not want
  size_t skipBytes = subset.start_line*header->samples*header->bands*elementBytes + header->offset;
  fprintf(stderr, "skipBytes: %d\n", skipBytes);
  fseek(fp, skipBytes, SEEK_SET);

  int numLinesToRead = subset.end_line - subset.start_line;


  //read in line by line
  for (int i=0; i < numLinesToRead; i++){
    char *line = (char*)malloc(elementBytes*header->bands*header->samples);
    int sizeRead = fread(line, elementBytes, header->bands*header->samples, fp);
    if (sizeRead == 0){
			return HYPERSPECTRAL_FILE_READING_ERROR;
		}

		//convert to float, copy to total array
		for (int k=subset.start_band; k < subset.end_band; k++){
			for (int j=subset.start_sample; j < subset.end_sample; j++){
				float val;
				int position = 0;
				switch (header->interleave){
					case BIL_INTERLEAVE:
						position = k*header->samples + j;
					break;

					case BIP_INTERLEAVE:
						position = j*header->bands + k;
					break;
				}
				if (header->datatype == 4){
					val = ((float*)line)[position];
				} else if (header->datatype == 12){
					val = (((uint16_t*)line)[position])*1.0f;
				} else if (header->datatype == 2){
					val = (((int16_t*)line)[position])*1.0f;
				}
				data[i*(subset.end_band - subset.start_band)*(subset.end_sample - subset.start_sample) + (k - subset.start_band)*(subset.end_sample - subset.start_sample) + j-subset.start_sample] = val;
			}
		}
		free(line);



	}
	fclose(fp);
	return HYPERSPECTRAL_NO_ERR;
}

string getMatch(string input_string, regmatch_t *matchArray, int matchNum){
	int start = matchArray[matchNum].rm_so;
	int end = matchArray[matchNum].rm_eo;
	return input_string.substr(start, end - start);
}

string getValue(string hdrText, string property){
	//find the input property using regex
	regex_t propertyMatch;
	int numMatch = 2;
	regmatch_t *matchArray = (regmatch_t*)malloc(sizeof(regmatch_t)*numMatch);

	char regexExpr[MAX_CHAR] = "";
	strcat(regexExpr, property.c_str());
	strcat(regexExpr, "\\s*=\\s*([{|}|0-9|,| |.|a-z]+)"); //property followed by = and a set of number, commas, spaces or {}s

	int retcode = regcomp(&propertyMatch, regexExpr, REG_EXTENDED | REG_NEWLINE | REG_PERL);
	int match = regexec(&propertyMatch, hdrText.c_str(), numMatch, matchArray, 0);
	if (match != 0){
		fprintf(stderr, "Could not find parameter in file: %s\n", property.c_str());
		regfree(&propertyMatch);
		free(matchArray);
		return NULL;
	}
	string retVal = getMatch(hdrText, matchArray, 1);


	//cleanup
	regfree(&propertyMatch);
	free(matchArray);

	return retVal;
}

string getBasename(string filename){
	regex_t filenameMatch;
	int numMatch = 2;
	regmatch_t *matchArray = (regmatch_t*)malloc(sizeof(regmatch_t)*numMatch);
	int retcode = regcomp(&filenameMatch, "(.*)[.].*$", REG_EXTENDED);
	int match = regexec(&filenameMatch, filename.c_str(), numMatch, matchArray, 0);
	string baseName = getMatch(filename, matchArray, 1);
	regfree(&filenameMatch);
	free(matchArray);
	return baseName;
}

vector<float> getWavelengths(int bands, string wavelengthStr){
	vector<float> retWlens;
	bool useStandardValues = false;
	if (!wavelengthStr.empty()){
		//prepare regex
		regex_t numberMatch;
		int numMatch = 2;
		regmatch_t *matchArray = (regmatch_t*)malloc(sizeof(regmatch_t)*numMatch);
		char regexExpr[MAX_CHAR] = "([0-9|.]+)[,| |}]*";
		int retcode = regcomp(&numberMatch, regexExpr, REG_EXTENDED);

		//find start of number sequence
		int currStart = wavelengthStr.find_first_of('{') + 1;
		string substr = wavelengthStr.substr(currStart);

		//go through all bands
		for (int i=0; i < bands; i++){
			substr = wavelengthStr.substr(currStart);

			//extract wavelength
			if (regexec(&numberMatch, substr.c_str(), numMatch, matchArray, 0)){
				fprintf(stderr, "Could not extract wavelengths. Assuming standard values 1, 2, 3, ... .\n");
				useStandardValues = true;
				break;
			}

			string match = getMatch(substr.c_str(), matchArray, 1);
			retWlens.push_back(strtod(match.c_str(), NULL));

			//move to next
			currStart = currStart + matchArray[0].rm_eo;
		}
		regfree(&numberMatch);
		free(matchArray);
	} else {
		useStandardValues = true;
	}

	if (useStandardValues){
		retWlens.clear();
		for (int i=0; i < bands; i++){
			retWlens.push_back(i);
		}
	}
	return retWlens;
}



#include <fstream>
#include <iostream>
#include <math.h>
#include <cstring>
#include <sstream>
using namespace std;

void hyperspectral_write_header(const char *filename, int numBands, int numPixels, int numLines, std::vector<float> wlens){
	//write image header
	ostringstream hdrFname;
	hdrFname << filename << ".hdr";
	ofstream hdrOut(hdrFname.str().c_str());
	hdrOut << "ENVI" << endl;
	hdrOut << "samples = " << numPixels << endl;
	hdrOut << "lines = " << numLines << endl;
	hdrOut << "bands = " << numBands << endl;
	hdrOut << "header offset = 0" << endl;
	hdrOut << "file type = ENVI Standard" << endl;
	hdrOut << "data type = 4" << endl;
	hdrOut << "interleave = bil" << endl;
	hdrOut << "default bands = {55,41,12}" << endl;
	hdrOut << "byte order = 0" << endl;
	hdrOut << "wavelength = {";
	for (int i=0; i < wlens.size(); i++){
		hdrOut << wlens[i] << " ";
	}
	hdrOut << "}" << endl;
	hdrOut.close();
}

void hyperspectral_write_image(const char *filename, int numBands, int numPixels, int numLines, float *data){
	//prepare image file
	ostringstream imgFname;
	imgFname << filename << ".img";
	ofstream *hyspexOut = new ofstream(imgFname.str().c_str(),ios::out | ios::binary);

	//write image
	for (int i=0; i < numLines; i++){
		float *write_data = data + i*numBands*numPixels;
		hyspexOut->write((char*)(write_data), sizeof(float)*numBands*numPixels);
	}

	hyspexOut->close();
	delete hyspexOut;
}


