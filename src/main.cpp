
#include <iostream>
#include <iomanip> // extended formatting
#include <fstream>
#include <memory>
#include <cstring> // strerror

#include "stm32_crc.h"
#include "optionParser.hpp"



int main(int argc, char *argv[])
{
  std::fstream img_file;
  size_t bytes_total = 0;
  uint32_t crc = 0;
  char * buffer = nullptr;

  std::cout << "STM32 binary-image checksum tool" << std::endl;

  op::OptionParser opt;
  opt.add_option("h", "help", "shows help");
  opt.add_option("f", "file", "image file", "file.bin" );
  opt.add_option("p", "pad-file", "pad file up to a certain size");
  opt.add_option("s", "pad-size", "pad size in bytes or kilobytes (K)", "0");
  opt.add_option("b", "pad-byte", "pad byte", "255");

  // parse the options and verify that all went well. If not, errors and help will be shown
  bool correct_parsed = opt.parse_options(argc, argv);

  if (!correct_parsed) {
    // information is displayed automatically by optionParser
    return EXIT_FAILURE;
  }

  if (op::str2bool(opt["h"])) {
    opt.show_help();
    return 0;
  }

  // convert arguments
  uint32_t pad_file = op::str2bool(opt["p"]);
  int pad_byte = op::str2int(opt["b"]);
  int pad_size = -1;
  if (opt["s"].back() == 'k' || opt["s"].back() == 'K') {
    pad_size = 1024 * op::str2int(opt["s"].substr(0, opt["s"].length()-1));
  } else {
    pad_size = op::str2int(opt["s"]);
  }

  stm32crc_init();

  img_file.open(opt["f"], std::ios::in | std::ios::binary);
  if (!img_file.is_open()) {
    std::cout << "Error opening file: " << opt["f"] << std::endl;
    return 2;
  }

  // get length of file
  img_file.seekg(0, img_file.end);
  bytes_total = img_file.tellg();

  // parameter validation
  if (pad_file) {
    if (pad_size < 0) {
      std::cout << "Padded file size must be a positive integer, padded size: " << pad_size << std::endl;
      return 2;
    }
    if (pad_size % 4 != 0) {
      std::cout << "Padded file size must be multiple of 4 bytes, padded size: " << pad_size << std::endl;
      return 2;
    }
    if (static_cast<size_t>(pad_size) < (bytes_total+4)) {
      std::cout << "Padded file size must be at least 4 bytes greater then actual file size, actual size: " << bytes_total << std::endl;
      return 2;
    }
    if (pad_byte < 0 || pad_byte > 255) {
      std::cout << "Padding byte must be a 8bit unsigned integer, actual value: " << pad_byte << std::endl;
      return 2;
    }
  }

  if (bytes_total % 4 != 0) {
    std::cout << "File size must be multiple of 4 bytes, actual size: " << bytes_total << std::endl;
    img_file.close();
    return 3;
  }

  img_file.seekg(0, img_file.beg);
  buffer = new char [bytes_total];
  img_file.read(buffer, bytes_total);
  if (img_file.fail()) {
    std::cout << "Error reading file into buffer" << std::endl;
    img_file.close();
    delete[] buffer;
    return 3;
  }
  std::cout << "Bytes read: " << bytes_total << std::endl;

  crc = stm32crc_crc32((uint8_t *)buffer, bytes_total);

  img_file.close();
  delete[] buffer;

  if (crc == 0) {
    if (pad_file) {
      std::cout << "CRC is 0, but file is paddded anyway." << std::endl;
    } else {
      std::cout << "CRC is 0, so it has already been appended to file " << opt["f"] << std::endl;
      return 4;
    }
  }

  img_file.open(opt["f"], std::ios::out | std::fstream::app | std::ios::binary);
  if (!img_file.is_open()) {
    std::cout << "Error opening file for writing: " << opt["f"] << std::endl;
    return 2;
  }

  if (pad_file) {
    uint32_t pad_len = (pad_size - 4 - bytes_total);
    buffer = new char [pad_len];
    std::memset((void*) buffer, 0xFF, pad_len);
    crc = stm32crc_crc32((uint8_t *)buffer, pad_len);
    img_file.write(buffer, pad_len);
    delete[] buffer;
    if (img_file.fail()) {
      std::cout << "Error padding file with " << pad_len << " bytes" << std::endl;
      return 2;
    } else {
      std::cout << "File padded with " << pad_len << " bytes" << std::endl;
    }
  }

  img_file.write((char*)&crc, sizeof(crc));
  if (img_file.fail()) {
    std::cout << "Error appending CRC to file " << opt["f"] << std::endl;
    img_file.close();
    return 5;
  }
  img_file.close();

  std::cout.setf(std::ios::hex, std::ios::basefield);
  std::cout.setf(std::ios::showbase);
  std::cout << "CRC appended to file: " << crc << std::endl;

  return 0;
}

