# Co-registration

Hyperspektrale bilder lagret som .img:

`struct hyspex_header header;
hyperspectral_read_header(filename, &header);

float *image = new float[header.samples*header.lines*header.bands]();
hyperspectral_read_image(filename, &header, image);`

Aksesserer piksel på posisjon (linjenummer, samplenummer, bølgelengdenummer) direkte i dataene med image[header.samples x header.bands x linjenummer + header.samples x bølgelengdenummer + samplenummer]. Ærnte særlig abstrakt, men hvor abstrakt man vil ha det kommer an på hva man skal gjøre og hvor lett man vil gjøre ting for seg selv.

Kan også skrive til fil, se headerfila. Må skrive både headerfila og bildefila eksplisitt fordi ja nei :') Du kan sikkert kombinere dem i en funksjon om du har ille lyst.

Ligger begynnelsen på en CMakeLists.txt-fil om atte du vil bruke cmake (mkdir build, cd build, cmake .., make). Kompilerer main.cpp, basic eksempel som dytter ett av bølgelengdebåndene til standard output (./hyperread_example [bildefil] > tjafs, åpne tjafs i e.g. MATLAB (A = load('tjafs'); imagesc(A))).

readimage.cpp krever boost_regex, men funksjonskallene som er brukt er egentlig klin like GNU sin regeximplementasjon, og kan i teorien byttes ut ganske lett med GNU sin innebygde regex ved lyst og (u)vilje.

Hyperspektrale bilder lagret som .mat:

Biblioteker:

libmatio
-> dep: zlib
-> optional dep: HDF5

mat_t matfp = Mat_Open(filename,MAT_ACC_RDONLY); henter en peker til filen .mat. Henter deretter ut data med matvar_t *HSId = Mat_VarRead(matfp,"HSI"), som er nok en peker, hvor til slutt data lagres i et array med short unsigned *hData = static_cast<uint16_t*>(HSId->data).

Gammel dokumentasjon finnes: http://libmatio.sourcearchive.com/documentation/1.3.4-3/main.html

Lagring som .tif:

Biblioteker:

libtiffio

Lagrer bildene ut fra spesifikasjonene satt av "TIFFSetField"
