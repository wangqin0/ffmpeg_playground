#include <iostream>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavdevice/avdevice.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
}

void saveFrame(AVFrame* avFrame, int width, int height, int frameIndex);

auto main() -> int {
  std::string file = "../../../data/file_example_MP4_1920_18MG.mp4";

  std::cout << "FFmpeg version: " << av_version_info() << "\n\n";

  AVFormatContext* pFormatCtx = nullptr;

  // now we can actually open the file:
  // the minimum information required to open a file is its URL, which is
  // passed to avformat_open_input(), as in the following code:
  if (avformat_open_input(&pFormatCtx, file.c_str(), nullptr, nullptr) != 0) {
    std::cerr << "Could not open file" << std::endl;
    return -1;
  }

  if (avformat_find_stream_info(pFormatCtx, nullptr) < 0) {
    std::cerr << "Could not find stream information" << std::endl;
    return -1;
  }

  av_dump_format(pFormatCtx, 0, file.c_str(), 0);
  // pFormatCtx->streams is just an array of AVStream pointers of size pFormatCtx->nb_streams

  int videoStream = -1;

  for (unsigned int i = 0; i < pFormatCtx->nb_streams; ++i) {
    const AVStream* stream = pFormatCtx->streams[i];
    const AVCodecParameters* codecPar = stream->codecpar;
    std::cout << "Stream " << i << ": " << av_get_media_type_string(codecPar->codec_type)
              << std::endl;
    std::cout << "  Codec: " << avcodec_get_name(codecPar->codec_id) << std::endl;
    std::cout << "  Width: " << codecPar->width << std::endl;
    std::cout << "  Height: " << codecPar->height << std::endl;
    std::cout << "  Sample Rate: " << codecPar->sample_rate << std::endl;
    std::cout << "  Channels: " << codecPar->ch_layout.nb_channels << std::endl;
  }

  // Close the video file
  avformat_close_input(&pFormatCtx);

  return 0;
}
