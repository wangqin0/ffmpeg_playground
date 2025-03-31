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

  // pFormatCtx->streams is just an array of AVStream pointers of size pFormatCtx->nb_streams

  int videoStream = -1;

  for (unsigned int i = 0; i < pFormatCtx->nb_streams; ++i) {
    const AVStream* stream = pFormatCtx->streams[i];
    const AVCodecParameters* codecPar = stream->codecpar;

    if (codecPar->codec_type == AVMEDIA_TYPE_VIDEO) {
      videoStream = i;
    }
  }

  if (videoStream == -1) {
    std::cerr << "Could not find a video stream" << std::endl;
    return -1;
  }

  const AVCodec* pCodec =
    avcodec_find_decoder(pFormatCtx->streams[videoStream]->codecpar->codec_id);
  if (!pCodec) {
    std::cerr << "Unsupported codec" << std::endl;
    return -1;
  }

  AVCodecContext* pCodecCtx = avcodec_alloc_context3(pCodec);
  if (!pCodecCtx) {
    std::cerr << "Could not allocate codec context" << std::endl;
    return -1;
  }

  // Copy codec parameters from input stream to output codec context
  if (avcodec_parameters_to_context(pCodecCtx, pFormatCtx->streams[videoStream]->codecpar) < 0) {
    std::cerr << "Could not copy codec parameters to context" << std::endl;
    return -1;
  }

  if (avcodec_open2(pCodecCtx, pCodec, nullptr) < 0) {
    std::cerr << "Could not open codec" << std::endl;
    return -1;
  }

  AVFrame* pFrame = av_frame_alloc();
  if (!pFrame) {
    std::cerr << "Could not allocate frame" << std::endl;
    return -1;
  }

  /**
   * Since we're planning to output PPM files, which are stored in 24-bit
   * RGB, we're going to have to convert our frame from its native format
   * to RGB. ffmpeg will do these conversions for us. For most projects
   * (including ours) we're going to want to convert our initial frame to
   * a specific format. Let's allocate a frame for the converted frame
   * now.
   */

  AVFrame* pFrameRGB = av_frame_alloc();
  if (!pFrameRGB) {
    std::cerr << "Could not allocate RGB frame" << std::endl;
    return -1;
  }

  // Even though we've allocated the frame, we still need a place to put
  // the raw data when we convert it. We use avpicture_get_size to get
  // the size we need, and allocate the space manually.

  // Determine required buffer size and allocate buffer
  // numBytes = avpicture_get_size(AV_PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height);
  // https://ffmpeg.org/pipermail/ffmpeg-devel/2016-January/187299.html
  // what is 'linesize alignment' meaning?:
  // https://stackoverflow.com/questions/35678041/what-is-linesize-alignment-meaning
  size_t numBytes =
    av_image_get_buffer_size(AV_PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height, 32);
  uint8_t* buffer = static_cast<uint8_t*>(av_malloc(numBytes * sizeof(uint8_t)));

  /**
   * Now we use avpicture_fill() to associate the frame with our newly
   * allocated buffer. About the AVPicture cast: the AVPicture struct is
   * a subset of the AVFrame struct - the beginning of the AVFrame struct
   * is identical to the AVPicture struct.
   */
  // Assign appropriate parts of buffer to image planes in pFrameRGB
  // Note that pFrameRGB is an AVFrame, but AVFrame is a superset
  // of AVPicture
  // Picture data structure - Deprecated: use AVFrame or imgutils functions
  // instead
  // https://www.ffmpeg.org/doxygen/3.0/structAVPicture.html#a40dfe654d0f619d05681aed6f99af21b
  // avpicture_fill( // [12]
  //     (AVPicture *)pFrameRGB,
  //     buffer,
  //     AV_PIX_FMT_RGB24,
  //     pCodecCtx->width,
  //     pCodecCtx->height
  // );

  av_image_fill_arrays(pFrameRGB->data,
                       pFrameRGB->linesize,
                       buffer,
                       AV_PIX_FMT_RGB24,
                       pCodecCtx->width,
                       pCodecCtx->height,
                       32);

  /**
   * What we're going to do is read through the entire video stream by
   * reading in the packet, decoding it into our frame, and once our
   * frame is complete, we will convert and save it.
   */

  AVPacket* pPacket = av_packet_alloc();
  if (!pPacket) {
    std::cerr << "Could not allocate packet" << std::endl;
    return -1;
  }

  struct SwsContext* sws_ctx = sws_getContext(pCodecCtx->width,
                                              pCodecCtx->height,
                                              pCodecCtx->pix_fmt,
                                              pCodecCtx->width,
                                              pCodecCtx->height,
                                              AV_PIX_FMT_RGB24,
                                              SWS_BILINEAR,
                                              nullptr,
                                              nullptr,
                                              nullptr);

  /**
   * The process, again, is simple: av_read_frame() reads in a packet and
   * stores it in the AVPacket struct. Note that we've only allocated the
   * packet structure - ffmpeg allocates the internal data for us, which
   * is pointed to by packet.data. This is freed by the av_free_packet()
   * later. avcodec_decode_video() converts the packet to a frame for us.
   * However, we might not have all the information we need for a frame
   * after decoding a packet, so avcodec_decode_video() sets
   * frameFinished for us when we have decoded enough packets the next
   * frame.
   * Finally, we use sws_scale() to convert from the native format
   * (pCodecCtx->pix_fmt) to RGB. Remember that you can cast an AVFrame
   * pointer to an AVPicture pointer. Finally, we pass the frame and
   * height and width information to our SaveFrame function.
   */

  constexpr int maxFramesToDecode = 100;

  // TODO: RAII for pPacket
  int i;
  for (i = 0; av_read_frame(pFormatCtx, pPacket) >= 0 && i < maxFramesToDecode; ++i) {
    if (pPacket->stream_index != videoStream) {
      av_packet_unref(pPacket);
      continue;
    }
    if (avcodec_send_packet(pCodecCtx, pPacket) < 0) {
      std::cerr << "Error sending packet to codec context" << std::endl;
      av_packet_unref(pPacket);
      break;
    }
    while (true) {
      int ret = avcodec_receive_frame(pCodecCtx, pFrame);
      if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
        // mostly EAGAIN -> B frame, but also EOF
        std::cerr << "Error receiving frame:" << ret << std::endl;
        break;
      } else if (ret < 0) {
        std::cerr << "Error receiving frame from codec context" << std::endl;
        break;
      }

      // Convert the image from its native format to RGB
      sws_scale(sws_ctx,
                const_cast<const uint8_t* const*>(pFrame->data),
                pFrame->linesize,
                0,
                pCodecCtx->height,
                pFrameRGB->data,
                pFrameRGB->linesize);

      if (++i <= maxFramesToDecode) {
        saveFrame(pFrameRGB, pCodecCtx->width, pCodecCtx->height, i);

        // print log information
        printf("Frame %c (%lld) pts %lld dts %lld %dx%d]\n",
               av_get_picture_type_char(pFrame->pict_type),
               pCodecCtx->frame_num,
               pFrameRGB->pts,
               pFrameRGB->pkt_dts,
               pCodecCtx->width,
               pCodecCtx->height);
      }
    }
  }

  /**
   * Cleanup.
   */

  // Free the RGB image
  av_free(buffer);
  av_frame_free(&pFrameRGB);
  av_free(pFrameRGB);

  // Free the YUV frame
  av_frame_free(&pFrame);
  av_free(pFrame);

  // Close the codecs
  avcodec_free_context(&pCodecCtx);

  // Close the video file
  avformat_close_input(&pFormatCtx);

  return 0;
}

void saveFrame(AVFrame* avFrame, int width, int height, int frameIndex) {
  FILE* pFile;
  char szFilename[32];
  int y;
  // Open file
  snprintf(szFilename, sizeof(szFilename), "frame%d.ppm", frameIndex);
  pFile = fopen(szFilename, "wb");
  if (pFile == nullptr) {
    std::cerr << "Could not open file " << szFilename << std::endl;
    return;
  }

  fprintf(pFile, "P6\n%d %d\n255\n", width, height);

  for (y = 0; y < height; ++y) {
    fwrite(avFrame->data[0] + y * avFrame->linesize[0], 1, width * 3, pFile);
  }
  fclose(pFile);
}
