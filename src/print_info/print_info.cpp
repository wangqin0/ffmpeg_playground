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

auto main() -> int {
  AVFormatContext *pFormatCtx = nullptr;
  // Open the video file
  if (avformat_open_input(&pFormatCtx, "../../../data/file_example_MP4_1920_18MG.mp4", nullptr, nullptr) != 0) {
    std::cerr << "Could not open file" << std::endl;
    return -1;
  }

  if (avformat_find_stream_info(pFormatCtx, nullptr) < 0) {
    std::cerr << "Could not find stream information" << std::endl;
    return -1;
  }

  av_dump_format(pFormatCtx, 0, "../../../data/file_example_MP4_1920_18MG.mp4", 0);
  // pFormatCtx->streams is just an array of AVStream pointers of size pFormatCtx->nb_streams

  for (unsigned int i = 0; i < pFormatCtx->nb_streams; ++i) {
    const AVStream * stream = pFormatCtx->streams[i];
    const AVCodecParameters *codecPar = stream->codecpar;
    std::cout << "Stream " << i << ": " << av_get_media_type_string(codecPar->codec_type) << std::endl;
    std::cout << "  Codec: " << avcodec_get_name(codecPar->codec_id) << std::endl;
    std::cout << "  Width: " << codecPar->width << std::endl;
    std::cout << "  Height: " << codecPar->height << std::endl;
    std::cout << "  Sample Rate: " << codecPar->sample_rate << std::endl;
    std::cout << "  Channels: " << codecPar->ch_layout.nb_channels << std::endl;

    if (codecPar->codec_type == AVMEDIA_TYPE_VIDEO) {
      AVCodec *pCodec = avcodec_find_decoder(codecPar->codec_id);
    }
  }

  return 0;
}
