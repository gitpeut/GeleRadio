#ifndef VS1053G_H
#define VS1053G_H

#include <sys/stat.h>
#include <VS1053.h>
#include <TFT_eSPI.h>
extern TFT_eSPI tft;

class VS1053g: public VS1053{
private:


 
  void *(*g_calloc)(size_t num, size_t size);
  void *(*g_malloc)( size_t size);
 
  size_t skiplast=0;

  int VS1053_file_size( const char *filename);
  void write_VS1053_registers( unsigned short *pluginr, size_t valuecount);
  int read_VS1053_bin( const char *bin_filename);
  int read_VS1053_plg( const char *filename );
  int write_VS1053_binfile( unsigned short *pluginv, size_t valuecount, const char *bin_filename );
  
public:
   VS1053g( uint8_t _cs_pin, uint8_t _dcs_pin, uint8_t _dreq_pin);
 
// allows for unedited raw .plg files to be uploaded to SPIFFS
// and to be read and patch VS1053. After the .plg file has been 
// read, a .plg.bin is written and the plg file is deleted.
// if a new .plg file isplaced on SPIFFS this new plg file is used 
// to patch VS1053.

  void patch_VS1053( const char *filename, size_t skip_last_bytes = 0);

// after loading the spectrum analyzer plugin these functions display
// a spectrum analyzer.
// from https://github.com/blotfi/ESP32-Radio-with-Spectrum-analyzer

  uint8_t     bands = 14; // Number of bands Spectrum Analyzer
  uint8_t     prevbands = 0; // Number previous band Spectrum Analyzer
  uint8_t     spectrum[14][3]; // Array per Spectrum Analyzer
                               // [0] current height [1] previous height [2] peak value
  
  uint8_t     spectrum_top    = 50; //120; //Spectrum graph top offset
  uint8_t     spectrum_height = 70; //высота графика = Spectrum graph height   
  
  void getBands();
  void displaySpectrum( uint16_t *askcolor=NULL);

};


#endif 