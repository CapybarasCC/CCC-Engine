#include "GifEncoder.h"
#include <SFML/Graphics.hpp>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <memory>
#include <stb_image.h>
#include <string.h>
#include <vector>

#define NOW                                                                    \
  (std::chrono::duration_cast<std::chrono::microseconds>(                      \
       std::chrono::system_clock::now().time_since_epoch())                    \
       .count())

class Bitmap {
public:
  Bitmap() = default;

  Bitmap(uint8_t *_data, GifEncoder::PixelFormat _format, int _width,
         int _height)
      : data(_data), format(_format), width(_width), height(_height),
        needFree(false) {}

  void release() {
    if (needFree) {
      free(data);
      needFree = false;
    }
    data = nullptr;
    format = GifEncoder::PIXEL_FORMAT_UNKNOWN;
    width = 0;
    height = 0;
  }

public:
  uint8_t *data = nullptr;
  GifEncoder::PixelFormat format = GifEncoder::PIXEL_FORMAT_UNKNOWN;
  int width = 0;
  int height = 0;

private:
  bool needFree = false;
};

class GifManager {
public:
  void configure(int quality = 10, int delay = 10,
                 bool use_global_color = false,
                 GifEncoder::PixelFormat format =
                     GifEncoder::PixelFormat::PIXEL_FORMAT_RGBA,
                 bool skip_first = false);
  void startGif(const std::string &filename, int size_x, int size_y,
                int nbr_frames);
  bool addFrame(sf::Image image);
  void addFrameVector(std::vector<sf::Image> frames);
  void endGif();
  void releaseImages();
  // Setters
  void setDelay(int delay) { m_delay = delay; };
  void setQuality(int quality) { m_quality = quality; };

  // Methods
  void setPreAllocSize();

  // Atributes
public:
    GifEncoder              m_gif_encoder;
    std::vector<Bitmap>     m_frames;
    GifEncoder::PixelFormat m_format;
    std::string             m_filename;
    bool                    m_pre_alloc;
    int                     m_pre_alloc_size;
    int                     m_quality;
    int                     m_delay;
    int                     m_frames_count;
    bool                    m_use_global_color;
    int                     m_pixe_color_qty;
    int                     m_size_x;
    int                     m_size_y;
    bool                    m_skip_first;
};