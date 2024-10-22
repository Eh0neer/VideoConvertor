#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libavutil/imgutils.h>
}

using namespace std;

// Пример кода для конвертации
void convertVideo(const std::string &inputFile, const std::string &outputFile) {
  av_register_all();

  AVFormatContext *inputFormatContext = nullptr;
  if (avformat_open_input(&inputFormatContext, inputFile.c_str(), nullptr,
                          nullptr) < 0) {
    cerr << "Could not open input file: " << inputFile << endl;
    return;
  }

  if (avformat_find_stream_info(inputFormatContext, nullptr) < 0) {
    cerr << "Could not find stream information for: " << inputFile << endl;
    avformat_close_input(&inputFormatContext);
    return;
  }

  // Создаем выходной контекст
  AVFormatContext *outputFormatContext = nullptr;
  avformat_alloc_output_context2(&outputFormatContext, nullptr, nullptr,
                                 outputFile.c_str());
  if (!outputFormatContext) {
    cerr << "Could not create output context for: " << outputFile << endl;
    avformat_close_input(&inputFormatContext);
    return;
  }

  // Копируем потоки из входного контекста в выходной
  for (unsigned int i = 0; i < inputFormatContext->nb_streams; i++) {
    AVStream *inStream = inputFormatContext->streams[i];
    AVStream *outStream = avformat_new_stream(outputFormatContext, nullptr);
    if (!outStream) {
      cerr << "Failed to allocate output stream." << endl;
      avformat_close_input(&inputFormatContext);
      avformat_free_context(outputFormatContext);
      return;
    }

    // Копируем параметры кодека
    avcodec_parameters_copy(outStream->codecpar, inStream->codecpar);
  }

  // Открываем выходной файл
  if (avio_open(&outputFormatContext->pb, outputFile.c_str(), AVIO_FLAG_WRITE) <
      0) {
    cerr << "Could not open output file: " << outputFile << endl;
    avformat_close_input(&inputFormatContext);
    avformat_free_context(outputFormatContext);
    return;
  }

  // Записываем заголовок
  if (avformat_write_header(outputFormatContext, nullptr) < 0) {
    cerr << "Error occurred when writing header for: " << outputFile << endl;
    avformat_close_input(&inputFormatContext);
    avformat_free_context(outputFormatContext);
    return;
  }

  // Здесь будет код для записи данных из входного потока в выходной

  av_write_trailer(outputFormatContext);
  avformat_close_input(&inputFormatContext);
  avformat_free_context(outputFormatContext);
  cout << "Conversion completed for: " << inputFile << endl;
}


int main() {
  const string outputExtension = ".mov"; // Укажите нужный формат
  filesystem::path currentPath = filesystem::current_path();

  // Перебираем все файлы в текущей директории
  for (const auto &entry : filesystem::directory_iterator(currentPath)) {
    if (entry.is_regular_file()) {
      string inputFile = entry.path().string();
      string outputFile =
          entry.path().replace_extension(outputExtension).string();

      // Конвертация видеофайла
      convertVideo(inputFile, outputFile);
    }
  }

  return 0;
}
