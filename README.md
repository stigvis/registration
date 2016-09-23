# Co-registration

Sammenrasket tjafs for å lese inn hyperspektralbilder i se pluss pluss.

Les inn headerfilen med

`struct hyspex_header header;
hyperspectral_read_header(filename, &header);

float *image = new float[header.samples*header.lines*header.bands]();
hyperspectral_read_image(filename, &header, image);`

Aksesserer piksel på posisjon (linjenummer, samplenummer, bølgelengdenummer) direkte i dataene med image[header.samples*header.bands*linjenummer + header.samples*bølgelengdenummer + samplenummer]. Ærnte særlig abstrakt, men hvor abstrakt man vil ha det kommer an på hva man skal gjøre og hvor lett man vil gjøre ting for seg selv.

Kan også skrive til fil, se headerfila. Må skrive både headerfila og bildefila eksplisitt fordi ja nei :') Du kan sikkert kombinere dem i en funksjon om du har ille lyst.

Ligger begynnelsen på en CMakeLists.txt-fil om atte du vil bruke cmake (mkdir build, cd build, cmake .., make). Kompilerer main.cpp, basic eksempel som dytter ett av bølgelengdebåndene til standard output (./hyperread_example [bildefil] > tjafs, åpne tjafs i e.g. MATLAB (A = load('tjafs'); imagesc(A))).

readimage.cpp krever boost_regex, men funksjonskallene som er brukt er egentlig klin like GNU sin regeximplementasjon, og kan i teorien byttes ut ganske lett med GNU sin innebygde regex ved lyst og (u)vilje.
