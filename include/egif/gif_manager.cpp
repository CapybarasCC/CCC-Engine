#include <gif_manager.h>

using namespace std;

void GifManager::configure(int quality, int delay, bool use_global_color,
                           GifEncoder::PixelFormat format, bool skip_first) {

  m_quality = quality;
  m_delay = delay;
  m_use_global_color = use_global_color;
  m_pixe_color_qty = 0;
  m_pre_alloc = false;
  if (format == GifEncoder::PixelFormat::PIXEL_FORMAT_RGB ||
      format == GifEncoder::PixelFormat::PIXEL_FORMAT_BGR) {
    m_pixe_color_qty = 3;
    m_pre_alloc = true;
  } else if (format == GifEncoder::PixelFormat::PIXEL_FORMAT_BGRA ||
             format == GifEncoder::PixelFormat::PIXEL_FORMAT_RGBA) {
    m_pixe_color_qty = 4;
    m_pre_alloc = true;
  }
  m_format = format;
  m_skip_first = skip_first;
}

void GifManager::startGif(const string& filename, int size_x, int size_y, int nbr_frames)
{
    m_pre_alloc_size = size_x * size_y * 3;
    m_size_x = size_x;
    m_size_y = size_y;
    m_frames_count = 0;
    m_filename = filename;

    if (m_use_global_color)
    {
        m_pre_alloc_size = m_pre_alloc_size * nbr_frames;
    }

    if (!m_gif_encoder.open(
          filename, size_x, size_y, m_quality, m_use_global_color, 0, m_pre_alloc_size))
    {
        fprintf(stderr, "Error open gif file\n");
    }
}

bool GifManager::addFrame(sf::Image image) {
  Bitmap frame((uint8_t *)image.getPixelsPtr(), m_format, m_size_x, m_size_y);
  if (m_skip_first) {
    m_skip_first = false;
    return true;
  }

  if (!m_gif_encoder.push(frame.format, frame.data, frame.width, frame.height,
                          m_delay)) {
    fprintf(stderr, "Error add a frame\n");
    return false;
  }

  return true;
}

void GifManager::addFrameVector(std::vector<sf::Image> frames) {}

void GifManager::endGif() {
  if (!m_gif_encoder.close()) {
    fprintf(stderr, "Error close gif file\n");
    releaseImages();
    return;
  }
  std::cout << "Finished GIF: " << m_filename << std::endl;
}

void GifManager::releaseImages() {
  for (auto &frame : m_frames) {
    frame.release();
  }
  m_frames.clear();
}